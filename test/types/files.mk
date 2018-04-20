# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/string \
    fly/types

# Define libraries to link
LDLIBS_$(d) := -lpthread

# Define source files
$(eval $(call WILDCARD_SOURCES))
