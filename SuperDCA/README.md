# SuperDCA

## About

SuperDCA is a tool for global direct couplings analysis (DCA) of input genome alignments. SuperDCA implements the pseudo-likelihood maximization DCA algorithm (plmDCA) in a fairly efficient manner and is capable of inferring parameters for genome alignments with at least up to 10^5 significant loci and thousands of samples in a shared-memory multi-core compute environment.

## Building SuperDCA[](#building-superdca)

Before compiling SuperDCA we need to patch external code dependencies. Go to the `SuperDCA/externals` directory and apply the patches:

```
sh apply_CppNumericalSolvers_patch.sh
sh apply_vecmathlib_patch.sh
```

The first patch will extend the CppNumericalSolvers interface such that the LBFGS optimizer code is faster when used by SuperDCA. The second patch will fix a gcc compilation issue when using versions older than v7.3.

In order to compile the SuperDCA binary, go to the `SuperDCA/build` directory (create one if necessary; in-source builds are strongly discouraged) and give these commands:

```
cmake -DCMAKE_BUILD_TYPE=Release ..
make SuperDCA
```

This should set up the CMake project, compile the binary and place it into the `bin` directory. If not, then take a look at *Compile-time dependencies* below.

The SuperDCA binary will by default be statically linked, except for the standard C++ runtime library, which is unfeasible to link statically, and [TBB](https://www.threadingbuildingblocks.org/) that can only be linked dynamically. Installing SuperDCA to another location is as easy as copying the binary (given that the [TBB](https://www.threadingbuildingblocks.org/) runtime library is properly installed on your system).

### Compile-time dependencies

The SuperDCA code is written in C++ and wrapped into a [CMake](https://cmake.org/) project. It relies on several external libraries, most of which are fairly common in C++ software development. Your build environment must have the following requirements and compile-time dependencies satisfied:

* A C++14 compliant compiler (development was done using the [GNU C++ compiler](https://gcc.gnu.org/))
* [CMake](https://cmake.org/)
* [Boost](https://www.boost.org/)
* [Intel(R) Threading Building Blocks (TBB)](https://www.threadingbuildingblocks.org/)
* [Eigen v3.3.4](https://eigen.tuxfamily.org/) or later

You may need to set the [`CMAKE_MODULE_PATH`](https://cmake.org/cmake/help/latest/variable/CMAKE_MODULE_PATH.html) environment variable in order for CMake to find all relevant packages.

In addition, SuperDCA depends on the following external libraries (with minor modifications) that came along with the SuperDCA project if you cloned it with `git clone --recursive`:

* [CppNumericalSolvers](https://github.com/PatWie/CppNumericalSolvers) for the LBFGS function optimizer
* [vecmathlib](https://bitbucket.org/eschnett/vecmathlib/wiki/Home) for vectorized math functions

## Using SuperDCA



### Command line options

SuperDCA has a command line interface. Use `SuperDCA -h` or `SuperDCA --help` to get a list of available command line options.

### Parallel execution

SuperDCA will by default use all hardware threads that the host system exposes. Use the `--threads=<number of threads>` option to override the default.

###

