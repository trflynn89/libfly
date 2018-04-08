# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/config \
    fly/logger \
    fly/parser \
    fly/path \
    fly/string \
    fly/system \
    fly/task

# Define libraries to link
LDLIBS_$(d) := -lpthread

# Define source files
$(eval $(call WILDCARD_SOURCES))

# Define mocked system calls
$(eval $(call MOCK_SYSTEM_CALL, fts_read))
$(eval $(call MOCK_SYSTEM_CALL, inotify_add_watch))
$(eval $(call MOCK_SYSTEM_CALL, inotify_init1))
$(eval $(call MOCK_SYSTEM_CALL, getenv))
$(eval $(call MOCK_SYSTEM_CALL, poll))
$(eval $(call MOCK_SYSTEM_CALL, read))
$(eval $(call MOCK_SYSTEM_CALL, remove))
