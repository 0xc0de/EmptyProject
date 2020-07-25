cmake_minimum_required(VERSION 3.7.2)

project( EmptyProject )

set( PROJECT_OUTPUT_DIR "D:/EmptyProject" )

include( AngieEngine/Source/Engine/AngieConfig.cmake )

# Engine
include_directories( AngieEngine/Source/Engine )
add_definitions( -DANGIE_STATIC_LIBRARY )
add_subdirectory( AngieEngine/Source )

add_subdirectory( Source )
set_target_properties( EmptyProject PROPERTIES FOLDER "Project" )
