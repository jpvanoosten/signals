cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( blocked_slots LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    blocked_slots.cpp
)

add_executable( blocked_slots ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( blocked_slots
    PUBLIC ../../
)