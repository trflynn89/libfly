# Define the list of available mocked system calls
SYSTEM_CALLS_$(d) := \
	${shell grep -ohP "(?<=__wrap_)[a-zA-Z0-9_]+" "$(d)/nix/mock_calls.cpp"}

# Define source files
SRC_$(d) := \
    $(d)/mock_system.cpp \
    $(d)/nix/mock_calls.cpp \
    $(d)/nix/mock_list.cpp

# Define linker flags
LDFLAGS += \
    -static-libstdc++ \
    $(foreach mock, $(SYSTEM_CALLS_$(d)), -Wl,--wrap=$(mock))
