# Version $Id:$

cmake_minimum_required(VERSION 3.1)

option( APEGRUNT_ENABLE_BOOST "Find Boost and, if successful, enable use in APEGRUNT" true )
option( APEGRUNT_ENABLE_TBB "Find TBB and, if successful, enable use in APEGRUNT" true )
option( APEGRUNT_ENABLE_COMPILER_INTRINSICS "Find compiler intrinsics headers and, if successful, enable use in APEGRUNT" true )
option( APEGRUNT_ENABLE_GPROF "Generate instrumented binaries for profiling with gprof" false ) # off by default
option( APEGRUNT_ENABLE_DOXYGEN "Find Doxygen and enable documentation generation" true ) 
