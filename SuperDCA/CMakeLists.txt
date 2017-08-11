# Version $Id: $

cmake_minimum_required(VERSION 3.1)

# For some reason CMake (at least 3.1.1, 3.3.1) does not always honor the
# compiler environment variables unless we define them in the local scope
set( CMAKE_CXX_COMPILER $ENV{CMAKE_CXX_COMPILER} )
set( CMAKE_C_COMPILER $ENV{CMAKE_C_COMPILER} )

project( SUPERDCA )
set( SUPERDCA_INCLUDE_DIR ${${PROJECT_NAME}_SOURCE_DIR}/include )

include( SuperDCA_options.cmake NO_POLICY_SCOPE )
include( git_commit_info.cmake NO_POLICY_SCOPE )

add_subdirectory( externals )

add_subdirectory( src )
#add_subdirectory( test )
