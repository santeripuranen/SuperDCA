# Version $Id: $

cmake_minimum_required(VERSION 3.1)

project( SUPERDCA_EXTERNALS )

# Internal helper function for printing of status messages
function( superdca_message message_string )
	set( PREFIX "SuperDCA setup:" )
	if( ARGN MATCHES INDENT )
		string( REGEX REPLACE "." " " PAD ${PREFIX} )
		message( "${PAD} ${message_string}" )
	else()
		message( "${PREFIX} ${message_string}" )
	endif()
endfunction()

# Internal helper function for printing of "add target" messages
function( superdca_target_message target_name )
	superdca_message( "add target ${target_name}" )
endfunction()

if( DEFINED ENV{CMAKE_MODULE_PATH} )
  string( REPLACE ":" ";" SYSTEM_CMAKE_MODULE_PATH "$ENV{CMAKE_MODULE_PATH}" )
  set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${SYSTEM_CMAKE_MODULE_PATH} )
  superdca_message( "CMake module search path detected from system config" )
endif()

if( CMAKE_MODULE_PATH )
  superdca_message( "CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}" INDENT )
endif()

# Setup of external dependencies
include( boost.cmake NO_POLICY_SCOPE )
include( cuda.cmake NO_POLICY_SCOPE )
include( tbb.cmake NO_POLICY_SCOPE )
include( mpi.cmake NO_POLICY_SCOPE )
include( eigen3.cmake NO_POLICY_SCOPE )

# Setup external libraries that are supplied with the SuperDCA package, but are not part of the SuperDCA source code base.
include( CppNumericalSolvers.cmake NO_POLICY_SCOPE )
include( vecmathlib.cmake NO_POLICY_SCOPE )

# Setup the apegrunt library. SuperDCA is built on functionality provided by apegrunt and won't compile without it.
include( apegrunt.cmake NO_POLICY_SCOPE )
