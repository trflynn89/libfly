# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/config \
    fly/logger \
    fly/parser \
    fly/path \
    fly/socket \
    fly/string \
    fly/system \
    fly/task

# Define libraries to link
LDLIBS_$(d) := -lpthread

# Define source files
$(eval $(call WILDCARD_SOURCES))

# Define mocked system calls
$(eval $(call MOCK_SYSTEM_CALL, fcntl))
$(eval $(call MOCK_SYSTEM_CALL, socket))
