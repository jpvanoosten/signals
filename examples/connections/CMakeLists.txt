cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( connections LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    connections.cpp
)

add_executable( connections ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( connections
    PUBLIC ../../
)