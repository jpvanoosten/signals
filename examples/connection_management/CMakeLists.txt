cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( connection_management LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    connection_management.cpp
)

add_executable( connection_management ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( connection_management
    PUBLIC ../../
)