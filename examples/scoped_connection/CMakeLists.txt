cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( scoped_connection LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    scoped_connection.cpp
)

add_executable( scoped_connection ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( scoped_connection
    PUBLIC ../../
)