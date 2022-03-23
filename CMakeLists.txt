cmake_minimum_required(VERSION 3.21)
project(ldd)

set(CMAKE_CXX_STANDARD 17)

add_executable(ldd
        main.cpp elf/elf.cpp elf/elf.h ldd/ldd.cpp ldd/ldd.h)
