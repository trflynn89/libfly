# Set of API functions for using the build system. Applications using this build
# system should include this file first.

# Set the default target, if one has not already been defined.
# $(1) = The target's name.
define SET_DEFAULT_TARGET

target ?= $$(strip $(1))

endef

# Function to define a target.
# $(1) = The target's name.
# $(2) = The target's source directory.
# $(3) = The target's type (BIN, QT5, or LIB).
define ADD_TARGET

ifeq ($(target),$$(strip $(1)))
    TARGET_PATH := $$(strip $(2))
    TARGET_TYPE := $$(strip $(3))
endif

endef

# Function to define a unit test target.
# $(1) = The target's name.
# $(2) = The target's source directory.
define ADD_TEST_TARGET

$(call ADD_TARGET, $(1), $(2), BIN)
TEST_TARGETS += $$(strip $(1))

endef
