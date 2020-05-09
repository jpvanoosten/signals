cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( hello_world LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    hello_world.cpp
)

add_executable( hello_world ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( hello_world
    PUBLIC ../../
)