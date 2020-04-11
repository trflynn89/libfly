# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/coders \
    fly/coders/huffman \
    fly/config \
    fly/logger \
    fly/parser \
    fly/path \
    fly/socket \
    fly/system \
    fly/task \
    fly/types/bit_stream \
    fly/types/bit_stream/detail \
    fly/types/json

# Add libfly.so to release package
$(eval $(call ADD_REL_LIB, libfly))

# Add all header files to release package
$(eval $(call ADD_REL_INC, libfly, $(d), *.hpp, fly))

# Add make system files to release package
$(eval $(call ADD_REL_SRC, libfly, $(BUILD_ROOT), *.mk, fly))
$(eval $(call ADD_REL_SRC, libfly, $(BUILD_ROOT)/../ci, *.js, fly))
