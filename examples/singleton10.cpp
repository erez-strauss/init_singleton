
#include <app_singletons.h>

#include <iostream>

template<typename T>
struct CDReporter
{
    CDReporter() { std::cout << "constructor: " << __PRETTY_FUNCTION__ << " this: " << (void*)this << std::endl; }
    ~CDReporter() { std::cout << "destructor: " << __PRETTY_FUNCTION__ << " this: " << (void*)this << std::endl; }
};

struct ComponentE : public CDReporter<ComponentE>
{
    ComponentE() {}
};
struct ComponentD : public CDReporter<ComponentD>
{
    ComponentD() {}
};
struct ComponentC : public CDReporter<ComponentC>
{
    ComponentC() : _d(es::init::singleton<ComponentD>::instance()) { es::init::singleton<ComponentE>::instance(); }
    ComponentD& _d;
};
struct ComponentB : public CDReporter<ComponentB>
{
    ComponentB() { es::init::singleton<ComponentC>::instance(); }
};
struct ComponentA : public CDReporter<ComponentA>
{
    ComponentA() { es::init::singleton<ComponentB>::instance(); }
};
class Engine
{
public:
    Engine() : _a(es::init::singleton<ComponentA>::instance()) {}
    ComponentA& _a;
};

int main()
{
    Engine e{};

    return 0;
}
