
#include <iostream>

#include <singleton.h>

extern char **environ;
extern char **argv;

template<typename T>
struct early_initializer {
  [[ using gnu : used, constructor ]] static void init() {
	std::ios_base::Init z;
       	std::cout << "zzz constructor from Data::init() " << __PRETTY_FUNCTION__ << " environ: " << (void*)environ << std::endl;
       	std::cout << "zzz constructor from Data::init() " << __PRETTY_FUNCTION__ << " argv: " << (void*)argv << std::endl;
	es::init::singleton<T>::instance();
    if constexpr (true) {
    }
  }
};
    
struct Data {
  Data()
  {
    std::cout << "Data constructor\n";
  }
#if 0
  [[ using gnu : used, constructor ]] static void init() {
	std::ios_base::Init z;
       	std::cout << "zzz constructor from Data::init() " << __PRETTY_FUNCTION__ << std::endl;
    if constexpr (true) {
    }
  }
  [[ using gnu : used, section(".init_array") ]] inline static void (*first_init)() /* __attribute__((section(".init_array,\"aGw\""), used)) */ {
      []() -> void {
	std::ios_base::Init z; std::cout << "zzz constructor from lambda " << __PRETTY_FUNCTION__ << std::endl;
	}
	};
#endif//0
};

int main()
{
  std::cout << "main start\n";
  early_initializer<Data>::init();
  es::init::singleton<long>::instance();
  es::init::singleton<Data>::instance();
  std::cout << "main end\n";
  return 0;
}

