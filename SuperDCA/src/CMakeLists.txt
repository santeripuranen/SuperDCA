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
	${CUDA_INCLUDE_DIRS}
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
endif()

# general optimization flags	
set( SUPERDCA_GCC_OPTIMIZATION_FLAGS "${SUPERDCA_GCC_OPTIMIZATION_FLAGS} -O3 -mavx -funroll-loops -fabi-version=6 --param max-inline-insns-auto=100 --param early-inlining-insns=200 -flto -ffat-lto-objects" )
set( SUPERDCA_GCC_OPTIMIZATION_FLAGS "${SUPERDCA_GCC_OPTIMIZATION_FLAGS} -fdata-sections -ffunction-sections -Wl,--gc-sections" )
set( SUPERDCA_GCC_OPTIMIZATION_FLAGS "${SUPERDCA_GCC_OPTIMIZATION_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden" )
set( SUPERDCA_GCC_OPTIMIZATION_FLAGS "${SUPERDCA_GCC_OPTIMIZATION_FLAGS} -fwhole-program" )
set( SUPERDCA_GCC_OPTIMIZATION_FLAGS "${SUPERDCA_GCC_OPTIMIZATION_FLAGS} -fdevirtualize-at-ltrans" )
#set( SUPERDCA_GCC_OPTIMIZATION_FLAGS "${SUPERDCA_GCC_OPTIMIZATION_FLAGS} -fprefetch-loop-arrays" ) # seems to make SuperDCA run slower

# release build flags
set( SUPERDCA_GCC_RELEASE_FLAGS "-w -Wl,--strip-all" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall ${SUPERDCA_GCC_OPTIMIZATION_FLAGS} ${SUPERDCA_GCC_RELEASE_FLAGS}")

# debug build flags
set( SUPERDCA_GCC_DEBUG_FLAGS "-pg -g -ftree-vectorizer-verbose=2" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall ${SUPERDCA_GCC_OPTIMIZATION_FLAGS} ${SUPERDCA_GCC_DEBUG_FLAGS}")

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SUPERDCA_GCC_OPTIMIZATION_FLAGS}" )

#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wignored-attributes" )

set( CMAKE_LINKER "ld.gold" )

# Add Boost libraries
if( NOT SUPERDCA_NO_BOOST )
	set_target_properties( SuperDCA PROPERTIES COMPILE_FLAGS "--std=c++14 ${SUPERDCA_CXX_STD_VERSION_FLAG} ${SUPERDCA_GCC_OPTIMIZATION_FLAGS}" )
	target_link_libraries( SuperDCA ${Boost_LIBRARIES} )
endif()

# Add CUDA libraries
if( NOT SUPERDCA_NO_CUDA )
	target_link_libraries( SuperDCA ${CUDA_LIBRARIES} )
endif()

# Add TBB libraries
if( NOT SUPERDCA_NO_TBB )
	#superdca_message( "TBB_LIBRARIES=${TBB_LIBRARIES}" )
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
