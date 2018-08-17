# Version $Id: $

# SuperDCA src

############################
## Add sources and includes
###

include_directories(
	${SUPERDCA_INCLUDE_DIR}
	${APEGRUNT_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${EIGEN3_INCLUDE_DIR}
	${TBB_INCLUDE_DIRS}
	${CPPNUMERICALSOLVERS_INCLUDE_DIR}
	${VECMATHLIB_INCLUDE_DIR}
)

link_directories( ${Boost_LIBRARY_DIRS} )

set( SUPERDCA_SOURCES
	SuperDCA_options.cpp
	plmDCA_options.cpp
	SuperDCA_commons.cpp
) # *.cpp *.hpp *.cc

#################################
## Add libraries and executables
###

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

if( SUPERDCA_SOURCES )
	add_executable( SuperDCA
		SuperDCA.cpp
		${SUPERDCA_SOURCES}
	)
	target_link_libraries( SuperDCA apegrunt )
	set_target_properties( SuperDCA PROPERTIES COMPILE_FLAGS "--std=c++14" )
endif()

# general optimization flags	
set( SUPERDCA_GCC_OPTIMIZATION_FLAGS "${SUPERDCA_GCC_OPTIMIZATION_FLAGS} -O3 -mavx -ftree-vectorize -fwhole-program -flto -ffat-lto-objects" )

# release build flags
set( SUPERDCA_GCC_RELEASE_FLAGS "-w -Wl,--strip-all -fvisibility=hidden -fvisibility-inlines-hidden" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall ${SUPERDCA_GCC_OPTIMIZATION_FLAGS} ${SUPERDCA_GCC_RELEASE_FLAGS}")

# debug build flags
set( SUPERDCA_GCC_DEBUG_FLAGS "-pg -g -ftree-vectorizer-verbose=2" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall ${SUPERDCA_GCC_OPTIMIZATION_FLAGS} ${SUPERDCA_GCC_DEBUG_FLAGS}")

#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SUPERDCA_GCC_OPTIMIZATION_FLAGS}" )

# set preferred linker
set( CMAKE_LINKER "ld.gold" )

# Add Boost libraries
if( NOT SUPERDCA_NO_BOOST )
	target_link_libraries( SuperDCA ${Boost_LIBRARIES} )
endif()

# Add TBB libraries
if( NOT SUPERDCA_NO_TBB )
	target_link_libraries( SuperDCA ${TBB_LIBRARIES} )
endif()

# Add CppNumericalSorvers libraries
if( NOT SUPERDCA_NO_CPPNUMERICALSOLVERS )
	target_link_libraries( SuperDCA ${CPPNUMERICALSOLVERS_LIBRARIES} )
endif()

if( UNIX )
	target_link_libraries( SuperDCA pthread )
#	target_link_libraries( SuperDCA Threads::Threads )
endif( UNIX )

# Prevent linking against shared libraries on OS X;
# Apple gcc always links against a shared version of a library if present,
# regardless of -Bstatic or equivalent linker flags.
if(APPLE)
	set_target_properties( SuperDCA PROPERTIES LINK_SEARCH_END_STATIC TRUE )
endif(APPLE)
