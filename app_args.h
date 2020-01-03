
#pragma once

#include <singleton.h>

// #include <iostream>

namespace es::init {

class app_args
{
public:
    app_args()
    {
        for (auto ii = 0; ii < app_argc; ++ii)
            ;  // std::cout << "app arg[" << ii << "]: " << app_argv[ii] << std::endl;
    }
    template<typename F>
    void for_each(F&& f)
    {
        for (auto ii = 0; ii < app_argc; ++ii) f(ii, app_argv[ii]);
    }
};

[[using gnu: used]] static inline auto& args{singleton<app_args, early_initializer>::instance()};

}  // namespace es::init
