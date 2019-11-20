# Define compilation functions for each object and binary target. Every source
# directory should contain a files.mk file. Every binary target directory must
# also contain a files.mk file, and include the source directorises it depends
# upon. What each of the files.mk files must define is described below.

# Build tools
COMP_CC := $(Q)$(CC) $$(CFLAGS) -o $$@ -c $$<
LINK_CC := $(Q)$(CC) $$(CFLAGS) -o $$@ $$(OBJS) $$(LDFLAGS) $$(LDLIBS)

COMP_CXX := $(Q)$(CXX) $$(CXXFLAGS) -o $$@ -c $$<
LINK_CXX := $(Q)$(CXX) $$(CXXFLAGS) -o $$@ $$(OBJS) $$(LDFLAGS) $$(LDLIBS)

SHARED_CC := $(Q)$(CC) $$(CFLAGS) -shared -Wl,-soname,$$(@F) -o $$@ $$(OBJS) $$(LDFLAGS)
SHARED_CXX := $(Q)$(CXX) $$(CXXFLAGS) -shared -Wl,-soname,$$(@F) -o $$@ $$(OBJS) $$(LDFLAGS)

UIC := $(Q)$(QT_UIC) $$< -o $$@
MOC := $(Q)$(QT_MOC) $$< -o $$@
RCC := $(Q)$(QT_RCC) $$< -o $$@

STATIC := $(Q)$(AR) rcs $$@ $$(OBJS)
STRIP := $(Q)strip $$@

# Link a binary target from a set of object files.
#
# $(1) = The target's name.
# $(2) = The path to the target output binary.
define BIN_RULES

