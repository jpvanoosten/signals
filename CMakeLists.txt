cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set( BUILD_EXAMPLES ON CACHE BOOL "Build examples." )

project( signals LANGUAGES CXX )

# Enable testing.
include(CTest)

if( BUILD_EXAMPLES )
    add_subdirectory( examples )
endif( BUILD_EXAMPLES )

if( BUILD_TESTING )
    add_subdirectory( tests )
    # Set the startup project.
    set_directory_properties( PROPERTIES 
        VS_STARTUP_PROJECT signal_tests
    )
endif( BUILD_TESTING )


