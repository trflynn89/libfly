include $(SOURCE_ROOT)/extern/catchorg/flags.mk

SRC_DIRS_$(d) += \
    test/coders \
    test/config \
    test/logger \
    test/parser \
    test/path \
    test/socket \
    test/system \
    test/task \
    test/traits \
    test/types/bit_stream \
    test/types/concurrency \
    test/types/json \
    test/types/numeric \
    test/types/string \
    test/util

ifeq ($(SYSTEM), LINUX)
    SRC_DIRS_$(d) += \
        test/mock
endif

SRC_$(d) := \
    $(d)/fly.cpp \
    $(d)/main.cpp

# On Linux, define the list of available mocked system calls.
ifeq ($(SYSTEM), LINUX)
    SYSTEM_CALLS_$(d) := \
        ${shell grep -ohP "(?<=__wrap_)[a-zA-Z0-9_]+" "$(d)/mock/nix/mock_calls.cpp"}

    LDFLAGS_$(d) += \
        -static-libstdc++ \
        $(foreach mock, $(SYSTEM_CALLS_$(d)), -Wl,--wrap=$(mock))
endif
