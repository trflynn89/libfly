include $(SOURCE_ROOT)/extern/catchorg/flags.mk

SRC_DIRS_$(d) += \
    $(d)/coders \
    $(d)/config \
    $(d)/logger \
    $(d)/net \
    $(d)/parser \
    $(d)/path \
    $(d)/socket \
    $(d)/system \
    $(d)/task \
    $(d)/traits \
    $(d)/types \
    $(d)/util

ifeq ($(SYSTEM), LINUX)
    SRC_DIRS_$(d) += \
        $(d)/mock
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
