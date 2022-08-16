ifeq ($(SYSTEM), LINUX)
    SRC_DIRS_$(d) := \
        $(d)/nix
else ifeq ($(SYSTEM), MACOS)
    SRC_DIRS_$(d) := \
        $(d)/mac
endif

SRC_$(d) := \
    $(d)/path_config.cpp \
    $(d)/path_monitor.cpp
