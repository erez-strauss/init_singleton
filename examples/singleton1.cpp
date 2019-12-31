#include <singleton.h>

#include <iostream>

// https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines  - read - no globals - no singletons ?

auto &ref0{es::init::singleton<int>::instance()};
auto &ref1{es::init::singleton<int>::instance()};

class SingleData
{
public:
    SingleData()
    {  // volatile ::std::ios_base::Init z{} ;
        std::cout << "SingleData::ctor: " << __PRETTY_FUNCTION__ << std::endl;
    }
    ~SingleData() { std::cout << "SingleData::dtor: " << __PRETTY_FUNCTION__ << std::endl; }
    int _x{0};
};

class SingleDataX
{
public:
    SingleDataX() { std::cout << "SingleDataX::ctor: " << __PRETTY_FUNCTION__ << std::endl; }
    ~SingleDataX()
    {
        std::cout << "SingleDataX::dtor: " << __PRETTY_FUNCTION__ << " "
                  << (void *)&es::init::singleton<SingleData>::instance() << std::endl;
    }
};

int main()
{
    std::cout << "start of main()\n";

    auto &ref2{es::init::singleton<int>::instance()};
    std::cout << "Hello, World!" << std::endl;
    std::cout << "&ref0: " << (void *)&ref0 << " value: " << ref0 << '\n';
    ref0 = 7;
    std::cout << "&ref1: " << (void *)&ref1 << " value: " << ref1 << '\n';
    std::cout << "&ref2: " << (void *)&ref2 << " value: " << ref2 << '\n';

    auto &rd{es::init::singleton<SingleData>::instance()};
    std::cout << "single data addr: " << (void *)&rd << '\n';

    auto &rdx{es::init::singleton<SingleDataX>::instance()};
    std::cout << "single dataX addr: " << (void *)&rdx << '\n';

    std::cout << "end of main()\n";
    return 0;
}
