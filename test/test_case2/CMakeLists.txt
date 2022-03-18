cmake_minimum_required(VERSION 3.15)
project(test_case2)

add_library(lib SHARED lib/lib.cpp)

add_executable(main main.cpp)
target_link_libraries(main lib)