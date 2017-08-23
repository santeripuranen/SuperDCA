### Build info
This is a stripped **linux x86_64** build with **avx** instructions enabled, and using static linking when possible and feasible (see *Dynamic (runtime) library dependencies* below). The C++ runtime library (*libstdc++*) is dynamically linked. The binary was compiled with [gcc v7.2.0](https://gcc.gnu.org/) against [boost v1.64.0](https://www.boost.org/) and [eigen v3.3.4](https://eigen.tuxfamily.org/).

### Dynamic (runtime) library dependencies
SuperDCA requires the [Intel(R) Threading Building Blocks (TBB)](https://www.threadingbuildingblocks.org/) library. Your linux distribution may come with TBB pre-installed or available as an easy to install managed package. Otherwise you will need to [download](https://github.com/01org/tbb/releases/) and install the library yourself.

Please make sure that *libtbb.so* is on the dynamic linker search path (add path to the *LD_LIBRARY_PATH* environment variable if necessary).
