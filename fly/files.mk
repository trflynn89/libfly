SRC_DIRS_$(d) := \
    fly/coders \
    fly/coders/base64 \
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

# Add libfly.a and libfly.so to the archived release package.
$(eval $(call ADD_REL_LIB))

# Add all header files to the archived release package.
$(eval $(call ADD_REL_INC, $(d), *.hpp, fly))

# Add build system files to thearchived release package.
$(eval $(call ADD_REL_SRC, $(BUILD_ROOT), *.mk, fly))
