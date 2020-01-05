//
// The efficient Singleton with proper initialization and destruction order
//
// 1. Efficient, with no condition on every access
// 2. Multi dependency on other Singleton(s)
// 3. Multi thread safe
// 4. Proper initialization order with multiple compile units
// 5. Early initialization, lazy initialization
// 6. Detects circular dependency - generates exception
// 7. Proper destruction order
// 8. None intrusive, requires only default constructor, supports native types.
//
// See README.md for details.
// Discussion Proper Initialization / Destruction order:
// Simple Singleton can initialize properly when accessing one by one the singletons that require / depend on others.
// As each one of them is being initialized on the first instance() call.
// In order to solve their destructors order, their respective destructors need to be pushed into an atomic stack, and
// counter of active singletons should be increased. The SpecialDeleter of each one of the uniq_ptr<> of them,
// just reduces the counter of live singletons, and calls the deleter in the stack when the counter gets to Zero.
//
// reference to a singleton will be valid in the block scope.
// *** handle multiple calls to firstTimeGetInstance():
//    -- one after the other() , as the unique pointer objects are "reinitialized" by the compiler.
//         --> should return pointer to the same object.
//    -- one call during the firstTimeActive() - if same-thread - circular dependency
//
// MIT License
//
// Copyright (c) 2019,2020 Erez Strauss, erez@erezstrauss.com
//  http://github.com/erez-strauss/init_singleton/
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once

#include <atomic>
#include <exception>
#include <iostream>
#include <mutex>
#include <string_view>
#include <thread>
#include <type_traits>

namespace es::init {

__extension__ using uint128_t = unsigned __int128;

constexpr const bool verbose_singletons{false};

static constexpr bool USE_BUILTIN_16B
{
#if defined(__GNUC__) && defined(__clang__)
    false  // clang++ uses builtin for std::atomic<__int128>
#elif defined(__GNUC__)
    true  // enable the __sync... builtin functions instead of std::atomic<> that call atomic library.
#else
    false  // other compilers
#endif
};

template<typename T>
struct sequenced_ptr
{
    union
    {
        uint128_t _unit;
        struct
        {
            T*       _p;
            uint64_t _seq;
        } _s;
    } _u;
    sequenced_ptr load() noexcept
    {
        if constexpr (USE_BUILTIN_16B)
            return sequenced_ptr{__sync_val_compare_and_swap(&this->_u._unit, 0, 0)};
        else
            return sequenced_ptr{reinterpret_cast<std::atomic<uint128_t>*>(this)->load()};
    }
    bool cas(sequenced_ptr expected_value, const sequenced_ptr new_value)
    {
        if constexpr (USE_BUILTIN_16B)
            return __sync_bool_compare_and_swap(&this->_u._unit, expected_value._u._unit, new_value._u._unit);
        else
            return reinterpret_cast<std::atomic<uint128_t>*>(this)->compare_exchange_strong(expected_value._u._unit,
                                                                                            new_value._u._unit);
    }
};
static_assert(sizeof(sequenced_ptr<int>) == 16, "Wrong size for sequenced_ptr");

struct singleton_base
{
};

struct singletons_meta_data
{
    singletons_meta_data* _next{nullptr};
    void (*_func)(singletons_meta_data*){nullptr};
    void*       _p{nullptr};
    const char* _func_name{nullptr};
    uint32_t    _useCount{0};
    uint32_t    _flags{0};
};

class singletons_counter
{
    inline static std::atomic<int64_t> global_counter{0};

public:
    static auto get() { return global_counter.load(); };

    template<typename T, template<typename TT> class EI, typename M, typename InitT>
    friend class singleton;
};

template<typename T>
struct static_obj_stack
{
    inline static sequenced_ptr<T> top{0UL};
    inline static std::mutex       stack_mutex{};

    static void push(T* p)
    {
        sequenced_ptr<T> stack_top;
        sequenced_ptr<T> new_top;
        do
        {
            stack_top = top.load();
            new_top   = stack_top;

            p->_next         = stack_top._u._s._p;
            new_top._u._s._p = p;
            ++new_top._u._s._seq;
        } while (!top.cas(stack_top, new_top));
    }
    static T* pop()
    {
        sequenced_ptr<T> stack_top;
        sequenced_ptr<T> new_top;
        do
        {
            stack_top = top.load();
            if (!stack_top._u._s._p) return nullptr;
            new_top          = stack_top;
            new_top._u._s._p = stack_top._u._s._p->_next;
            ++new_top._u._s._seq;
        } while (!top.cas(stack_top, new_top));
        return stack_top._u._s._p;
    }

