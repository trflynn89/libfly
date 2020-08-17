# Define source files
SRC_$(d) := \
    $(d)/path_config.cpp \
    $(d)/path_monitor.cpp

ifeq ($(SYSTEM), LINUX)
    SRC_$(d) += \
        $(d)/nix/path_monitor_impl.cpp
else ifeq ($(SYSTEM), MACOS)
    SRC_$(d) += \
        $(d)/mac/path_monitor_impl.mm
endif
