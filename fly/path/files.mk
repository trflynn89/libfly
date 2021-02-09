ifeq ($(SYSTEM), LINUX)
    SRC_DIRS_$(d) := \
        $(d)/nix
else ifeq ($(SYSTEM), MACOS)
    SRC_DIRS_$(d) := \
        $(d)/mac
endif

$(eval $(call WILDCARD_SOURCES, CPP))
