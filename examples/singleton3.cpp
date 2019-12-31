//
// Created by erez on 12/13/19.
//

#include <singleton.h>
//
#include <singleton3.h>

#include <iostream>

// es::init::initializer<DataA> zzz;

void test3()
{
    std::cout << "total number of singletons: " << es::init::stack::size() << '\n';
    es::init::report_singletons_stack();
    //#if 0
    auto& a{es::init::singleton<DataA>::instance()};
    std::cout << a._a << '\n';

    auto& aaa{es::init::singleton<DataA, es::init::lazy_initializer, int>::instance()};
    std::cout << aaa._a << '\n';

    auto& aa{es::init::singleton<DataA>::instance()};
    std::cout << aa._a << '\n';

    auto& b{es::init::singleton<DataB>::instance()};
    std::cout << b._b << '\n';

    auto& c{es::init::singleton<DataC>::instance()};
    std::cout << c._c << '\n';

    auto& x{es::init::singleton<uint64_t>::instance()};
    std::cout << "x: " << x << '\n';
    x = 360;
    std::cout << "x: " << x << '\n';

    es::init::report_singletons_stack();
    std::cout << "total number of singletons: " << es::init::stack::size() << '\n';
    //#endif
}

int main()
{
    test3();
    return 0;
}
