//
// Created by erez on 12/13/19.
//

#include <singleton3.h>

DataA::DataA() : _a(11) { fprintf(stderr, "DataA() %p\n", (void*)this); }
