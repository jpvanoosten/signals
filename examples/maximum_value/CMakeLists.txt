cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( maximum_value LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
maximum_value.cpp
)

add_executable( maximum_value ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( maximum_value
    PUBLIC ../../
)