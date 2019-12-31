#include <app_singletons.h>
class DataA
{
public:
    DataA()
    {
        std::cout << "DataA()\n";
        es::init::args.for_each(
            [](int index, auto& arg) { std::cout << "DataA: arg[" << index << "]: '" << arg << "'" << std::endl; });
        es::init::env.for_each(
            [](int index, auto& earg) { std::cout << "DataA: env[" << index << "]: '" << earg << "'" << std::endl; });
    }
    ~DataA() { std::cout << "~DataA()\n"; }
};
int main()
{
    std::cout << "main() start\n";
    es::init::singleton<DataA>::instance();  // a singleton of a type DataA, initialized before main() starts, destroyed
                                             // after main() exits.
    std::cout << "main() end\n";
    return 0;
}
