# Version $Id:$

cmake_minimum_required(VERSION 3.1)

#option( SUPERDCA_ENABLE_EIGEN "Find Eigen3 and, if successful, enable use in SuperDCA" true )

##############
## Eigen3 setup
###

set( SUPERDCA_NO_EIGEN true CACHE INTERNAL "Don't use Eigen3, if true" ) # Initialize with default value 
if( SUPERDCA_ENABLE_EIGEN )
	superdca_message( "check for Eigen3" )
	# If Eigen3 is installed in non-standard location on your system, then please set
	# the EIGEN3_ROOT (shell) environment variable to point to the installation root.
	# Also append $EIGEN3_ROOT/cmake to CMAKE_MODULE_PATH, in order to ensure that
	# the FindEigen3.cmake file is found by CMake.
	find_package(Eigen3 3.3.4 REQUIRED)
	if( EIGEN3_FOUND )
		set( SUPERDCA_NO_EIGEN false CACHE INTERNAL "Don't use Eigen3, if true" )
		superdca_message("Found Eigen3 v${EIGEN3_VERSION}")
		superdca_message( "include dir: ${EIGEN3_INCLUDE_DIR}" INDENT )
		#set( EIGEN3_INCLUDE_DIR ${EIGEN3_INCLUDE_DIR} CACHE INTERNAL "Eigen3 include directories" )
	else()
		superdca_message( "WARNING: could not find Eigen3 headers" )
	endif()
endif()
if( SUPERDCA_NO_EIGEN )
	add_definitions( -DSUPERDCA_NO_EIGEN )
	superdca_message( "Eigen3 is DISABLED" )
else()
	superdca_message( "Eigen3 is enabled" )
endif()
