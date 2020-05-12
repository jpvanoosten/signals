cmake_minimum_required( VERSION 3.17.0 ) # Latest version of CMake when this file was created.

project( examples )

add_subdirectory( hello_world )
add_subdirectory( multiple_slots )
add_subdirectory( slot_arguments )
add_subdirectory( return_values )
add_subdirectory( maximum_value )
add_subdirectory( aggregate_values )
add_subdirectory( member_functions )
add_subdirectory( connection_management )
add_subdirectory( connections )
add_subdirectory( blocked_slots )
add_subdirectory( scoped_connection )
add_subdirectory( disconnect_slots )
add_subdirectory( signal_aliases )
add_subdirectory( delegates )

set_target_properties(
    hello_world
    multiple_slots
    slot_arguments
    return_values
    maximum_value
    aggregate_values
    member_functions
    connection_management
    connections
    blocked_slots
    scoped_connection
    disconnect_slots
    signal_aliases
    delegates
    PROPERTIES FOLDER examples
)
