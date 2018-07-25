# Define the path to the source directory
SOURCE_ROOT := $(CURDIR)/../..

# Define the project version
VERSION = $(shell cat $(SOURCE_ROOT)/VERSION.md)

# Import the build API
include api.mk

# Set default target
$(eval $(call SET_DEFAULT_TARGET, libfly))

# Main targets
$(eval $(call ADD_TARGET, libfly, fly, LIB))

# Unit tests
$(eval $(call ADD_TEST_TARGET, config_test, test/config))
$(eval $(call ADD_TEST_TARGET, logger_test, test/logger))
$(eval $(call ADD_TEST_TARGET, parser_test, test/parser))
$(eval $(call ADD_TEST_TARGET, path_test, test/path))
$(eval $(call ADD_TEST_TARGET, socket_test, test/socket))
$(eval $(call ADD_TEST_TARGET, system_test, test/system))
$(eval $(call ADD_TEST_TARGET, task_test, test/task))
$(eval $(call ADD_TEST_TARGET, traits_test, test/traits))
$(eval $(call ADD_TEST_TARGET, types_test, test/types))

# Import the build system
include build.mk
