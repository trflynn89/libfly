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
# $(1) = Path to directory where object files should be placed.
define OBJ_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(d)/*.mk

$$(strip $(1))/%.o: CFLAGS := $(CFLAGS_$(d)) $(CFLAGS)
$$(strip $(1))/%.o: CXXFLAGS := $(CXXFLAGS_$(d)) $(CXXFLAGS)

# C files
$$(strip $(1))/%.o: $(d)/%.c $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[Compile $$(notdir $$<)]"
	$(COMP_CC)

# CC files
$$(strip $(1))/%.o: $(d)/%.cc $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[Compile $$(notdir $$<)]"
	$(COMP_CXX)

# C++ files
$$(strip $(1))/%.o: $(d)/%.cpp $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[Compile $$(notdir $$<)]"
	$(COMP_CXX)

endef

# Compile source files to QT5 files.
# $(1) = Path to directory where object files should be placed.
# $(2) = Path to directory where generated files should be placed.
define QT5_RULES

.PRECIOUS: $(GEN_DIR)/%.uic.h $(GEN_DIR)/%.moc.cpp $(GEN_DIR)/%.rcc.cpp

# UIC files
$(GEN_DIR)/%.uic.h: $(d)/%.ui $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[UIC $$(notdir $$<)]"
	$(UIC)

# MOC files
$$(strip $(1))/%.moc.o: $(GEN_DIR)/%.moc.cpp
	@mkdir -p $$(@D)
	@echo "[Compile $$(notdir $$<)]"
	$(COMP_CXX)

$(GEN_DIR)/%.moc.cpp: $(d)/%.h $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[MOC $$(notdir $$<)]"
	$(MOC)

# RCC files
$$(strip $(1))/%.rcc.o: $(GEN_DIR)/%.rcc.cpp
	@mkdir -p $$(@D)
	@echo "[Compile $$(notdir $$<)]"
	$(COMP_CXX)

$(GEN_DIR)/%.rcc.cpp: $(d)/%.qrc $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo "[RCC $$(notdir $$<)]"
	$(RCC)

endef

# Compile a set of C/C++ files for a source directory. These rules aren't used
# by the make system, but are defined for convenience. These targets may be
# invoked by calling "make The/Directory/To/Build".
# $(1) = All object files to be built in this source directory.
# $(2) = All intermediate files to be cleaned with this directory.
define SRC_RULES

$(td): $(1)
	@echo "[Finished $$(notdir $$@)]"

$(td)_clean:
	@echo "[Clean $$(notdir $(td))]"
	$(Q)$(RM) $(2)

endef

# Link a binary target from a set of object files.
define BIN_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(d)/*.mk

$(TARGET_NAME): CFLAGS := $(CFLAGS_$(d)) $(CFLAGS)
$(TARGET_NAME): CXXFLAGS := $(CXXFLAGS_$(d)) $(CXXFLAGS)
$(TARGET_NAME): LDFLAGS := $(LDFLAGS_$(d)) $(LDFLAGS)
$(TARGET_NAME): LDLIBS := $(LDLIBS_$(d)) $(LDLIBS)

$(TARGET_NAME): $$(QT5_UICS) $$(QT5_MOCS) $$(QT5_RCCS) $$(OBJS) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)

	@echo "[Link $$(notdir $$@)]"
	$(LINK_CXX)

ifeq ($(release),1)
	@echo "[Strip $$(notdir $$@)]"
	$(STRIP)
endif

endef

# Link a library target from a set of object files.
define LIB_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(d)/*.mk

$(TARGET_NAME): CFLAGS := $(CFLAGS_$(d)) $(CFLAGS)
$(TARGET_NAME): CXXFLAGS := $(CXXFLAGS_$(d)) $(CXXFLAGS)
$(TARGET_NAME): LDFLAGS := $(LDFLAGS_$(d)) $(LDFLAGS)

$(TARGET_NAME): $$(OBJS) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)

ifeq ($(release),1)
	@echo "[Shared $$(notdir $$@)]"
	$(SHARED_CXX)
	$(STRIP)
else
	@echo "[Static $$(notdir $$@)]"
	$(STATIC)
endif

endef

# Build a release package.
define PKG_RULES

# Force repackaging if any build files change
MAKEFILES_$(d) := $(BUILD_ROOT)/*.mk

$(TARGET_PACKAGE): $(TARGET_NAME) $$(MAKEFILES_$(d))
ifneq ($(REL_CMDS),)
	@echo "[Package $$(notdir $$@)]"
	$(Q)$$(BUILD_REL)
endif

endef

# Define the rules to build a binary target. The files.mk should define:
# 1. SRC_DIRS_$(d) = The source directories to include in the build.
# 2. LDLIBS_$(d) = The libraries to be linked in the target binary.
# 3. SRC_$(d) = The sources to be built in the target binary.
define DEFINE_BIN_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR))

# Define source, object, dependency, and binary files
include $$(d)/files.mk
$$(eval $$(call BIN_OUT_FILES, $$(SRC_$$(d))))

# Include the source directories
$$(eval $$(call INCLUDE_SRC_DIRS, $$(SRC_DIRS_$$(d))))

# Define the compile rules
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))
$$(eval $$(call BIN_RULES))
$$(eval $$(call PKG_RULES))

# Include dependency files
-include $$(DEP_$$(d))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef

# Define the rules to build a QT5 target. The files.mk should define:
# 1. SRC_DIRS_$(d) = The source directories to include in the build.
# 2. LDLIBS_$(d) = The libraries to be linked in the target binary.
# 3. SRC_$(d) = The sources to be built in the target binary.
# 4. QT5_UIC_$(d) = The QT5 UIC source files.
# 5. QT5_MOC_$(d) = The QT5 MOC source files.
# 6. QT5_RCC_$(d) = The QT5 RCC source files.
define DEFINE_QT5_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR))

# Define source, object, dependency, and binary files
include $$(d)/files.mk
$$(eval $$(call QT5_OUT_FILES, $$(SRC_$$(d)), $$(QT5_UIC_$$(d)), $$(QT5_MOC_$$(d)), $$(QT5_RCC_$$(d))))

# Include the source directories
$$(eval $$(call INCLUDE_SRC_DIRS, $$(SRC_DIRS_$$(d))))

# Define the compile rules
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))
$$(eval $$(call QT5_RULES, $$(OBJ_DIR_$$(d))))
$$(eval $$(call BIN_RULES))
$$(eval $$(call PKG_RULES))

# Include dependency files
-include $$(DEP_$$(d))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef

# Define the rules to build a library target. The files.mk should define:
# 1. SRC_DIRS_$(d) = The source directories to include in the build.
# 2. SRC_$(d) = The sources to be built in the target library.
define DEFINE_LIB_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR))

# Define source, object, dependency, and binary files
include $$(d)/files.mk
$$(eval $$(call LIB_OUT_FILES, $$(SRC_$$(d))))

# Include the source directories
$$(eval $$(call INCLUDE_SRC_DIRS, $$(SRC_DIRS_$$(d))))

# Define the compile rules
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))
$$(eval $$(call LIB_RULES))
$$(eval $$(call PKG_RULES))

# Include dependency files
-include $$(DEP_$$(d))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef

# Define the rules to build a source directory. The files.mk should define:
# 1. SRC_$(d) = The sources to be built in the target.
define DEFINE_SRC_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR))

# Define source, object and dependency files
include $$(d)/files.mk
$$(eval $$(call OBJ_OUT_FILES, $$(SRC_$$(d))))

# Define the compile rules
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))
$$(eval $$(call SRC_RULES, $$(OBJ_$$(d)), $$(CLEAN_$$(d))))

# Include dependency files
-include $$(DEP_$$(d))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef
