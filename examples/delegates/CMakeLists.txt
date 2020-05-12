cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( delegates LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    delegates.cpp
)

add_executable( delegates ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( delegates
    PUBLIC ../../
)