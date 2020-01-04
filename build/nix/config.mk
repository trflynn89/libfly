# Define the default make configuration. Not all defaults are defined here, but
# all command line options are listed here for convenience.

# Toolchain to compile with
toolchain := clang

# Compile caching system
cacher :=

# Define debug vs. release
release := 0

# Build 32-bit or 64-bit target
arch := $(arch)

# Enable verbose builds
verbose := 0

# Define the toolchain binaries
ifeq ($(toolchain), clang)
    CC := clang
    CXX := clang++
    AR := llvm-ar
    STRIP := llvm-strip
else ifeq ($(toolchain), gcc)
    CC := gcc
    CXX := g++
    AR := ar
    STRIP := strip
else
    $(error Unrecognized toolchain $(toolchain), check config.mk)
endif

# Use a compiler cache if requested
ifneq ($(cacher), )
    CC := $(cacher) $(CC)
    CXX := $(cacher) $(CXX)
endif

# Define the output directories
ifeq ($(release), 1)
    OUT_DIR := $(CURDIR)/release/$(toolchain)/$(arch)
else
    OUT_DIR := $(CURDIR)/debug/$(toolchain)/$(arch)
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
    $(info Toolchain = $(toolchain))
    $(info Cacher = $(cacher))
    $(info Release = $(release))
    $(info Arch = $(arch))
endif
