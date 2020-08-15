# Define the directories to include and compile
SRC_DIRS_$(d) := \
    cpp/some_lib

# Define libraries to link
LDLIBS_$(d) := \
    -lfly

# Define source files
SRC_$(d) := \
    $(d)/some_lib.cpp
