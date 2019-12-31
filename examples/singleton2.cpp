//
// Created by erez on 12/10/19.
//

#include <singleton.h>

template<unsigned V>
class Data
{
public:
    Data() { fprintf(stdout, "Data::ctor %s\n", __PRETTY_FUNCTION__); }
    ~Data() { fprintf(stdout, "Data::dtor %s\n", __PRETTY_FUNCTION__); }
    static constexpr auto n() { return V; }
};

void test1() { auto &ref1{es::init::singleton<Data<0>>::instance()}; (void)&ref1; }
void test2()
{
     auto &ref1{es::init::singleton<Data<0>>::instance()}; (void)&ref1; 
     auto &ref2{es::init::singleton<Data<2>>::instance()}; (void)&ref2; 
     auto &ref3{es::init::singleton<Data<3>>::instance()}; (void)&ref3; 
}

int main()
{
    test1();
    test2();

    return 0;
}
