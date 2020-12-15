SRC_DIRS_$(d) := \
    fly/coders \
    fly/coders/huffman \
    fly/logger \
    fly/parser \
    fly/system \
    fly/task \
    fly/types/bit_stream \
    fly/types/bit_stream/detail \
    fly/types/json \
    fly/types/string \
    test/util

# Include the directories containing the benchmark tests.
SRC_DIRS_$(d) += \
    bench/json

SRC_$(d) := \
    $(d)/main.cpp \
    $(d)/stream_util.cpp

CXXFLAGS_$(d) += -I$(SOURCE_ROOT)/test/Catch2/single_include
CXXFLAGS_$(d) += \
    -DCATCH_CONFIG_PREFIX_ALL \
    -DCATCH_CONFIG_FAST_COMPILE \
    -DCATCH_CONFIG_ENABLE_OPTIONAL_STRINGMAKER
