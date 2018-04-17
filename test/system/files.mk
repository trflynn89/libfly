# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/config \
    fly/logger \
    fly/parser \
    fly/path \
    fly/string \
    fly/system \
    fly/task \
    fly/types

# Define linker flags
LDFLAGS_$(d) += -static-libstdc++

# Define libraries to link
LDLIBS_$(d) := -lpthread

# Define source files
$(eval $(call WILDCARD_SOURCES))

# Define mocked system calls
$(eval $(call MOCK_SYSTEM_CALL, read))
$(eval $(call MOCK_SYSTEM_CALL, sysinfo))
$(eval $(call MOCK_SYSTEM_CALL, times))
