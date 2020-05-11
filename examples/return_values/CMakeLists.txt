cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( return_values LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    return_values.cpp
)

add_executable( return_values ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( return_values
    PUBLIC ../../
)