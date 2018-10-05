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

UIC := $(Q)$(QT5_UIC) $$< -o $$@
MOC := $(Q)$(QT5_MOC) $$< -o $$@
RCC := $(Q)$(QT5_RCC) $$< -o $$@

STATIC := $(Q)$(AR) rcs $$@ $$(OBJS)

STRIP := $(Q)strip $$@

# Compile C/C++ files to object files.
#
# $(1) = Path to directory where object files should be placed.
define OBJ_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(d)/*.mk

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

# Link a binary target from a set of object files.
#
# $(1) = The target's name.
# $(2) = The path to the target output binary.
define BIN_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(d)/*.mk

$(2): OBJS := $$(OBJ_$$(strip $(1)))
$(2): CFLAGS := $(CFLAGS_$(d)) $(CFLAGS)
$(2): CXXFLAGS := $(CXXFLAGS_$(d)) $(CXXFLAGS)
$(2): LDFLAGS := $(LDFLAGS_$(d)) $(LDFLAGS)
$(2): LDLIBS := $(LDLIBS_$(d)) $(LDLIBS)

$(2): $$(OBJ_$$(strip $(1))) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)

	@echo "[Link $$(subst $(CURDIR)/,,$$@)]"
	$(LINK_CXX)

ifeq ($(release),1)
	@echo "[Strip $$(subst $(CURDIR)/,,$$@)]"
	$(STRIP)
endif

endef

# Compile source files to QT5 files.
#
# $(1) = Path to directory where object files should be placed.
# $(2) = Path to directory where generated files should be placed.
define QT5_RULES

.PRECIOUS: $(2)/%.uic.h $(2)/%.moc.cpp $(2)/%.rcc.cpp

# UIC files
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

# RCC files
$(1)/%.rcc.o: $(2)/%.rcc.cpp
	@mkdir -p $$(@D)
	@echo "[Compile $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CXX)

$(2)/%.rcc.cpp: $(d)/%.qrc $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[RCC $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(RCC)

endef

# Link a library target from a set of object files.
#
# $(1) = The target's name.
# $(2) = The path to the target output binary.
define LIB_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(d)/*.mk

$(2): OBJS := $$(OBJ_$$(strip $(1)))
$(2): CFLAGS := $(CFLAGS_$(d)) $(CFLAGS)
$(2): CXXFLAGS := $(CXXFLAGS_$(d)) $(CXXFLAGS)
$(2): LDFLAGS := $(LDFLAGS_$(d)) $(LDFLAGS)

$(2): $$(OBJ_$$(strip $(1))) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)

ifeq ($(release),1)
	@echo "[Shared $$(subst $(CURDIR)/,,$$@)]"
	$(SHARED_CXX)
	$(STRIP)
else
	@echo "[Static $$(subst $(CURDIR)/,,$$@)]"
	$(STATIC)
endif

endef

# Build a release package.
#
# $(1) = The target's name.
# $(2) = The path to the target output binary or library.
# $(3) = The path to the target release package.
define PKG_RULES

# Force repackaging if any build files change
MAKEFILES_$(d) := $(BUILD_ROOT)/*.mk

$(3): REL_CMDS := $$(REL_CMDS_$$(strip $(1)))
$(3): REL_NAME := $$(REL_NAME_$$(strip $(1)))

$(3): $(2) $$(MAKEFILES_$(d))
	$(Q)$$(BUILD_REL)

endef

# Define the rules to build a binary target. The files.mk should define:
#
#     SRC_DIRS_$(d) = The source directories to include in the build.
#     LDLIBS_$(d) = The libraries to be linked in the target binary.
#     SRC_$(d) = The sources to be built in the target binary.
#
# $(1) = The target's name.
# $(2) = The path to the target output binary.
# $(3) = The path to the target release package.
define DEFINE_BIN_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR))

# Define source, object, dependency, and binary files
include $$(d)/files.mk
$$(eval $$(call OBJ_OUT_FILES, $(1), $$(SRC_$$(d))))

# Include the source directories
$$(eval $$(call INCLUDE_SRC_DIRS, $(1), $$(SRC_DIRS_$$(d))))

# Define the compile rules
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))
$$(eval $$(call BIN_RULES, $(1), $(2)))
$$(eval $$(call PKG_RULES, $(1), $(2), $(3)))

# Include dependency files
-include $$(DEP_$$(d))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef

# Define the rules to build a QT5 target. The files.mk should define:
#
#     SRC_DIRS_$(d) = The source directories to include in the build.
#     LDLIBS_$(d) = The libraries to be linked in the target binary.
#     SRC_$(d) = The sources to be built in the target binary.
#     QT5_UIC_$(d) = The QT5 UIC source files.
#     QT5_MOC_$(d) = The QT5 MOC source files.
#     QT5_RCC_$(d) = The QT5 RCC source files.
#
# $(1) = The target's name.
# $(2) = The path to the target output binary.
# $(3) = The path to the target release package.
define DEFINE_QT5_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR))

# Define source, object, dependency, and binary files
include $$(d)/files.mk
$$(eval $$(call QT5_OUT_FILES, $(1), $$(SRC_$$(d)), $$(QT5_UIC_$$(d)), $$(QT5_MOC_$$(d)), $$(QT5_RCC_$$(d))))

# Include the source directories
$$(eval $$(call INCLUDE_SRC_DIRS, $(1), $$(SRC_DIRS_$$(d))))

# Add build flags needed for Qt5 projects
CFLAGS_$$(d) += $(QT5_CFLAGS)
CXXFLAGS_$$(d) += $(QT5_CFLAGS)
LDFLAGS_$$(d) += $(QT5_LDFLAGS)
LDLIBS_$$(d) += $(QT5_LDLIBS)

# Define the compile rules
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))
$$(eval $$(call QT5_RULES, $$(OBJ_DIR_$$(d)), $$(GEN_DIR_$$(d))))
$$(eval $$(call BIN_RULES, $(1), $(2)))
$$(eval $$(call PKG_RULES, $(1), $(2), $(3)))

# Include dependency files
-include $$(DEP_$$(d))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef

# Define the rules to build a library target. The files.mk should define:
#
#     SRC_DIRS_$(d) = The source directories to include in the build.
#     SRC_$(d) = The sources to be built in the target library.
#
# $(1) = The target's name.
# $(2) = The path to the target output library.
# $(3) = The path to the target release package.
define DEFINE_LIB_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR))

# Define source, object, dependency, and binary files
include $$(d)/files.mk
$$(eval $$(call OBJ_OUT_FILES, $(1), $$(SRC_$$(d))))

# Include the source directories
$$(eval $$(call INCLUDE_SRC_DIRS, $(1), $$(SRC_DIRS_$$(d))))

# Define the compile rules
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))
$$(eval $$(call LIB_RULES, $(1), $(2)))
$$(eval $$(call PKG_RULES, $(1), $(2), $(3)))

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
define DEFINE_SRC_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR))

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
