# Version $Id: $

# Aoegrunt src

############################
## Add sources and includes
###

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${APEGRUNT_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	${TBB_INCLUDE_DIRS}
)

link_directories( ${Boost_LIBRARY_DIRS} )

set( APEGRUNT_LIBRARY_SOURCES
	Apegrunt_options.cpp
	aligned_allocator.cpp
) # *.cpp *.hpp *.cc

#################################
## Add libraries and executables
###

set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

# Add apegrunt library
superdca_target_message( "libapegrunt" )
if( APEGRUNT_LIBRARY_SOURCES )
	#add_library( apegrunt SHARED ${APEGRUNT_LIBRARY_SOURCES} )
	add_library( apegrunt STATIC ${APEGRUNT_LIBRARY_SOURCES} )
	set_target_properties( apegrunt PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
	set_target_properties( apegrunt PROPERTIES LIBRARY_OUTPUT_NAME "libapegrunt.a" )
	set_target_properties( apegrunt PROPERTIES COMPILE_FLAGS "--std=c++14 ${SUPERDCA_CXX_STD_VERSION_FLAG}" )
	set( APEGRUNT_LIBRARIES ${CMAKE_BINARY_DIR}/lib/libapegrunt.a CACHE INTERNAL "Apegrunt library" )
	#set( APEGRUNT_LIBRARIES $<$<TARGET_PROPERTY:LIBRARY_OUTPUT_NAME>:apegrunt> CACHE INTERNAL "Apegrunt library" )
endif()

# general optimization flags	
set( APEGRUNT_GCC_OPTIMIZATION_FLAGS "${APEGRUNT_GCC_OPTIMIZATION_FLAGS} -O3 -mavx -funroll-loops -fabi-version=6 --param max-inline-insns-auto=100 --param early-inlining-insns=200 -flto -ffat-lto-objects" )
set( APEGRUNT_GCC_OPTIMIZATION_FLAGS "${APEGRUNT_GCC_OPTIMIZATION_FLAGS} -fdata-sections -ffunction-sections -Wl,--gc-sections" )
set( APEGRUNT_GCC_OPTIMIZATION_FLAGS "${APEGRUNT_GCC_OPTIMIZATION_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden" )
set( APEGRUNT_GCC_OPTIMIZATION_FLAGS "${APEGRUNT_GCC_OPTIMIZATION_FLAGS} -fwhole-program" )
set( APEGRUNT_GCC_OPTIMIZATION_FLAGS "${APEGRUNT_GCC_OPTIMIZATION_FLAGS} -fdevirtualize-at-ltrans" )
#set( APEGRUNT_GCC_OPTIMIZATION_FLAGS "${APEGRUNT_GCC_OPTIMIZATION_FLAGS} -fprefetch-loop-arrays" ) # seems to make SuperDCA run slower

# release build flags
set( APEGRUNT_LIBRARY_GCC_RELEASE_FLAGS "" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall ${APEGRUNT_GCC_OPTIMIZATION_FLAGS} ${APEGRUNT_GCC_RELEASE_FLAGS}")

# debug build flags
set( APEGRUNT_GCC_DEBUG_FLAGS "-pg -g -ftree-vectorizer-verbose=2" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall ${APEGRUNT_GCC_OPTIMIZATION_FLAGS} ${APEGRUNT_GCC_DEBUG_FLAGS}")

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${APEGRUNT_GCC_OPTIMIZATION_FLAGS}" )

set( CMAKE_LINKER "ld.gold" )

# Add Boost libraries
if( NOT APEGRUNT_NO_BOOST )
	target_link_libraries( apegrunt ${Boost_LIBRARIES} )
endif()

# Add TBB libraries
if( NOT APEGRUNT_NO_TBB )
	target_link_libraries( apegrunt ${TBB_LIBRARIES} )
endif()

if( UNIX )
	target_link_libraries( apegrunt pthread )
#	target_link_libraries( apegrunt Threads::Threads )
endif( UNIX )

# Prevent linking against shared libraries on OS X;
# Apple gcc always links against a shared version of a library if present,
# regardless of -Bstatic or equivalent linker flags.
if(APPLE)
	set_target_properties( libapegrunt PROPERTIES LINK_SEARCH_END_STATIC TRUE )
	set_target_properties( apegrunt PROPERTIES LINK_SEARCH_END_STATIC TRUE )
endif(APPLE)
