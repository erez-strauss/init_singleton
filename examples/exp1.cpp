
//
// Created by erez on 12/10/19.
//

#include <singleton.h>

#include <iostream>

namespace es::init::init {
class std_init
{
public:
    std_init() { volatile ::std::ios_base::Init x; }
};
}  // namespace es::init::init

template<unsigned V = 0>
class Data
{
public:
    Data()
    {
        // [[ using gnu : used ]]
        // static auto & init /* __attribute__((used)) */ {es::init::singleton<es::init::init::std_init>::instance()};
        es::init::singleton<es::init::init::std_init>::instance();

        std::cout << "Data::ctor " << __PRETTY_FUNCTION__ << std::endl;
    }
    ~Data() { std::cout << "Data::dtor " << __PRETTY_FUNCTION__ << std::endl; }
    static constexpr auto n() { return V; }
};

struct SData
{
    SData()
    {
        volatile ::std::ios_base::Init x;
        std::cout << "Data::ctor " << __PRETTY_FUNCTION__ << std::endl;
    }
};
[[using gnu: used, init_priority((101))]] static SData x{};
static Data                                            y{};

// static SData z{};

int main()
{
    std::cout << "main is done\n";
    return 0;
}
