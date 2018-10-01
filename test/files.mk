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

# Define libraries to link
LDLIBS_$(d) := \
    -latomic \
    -lpthread
