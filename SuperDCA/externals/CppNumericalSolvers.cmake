# Version $Id:$

cmake_minimum_required(VERSION 3.1)

#option( ${CMAKE_PROJECT_NAME}_ENABLE_CPPNUMERICALSOLVERS "Find CppNumericalSolvers and, if successful, enable use in ${CMAKE_PROJECT_NAME}" true )

##############
## CppNumericalSolvers setup
###

set( ${CMAKE_PROJECT_NAME}_NO_CPPNUMERICALSOLVERS true CACHE INTERNAL "Don't use CppNumericalSolvers, if true" ) # Initialize with default value 

if( ${CMAKE_PROJECT_NAME}_ENABLE_CPPNUMERICALSOLVERS )
	setup_message( "check for CppNumericalSolvers" )
	#set( CPPNUMERICALSOLVERS_NO_TESTING true )
	add_definitions( -DNDEBUG )
	#add_subdirectory( CppNumericalSolvers )
	# CppNumericalSolvers is not a CMake project any more, so we'll just define these (needed by SuperDCA) here 

	# Eigen3 Headerfiles
	find_package(Eigen3 REQUIRED)
	include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})

	# Definitions needed for SuperDCA integration
	set( CPPNUMERICALSOLVERS_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/CppNumericalSolvers/include CACHE INTERNAL "CppNumericalSolvers include dir" )
	set( CPPNUMERICALSOLVERS_FOUND true CACHE INTERNAL "CppNumericalSolvers found, if true" )

	if( CPPNUMERICALSOLVERS_FOUND )
		set( ${CMAKE_PROJECT_NAME}_NO_CPPNUMERICALSOLVERS false CACHE INTERNAL "Don't use CppNumericalSolvers, if true" )

		setup_message( "found CppNumericalSolvers" )
		setup_message( "include dir: ${CPPNUMERICALSOLVERS_INCLUDE_DIR}" INDENT )
		#setup_message( "runtime library: ${CPPNUMERICALSOLVERS_LIBRARIES}" INDENT )

		set( CPPNUMERICALSOLVERS_INCLUDE_DIRS ${CPPNUMERICALSOLVERS_INCLUDE_DIRS} CACHE INTERNAL "CppNumericalSolvers include directories" )
		set( CPPNUMERICALSOLVERS_LIBRARIES ${CPPNUMERICALSOLVERS_LIBRARIES} CACHE INTERNAL "CppNumericalSolvers libraries" )
	else()
		setup_message( "WARNING: could not find CppNumericalSolvers" )
	endif()		
endif()
if( ${CMAKE_PROJECT_NAME}_NO_CPPNUMERICALSOLVERS )
	add_definitions( -D${CMAKE_PROJECT_NAME}_NO_CPPNUMERICALSOLVERS )
	setup_message( "CppNumericalSolvers is DISABLED" )
else()
	setup_message( "CppNumericalSolvers is enabled" )
endif()	

