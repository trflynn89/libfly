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
    fly/types/json \
    fly/types/string \
    test/util

ifeq ($(SYSTEM), LINUX)
    SRC_DIRS_$(d) += \
        test/mock
endif

# Include the directories containing the unit tests.
SRC_DIRS_$(d) += \
    test/coders \
    test/config \
    test/logger \
    test/parser \
    test/path \
    test/socket \
    test/system \
    test/task \
    test/traits \
    test/types

SRC_$(d) := \
    $(d)/main.cpp

# All unit tests should have Catch2 on the include path.
CXXFLAGS_$(d) += -I$(SOURCE_ROOT)/test/Catch2/single_include
CXXFLAGS_$(d) += -DCATCH_CONFIG_FAST_COMPILE -DCATCH_CONFIG_ENABLE_OPTIONAL_STRINGMAKER

# On Linux, define the list of available mocked system calls.
ifeq ($(SYSTEM), LINUX)
    SYSTEM_CALLS_$(d) := \
        ${shell grep -ohP "(?<=__wrap_)[a-zA-Z0-9_]+" "$(d)/mock/nix/mock_calls.cpp"}

    LDFLAGS_$(d) += \
        -static-libstdc++ \
        $(foreach mock, $(SYSTEM_CALLS_$(d)), -Wl,--wrap=$(mock))
endif
