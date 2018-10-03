# Define the path to the source directory
SOURCE_ROOT := $(CURDIR)/../..

# Define the project version
VERSION = $(shell cat $(SOURCE_ROOT)/VERSION.md)

# Import the build API
include api.mk

# Build targets
$(eval $(call ADD_TARGET, libfly, fly, LIB))
$(eval $(call ADD_TEST_TARGET, libfly_unit_tests, test))

# Import the build system
include build.mk
