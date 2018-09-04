# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/config \
    fly/logger \
    fly/parser \
    fly/path \
    fly/system \
    fly/task \
    fly/types

# Define linker flags
LDFLAGS_$(d) += -static-libstdc++

# Define libraries to link
LDLIBS_$(d) := \
    -latomic \
    -lpthread

# Define source files
$(eval $(call WILDCARD_SOURCES))

SRC_$(d) += \
    $(d)/../util/capture_stream.cpp \
    $(d)/../util/path_util.cpp \
    $(d)/../util/waitable_task_runner.cpp

# Define mocked system calls
$(eval $(call MOCK_SYSTEM_CALL, read))
$(eval $(call MOCK_SYSTEM_CALL, sysinfo))
$(eval $(call MOCK_SYSTEM_CALL, times))
