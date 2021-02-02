# Linux Build System Example

This examples serves to demonstrate usage of the libfly Linux build system.

## Installing libfly

Installing libfly may be done from source or from a stable release package.

To install from source:

    make -C libfly/build/nix release=1 install

To install from a release package, download the [latest release](https://github.com/trflynn89/libfly/releases)
and extract the downloaded `.tar.gz` in the root system directory:

    tar -C / -xjf libfly-nix-[version].[arch].tar.bz2

## Example

The usage example contains a main `Makefile` which imports the libfly build system from its
installed location. It contains the following example targets:

1. libfly_c_example - A binary created from a C project.
2. libfly_c_example_tests - An example unit test of the C project.
3. libfly_cpp_example - A binary created from a C++ project.
4. libfly_cpp_example_tests - An example unit test of the C++ project.
5. libfly_jar_example - An executable JAR file created from a Java project.

To build all of the above:

    make -C libfly/examples/build
