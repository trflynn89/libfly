# Define source files
SRC_$(d) := \
    $(d)/system.cpp \
    $(d)/system_config.cpp \
    $(d)/system_monitor.cpp \
    $(d)/nix/system_impl.cpp

ifeq ($(SYSTEM), LINUX)
    SRC_$(d) += \
        $(d)/nix/system_monitor_impl.cpp
else ifeq ($(SYSTEM), MACOS)
    SRC_$(d) += \
        $(d)/mac/system_monitor_impl.cpp
endif
