cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( signal_aliases LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    signal_aliases.cpp
)

add_executable( signal_aliases ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( signal_aliases
    PUBLIC ../../
)