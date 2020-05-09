cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( slot_arguments LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    slot_arguments.cpp
)

add_executable( slot_arguments ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( slot_arguments
    PUBLIC ../../
)