# External Projects

This directory contains all third-party projects used by libfly. Most are for benchmarking and
testing purposes only.

The only dependency required by libfly is its build system (for Linux and macOS), flymake, which was
originally part of libfly itself:

* [flymake](https://github.com/trflynn89/flymake)

For unit testing, libfly uses the Catch2 framework:

* [Catch2](https://github.com/catchorg/Catch2)

All other submodules are only for benchmark comparisons:

* [{fmt}](https://github.com/fmtlib/fmt)
* [Boost.JSON](https://github.com/boostorg/json)
* [JSON for Modern C++](https://github.com/nlohmann/json)
