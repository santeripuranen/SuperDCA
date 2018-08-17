# Version $Id: $

cmake_minimum_required(VERSION 3.1)

project( SUPERDCA_DEPENDENCIES )

# Setup message helper function
include( setup_message.cmake NO_POLICY_SCOPE )

# initialize CMAKE_MODULE_PATH
include( initialize_cmake_module_path.cmake NO_POLICY_SCOPE )

# Setup of external dependencies
include( boost.cmake NO_POLICY_SCOPE )
include( tbb.cmake NO_POLICY_SCOPE )
include( eigen3.cmake NO_POLICY_SCOPE )

# Setup external libraries that are supplied with the SuperDCA package, but are not part of the SuperDCA source code base.
include( CppNumericalSolvers.cmake NO_POLICY_SCOPE )
include( vecmathlib.cmake NO_POLICY_SCOPE )

# Setup the apegrunt library. SuperDCA is built on functionality provided by apegrunt and won't compile without it.
include( apegrunt.cmake NO_POLICY_SCOPE )
