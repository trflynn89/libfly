# Define compilation functions for each object and binary target. Every source
# directory should contain a files.mk file. Every binary target directory must
# also contain a files.mk file, and include the source directorises it depends
# upon. What each of the files.mk files must define is described below.

# Build tools
COMP_CC := $(Q)$(CC) $$(CFLAGS) -o $$@ -c $$<
LINK_CC := $(Q)$(CC) $$(CFLAGS) -o $$@ $$(OBJS) $$(LDFLAGS) $$(LDLIBS)

COMP_CXX := $(Q)$(CXX) $$(CXXFLAGS) -o $$@ -c $$<
LINK_CXX := $(Q)$(CXX) $$(CXXFLAGS) -o $$@ $$(OBJS) $$(LDFLAGS) $$(LDLIBS)

COMP_MVN := $(Q)$(MVN) $(MVN_FLAGS) -f $$(POM) compile assembly:single

SHARED_CC := $(Q)$(CC) $$(CFLAGS) -shared -Wl,-soname,$$(@F) -o $$@ $$(OBJS) $$(LDFLAGS)
SHARED_CXX := $(Q)$(CXX) $$(CXXFLAGS) -shared -Wl,-soname,$$(@F) -o $$@ $$(OBJS) $$(LDFLAGS)
STATIC := $(Q)$(AR) rcs $$@ $$(OBJS)

# Link a binary target from a set of object files.
#
# $(1) = The target's name.
# $(2) = The path to the target output binary.
define BIN_RULES

t := $$(strip $(1))

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(wildcard $(d)/*.mk)

$(2): OBJS := $$(OBJ_$$(t))
$(2): CFLAGS := $(CFLAGS) $(CFLAGS_$(d))
$(2): CXXFLAGS := $(CXXFLAGS) $(CXXFLAGS_$(d))
$(2): LDFLAGS := $(LDFLAGS) $(LDFLAGS_$(d))
$(2): LDLIBS := $(LDLIBS) $(LDLIBS_$(d))

$(2): $$(OBJ_$$(t)) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)

	@echo -e "[$(RED)Link$(DEFAULT) $$(subst $(CURDIR)/,,$$@)]"
	$(LINK_CXX)

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
	@echo -e "[$(GREEN)Static$(DEFAULT) $$(subst $(CURDIR)/,,$$@)]"
	$(STATIC)

%.so.$(VERSION): $$(OBJ_$$(t)) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(GREEN)Shared$(DEFAULT) $$(subst $(CURDIR)/,,$$@)]"
	$(SHARED_CXX)

endef

# Build a JAR file for a Maven project.
#
# $(1) = The path to the target output JAR file.
# $(2) = The path to the target release package.
define MVN_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(wildcard $(d)/*.mk)

$(1): POM := $$(d)/pom.xml

$(1): $$(MAKEFILES_$(d)) $$(SRC_$$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(RED)Maven$(DEFAULT) $$(subst $(CURDIR)/,,$$@)]"
	$(COMP_MVN)

$(2):

endef

# Build a release package.
#
# $(1) = The target's name.
# $(2) = The path to the target output binary or libraries.
# $(3) = The path to the target release package.
define PKG_RULES

t := $$(strip $(1))

ifeq ($$(REL_CMDS_$$(t)),)

$(3):

else

# Force repackaging if any build files change
MAKEFILES_$(d) := $(BUILD_ROOT)/*.mk

$(3): REL_CMDS := $$(REL_CMDS_$$(t))
$(3): REL_NAME := $$(REL_NAME_$$(t))
$(3): ETC_TMP_DIR := $$(ETC_TMP_DIR_$$(t))
$(3): REL_BIN_DIR := $$(REL_BIN_DIR_$$(t))
$(3): REL_LIB_DIR := $$(REL_LIB_DIR_$$(t))
$(3): REL_INC_DIR := $$(REL_INC_DIR_$$(t))
$(3): REL_SRC_DIR := $$(REL_SRC_DIR_$$(t))
$(3): REL_UNINSTALL := $$(REL_UNINSTALL_$$(t))

$(3): $(2) $$(MAKEFILES_$(d))
	$(Q)$$(BUILD_REL)

endif

endef

# Compile C/C++ files to object files.
#
# $(1) = Path to directory where object files should be placed.
define OBJ_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(wildcard $(d)/*.mk)

# Use = instead of := because $$(@) would become an empty string if expanded now
$(1)/%.o: CFLAGS = $(CFLAGS) $(CFLAGS_$(d)) -MF $$(@:%.o=%.d)
$(1)/%.o: CXXFLAGS = $(CXXFLAGS) $(CXXFLAGS_$(d)) -MF $$(@:%.o=%.d)

# C files
$(1)/%.o: $(d)/%.c $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(CYAN)Compile$(DEFAULT) $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CC)

# CC files
$(1)/%.o: $(d)/%.cc $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(CYAN)Compile$(DEFAULT) $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CXX)

# C++ files
$(1)/%.o: $(d)/%.cpp $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(CYAN)Compile$(DEFAULT) $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CXX)

endef

# Define the rules to build a binary target. The files.mk should define:
#
#     SRC_DIRS_$(d) = The source directories to include in the build.
#     LDLIBS_$(d) = The libraries to be linked in the target binary.
#     SRC_$(d) = The sources to be built in the target binary.
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
$$(eval $$(call OBJ_OUT_FILES, $(1), $$(SRC_$$(d))))

# Include the source directories
$$(foreach dir, $$(SRC_DIRS_$$(d)), \
    $$(eval $$(call DEFINE_SRC_RULES, $(1), $$(dir))))

# Define the compile rules
$$(eval $$(call BIN_RULES, $(1), $(3)))
$$(eval $$(call PKG_RULES, $(1), $(3), $(4)))
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))

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

# Define the rules to build a JAR file for a Maven project. The files.mk
# should define:
#
#     SRC_$(d) = The sources to be built in the target libraries.
#
# $(1) = The target's name.
# $(2) = The path to the target root directory.
# $(3) = The path to the target output libraries.
# $(4) = The path to the target release package.
define DEFINE_MVN_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR, $(2)))

# Define source files
ifeq ($$(wildcard $$(d)/files.mk),)
    SRC_$$(d) := $$(shell find $$(d) -type f -name "*.java")
else
    include $$(d)/files.mk
endif

# Define the compile rules
$$(eval $$(call MVN_RULES, $(3), $(4)))

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
