# Define the directories to include and compile
SRC_DIRS_$(d) := \
    some_lib

# Define libraries to link
LDLIBS_$(d) := -lfly -lpthread

# Define source files
SRC_$(d) := \
    $(d)/main.cpp
