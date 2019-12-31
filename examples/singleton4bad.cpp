//
// Created by erez on 12/13/19.
//

#include <singleton4.h>

DataA::DataA() : _a(11)
{
    fprintf(stderr, "DataA() %p c: %p\n", (void*)this, (void*)&es::init::singleton<DataC>::instance());
}

DataB::DataB() : _b(22)
{
    fprintf(stderr, "DataB() %p a: %p\n", (void*)this, (void*)&es::init::singleton<DataA>::instance());
}

DataC::DataC() : _c(33)
{
    fprintf(stderr, "DataC() %p b: %p\n", (void*)this, (void*)&es::init::singleton<DataB>::instance());
}

void test4()
{
    //#if 0
    auto& a{es::init::singleton<DataA>::instance()};
    std::cout << a._a << '\n';

    auto& aa{es::init::singleton<DataA>::instance()};
    std::cout << aa._a << '\n';

    auto& b{es::init::singleton<DataB>::instance()};
    std::cout << b._b << '\n';

    auto& c{es::init::singleton<DataC>::instance()};
    std::cout << c._c << '\n';
    //#endif
}

int main()
{
    test4();
    return 0;
}
