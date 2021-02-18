SRC_DIRS_$(d) := \
    $(d)/socket \
    $(d)/socket/detail \
    $(d)/socket/detail/nix

$(eval $(call WILDCARD_SOURCES, CPP))
