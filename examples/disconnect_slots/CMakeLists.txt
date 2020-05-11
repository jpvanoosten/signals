cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( disconnect_slots LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    disconnect_slots.cpp
)

add_executable( disconnect_slots ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( disconnect_slots
    PUBLIC ../../
)