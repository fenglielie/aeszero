cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(aeszero VERSION 1.0)

include(cmake/zero.cmake)

zero_init()

zero_info()

add_subdirectory(src)

include(CTest)
add_subdirectory(test)

install(TARGETS aeszero)
set(CPACK_GENERATOR "ZIP;TGZ")
include(CPack)
