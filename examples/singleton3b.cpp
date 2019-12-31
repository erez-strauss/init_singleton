//
// Created by erez on 12/13/19.
//

#include <singleton3.h>

// auto& br { es::init::singleton<DataB>::instance() };

DataB::DataB() : _b(22) { fprintf(stderr, "DataB() %p\n", (void*)this); }

DataC::DataC() : _c(33)
{
    fprintf(stderr, "DataC() %p a: %p b: %p\n", (void*)this, (void*)&es::init::singleton<DataA>::instance(),
            (void*)&es::init::singleton<DataB>::instance());
}
