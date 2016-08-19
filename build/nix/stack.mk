# Define functions to modify the directory stack. The stack works by maintaining
# four variables:
#
# sp = The stack pointer
# pd = The previous directory
# d  = The current directory relative to the build directory
# td = The current directory relative to the top-level directory
#
# Each time a directory is entered, the stack pointer is "incremented" by
# appending ".x" to the current value of the pointer. So, a value of ".x.x.x"
# would indicate we are 3 levels deep in the stack.
#
# The previous directory is stored on a per stack-level basis. This is because
# we need to be able to include subdirectories without the current directory
# forgetting the previous directory.
#
# The current directory is defined when it is included with INCLUDE_*_DIR.

# Include the build rules for the given binary target directory.
# $(1) = The path to the directory to include.
# $(2) = The directory to include.
define INCLUDE_BIN_DIR

curr_dir := $$(strip $(1))/$$(strip $(2))
$(call DEFINE_BIN_RULES)

endef

# Include the build rules for the given library target directory.
# $(1) = The path to the directory to include.
# $(2) = The directory to include.
define INCLUDE_LIB_DIR

curr_dir := $$(strip $(1))/$$(strip $(2))
$(call DEFINE_LIB_RULES)

endef

# Include the build rules for the given target source directory.
# $(1) = The path to the directory to include.
# $(2) = The directory to include.
define INCLUDE_SRC_DIR

curr_dir := $$(strip $(1))/$$(strip $(2))
$(call DEFINE_SRC_RULES)

endef

# Include the build rules for each of the given target source directories.
# $(1) = The directories to include.
define INCLUDE_SRC_DIRS

$(foreach dir, $(1), $(call INCLUDE_SRC_DIR, $(SOURCE_ROOT), $(dir)))

endef

# Push the current directory to the stack.
define PUSH_DIR

sp := $$(sp).x
pd_$$(sp) := $$(d)

d := $$(curr_dir)
td := $$(subst $(SOURCE_ROOT)/,,$$(d))

endef

# Pop a directory from the stack.
define POP_DIR

d := $$(pd_$$(sp))
td := $$(subst $(SOURCE_ROOT)/,,$$(d))

sp := $$(basename $$(sp))

endef
