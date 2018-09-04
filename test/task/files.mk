# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/config \
    fly/logger \
    fly/parser \
    fly/path \
    fly/system \
    fly/task \
    fly/types \
    test/util

# Define libraries to link
LDLIBS_$(d) := \
    -latomic \
    -lpthread

# Define source files
$(eval $(call WILDCARD_SOURCES))