t := $$(strip $(1))

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(wildcard $(d)/*.mk)

$(2): OBJS := $$(OBJ_$$(t))
$(2): CFLAGS := $(CFLAGS_$(d)) $(CFLAGS)
$(2): CXXFLAGS := $(CXXFLAGS_$(d)) $(CXXFLAGS)
$(2): LDFLAGS := $(LDFLAGS_$(d)) $(LDFLAGS)
$(2): LDLIBS := $(LDLIBS_$(d)) $(LDLIBS)

$(2): $$(OBJ_$$(t)) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)

	@echo "[Link $$(subst $(CURDIR)/,,$$@)]"
	$(LINK_CXX)

ifeq ($(release),1)
	@echo "[Strip $$(subst $(CURDIR)/,,$$@)]"
	$(STRIP)
endif

endef

# Link static and shared libraries targets from a set of object files.
#
# $(1) = The target's name.
define LIB_RULES

t := $$(strip $(1))

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(wildcard $(d)/*.mk)

%.a %.so.$(VERSION): OBJS := $$(OBJ_$$(t))
%.a %.so.$(VERSION): CFLAGS := $(CFLAGS_$(d)) $(CFLAGS)
%.a %.so.$(VERSION): CXXFLAGS := $(CXXFLAGS_$(d)) $(CXXFLAGS)
%.a %.so.$(VERSION): LDFLAGS := $(LDFLAGS_$(d)) $(LDFLAGS)

%.a: $$(OBJ_$$(t)) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[Static $$(subst $(CURDIR)/,,$$@)]"
	$(STATIC)

%.so.$(VERSION): $$(OBJ_$$(t)) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[Shared $$(subst $(CURDIR)/,,$$@)]"
	$(SHARED_CXX)
	$(STRIP)

endef

# Build a release package.
#
# $(1) = The target's name.
# $(2) = The path to the target output binary or libraries.
# $(3) = The path to the target release package.
define PKG_RULES

t := $$(strip $(1))

# Force repackaging if any build files change
MAKEFILES_$(d) := $(BUILD_ROOT)/*.mk

$(3): REL_CMDS := $$(REL_CMDS_$$(t))
$(3): REL_NAME := $$(REL_NAME_$$(t))
$(3): ETC_TMP_DIR := $$(ETC_TMP_DIR_$$(t))
$(3): REL_BIN_DIR := $$(REL_BIN_DIR_$$(t))
$(3): REL_LIB_DIR := $$(REL_LIB_DIR_$$(t))
$(3): REL_INC_DIR := $$(REL_INC_DIR_$$(t))
$(3): REL_SRC_DIR := $$(REL_SRC_DIR_$$(t))

ifeq ($$(REL_CMDS_$$(t)), )
$(3):
else
$(3): $(2) $$(MAKEFILES_$(d))
	$(Q)$$(BUILD_REL)
endif

endef

# Compile C/C++ files to object files.
#
# $(1) = Path to directory where object files should be placed.
define OBJ_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(wildcard $(d)/*.mk)

$(1)/%.o: CFLAGS := $(CFLAGS_$(d)) $(CFLAGS)
$(1)/%.o: CXXFLAGS := $(CXXFLAGS_$(d)) $(CXXFLAGS)

# C files
$(1)/%.o: $(d)/%.c $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[Compile $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CC)

# CC files
$(1)/%.o: $(d)/%.cc $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[Compile $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CXX)

# C++ files
$(1)/%.o: $(d)/%.cpp $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[Compile $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CXX)

endef

# Generate C++ files from Qt resource collection files, and compile those C++
# files to object files.
#
# $(1) = Path to directory where object files should be placed.
# $(2) = Path to directory where generated files should be placed.
define QT_RULES

.PRECIOUS: $(2)/%.ui.h $(2)/%.moc.cpp $(2)/%.rcc.cpp

# UI files
$(2)/%.uic.h: $(d)/%.ui $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[UIC $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(UIC)

# MOC files
$(1)/%.moc.o: $(2)/%.moc.cpp
	@mkdir -p $$(@D)
	@echo "[Compile $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CXX)

$(2)/%.moc.cpp: $(d)/%.h $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[MOC $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(MOC)

# QRC files
$(1)/%.rcc.o: $(2)/%.rcc.cpp
	@mkdir -p $$(@D)
	@echo "[Compile $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CXX)

$(2)/%.rcc.cpp: $(d)/%.qrc $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[RCC $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(RCC)

endef

# Define the rules to build a binary target. The files.mk should define:
#
#     SRC_DIRS_$(d) = The source directories to include in the build.
#     LDLIBS_$(d) = The libraries to be linked in the target binary.
#     SRC_$(d) = The sources to be built in the target binary.
#
# If the binary target is a Qt binary, the files.mk should also define:
#
#     QT_UI_$(d) = Any Qt user interface files.
#     QT_MOC_$(d) = Any Qt meta-object header files.
#     QT_QRC_$(d) = Any Qt resource collection files.
#     QT_MODULES_$(d) = Any Qt modules to link.
#
# $(1) = The target's name.
# $(2) = The path to the target root directory.
# $(3) = The path to the target output libraries.
# $(4) = The path to the target release package.
define DEFINE_BIN_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR, $(2)))

# Define source, object, dependency, and binary files
include $$(d)/files.mk
$$(eval $$(call QT_OUT_FILES, $(1), \
    $$(QT_UI_$$(d)), $$(QT_MOC_$$(d)), $$(QT_QRC_$$(d)), $$(QT_MODULES_$$(d))))
$$(eval $$(call OBJ_OUT_FILES, $(1), $$(SRC_$$(d))))

# Include the source directories
$$(foreach dir, $$(SRC_DIRS_$$(d)), \
    $$(eval $$(call DEFINE_SRC_RULES, $(1), $$(dir))))

# Define the compile rules
$$(eval $$(call BIN_RULES, $(1), $(3)))
$$(eval $$(call PKG_RULES, $(1), $(3), $(4)))
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))
$$(eval $$(call QT_RULES, $$(OBJ_DIR_$$(d)), $$(GEN_DIR_$$(d))))

# Include dependency files
-include $$(DEP_$$(d))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef

# Define the rules to build static and shared library targets. The files.mk
# should define:
#
#     SRC_DIRS_$(d) = The source directories to include in the build.
#     SRC_$(d) = The sources to be built in the target libraries.
#
# $(1) = The target's name.
# $(2) = The path to the target root directory.
# $(3) = The path to the target output libraries.
# $(4) = The path to the target release package.
define DEFINE_LIB_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR, $(2)))

# Define source, object, dependency, and binary files
include $$(d)/files.mk
$$(eval $$(call OBJ_OUT_FILES, $(1), $$(SRC_$$(d))))

# Include the source directories
$$(foreach dir, $$(SRC_DIRS_$$(d)), \
    $$(eval $$(call DEFINE_SRC_RULES, $(1), $$(dir))))

# Define the compile rules
$$(eval $$(call LIB_RULES, $(1)))
$$(eval $$(call PKG_RULES, $(1), $(3), $(4)))
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))

# Include dependency files
-include $$(DEP_$$(d))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef

# Define the rules to build a source directory. The files.mk is optional for
# source directories. If not found, all source files in the directory will be
# built. If found, the files.mk should define:
#
#     SRC_$(d) = The sources to be built in the target.
#
# $(1) = The target's name.
# $(2) = The path to the target root directory.
define DEFINE_SRC_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR, $(2)))

# Define source, object and dependency files
ifeq ($$(wildcard $$(d)/files.mk),)
    SRC_$$(d) := \
        $$(wildcard $$(d)/*.c) \
        $$(wildcard $$(d)/*.cc) \
        $$(wildcard $$(d)/*.cpp)
else
    include $$(d)/files.mk
endif

$$(eval $$(call OBJ_OUT_FILES, $(1), $$(SRC_$$(d))))

# Define the compile rules
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))

# Include dependency files
-include $$(DEP_$$(d))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef
