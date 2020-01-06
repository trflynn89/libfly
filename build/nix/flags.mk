# Define build flags for C/C++ files.

# Remove built-in rules
MAKEFLAGS += --no-builtin-rules --no-print-directory
.SUFFIXES:

# Use bash instead of sh
SHELL := /bin/bash

# Linker flags
LDFLAGS := -L$(LIB_DIR)
LDLIBS :=

# Compiler flags for both C and C++ files
CF_ALL := -MMD -MP -fPIC
CF_ALL += -I$(SOURCE_ROOT) -I$(GEN_DIR)

ifeq ($(arch), x86)
    CF_ALL += -m32
endif

# Warning flags
CF_ALL += \
    -Wall \
    -Wextra \
    \
    -Werror \
    \
    -Wcast-align \
    -Wcast-qual \
    -Wctor-dtor-privacy \
    -Wdisabled-optimization \
    -Wfloat-equal \
    -Winline \
    -Winvalid-pch \
    -Wmissing-declarations \
    -Wpointer-arith \
    -Wnon-virtual-dtor \
    -Wold-style-cast \
    -Woverloaded-virtual \
    -Wredundant-decls \
    -Wshadow \
    -Wsign-conversion \
    -Wstrict-overflow=5 \
    -Wundef \
    -Wunreachable-code \
    -Wunused \
    -Wzero-as-null-pointer-constant \
    \
    -pedantic

ifeq ($(toolchain), clang)
    CF_ALL += \
        -Wnewline-eof
else ifeq ($(toolchain), gcc)
    CF_ALL += \
        -Wlogical-op \
        -Wnoexcept \
        -Wredundant-decls \
        -Wsign-promo \
        -Wsuggest-final-methods \
        -Wsuggest-final-types \
        -Wsuggest-override
endif

# Optimize release builds, and add debug symbols / use address sanitizer for
# debug builds
ifeq ($(release), 1)
    CF_ALL += -O2
else
    CF_ALL += -O0 -g -fsanitize=address -fno-omit-frame-pointer

    ifeq ($(toolchain), clang)
        CF_ALL += -fprofile-instr-generate -fcoverage-mapping
    else ifeq ($(toolchain), gcc)
        CF_ALL += --coverage
    endif
endif

# C and C++ specific flags
CFLAGS := -std=c17 $(CF_ALL)
CXXFLAGS := -std=c++17 $(CF_ALL)

# Use LLD linker with clang toolchain. Use gold linker with gcc toolchain.
ifeq ($(toolchain), clang)
    LDFLAGS += -fuse-ld=lld
else ifeq ($(toolchain), gcc)
    LDFLAGS += -fuse-ld=gold
endif

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
