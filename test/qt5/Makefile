# Example usage of the build system.

# Define the path to the source directory
SOURCE_ROOT := $(CURDIR)/../..

# Define the project version
VERSION := $(shell cat $(SOURCE_ROOT)/VERSION.md)

# Import the build API
include ../../build/nix/api.mk

# Build targets
$(eval $(call ADD_TARGET, libfly_qt5_example, test/qt5/example, QT5))

# Import the build system
include ../../build/nix/build.mk
