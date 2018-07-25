# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/config \
    fly/logger \
    fly/parser \
    fly/path \
    fly/socket \
    fly/system \
    fly/task \
    fly/types

# Add libfly.so to release package
$(eval $(call ADD_REL_LIB, $(TARGET_NAME)))

# Add all header files to release package
$(eval $(call ADD_REL_INC, $(d), *.h))

# Add make system files to release package
$(eval $(call ADD_REL_SRC, $(BUILD_ROOT), *.mk))
$(eval $(call ADD_REL_CMD, mv $(REL_SRC_DIR)/$(notdir $(BUILD_ROOT)) $(REL_SRC_DIR)/fly))
