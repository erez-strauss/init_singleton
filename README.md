# init_singleton
Proper Initialization of singletons and resolve initialization order issues

## Implementation
In order to remove any guard access from the critical path, an atomic<> function pointer is used to get the reference to the
instance of the given singleton.
This pointer is initialized, at program load time, to point to an initialization function, that upon successful creation of the
singleton object, will change the value of the atomic pointer to refer to a simple method that knows that the
pointer to the instance is valid.

From that point on, there is no need to check whether the object exists.

In order to call their destructors, there are unique_ptr<> pointing to the singleton, with a specialized deleter, which reduces the global counter of active singletons (of any type)
once there are no more active singletons, the code will destroy them one by one, in reverse order of creation.

## Usage examples

```c++
#include <singleton.h>
int main()
{
    es::init::singleton<int>::instance() = 4; // a singleton of a type int, initialized before main() starts, destroyed after main() exits.
    return 0;
}
```
singleton6.cpp

```cpp
#include <singleton.h>
class DataA { public: DataA(){std::cout << "DataA()\n";} ~DataA() {std::cout << "~DataA()\n";}};
class DataB { public: DataB(){std::cout << "DataB()\n";} ~DataB() {std::cout << "~DataB()\n";}};
int main()
{
    std::cout << "main() start\n";
 // a singleton of a type DataA, initialized before main() starts, destroyed after main() exits.
    es::init::singleton<DataA>::instance();
 // a singleton of a type DataB, initialized on first access to it, destroyed after main() exits.
    es::init::singleton<DataB, es::init::lazy_initializer>::instance();
    std::cout << "main() end\n";
    return 0;
}

$ ./singleton7
DataA()
main() start
DataB()
main() end
~DataB()
~DataA()
```
singleton7.cpp

The above example shows the early initialized singleton of type DataA, that is initialized before main()
and the lazy initialized singleton of type DataB.

One problem with early initialized objects how to access the command line arguments before the main() starts.
to solve this the early_initializer template type, adds a method init(int,char\*\*) with attribute constructor
which is called before main, but with the proper arguments.
The app_singletons.h creates two singletons *es::init::args* for the
arguments from command line and *es::init::env* for the environment variables.

The following example shows an early initialized singleton, which access the command line argument and the environment variables 
```cpp
#include <app_singletons.h>
class DataA { public: DataA(){std::cout << "DataA()\n";
   es::init::args.for_each(
        [](int index, auto& arg) { std::cout << "DataA: arg[" << index << "]: '" << arg << "'" << std::endl; });
   es::init::env.for_each(
        [](int index, auto& earg) { std::cout << "DataA: env[" << index << "]: '" << earg << "'" << std::endl; });
} ~DataA() {std::cout << "~DataA()\n";}};
int main()
{
    std::cout << "main() start\n";
    es::init::singleton<DataA>::instance(); // a singleton of a type DataA, initialized before main() starts, destroyed after main() exits.
    std::cout << "main() end\n";
    return 0;
}

$ ./singleton8 a b c
DataA()
DataA: arg[0]: './singleton8'
DataA: arg[1]: 'a'
DataA: arg[2]: 'b'
DataA: arg[3]: 'c'
DataA: env[0]: 'SHELL=/bin/bash'
.....
DataA: env[63]: '_=./singleton8'
main() start
main() end
~DataA()
```
singleton8.cpp


## The search for a good Singleton

1. Efficient, with no condition on every access, no guard variable
2. Multi dependency, on other singleton(s)
3. Multi thread safe
4. Proper initialization order with multiple compile units
5. Very early or lazy initialization
6. Detects circular dependency tree
7. Proper destruction order
8. None intrusive, requires only default constructor, supports native types.
9. Early init / Lazy init - resolved the command line arguments and environment veriables for early initialized objects.

TODO:
 - benchmark, vs Meyers singleton, assembly, and performance.
 - improve CMakeList.txt & Makefile
 - complete this README.md
 - more google tests and test scripts
 - a wiki page
 - port to windows, as this version was test on Linux only, both g++ and clang++

Notes:
We use early initialization, by default,
 - in order to guarantee that the c++ iostream are available, the code instantiates ::std::ios_base::Init object, which initializes the cout/cerr streams.
 - the early_initializer provides access to command line arguments to early initialized object before entering main()
 
