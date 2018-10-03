# Define build flags for C/C++ files.

# Remove built-in rules
MAKEFLAGS += --no-builtin-rules --no-print-directory
.SUFFIXES:

# Use bash instead of sh
SHELL := /bin/bash

# Linker flags
LDFLAGS := -L$(LIB_DIR) -fuse-ld=gold
LDLIBS :=

# Compiler flags for both C and C++ files
CF_ALL := -MMD -MP
CF_ALL += -Wall -Wextra -Werror
CF_ALL += -I$(SOURCE_ROOT) -I$(GEN_DIR)

ifeq ($(arch), x86)
    CF_ALL += -m32
endif

# Unit tests include Google test
CF_ALL += -isystem $(SOURCE_ROOT)/test/googletest/googletest/include
CF_ALL += -I$(SOURCE_ROOT)/test/googletest/googletest

# Optimize release builds, and add debug symbols / use address sanitizer for
# debug builds - but only use address sanitizer on 64-bit builds:
# https://github.com/google/sanitizers/issues/954
ifeq ($(release), 1)
    CF_ALL += -O2 -fPIC
else
    CF_ALL += -O0 -g --coverage

    ifeq ($(arch), x64)
        CF_ALL += -DFLY_USE_SANITIZER -fsanitize=address -fno-omit-frame-pointer
    endif
endif

# Qt5 flags
QT5_DIR := /opt/Qt/5.7/gcc_64
QT5_BIN := $(QT5_DIR)/bin
QT5_INC := $(QT5_DIR)/include
QT5_LIB := $(QT5_DIR)/lib

QT5_UIC := $(QT5_BIN)/uic
QT5_MOC := $(QT5_BIN)/moc
QT5_RCC := $(QT5_BIN)/rcc

# TODO this won't work anymore, TARGET_TYPE doesn't exist
ifeq ($(TARGET_TYPE), QT5)
    CF_ALL += -fPIC -I$(QT5_INC) -I$(QT5_INC)/QtWidgets -I$(QT5_INC)/QtGui -I$(QT5_INC)/QtCore
    LDFLAGS += -Wl,-rpath,$(QT5_LIB) -L$(QT5_LIB)
    LDLIBS += -lQt5Widgets -lQt5Gui -lQt5Core -lGL
endif

# C and C++ specific flags
CFLAGS := -std=c14 $(CF_ALL)
CXXFLAGS := -std=c++14 $(CF_ALL)

# gcov flags
GCOV_FLAGS := -l

# tar flags
ifeq ($(verbose), 1)
    TAR_EXTRACT_FLAGS := -xjvf
    TAR_CREATE_FLAGS := -cjvf
else
    TAR_EXTRACT_FLAGS := -xjf
    TAR_CREATE_FLAGS := -cjf
endif
