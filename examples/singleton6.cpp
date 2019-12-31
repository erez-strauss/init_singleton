#include <singleton.h>
int main()
{
    es::init::singleton<int>::instance() = 4; // a singleton of a type int, initialized before main() starts, destroyed after main() exits.
    return 0;
}

