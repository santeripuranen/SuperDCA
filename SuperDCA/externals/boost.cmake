# Version $Id:$

cmake_minimum_required(VERSION 3.1)

option( SUPERDCA_ENABLE_BOOST "Find Boost and, if successful, enable use in SuperDCA" true )

###############
## Boost setup
###

# Make sure the v3 of boost.filesystem library is used
add_definitions( -DBOOST_FILESYSTEM_VERSION=3 )
# When cross-compiling, we need to explicitly state that
# the thread library needs to be linked
add_definitions( -DBOOST_THREAD_USE_LIB )

set( SUPERDCA_NO_BOOST true CACHE INTERNAL "Don't use Boost, if true" ) # Initialize with default value
if( SUPERDCA_ENABLE_BOOST )
	superdca_message( "check for Boost" )
	# List of usable boost versions.

	set( Boost_USE_STATIC_LIBS ON )
	set( Boost_USE_MULTITHREADED TRUE )
	
	find_package( Boost REQUIRED program_options filesystem iostreams system timer chrono ) #date_time thread
	if( Boost_FOUND )
		set( SUPERDCA_NO_BOOST false CACHE INTERNAL "Don't use Boost, if true" )
		superdca_message( "found Boost v${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}" )
		superdca_message( "include dir: ${Boost_INCLUDE_DIR}" INDENT )
		superdca_message( "library dir: ${Boost_LIBRARY_DIRS}" INDENT )
		
		# Place into global scope
		set( Boost_INCLUDE_DIR ${Boost_INCLUDE_DIR} CACHE INTERNAL "Boost include directory" )
		set( Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} CACHE INTERNAL "Boost library directory" )
		set( Boost_LIBRARIES ${Boost_LIBRARIES} CACHE INTERNAL "List of linkable Boost libraries" )
		
		# stop compiler from nagging about deprecated auto_ptr in boost v1.59.0 and earlier
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations" CACHE INTERNAL "" )
	else()
		superdca_message( "WARNING: could not find Boost" )
	endif()
endif()
if( SUPERDCA_NO_BOOST )
	add_definitions( -DSUPERDCA_NO_BOOST )
	superdca_message( "Boost is DISABLED" )
else()
	superdca_message( "Boost is enabled" )
endif()
