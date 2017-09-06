# Use git branch and commit hash information in a CMake project
#
# http://xit0.org/2013/04/cmake-use-git-branch-and-commit-details-in-project/

# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE SUPERDCA_GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE SUPERDCA_GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_definitions("-DSUPERDCA_GIT_COMMIT_HASH=${SUPERDCA_GIT_COMMIT_HASH}")
add_definitions("-DSUPERDCA_GIT_BRANCH=${SUPERDCA_GIT_BRANCH}")
