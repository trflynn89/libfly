# Define flags to pass to the various compilation tools for the current build configuration and
# host system environment.
#
# The following flags are defined:
#
#     CFLAGS = Compiler flags for C and Objective-C files.
#     CXXFLAGS = Compiler flags for C++ and Objective-C++ files.
#     LDFLAGS = Linker flags for C-family targets.
#     LDLIBS = Linker libraries required by the STL for C-family targets.
#
#     JFLAGS = Compiler flags for Java files.
#
#     STRIP_FLAGS = Flags to be used when stripping symbols from a target.
#     JAR_CREATE_FLAGS = Flags to be used when creating a JAR archive
#     TAR_EXTRACT_FLAGS = Flags to be used when extracting a tar archive.
#     TAR_CREATE_FLAGS = Flags to be used when creating a tar archive
#     ZIP_EXTRACT_FLAGS = Flags to be used when extracting a zip archive.
#
# The application may define the following variables in any files.mk to define compiler/linker flags
# on a per-directory level. These variables are defaulted to the values of the parent directory, so
# generally these variables should be treated as append-only (+=). But this behavior may be avoided
# by assigning values instead (:=).
#
#     CFLAGS_$(d)
#     CXXFLAGS_$(d)
#     LDFLAGS_$(d)
#     LDLIBS_$(d)
#     JFLAGS_$(d)
#
# The resulting flags used when compiling a directory are the global flags defined in this file
# followed by any  _$(d) variants defined in the directory's files.mk.

# Remove built-in rules.
MAKEFLAGS += --no-builtin-rules --no-print-directory
.SUFFIXES:

# Use bash instead of sh.
SHELL := /bin/bash

# Linker flags.
LDFLAGS := -L$(INSTALL_LIB_DIR)
LDLIBS := -lpthread

ifeq ($(SYSTEM), LINUX)
    LDLIBS += -latomic
endif

# Compiler flags for all C-family files.
CF_ALL := -MD -MP -fPIC
CF_ALL += -I$(SOURCE_ROOT) -I$(INSTALL_INC_DIR)

ifeq ($(SYSTEM), MACOS)
    CF_ALL += -isysroot $(XCODE)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
endif

# Compiler flags for Java files.
JFLAGS :=

ifeq ($(arch), x86)
    CF_ALL += -m32
endif

# Error and warning flags.
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

JFLAGS += \
    -Werror \
    -Xlint

# Add debug symbols & use address sanitizer for debug builds, optimize release builds, and add
# profiling symbols for profile builds.
ifeq ($(mode), debug)
    CF_ALL += -O0 -g -fsanitize=address -fno-omit-frame-pointer
    JFLAGS += -g:lines,vars,source

    ifeq ($(toolchain), clang)
        CF_ALL += -fprofile-instr-generate -fcoverage-mapping
    else ifeq ($(toolchain), gcc)
        CF_ALL += --coverage
    endif
else ifeq ($(mode), release)
    CF_ALL += -O2 -DNDEBUG
    JFLAGS += -g:none
else ifeq ($(mode), profile)
    ifeq ($(toolchain), gcc)
        CF_ALL += -O2 -g -pg
        LDFLAGS += -pg
    else
        $(error Profiling not supported with toolchain $(toolchain), check flags.mk)
    endif
endif

# Enable verbose Java output.
ifeq ($(verbose), 1)
    JFLAGS += -verbose
endif

# C and C++ specific flags.
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

# strip flags.
ifeq ($(SYSTEM), LINUX)
    STRIP_FLAGS := -s
else ifeq ($(SYSTEM), MACOS)
    STRIP_FLAGS := -rSx
else
    $(error Unrecognized system $(SYSTEM), check flags.mk)
endif

# jar flags.
ifeq ($(verbose), 1)
    JAR_CREATE_FLAGS := cvef
else
    JAR_CREATE_FLAGS := cef
endif

# tar flags.
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

# zip flags.
ifeq ($(verbose), 0)
    ZIP_EXTRACT_FLAGS := -q
endif
