# Version $Id:$

cmake_minimum_required(VERSION 3.1)

#option( SUPERDCA_ENABLE_MPI "Find MPI and, if successful, enable use in SuperDCA" true )

##############
## MPI setup
###

set( SUPERDCA_NO_MPI true CACHE INTERNAL "Don't use MPI, if true" ) # Initialize with default value 
if( SUPERDCA_ENABLE_MPI )
	superdca_message( "check for MPI" )
	find_package( MPI QUIET )
	if( MPI_FOUND )
		set( SUPERDCA_NO_MPI false CACHE INTERNAL "Don't use MPI, if true" )
		#superdca_message( "found MPI interface v${MPI_INTERFACE_VERSION}" )
		#superdca_message( "include dirs: ${MPI_INCLUDE_DIRS}" INDENT )
		#superdca_message( "libraries: ${MPI_LIBRARIES}" INDENT )

		set( MPI_INCLUDE_DIRS ${MPI_INCLUDE_DIRS} CACHE INTERNAL "MPI include directories" )
		set( MPI_LIBRARIES ${MPI_LIBRARIES} CACHE INTERNAL "MPI libraries" )
	else()
		superdca_message( "WARNING: could not find MPI headers and libraries" )
	endif()
endif()
if( SUPERDCA_NO_MPI )
	add_definitions( -DSUPERDCA_NO_MPI )
	superdca_message( "MPI is DISABLED" )
else()
	superdca_message( "MPI is enabled" )
endif()
