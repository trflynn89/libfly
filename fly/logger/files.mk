SRC_DIRS_$(d) := \
    $(d)/detail \
    $(d)/detail/nix

$(eval $(call WILDCARD_SOURCES, CPP))
