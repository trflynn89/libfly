# Define the path to the source directory
SOURCE_ROOT := $(CURDIR)

# Define the library version
VERSION = $(shell cat $(SOURCE_ROOT)/VERSION.md)

# Import the build API
include $(SOURCE_ROOT)/build/api.mk

# Set default target
$(eval $(call SET_DEFAULT_TARGET, libfly))

# Main targets
$(eval $(call ADD_TARGET, libfly, fly, LIB))

# Unit tests
$(eval $(call ADD_TEST_TARGET, concurrency_test, test/concurrency))
$(eval $(call ADD_TEST_TARGET, config_test, test/config))
$(eval $(call ADD_TEST_TARGET, file_test, test/file))
$(eval $(call ADD_TEST_TARGET, socket_test, test/socket))
$(eval $(call ADD_TEST_TARGET, string_test, test/string))
$(eval $(call ADD_TEST_TARGET, traits_test, test/traits))

# Import the build system
include $(SOURCE_ROOT)/build/build.mk
