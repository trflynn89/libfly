# Define build flags for C/C++ files.

# Remove built-in rules
MAKEFLAGS += --no-builtin-rules --no-print-directory
.SUFFIXES:

# Use bash instead of sh
SHELL := /bin/bash

# Compiler flags for both C and C++ files
CF_ALL := -MMD -MP
CF_ALL += -Wall -Wextra
CF_ALL += -I$(SOURCE_ROOT)

ifeq ($(arch), i386)
    CF_ALL += -m32
endif

# Unit tests include Google test
ifneq ($(findstring $(target),$(TEST_TARGETS)),)
    CF_ALL += -isystem $(SOURCE_ROOT)/test/googletest/include
    CF_ALL += -I$(SOURCE_ROOT)/test/googletest
endif

# Optimize release builds and add GDB symbols to debug builds
ifeq ($(release), 1)
    CF_ALL += -O2 -Werror

    ifeq ($(TARGET_TYPE), LIB)
        CF_ALL += -fPIC
    endif
else
    CF_ALL += -g
endif

# C and C++ specific flags
CFLAGS := -std=c14 $(CF_ALL)
CXXFLAGS := -std=c++14 $(CF_ALL)

# Linker flags
LDFLAGS := -L$(LIB_DIR)
LDLIBS :=

# Tar flags
ifeq ($(verbose), 1)
    TAR_EXTRACT_FLAGS := -xjvf
    TAR_CREATE_FLAGS := -cjvf
else
    TAR_EXTRACT_FLAGS := -xjf
    TAR_CREATE_FLAGS := -cjf
endif
