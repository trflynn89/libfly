# Define system dependent variables.

SYSTEM := $(shell uname -m)
SUDO := $(shell which sudo)

# Determine default architecture
SUPPORTED_ARCH := x64 x86

ifneq ($(findstring x86_64, $(SYSTEM)),)
    arch := x64
else ifeq ($(arch), x64)
    $(error Cannot build 64-bit architecture on 32-bit machine)
else
    arch := x86
endif

ifneq ($(arch), $(filter $(SUPPORTED_ARCH), $(arch)))
    $(error Architecture $(arch) not supported, check system.mk)
endif

# Determine installation directories
INSTALL_BIN_DIR := /usr/bin
INSTALL_INC_DIR := /usr/include
INSTALL_SRC_DIR := /usr/src
INSTALL_LIB_DIR := /usr/lib

# Determine host operating system
ifneq ($(wildcard /etc/debian_version),)
    HOST := DEBIAN
else ifneq ($(wildcard /etc/redhat-release),)
    HOST := REDHAT
else
    $(error Could not determine host operating system, check system.mk)
endif
