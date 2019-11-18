# Set of API functions for using the build system. Applications using this build
# system should include this file first.

# List of all target names
TARGETS :=

# List of all test target names
TEST_TARGETS :=

# Function to define a target.
#
# $(1) = The target's name.
# $(2) = The target's source directory.
# $(3) = The target's type (BIN, LIB, or TEST).
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
