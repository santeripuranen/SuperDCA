# Version $Id:$

cmake_minimum_required(VERSION 3.1)

option( SUPERDCA_ENABLE_VECMATHLIB "Find vecmathlib and, if successful, enable use in SuperDCA" true )

##############
## vecmathlib
###

set( SUPERDCA_NO_VECMATHLIB true CACHE INTERNAL "Don't use vecmathlib, if true" ) # Initialize with default value 
if( SUPERDCA_ENABLE_VECMATHLIB )
	superdca_message( "check for vecmathlib" )
	set( VECMATHLIB_FOUND true )
	set( VECMATHLIB_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/vecmathlib" )
	if( VECMATHLIB_FOUND )
		set( SUPERDCA_NO_VECMATHLIB false CACHE INTERNAL "Don't use vecmathlib, if true" )
		superdca_message( "found vecmathlib" )
		superdca_message( "include dirs: ${VECMATHLIB_INCLUDE_DIR}" INDENT )

		set( VECMATHLIB_INCLUDE_DIR ${VECMATHLIB_INCLUDE_DIR} CACHE INTERNAL "vecmathlib include directory" )
	else()
		superdca_message( "WARNING: could not find vecmathlib headers" )
	endif()
endif()
if( SUPERDCA_NO_VECMATHLIB )
	add_definitions( -SUPERDCA_NO_VECMATHLIB )
	superdca_message( "vecmathlib is DISABLED" )
else()
	superdca_message( "vecmathlib is enabled" )
endif()
