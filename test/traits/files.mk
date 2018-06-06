# Define the directories to include and compile
SRC_DIRS_$(d) :=

# Define libraries to link
LDLIBS_$(d) := \
    -latomic \
    -lpthread

# Define source files
$(eval $(call WILDCARD_SOURCES))