The early initialization takes place before the main starts.

There are many reasons not to use global variables and singletons, and there are many good reasons to use singletons.

I'll focus on the good reasons, and work towards a good singleton that address many of the good reasons to avoid it.


## Singleton use cases

* Initialization order of application components holding references or using other components at init time, using singlton<> of different types.
   The following example (examples/singleton10.cpp) shows few classes (I use struct to simpify the example text)
   
   - ComponentE - has no dependencies - used as singleton inside ComponentC constructor
   - ComponentD - has no dependencies - used as singleton as initialization value to ComponenD reference in componentC
   - ComponentC - depends on D and E
   - ComponentB - depend on C, as it access the C singleton
   - ComponentA - depends on B, and indirectly on all the others
   - Engine - depends on ComponentA
   
   All the above component can be in a single cpp file or each in its own cpp files, and their initialization order
   is well defined, and does not depent on linkage order.

```   
#include <app_singletons.h>
#include <iostream>

template<typename T>
struct CDReporter
{
CDReporter() { std::cout << "constructor: " << __PRETTY_FUNCTION__ << " this: " << (void*)this << std::endl; }
~CDReporter() { std::cout << "destructor: " << __PRETTY_FUNCTION__ << " this: " << (void*)this << std::endl; }
};

struct ComponentE : public CDReporter<ComponentE> { ComponentE() { } };
struct ComponentD : public CDReporter<ComponentD> { ComponentD() { } };
struct ComponentC : public CDReporter<ComponentC> { ComponentC() : _d(es::init::singleton<ComponentD>::instance()) { es::init::singleton<ComponentE>::instance(); } ComponentD& _d; };
struct ComponentB : public CDReporter<ComponentB> { ComponentB() { es::init::singleton<ComponentC>::instance(); } };
struct ComponentA : public CDReporter<ComponentA> { ComponentA() { es::init::singleton<ComponentB>::instance(); } };
class Engine { public: Engine() : _a(es::init::singleton<ComponentA>::instance()) {} ComponentA& _a; };

int main() { Engine e{}; return 0; }
```

* Deep nested function that internally needs access to a singleton information for decision

   This example9.cpp shows access to command-line arguments from the SetupInfo object.
   It eliminate the need to pass references to singleton<> objects from containing Objects down to their internal objects (ProcessorA - internal to B, while B internal to C)
   As this implementation is faster than others it reduces the overhead of accessing singletons objects.

```
#include <app_singletons.h>
#include <iostream>

class SetupInfo {
    bool _verbose{false};
public:
    SetupInfo() {
        es::init::args.for_each([&](auto, auto opt) {
            if (opt && opt[0] == '-' && opt[1] == 'v' && !opt[2]) _verbose = true; });
    }
    bool verbose() { return _verbose; }
};

class ProcessorA {
public:
    int action() {
        if (es::init::singleton<SetupInfo>::instance().verbose()) {
            // real action.
            std::cout << "verbose\n";
            return 1;
        }
        return 0;
    }
};

class ProcessorB {
public:
    ProcessorB(ProcessorA& pa) : _processorA(pa) {}
    int         action() { return _processorA.action(); }
    ProcessorA& _processorA;
};

class ProcessorC {
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
```

## Comparison to other Singleton implementation

So first I start with the reference implementation, and will address its drawbacks.
The reference will be Meyers Singleton:

```
template<typename T>
T* instance() {
   static T* the_one = new T{};
   return the_one;
}
```
The problems with the above:

 1. There is a guard variable to make sure we initialized the 'the_one' variable.
 2. Every time we access the singleton<T>::instance(); we have a conditional code checking that variable.
 3. There is no destructor. and if we replace the static plain pointer with std::unique_ptr<T> we have no guarenteed ordered of destruction between compile units (check order of destruction)
 4. The first call by the user is the one that will create the object, sometime we want it to be created even before main() starts.

To optimize the accessor, I'll switch the pointer to the accessing functions, at initialization, to a function that already knows that it was created.
the idea is to hold atomic<> pointer to a function which retrieves the singleton reference.
This pointer is initialized at program load to point to initializer function, that changes it to point to an optimized function that knows that it was already initialized.


