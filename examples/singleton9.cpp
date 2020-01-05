
#include <app_singletons.h>

#include <iostream>

class SetupInfo
{
    bool _verbose{false};

public:
    SetupInfo()
    {
        es::init::args.for_each([&](auto, auto opt) {
            if (opt && opt[0] == '-' && opt[1] == 'v' && !opt[2]) _verbose = true;
        });
    }
    bool verbose() { return _verbose; }
};

class ProcessorA
{
public:
    int action()
    {
        if (es::init::singleton<SetupInfo>::instance().verbose())
        {
            // real action.
            std::cout << "verbose\n";
            return 1;
        }
        return 0;
    }
};
class ProcessorB
{
public:
    ProcessorB(ProcessorA& pa) : _processorA(pa) {}
    int         action() { return _processorA.action(); }
    ProcessorA& _processorA;
};
class ProcessorC
{
public:
    ProcessorC(ProcessorB& pb) : _processorB(pb) {}
    int         action() { return _processorB.action(); }
    ProcessorB& _processorB;
};

int main()
{
    ProcessorA a;
    ProcessorB b{a};
    ProcessorC c{b};

    c.action();
}
