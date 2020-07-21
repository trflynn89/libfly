# Set of API functions for using the build system. Applications using this build
# system should include this file first.

# Verify expected variables
ifeq ($(SOURCE_ROOT),)
    $(error SOURCE_ROOT must be defined)
else ifeq ($(wildcard $(SOURCE_ROOT)/.*),)
    $(error SOURCE_ROOT $(SOURCE_ROOT) does not exist)
endif

ifeq ($(VERSION),)
    $(error VERSION must be defined)
else ifneq ($(words $(subst ., ,$(VERSION))), 3)
    $(error VERSION $(VERSION) must be of the form X.Y.Z)
endif

# List of all target names
TARGETS :=

# List of all test target names
TEST_TARGETS :=

# List of paths to ignore during code coverage reporting
COVERAGE_BLACKLIST :=

# Define a target.
#
# $(1) = The target's name.
# $(2) = The target's source directory.
# $(3) = The target's type (BIN, LIB, MVN, or TEST).
define ADD_TARGET

TARGETS += $(1)
TARGET_PATH_$$(strip $(1)) := $(2)

ifeq ($(strip $(3)), TEST)
    TEST_TARGETS += $(1)
    TARGET_TYPE_$$(strip $(1)) := BIN
else
    TARGET_TYPE_$$(strip $(1)) := $(3)
endif

endef

# Add a path the the code coverage blacklist.
#
# $(1) = The path to ignore.
define IGNORE_FOR_COVERAGE

ifeq ($$(wildcard $(SOURCE_ROOT)/$$(strip $(1))),)
    $$(error Could not find path $$(strip $(1)), check your Makefile)
endif

COVERAGE_BLACKLIST += $(1)

endef
