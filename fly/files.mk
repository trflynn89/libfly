# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/config \
    fly/file \
    fly/logging \
    fly/socket \
    fly/string \
    fly/system \
    fly/task

# Set the release configuration spec to use
$(eval $(call SET_REL_CONF, $(SOURCE_ROOT)/libfly.spec))

# Add libfly.so to RPM
$(eval $(call ADD_REL_LIB, $(TARGET_NAME)))

# Add all header files to RPM
$(eval $(call ADD_REL_INC, $(SOURCE_ROOT)/fly, *.h))

# Add make system file to RPM
$(eval $(call ADD_REL_SRC, $(BUILD_ROOT), *.mk))
$(eval $(call ADD_REL_CMD, mv $(REL_SRC_DIR)/$(notdir $(BUILD_ROOT)) $(REL_SRC_DIR)/fly))
