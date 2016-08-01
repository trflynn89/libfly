# Example usage of the build system.

# Define the path to the source directory
SOURCE_ROOT := $(CURDIR)

# Import the build API
include /usr/src/fly/api.mk

# Set default target
$(eval $(call SET_DEFAULT_TARGET, libflytest))

# Main targets
$(eval $(call ADD_TARGET, libflytest, $(SOURCE_ROOT), BIN))

# Import the build system
include /usr/src/fly/build.mk
