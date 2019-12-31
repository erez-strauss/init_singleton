//
// Created by erez on 12/10/19.
//

#include <iostream>
#include <singleton.h>

namespace es::init {
class std_init {
	public:
	std_init() {
		volatile ::std::ios_base::Init x;
	}
};
}

template<unsigned V>
class Data
{
public:
    Data() {
          es::init::singleton<es::init::std_init>::instance();
          //  [[ using gnu : used ]]
	  //  static auto & init /* __attribute__((used)) */ {es::init::singleton<es::init::std_init>::instance()};

	    std::cout << "Data::ctor " <<  __PRETTY_FUNCTION__ << std::endl;
    }
    ~Data() {
	    std::cout << "Data::dtor " <<  __PRETTY_FUNCTION__ << std::endl;
    }
    uint64_t _data {0};
    static constexpr auto n() { return V; }
};

void test1() { es::init::singleton<Data<0>>::instance(); }
void test2()
{
    es::init::singleton<Data<0>>::instance();
    es::init::singleton<Data<2>>::instance();
    es::init::singleton<Data<3>>::instance();
}

void test3()
{
	volatile int x {0};
	for (x = 0; x < 1000; ++x)
             es::init::singleton<Data<0>>::instance();
}

void test4()
{
	volatile int x {0};
	for (x = 0; x < 1000; ++x)
	{
             volatile auto & z { es::init::singleton<Data<0>>::instance() };
	     (void)&z;
	}
	
}
void test5()
{
	volatile int x {0};
	for (x = 0; x < 1000; ++x)
             es::init::singleton<Data<0>>::instance()._data++;
	// for (x = 0; x < 1000; ++x)
        //     volatile auto & z { es::init::singleton<Data<0>>::instance() };
}






int main()
{
    test1();
    test2();
    test3();

    return 0;
}
