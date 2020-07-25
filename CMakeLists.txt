cmake_minimum_required(VERSION 3.7.2)

project( MyProject )

set( PROJECT_OUTPUT_DIR "D:/AngieBinary" )

include( AngieEngine/trunk/Source/Engine/AngieConfig.cmake )

# Engine
include_directories( AngieEngine/trunk/Source/Engine )
add_definitions( -DANGIE_STATIC_LIBRARY )
add_subdirectory( AngieEngine/trunk/Source )

add_subdirectory( EmptyProject )
set_target_properties( EmptyProject PROPERTIES FOLDER "Project" )
