# Version $Id:$

cmake_minimum_required(VERSION 3.1)

option( SUPERDCA_ENABLE_TBB "Find TBB and, if successful, enable use in SuperDCA" true )

##############
## Threading Building Blocks setup
###

set( SUPERDCA_NO_TBB true CACHE INTERNAL "Don't use TBB, if true" ) # Initialize with default value 
if( SUPERDCA_ENABLE_TBB )
	superdca_message( "check for TBB" )
	# If FindTBB.cmake is not present in your system, then
	# get it from http://findtbb.googlecode.com/svn/trunk
	# FindTBB.cmake can be installed anywhere as long as
	# the CMAKE_MODULE_PATH (shell) environment variable
	# is set to point to that location.
	find_package( TBB QUIET )
	if( TBB_FOUND )
		set( SUPERDCA_NO_TBB false CACHE INTERNAL "Don't use TBB, if true" )
		superdca_message( "found TBB interface v${TBB_INTERFACE_VERSION}" )
		superdca_message( "include dirs: ${TBB_INCLUDE_DIRS}" INDENT )
		superdca_message( "libraries: ${TBB_LIBRARIES}" INDENT )

		set( TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIRS} CACHE INTERNAL "TBB include directories" )
		set( TBB_LIBRARIES ${TBB_LIBRARIES} CACHE INTERNAL "TBB libraries" )
	else()
		superdca_message( "WARNING: could not find TBB headers and libraries" )
	endif()
endif()
if( SUPERDCA_NO_TBB )
	add_definitions( -DSUPERDCA_NO_TBB )
	superdca_message( "TBB is DISABLED" )
else()
	superdca_message( "TBB is enabled" )
endif()
