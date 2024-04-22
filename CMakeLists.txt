cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(TestCpp VERSION 1.0)

include(cmake/zero.cmake)

zero_init()

zero_info()

add_subdirectory(src)
