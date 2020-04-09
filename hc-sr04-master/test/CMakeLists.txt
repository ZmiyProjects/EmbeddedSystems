cmake_minimum_required(VERSION 3.14)
project(test C)

set(CMAKE_C_STANDARD 11)

add_executable(test main.c lib_hc.h lib_hc.c)