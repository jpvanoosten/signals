cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( multiple_slots LANGUAGES CXX )

set( HEADER_FILES
    ../../signals.hpp
    ../../optional.hpp
)

set( SOURCE_FILES
    multiple_slots.cpp
)

add_executable( multiple_slots ${HEADER_FILES} ${SOURCE_FILES} )

target_include_directories( multiple_slots
    PUBLIC ../../
)