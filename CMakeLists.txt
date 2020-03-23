cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project( signals LANGUAGES CXX )

# Enable testing.
include(CTest)

if( BUILD_TESTING )
    add_subdirectory( tests )
endif( BUILD_TESTING )


