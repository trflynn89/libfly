# Define build flags for C/C++ files.

# Remove built-in rules
MAKEFLAGS += --no-builtin-rules --no-print-directory
.SUFFIXES:

# Use bash instead of sh
SHELL := /bin/bash

# Linker flags
LDFLAGS := -L$(INSTALL_LIB_DIR)
LDLIBS := -lpthread

ifeq ($(SYSTEM), LINUX)
    LDLIBS += -latomic
endif

# Compiler flags for both C and C++ files
CF_ALL := -MD -MP -fPIC
CF_ALL += -I$(SOURCE_ROOT) -I$(INSTALL_INC_DIR)

ifeq ($(SYSTEM), MACOS)
    CF_ALL += -isysroot $(XCODE)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
endif

# Compiler flags for Maven projects
MVN_FLAGS := -Doutput=$(MVN_DIR)

ifeq ($(arch), x86)
    CF_ALL += -m32
endif

# Error and warning flags
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
    -Winvalid-pch \
    -Wmissing-declarations \
    -Wpointer-arith \
    -Wnon-virtual-dtor \
    -Wold-style-cast \
    -Woverloaded-virtual \
    -Wredundant-decls \
    -Wshadow \
    -Wstrict-overflow=5 \
    -Wundef \
    -Wunreachable-code \
    -Wunused \
    -Wzero-as-null-pointer-constant \
    \
    -pedantic

ifeq ($(mode), debug)
    CF_ALL += \
        -Winline
endif

ifeq ($(toolchain), clang)
    CF_ALL += \
        -Wnewline-eof \
        -Wsign-conversion
else ifeq ($(toolchain), gcc)
    CF_ALL += \
        -Wnull-dereference \
        -Wredundant-decls \
        -Wsign-promo

    ifeq ($(SYSTEM), LINUX)
        CF_ALL += \
            -Wlogical-op \
            -Wsuggest-override

        ifeq ($(mode), debug)
            CF_ALL += \
                -Wsuggest-final-methods \
                -Wsuggest-final-types
        endif
    endif
endif

# Add debug symbols & use address sanitizer for debug builds, optimize release builds, and add
# profiling symbols for profile builds.
ifeq ($(mode), debug)
    CF_ALL += -O0 -g -fsanitize=address -fno-omit-frame-pointer
    MVN_FLAGS += -Dmaven.compiler.debuglevel=lines,vars,source

    ifeq ($(toolchain), clang)
        CF_ALL += -fprofile-instr-generate -fcoverage-mapping
    else ifeq ($(toolchain), gcc)
        CF_ALL += --coverage
    endif
else ifeq ($(mode), release)
    CF_ALL += -O2
    MVN_FLAGS += -Dmaven.compiler.debuglevel=none
else ifeq ($(mode), profile)
    ifeq ($(toolchain), gcc)
        CF_ALL += -O2 -g -pg
        LDFLAGS += -pg
    else
        $(error Profiling not supported with toolchain $(toolchain), check flags.mk)
    endif
endif

# Suppress output on Maven projects
ifneq ($(verbose), 1)
    MVN_FLAGS += -q
endif

# C and C++ specific flags
CFLAGS := -std=c17 $(CF_ALL)
CXXFLAGS := -std=c++17 $(CF_ALL)

# On Linux: Use LLD linker with clang toolchain. Use gold linker with gcc toolchain.
ifeq ($(SYSTEM), LINUX)
    ifeq ($(toolchain), clang)
        LDFLAGS += -fuse-ld=lld
    else ifeq ($(toolchain), gcc)
        LDFLAGS += -fuse-ld=gold
    endif
endif

# On macOS: Link commonly used frameworks.
ifeq ($(SYSTEM), MACOS)
    LDFLAGS += \
        -framework CoreFoundation \
        -framework CoreServices \
        -framework Foundation
endif

# gcov flags
GCOV_FLAGS := -l

# strip flags
ifeq ($(SYSTEM), LINUX)
    STRIP_FLAGS := -s
else ifeq ($(SYSTEM), MACOS)
    STRIP_FLAGS := -rSx
else
    $(error Unrecognized system $(SYSTEM), check flags.mk)
endif

# tar flags
ifeq ($(verbose), 1)
    TAR_EXTRACT_FLAGS := -xjvf
    TAR_CREATE_FLAGS := -cjvf
else
    TAR_EXTRACT_FLAGS := -xjf
    TAR_CREATE_FLAGS := -cjf
endif

ifeq ($(SYSTEM), MACOS)
    TAR_EXTRACT_FLAGS := -mo $(TAR_EXTRACT_FLAGS)
endif
