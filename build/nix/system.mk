# Define system dependent variables.

SYSTEM := $(shell gcc -dumpmachine)

# Determine default architecture
SUPPORTED_ARCH := x86_64 i386

ifneq ($(findstring x86_64, $(SYSTEM)),)
    arch := x86_64
else ifeq ($(arch),x86_64)
    $(error Cannot build 64-bit architecture on 32-bit machine)
else
    arch := i386
endif

ifneq ($(arch), $(filter $(SUPPORTED_ARCH), $(arch)))
    $(error Architecture $(arch) not supported, check system.mk)
endif

# Determine number of cores
NUM_CORES := $(shell grep -c ^processor /proc/cpuinfo)

# Determine installation directories
INSTALL_BIN_DIR := /usr/bin
INSTALL_INC_DIR := /usr/include
INSTALL_SRC_DIR := /usr/src

ifeq ($(arch), x86_64)
    INSTALL_LIB_DIR := /usr/lib64
else ifeq ($(arch), i386)
    INSTALL_LIB_DIR := /usr/lib
endif
