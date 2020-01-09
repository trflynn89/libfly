# Define source files
SRC_$(d) := \
    $(d)/mock_system.cpp \
    $(d)/nix/mock_calls.cpp

# Define compiler flags
CXXFLAGS_$(d) += -Wno-missing-declarations
