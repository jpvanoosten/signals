cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( aggregate_values LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    aggregate_values.cpp
)

add_executable( aggregate_values ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( aggregate_values
    PUBLIC ../../
)