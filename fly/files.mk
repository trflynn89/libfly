SRC_DIRS_$(d) := \
    fly/coders \
    fly/coders/base64 \
    fly/coders/huffman \
    fly/config \
    fly/logger \
    fly/logger/detail \
    fly/logger/detail/nix \
    fly/parser \
    fly/path \
    fly/socket \
    fly/socket/nix \
    fly/system \
    fly/task \
    fly/types/bit_stream \
    fly/types/bit_stream/detail \
    fly/types/json

ifeq ($(SYSTEM), LINUX)
    SRC_DIRS_$(d) += \
        fly/path/nix \
        fly/system/nix
else ifeq ($(SYSTEM), MACOS)
    SRC_DIRS_$(d) += \
        fly/path/mac \
        fly/system/mac
endif

# Add libfly.a and libfly.so to the archived release package.
$(eval $(call ADD_REL_LIB))

# Add all header files to the archived release package.
$(eval $(call ADD_REL_INC, $(d), *.hpp, fly))
