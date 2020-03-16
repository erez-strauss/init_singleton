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
// *** handle multiple calls to first_time_get_instance():
//    -- one after the other() , as the unique pointer objects are "reinitialized" by the compiler.
//         --> should return pointer to the same object.
//    -- one call during the first_time_get_instance - if same-thread - circular dependency
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

constexpr const bool verbose_singletons
{
#if defined(INIT_SINGLETON_VERBOSE)
    true
#else
    false
#endif
};

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

namespace details_static_instances_counting {

inline std::atomic<long> global_static_instances_counter;  // do NOT initialize, default zero
static_assert(std::is_trivially_constructible_v<std::atomic<long>>, "std::atomic should be trivially constructable");

template<typename F>
class InstancesCounterZeroActivated
{
public:
    InstancesCounterZeroActivated() noexcept { ++global_static_instances_counter; }

    InstancesCounterZeroActivated(const InstancesCounterZeroActivated&) = delete;
    InstancesCounterZeroActivated(InstancesCounterZeroActivated&&)      = delete;
    InstancesCounterZeroActivated& operator=(const InstancesCounterZeroActivated&) = delete;
    InstancesCounterZeroActivated& operator=(InstancesCounterZeroActivated&&) = delete;

    ~InstancesCounterZeroActivated() noexcept
    {
        if (--global_static_instances_counter == 0)
        {
            F{}();
            return;
        }
        if (global_static_instances_counter < 0)
        {
            std::cerr << "Error: negative static counter:" << global_static_instances_counter << std::endl;
        }
    }
    static long get_counter() noexcept { return global_static_instances_counter; }
};
}  // namespace details_static_instances_counting

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

class alignas(64) tc_spin_lock
{
public:
    void lock()
    {
        auto& spinlock{*reinterpret_cast<std::atomic<bool>*>(&_spinlock)};
        bool  b = false;
        while (spinlock.compare_exchange_strong(b, true))
        {
            b = false;
        }
    }
    void unlock()
    {
        auto& spinlock{*reinterpret_cast<std::atomic<bool>*>(&_spinlock)};
        spinlock = false;
    }

    bool _spinlock;
    static_assert(sizeof(_spinlock) == sizeof(std::atomic<bool>), "missmatching sizes");
};
static_assert(std::is_trivially_constructible_v<es::init::tc_spin_lock>,
              "es::init::spin_lock is not trivially constructed");

struct singleton_base
{
};

struct singletons_meta_data
{
    singletons_meta_data* _next;
    void (*_func)();
    void*        _p;
    const char*  _func_name;
    uint32_t     _init_count;
    uint32_t     _flags;
    tc_spin_lock _lock;
};
static_assert(std::is_trivially_constructible_v<singletons_meta_data>,
              "singletons_meta_data is not trivially constructed");

inline std::ostream& operator<<(std::ostream& os, const singletons_meta_data& md)
{
    os << "singleton meta data: " << (void*)&md << " p: " << (void*)md._p << " init count: " << md._init_count
       << " flags: " << md._flags << " func name: " << (md._func_name ? md._func_name : "''");
    return os;
}

template<typename T>
struct static_obj_stack
{
    inline static sequenced_ptr<T> top{0UL};

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
        sequenced_ptr<T> stack_top{};
        sequenced_ptr<T> new_top{};
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
        uint64_t n{0};
        for (T* p = top._u._s._p; p != nullptr; p = p->_next) ++n;
        return n;
    }
};

using stack = static_obj_stack<singletons_meta_data>;
static inline std::atomic<bool> clean_up_phase{false};
static inline int               app_argc{0};
static inline char**            app_argv{nullptr};

inline void empty_stack()
{
    clean_up_phase = true;

    uint64_t n{0};
    while (auto p = stack::pop())
    {
        if constexpr (es::init::verbose_singletons)
        {
            std::cout << "empty_stack[" << n << "]: " << *p << std::endl;
            ++n;
        }
        if (auto f = p->_func)
        {
            p->_func = nullptr;
            f();
        }
    }
}

