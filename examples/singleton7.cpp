#include <singleton.h>
class DataA { public: DataA(){std::cout << "DataA()\n";} ~DataA() {std::cout << "~DataA()\n";}};
class DataB { public: DataB(){std::cout << "DataB()\n";} ~DataB() {std::cout << "~DataB()\n";}};
int main()
{
    std::cout << "main() start\n";
    es::init::singleton<DataA>::instance(); // a singleton of a type DataA, initialized before main() starts, destroyed after main() exits.
    es::init::singleton<DataB, es::init::lazy_initializer>::instance(); // a singleton of a type DataA, initialized before main() starts, destroyed after main() exits.
    std::cout << "main() end\n";
    return 0;
}
