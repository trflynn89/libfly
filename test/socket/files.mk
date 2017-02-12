# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/config \
    fly/file \
    fly/logger \
    fly/parser \
    fly/socket \
    fly/string \
    fly/system \
    fly/task \
    test

# Define libraries to link
LDLIBS_$(d) := -lpthread

# Define source files
$(eval $(call WILDCARD_SOURCES))
