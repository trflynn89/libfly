# Define the default make configuration. Not all defaults are defined here, but
# all command line options are listed here for convenience.

# Define debug vs. release
release := 0

# Build 32-bit or 64-bit target
arch := $(arch)

# Enable verbose builds
verbose := 0

# Arguments to give to target when run
args :=

# Define the output directories
ifeq ($(release), 1)
    OUT_DIR := $(CURDIR)/release-$(arch)
else
    OUT_DIR := $(CURDIR)/debug-$(arch)
endif

BIN_DIR := $(OUT_DIR)/bin
LIB_DIR := $(OUT_DIR)/lib
GEN_DIR := $(OUT_DIR)/gen
OBJ_DIR := $(OUT_DIR)/obj
ETC_DIR := $(OUT_DIR)/etc

# Use @ suppression in non-verbose builds
ifeq ($(verbose),0)
    Q := @
else
    $(info Bin dir = $(BIN_DIR))
    $(info Lib dir = $(LIB_DIR))
    $(info Gen dir = $(GEN_DIR))
    $(info Obj dir = $(OBJ_DIR))
    $(info Etc dir = $(ETC_DIR))
    $(info Release = $(release))
    $(info Arch = $(arch))
endif
