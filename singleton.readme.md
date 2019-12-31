
## The search for a good Singleton

1. Efficient,  with no condition on every access, no guard variable
2. Multi dependency, on other singleton(s)
3. Multi thread safe
4. Proper initialization order with multiple compile units
5. Very early or lazy initialization
6. Detects circular dependency tree
7. Proper destruction order
8. None intrusive, requires only default constructor, supports native types.
* named

TODO:
 - benchmark, vs Meyers singleton, assembly, and performance.
 - CMakeList.txt & Makefile
 - complete readme.md
 - complete the gtests, and a test script
 - upload to erez-strauss/singleton/singleton.h
 - upload also wiki page


 define a used variable, with init_priority(101), with value returned by the firstrime - 
 remove the construcctor attribute from the firsttime function.
 
Using std::atomic<>, std::unique_ptr<>, std:mutex, std::guard<>

Notes:
We use early initialization, by default,
 in after the c++ library was initialized,
 in order to guarentee that the c++ iostream are available, the code instantiates ::std::ios_base::Init object, which initializes the cout/cerr streams.
 
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











