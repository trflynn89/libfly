# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/config \
    fly/logger \
    fly/parser \
    fly/path \
    fly/socket \
    fly/system \
    fly/task \
    fly/types \
    test/mock \
    test/util

# Include the directories containing the unit tests
SRC_DIRS_$(d) += \
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
    $(d)/googletest/googletest/src/gtest-all.cc \
    $(d)/googletest/googletest/src/gtest_main.cc

# Define the list of available mocked system calls
SYSTEM_CALLS_$(d) := \
    ${shell grep -ohP "(?<=__wrap_)[a-zA-Z0-9_]+" "$(d)/mock/nix/mock_calls.cpp"}

# Define linker flags
LDFLAGS_$(d) += \
    -static-libstdc++ \
    $(foreach mock, $(SYSTEM_CALLS_$(d)), -Wl,--wrap=$(mock))

# Define libraries to link
LDLIBS_$(d) := \
    -latomic \
    -lpthread \
    -lstdc++fs
