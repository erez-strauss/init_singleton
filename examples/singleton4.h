//
// Created by erez on 12/13/19.
//

#pragma once

#include <singleton.h>

#include <iostream>

class DataA
{
public:
    DataA();
    uint64_t _a{0};
};
class DataB
{
public:
    DataB();
    uint64_t _b{0};
};
class DataC
{
public:
    DataC();
    uint64_t _c{0};
};

namespace {
static inline auto& ar{es::init::singleton<DataA>::instance()};
static inline auto& br{es::init::singleton<DataB>::instance()};
static inline auto& cr{es::init::singleton<DataC>::instance()};
}  // namespace
