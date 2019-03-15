# Example usage of the build system.

# Define the path to the source directory
SOURCE_ROOT := $(CURDIR)

# Define the project version
VERSION = 1.0.0

# Import the build API
include /usr/src/fly/api.mk

# Main targets
$(eval $(call ADD_TARGET, libfly_build_test, ., BIN))

# Import the build system
include /usr/src/fly/build.mk
