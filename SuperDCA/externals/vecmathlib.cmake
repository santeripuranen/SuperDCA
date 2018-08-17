# Version $Id:$

cmake_minimum_required(VERSION 3.1)

#option( ${CMAKE_PROJECT_NAME}_ENABLE_VECMATHLIB "Find vecmathlib and, if successful, enable use in ${CMAKE_PROJECT_NAME}" true )

##############
## vecmathlib
###

set( ${CMAKE_PROJECT_NAME}_NO_VECMATHLIB true CACHE INTERNAL "Don't use vecmathlib, if true" ) # Initialize with default value 
if( ${CMAKE_PROJECT_NAME}_ENABLE_VECMATHLIB )
	setup_message( "check for vecmathlib" )
	set( VECMATHLIB_FOUND true )
	set( VECMATHLIB_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/vecmathlib" )
	if( VECMATHLIB_FOUND )
		set( ${CMAKE_PROJECT_NAME}_NO_VECMATHLIB false CACHE INTERNAL "Don't use vecmathlib, if true" )
		setup_message( "found vecmathlib" )
		setup_message( "include dirs: ${VECMATHLIB_INCLUDE_DIR}" INDENT )

		set( VECMATHLIB_INCLUDE_DIR ${VECMATHLIB_INCLUDE_DIR} CACHE INTERNAL "vecmathlib include directory" )
	else()
		setup_message( "WARNING: could not find vecmathlib headers" )
	endif()
endif()
if( ${CMAKE_PROJECT_NAME}_NO_VECMATHLIB )
	add_definitions( -${CMAKE_PROJECT_NAME}_NO_VECMATHLIB )
	setup_message( "vecmathlib is DISABLED" )
else()
	setup_message( "vecmathlib is enabled" )
endif()
