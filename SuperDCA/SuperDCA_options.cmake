# Version $Id:$

cmake_minimum_required(VERSION 3.1)

option( SUPERDCA_ENABLE_BOOST "Find Boost and, if successful, enable use in SUPERDCA" true )
option( SUPERDCA_ENABLE_TBB "Find TBB and, if successful, enable use in SUPERDCA" true )
option( SUPERDCA_ENABLE_CUDA "Find CUDA and, if successful, enable use in SUPERDCA" false ) # off by default
option( SUPERDCA_ENABLE_LAPACK "Find LAPACK and, if successful, enable use in SUPERDCA" false ) # off by default
option( SUPERDCA_ENABLE_MPI "Find MPI and, if successful, enable use in SUPERDCA" false ) # off by default
option( SUPERDCA_ENABLE_COMPILER_INTRINSICS "Find compiler intrinsics headers and, if successful, enable use in SUPERDCA" true )
option( SUPERDCA_ENABLE_GPROF "Generate instrumented binaries for profiling with gprof" false ) # off by default
option( SUPERDCA_ENABLE_DOXYGEN "Find Doxygen and enable documentation generation" false ) 
