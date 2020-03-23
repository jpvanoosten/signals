cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( signals_tests )

# Enable testing.
include(CTest)

add_executable( test1 test1.cpp )

add_test( test1 test1 )


