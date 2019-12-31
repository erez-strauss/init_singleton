# init_singleton
Proper Initialization of singletons and resolve all initialization order issues


## The search for a good Singleton

1. Efficient,  with no condition on every access, no guard variable
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
 - complete readme.md
 - complete the gtests, and a test script
 - upload a wiki page
 - port to windows, as this version was test on Linux only, both g++ and clang++

Using std::atomic<>, std::unique_ptr<>, std:mutex, std::guard<>

Notes:
We use early initialization, by default,
 - in order to guarentee that the c++ iostream are available, the code instantiates ::std::ios_base::Init object, which initializes the cout/cerr streams.
 - the early_args_initializer provides access to command line arguments to ealry initilized object before entring main()
 
The early initialization takes place before the main starts.

There are many reasons not to use global variables and singletons, and there are many good reasons to use singletons.

I'll focus on the good reasons, and work towards a good singleton that address many of the good reasons to avoid it.

##

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
the idea is to 











