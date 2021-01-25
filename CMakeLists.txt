cmake_minimum_required(VERSION 3.17)
project(datAlloc C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-D DA_DEBUG)

add_executable(datAlloc src/UsageSample.c src/datAlloc/datAlloc.c src/datAlloc/datAlloc.h)