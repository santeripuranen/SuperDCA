# Extend the CppNumericalSolvers interface
patch CppNumericalSolvers/include/cppoptlib/problem.h CppNumericalSolvers.problem.patch
# Use the extended interface in the morethuente linesearch routine
patch CppNumericalSolvers/include/cppoptlib/linesearch/morethuente.h CppNumericalSolvers.linesearch_morethuente.patch