inline void report_singletons_stack()
{
    uint64_t n{0};
    for (singletons_meta_data* p = stack::top._u._s._p; p != nullptr; p = p->_next)
    {
        std::cout << "singletons_stack_meta_data_node[" << n << "]: " << *p << std::endl;
        ++n;
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

struct ActionOnZero
{
    void operator()() const
    {
        static std::atomic<bool> activated{false};

        if constexpr (verbose_singletons)
            std::cout << "ActionOnZero(): calling empty_stack(): from: " << __PRETTY_FUNCTION__ << "\n";
        if (activated.exchange(true))
        {
            std::cout << "ActionOnZero(): Already activated\n";
            return;
        }
        empty_stack();
    }
};

template<typename T, template<typename TT> class EI = early_initializer, typename M = void,
         typename InitT = ::std::ios_base::Init>
class singleton : public singleton_base, EI<singleton<T, EI, M, InitT>>
{
    static void active_delete()
    {
        std::lock_guard<tc_spin_lock> guard(singleton_meta_data_node._lock);

        static uint64_t active_delete_count{0};
        ++active_delete_count;
        if constexpr (es::init::verbose_singletons)
        {
            std::cerr << "active_delete - " << __PRETTY_FUNCTION__ << " " << (void*)singleton_meta_data_node._p << " "
                      << active_delete_count << std::endl;
        }
        // call dtor, without releasing memory, which is statically allocated in the union.
        static_cast<T*>(singleton_meta_data_node._p)->~T();
        singleton_meta_data_node._p = nullptr;
    }

    static T& first_time_get_instance()
    {
        InitT init_object{};  // make sure, one can use the std::cout std::cerr streams from Singletons code

        if constexpr (es::init::verbose_singletons)
        {
            std::cerr << "Info: firstTimeGetInstance: initializing Singleton: " << __PRETTY_FUNCTION__ << " "
                      << singleton_meta_data_node << std::endl;
        }

        if (clean_up_phase)
        {
            std::cerr << "Warning: initializing at clean up phase - " << __PRETTY_FUNCTION__ << std::endl;
        }

        if (!singleton_meta_data_node._p)
        {
            if (singleton_meta_data_node._flags & 0x1U)
            {
                throw std::logic_error(std::string{"Error: circular dependency "} + __PRETTY_FUNCTION__);
            }
            std::lock_guard<tc_spin_lock> guard(singleton_meta_data_node._lock);

            if (!singleton_meta_data_node._p)
            {
                if (singleton_meta_data_node._init_count > 0)
                {
                    std::cerr
                        << "Warning: 1 first_time_get_instance: initializing Singleton, more than once: init_count: "
                        << singleton_meta_data_node._init_count << " - " << __PRETTY_FUNCTION__ << " "
                        << singleton_meta_data_node << std::endl;
                }

                singleton_meta_data_node._flags |= 0x1U;
                static details_static_instances_counting::InstancesCounterZeroActivated<ActionOnZero> iCounter{};
                new (&_u._instance) T{};

                if constexpr (es::init::verbose_singletons)
                {
                    if (singleton_meta_data_node._init_count > 0)
                        std::cerr << "Warning: 2 first_time_get_instance: initializing Singleton, more than once: "
                                     "init_count: "
                                  << singleton_meta_data_node._init_count << " - " << __PRETTY_FUNCTION__ << " "
                                  << singleton_meta_data_node << std::endl;
                }

                singleton_meta_data_node._func      = active_delete;
                singleton_meta_data_node._p         = (void*)&_u._instance;
                singleton_meta_data_node._func_name = __PRETTY_FUNCTION__;
                singleton_meta_data_node._init_count++;
                stack::push(&singleton_meta_data_node);
                singleton_meta_data_node._flags &= ~0x1U;
            }
        }
        _get_instance = optimized_get_instance;

        return _u._instance;
    }

    [[using gnu: hot]] static T& optimized_get_instance() { return _u._instance; }

    inline static std::atomic<T& (*)()> _get_instance{first_time_get_instance};
    inline static union U
    {
        U() {} // Do nothing constructor.
        ~U() {}
        char _x;
        T    _instance;
    } _u;
    inline static singletons_meta_data singleton_meta_data_node{nullptr, nullptr, nullptr, nullptr, 0, 0, {false}};

public:
    [[using gnu: hot]] static T& instance() { return _get_instance.load()(); }
};

}  // namespace es::init
