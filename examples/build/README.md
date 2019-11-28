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

The usage example contains a main `Makefile` which imports the libfly build
system from its installed location. It has a single binary target with one
source directory, defined in `files.mk`.

To build:

    make -C libfly/examples/build
