# Define the path to the source directory
SOURCE_ROOT := $(CURDIR)/../..

# Define the project version
VERSION := $(shell cat $(SOURCE_ROOT)/VERSION.md)

# Import the build API
include api.mk

# Main targets
$(eval $(call ADD_TARGET, libfly, fly, LIB))

# Test targets
$(eval $(call ADD_TARGET, libfly_unit_tests, test, TEST))
$(eval $(call ADD_TARGET, libfly_qt5_example, test/qt5, QT5))

# Import the build system
include build.mk
