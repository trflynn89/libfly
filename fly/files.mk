# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/coders \
    fly/config \
    fly/logger \
    fly/parser \
    fly/path \
    fly/sanitizer \
    fly/socket \
    fly/system \
    fly/task \
    fly/types

# Add libfly.so to release package
$(eval $(call ADD_REL_LIB, libfly))

# Add all header files to release package
$(eval $(call ADD_REL_INC, libfly, $(d), *.h, fly))

# Add make system files to release package
$(eval $(call ADD_REL_SRC, libfly, $(BUILD_ROOT), *.mk, fly))
