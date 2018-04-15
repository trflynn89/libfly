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
$(eval $(call MOCK_SYSTEM_CALL, accept))
$(eval $(call MOCK_SYSTEM_CALL, bind))
$(eval $(call MOCK_SYSTEM_CALL, connect))
$(eval $(call MOCK_SYSTEM_CALL, fcntl))
$(eval $(call MOCK_SYSTEM_CALL, gethostbyname))
$(eval $(call MOCK_SYSTEM_CALL, getsockopt))
$(eval $(call MOCK_SYSTEM_CALL, listen))
$(eval $(call MOCK_SYSTEM_CALL, recv))
$(eval $(call MOCK_SYSTEM_CALL, recvfrom))
$(eval $(call MOCK_SYSTEM_CALL, send))
$(eval $(call MOCK_SYSTEM_CALL, sendto))
$(eval $(call MOCK_SYSTEM_CALL, setsockopt))
$(eval $(call MOCK_SYSTEM_CALL, socket))
