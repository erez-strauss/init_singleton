
#include <iostream>

[[using gnu: constructor(101)]] void stuff_cxx(int argc, char **argv)
{
    static ::std::ios_base::Init x;
    for (int i = 0; i < argc; i++)
        std::cout << "" << __PRETTY_FUNCTION__ << ": argv[" << i << "] = '" << argv[i] << "'\n";
}

__attribute__((constructor)) void stuff(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        printf("%s: argv[%d] = '%s'\n", __PRETTY_FUNCTION__, i, argv[i]);
    }
}

int main(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        printf("%s: argv[%d] = '%s'\n", __PRETTY_FUNCTION__, i, argv[i]);
    }
    return 0;
}
