# libfly

[![Build Status](https://dev.azure.com/trflynn89/libfly/_apis/build/status/trflynn89.libfly?branchName=main)](https://dev.azure.com/trflynn89/libfly/_build/latest?definitionId=5&branchName=main) [![codecov](https://codecov.io/gh/trflynn89/libfly/branch/main/graph/badge.svg)](https://codecov.io/gh/trflynn89/libfly)  [![Codacy Badge](https://api.codacy.com/project/badge/Grade/9de3533a8aef4358895a018f91e90bd4)](https://www.codacy.com/manual/trflynn89/libfly?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=trflynn89/libfly&amp;utm_campaign=Badge_Grade)

libfly is a C++20 utility library for Linux, macOS, and Windows. It primarily serves as a playground
for learning new C++ standards and exploring interests.

## Features

* A [string library](fly/types/string/string.hpp) for all `std::basic_string` specializations,
  including:
    * A compile-time-validated string formatter based on `std::format`
    * A type-safe lexical type converter
    * UTF-8, UTF-16, and UTF-32 support
* A [JSON type](fly/types/json/json.hpp) as a first-class container
    * Designed to feel like a Python dictionary
* [Instrumentation and logging](fly/logger/logger.hpp)
    * Provides synchronous and asynchronous loggers
    * Supports logging to the console ([with color](fly/logger/styler.hpp)), a file, or any
      user-defined sink
* Networking
    * [TCP](fly/net/socket/tcp_socket.hpp) and [UDP](fly/net/socket/udp_socket.hpp) sockets with
      [IPv4](fly/net/ipv4_address.hpp) and [IPv6](fly/net/ipv6_address.hpp) support
    * An [asynchronous socket](fly/net/socket/socket_service.hpp) IO service
* Coders
    * Base64 [encoder and decoder](fly/coders/base64/base64_coder.hpp)
    * Huffman [encoder](fly/coders/huffman/huffman_encoder.hpp) and [decoder](fly/coders/huffman/huffman_decoder.hpp)
        * Includes a [bitwise IO stream](fly/types/bit_stream/detail/bit_stream.hpp) for reading
          and writing content bit-by-bit
* [Path change monitoring](fly/path/path_monitor.hpp)
    * Monitors any number of files or directories for creation, deletion, or change
* [Configuration management](fly/config/config_manager.hpp)
    * Built around the path monitor to detect changes to a configuration file and automatically
      update configuration objects
* [Task system](fly/task/task_runner.hpp) for policy-based asynchronous execution of tasks
* [INI](fly/parser/ini_parser.hpp) and [JSON](fly/parser/json_parser.hpp) file parsers
* [System resource monitor](fly/system/system_monitor.hpp)
* Thread-safe [queue](fly/types/concurrency/concurrent_queue.hpp) and [stack](fly/types/concurrency/concurrent_stack.hpp)
* [Type traits](fly/traits/traits.hpp) to extend the STL's `<type_traits>`
* Type-safe, fixed-width [integer literal suffixes](fly/types/numeric/literals.hpp)

## Building

After cloning libfly, [external dependencies](extern) should be fetched as well.

```bash
git submodule update --init
```

Of those dependencies, only [flymake](https://github.com/trflynn89/flymake) is required (on Linux
and macOS) to compile the libfly library. [Catch2](https://github.com/catchorg/Catch2) is required
only for building unit tests. All other dependencies are for benchmarking purposes, and not used by
libfly itself.

### Linux and macOS

On Linux and macOS, libfly is compiled using the [flymake](https://github.com/trflynn89/flymake)
build system, which is a GNU Makefile system. To build all libfly targets with the default
configuration:

```bash
make -C libfly/build/nix
```

The following individual Make targets are defined:

* `libfly` - Compiles the libfly library to static and shared library files.
* `libfly_unit_tests` - Compiles libfly unit tests.
* `libfly_benchmarks` - Compiles libfly performance benchmarks.

See the flymake README for [other make goals](https://github.com/trflynn89/flymake/blob/main/README.md#make-goals)
and [build configurations](https://github.com/trflynn89/flymake/blob/main/README.md#build-configuration).

### Windows

On Windows, libfly is compiled with Visual Studio. A [solution file](build/win/libfly.sln) is
provided with the following projects:

* [libfly](build/win/libfly/libfly.vcxproj) - Compiles libfly library to static library files.
* [libfly_unit_tests](build/win/libfly_unit_tests/libfly_unit_tests.vcxproj) - Compiles libfly unit
  tests.
* [libfly_benchmarks](build/win/libfly_benchmarks/libfly_benchmarks.vcxproj) - Compiles libfly
  performance benchmarks.

## Directory structure

* [fly](fly) - Contains the primary source and header files for libfly.
* [build](build) - Contains the build files for all platforms and the CI configuration.
* [test](test) - Contains the libfly unit tests.
* [bench](bench) - Contains performance benchmarks of various libfly components.
* [extern](extern) - Contains all third-party projects used by libfly.
