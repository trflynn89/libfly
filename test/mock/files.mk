SRC_$(d) := \
    $(d)/mock_system.cpp \
    $(d)/nix/mock_calls.cpp

CXXFLAGS_$(d) += -Wno-missing-declarations
