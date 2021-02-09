SRC_DIRS_$(d) := \
    $(d)/nix

SRC_$(d) := \
    $(d)/mock_system.cpp

CXXFLAGS_$(d) += -Wno-missing-declarations
