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
ifeq ($(qt), 1)
    $(eval $(call ADD_TARGET, libfly_qt_example, test/qt, QT))
endif

# Import the build system
include build.mk