    static uint64_t size()
    {
        std::lock_guard<std::mutex> guard(stack_mutex);
        uint64_t                    n{0};
        for (T* p = top._u._s._p; p != nullptr; p = p->_next) ++n;
        return n;
    }
};

using stack = static_obj_stack<singletons_meta_data>;
static inline std::atomic<bool> clean_up_phase{false};
static inline int               app_argc{0};
static inline char**            app_argv{nullptr};

inline void emptyStack()
{
    std::lock_guard<std::mutex> guard(stack::stack_mutex);

    clean_up_phase = true;

    uint64_t n{0};
    while (auto p = stack::pop())
    {
        if constexpr (es::init::verbose_singletons)
        {
            std::cout << "emptyStack[" << n++ << "]: node: " << (void*)p << " func_name: " << p->_func_name
                      << std::endl;
        }
        if (auto f = p->_func)
        {
            p->_func = nullptr;
            f(p);
        }
    }
}

inline void report_singletons_stack()
{
    std::lock_guard<std::mutex> guard(stack::stack_mutex);
    uint64_t                    n{0};
    for (singletons_meta_data* p = stack::top._u._s._p; p != nullptr; p = p->_next)
    {
        std::cout << "singletons_stack_meta_data_node[" << ++n << "]: " << (void*)p << " func_name: " << p->_func_name
                  << std::endl;
    }
}

template<typename T>
struct early_initializer_no_args
{
    [[using gnu: used, constructor]] static void early_init()
    {
        std::ios_base::Init z;
        T::instance();
    }
};

template<typename T>
struct early_initializer
{
    [[using gnu: used, constructor]] static void early_init(int argc, char** argv)
    {
        std::ios_base::Init z;
        if (es::init::app_argc != argc) es::init::app_argc = argc;
        if (es::init::app_argv != argv) es::init::app_argv = argv;
        T::instance();
    }
};

template<typename T>
struct lazy_initializer
{  // empty - do nothing.
};

template<typename T, template<typename TT> class EI = early_initializer, typename M = void,
         typename InitT = ::std::ios_base::Init>
class singleton : public singleton_base, EI<singleton<T, EI, M, InitT>>
{
    struct SpecialDeleter
    {
        void operator()(T* p) const
        {
            static bool activated{false};
            if constexpr (verbose_singletons)
            {
                std::cout << "SpecialDeleter start " << singletons_counter::global_counter.load() << " "
                          << __PRETTY_FUNCTION__ << " " << (void*)p << " " << (activated ? 'T' : 'F') << std::endl;
            }
            activated = true;
            if (p)
            {
                std::lock_guard<std::mutex> guard(_mutex);
                if (_getInstance == optGetInstance) _getInstance = firstTimeGetInstance;
                singleton_meta_data_node._p = (void*)p;
                --singletons_counter::global_counter;
                if constexpr (es::init::verbose_singletons)
                {
                    std::cout << "SpecialDeleter - done " << singletons_counter::global_counter.load() << " "
                              << __PRETTY_FUNCTION__ << std::endl;
                }
                if (!singletons_counter::global_counter) emptyStack();
            }
        }
    };

    static void activeDelete(singletons_meta_data* np)
    {
        static uint64_t active_delete_count{0};
        ++active_delete_count;
        if constexpr (es::init::verbose_singletons)
        {
            std::cerr << "activeDelete - " << __PRETTY_FUNCTION__ << " " << (void*)np->_p << " " << active_delete_count
                      << std::endl;
        }
        delete static_cast<T*>(np->_p);
        np->_p = nullptr;
    }

    static T& firstTimeGetInstance()
    {
        volatile InitT init_object{};  // make sure, one can use the std::cout std::cerr streams from Singletons code

        if (clean_up_phase)
            std::cerr << "Warning: initializing at clean up phase - " << __PRETTY_FUNCTION__ << std::endl;

        if (!_instance)
        {
            if (singleton_meta_data_node._flags & 0x1)
            {
                throw std::logic_error(std::string{"Error: circular dependency "} + __PRETTY_FUNCTION__);
            }
            std::lock_guard<std::mutex> guard(_mutex);
            if (!_instance)
            {
                singleton_meta_data_node._flags = 0x1;
                _instance                       = std::unique_ptr<T, SpecialDeleter>{new T{}, SpecialDeleter{}};
                ++singletons_counter::global_counter;

                if (!singleton_meta_data_node._p)
                {
                    singleton_meta_data_node._func      = activeDelete;
                    singleton_meta_data_node._p         = (void*)&*_instance;
                    singleton_meta_data_node._func_name = __PRETTY_FUNCTION__;
                    stack::push(&singleton_meta_data_node);
                }
            }
        }
        _getInstance = optGetInstance;
        singleton_meta_data_node._flags &= ~0x1U;

        return *_instance;
    }

    static T& optGetInstance() { return *_instance; }

    inline static std::atomic<T& (*)()>              _getInstance{firstTimeGetInstance};
    inline static std::unique_ptr<T, SpecialDeleter> _instance{nullptr};
    inline static std::mutex                         _mutex{};
    inline static singletons_meta_data               singleton_meta_data_node{nullptr, nullptr, nullptr, nullptr, 0, 0};

    static_assert(sizeof(_instance) == 8, "instace size should be 8 bytes");

public:
    [[using gnu: hot]] static T& instance() { return _getInstance.load()(); }
};

}  // namespace es::init
