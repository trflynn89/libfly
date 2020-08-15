# Define the directories to include and compile
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

# Include the directories containing the unit tests
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

# Define source files
SRC_$(d) := \
    $(d)/main.cpp

# Define compiler flags
CXXFLAGS_$(d) += -I$(SOURCE_ROOT)/test/Catch2/single_include
CXXFLAGS_$(d) += -DCATCH_CONFIG_FAST_COMPILE -DCATCH_CONFIG_ENABLE_OPTIONAL_STRINGMAKER

ifeq ($(SYSTEM), LINUX)
    # Define the list of available mocked system calls
    SYSTEM_CALLS_$(d) := \
        ${shell grep -ohP "(?<=__wrap_)[a-zA-Z0-9_]+" "$(d)/mock/nix/mock_calls.cpp"}

    # Define linker flags
    LDFLAGS_$(d) += \
        -static-libstdc++ \
        $(foreach mock, $(SYSTEM_CALLS_$(d)), -Wl,--wrap=$(mock))
endif
