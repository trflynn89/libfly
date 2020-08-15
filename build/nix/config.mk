# Define the default make configuration. Not all defaults are defined here, but
# all command line options are listed here for convenience.

# Compilation toolchain (clang, gcc)
toolchain := clang

# Compilation mode (debug, release, profile)
mode := debug

# Build 32-bit or 64-bit target
arch := $(arch)

# Compile caching system
cacher :=

# Enable stylized builds
stylized := 1

# Enable verbose builds
verbose := 0

# Define the toolchain binaries
ifeq ($(toolchain), clang)
    ifeq ($(SYSTEM), LINUX)
        CC := clang
        CXX := clang++
        AR := llvm-ar
        STRIP := llvm-strip
    else ifeq ($(SYSTEM), MACOS)
        TOOLCHAIN := $(XCODE)/Toolchains/XcodeDefault.xctoolchain/usr/bin

        CC := $(TOOLCHAIN)/clang
        CXX := $(TOOLCHAIN)/clang++
        AR := $(TOOLCHAIN)/ar
        STRIP := $(TOOLCHAIN)/strip
    else
        $(error Unrecognized system $(SYSTEM), check config.mk)
    endif
else ifeq ($(toolchain), gcc)
    CC := gcc
    CXX := g++
    AR := ar
    STRIP := strip
else
    $(error Unrecognized toolchain $(toolchain), check config.mk)
endif

MVN := mvn

# Validate the provided compilation mode.
SUPPORTED_MODES := debug release profile

ifneq ($(mode), $(filter $(SUPPORTED_MODES), $(mode)))
    $(error Compilation mode $(mode) not supported, check config.mk)
endif

# Use a compiler cache if requested
ifneq ($(cacher), )
    CC := $(cacher) $(CC)
    CXX := $(cacher) $(CXX)
endif

# Define the output directories
OUT_DIR := $(CURDIR)/$(mode)

CPP_DIR := $(OUT_DIR)/$(toolchain)/$(arch)
MVN_DIR := $(OUT_DIR)/$(MVN)

BIN_DIR := $(CPP_DIR)/bin
LIB_DIR := $(CPP_DIR)/lib
OBJ_DIR := $(CPP_DIR)/obj
ETC_DIR := $(CPP_DIR)/etc

# ANSI escape sequences to use in stylized builds.
ifeq ($(stylized), 1)
    DEFAULT := \x1b[0m
    BLACK := \x1b[1;30m
    RED := \x1b[1;31m
    GREEN := \x1b[1;32m
    YELLOW := \x1b[1;33m
    BLUE := \x1b[1;34m
    MAGENTA := \x1b[1;35m
    CYAN := \x1b[1;36m
    WHITE := \x1b[1;37m
endif

# Use @ suppression in non-verbose builds
ifeq ($(verbose), 0)
    Q := @
else
    $(info Bin dir = $(BIN_DIR))
    $(info Lib dir = $(LIB_DIR))
    $(info Obj dir = $(OBJ_DIR))
    $(info Etc dir = $(ETC_DIR))
    $(info Mvn dir = $(MVN_DIR))
    $(info Toolchain = $(toolchain))
    $(info Mode = $(mode))
    $(info Arch = $(arch))
    $(info Cacher = $(cacher))
endif
