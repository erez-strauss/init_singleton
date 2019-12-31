cmake_minimum_required(VERSION 3.14)
project(es_init_singleton)

include_directories(SYSTEM ./ ../)
include_directories(SYSTEM ./examples ./tests)

set(CMAKE_CXX_STANDARD 20)
add_compile_options( -mcx16 )

add_executable(singleton1 examples/singleton1.cpp singleton.h)

add_executable(singleton2 examples/singleton2.cpp singleton.h)

add_executable(singleton3 examples/singleton3.cpp examples/singleton3.h examples/singleton3a.cpp examples/singleton3b.cpp)

add_executable(singleton5 examples/singleton5.cpp singleton.h)
