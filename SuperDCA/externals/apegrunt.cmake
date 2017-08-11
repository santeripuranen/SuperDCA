# Version $Id:$

cmake_minimum_required(VERSION 3.1)

option( SUPERDCA_ENABLE_APEGRUNT "Find Apegrunt and, if successful, enable use in SuperDCA" true )

##############
## Apegrunt setup
###

set( SUPERDCA_NO_APEGRUNT true CACHE INTERNAL "Don't use Apegrunt, if true" ) # Initialize with default value 

if( SUPERDCA_ENABLE_APEGRUNT )
	superdca_message( "check for Apegrunt" )
	add_definitions( -DNDEBUG )
	add_subdirectory( apegrunt )

	if( APEGRUNT_FOUND )
		set( SUPERDCA_NO_APEGRUNT false CACHE INTERNAL "Don't use Apegrunt, if true" )

		superdca_message( "found Apegrunt" )
		superdca_message( "include dir: ${APEGRUNT_INCLUDE_DIR}" INDENT )
		superdca_message( "library: ${APEGRUNT_LIBRARIES}" INDENT )

		set( APEGRUNT_INCLUDE_DIR ${APEGRUNT_INCLUDE_DIR} CACHE INTERNAL "Apegrunt include directory" )
		set( APEGRUNT_LIBRARIES ${APEGRUNT_LIBRARIES} CACHE INTERNAL "Apegrunt libraries" )
	else()
		superdca_message( "WARNING: could not find Apegrunt" )
	endif()		
endif()
if( SUPERDCA_NO_APEGRUNT )
	add_definitions( -DSUPERDCA_NO_APEGRUNT )
	superdca_message( "Apegrunt is DISABLED" )
else()
	superdca_message( "Apegrunt is enabled" )
endif()	

