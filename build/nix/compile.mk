# Define compilation functions for each object and binary target. Every source
# directory should contain a files.mk file. Every binary target directory must
# also contain a files.mk file, and include the source directorises it depends
# upon. What each of the files.mk files must define is described below.

# Build tools
COMP_CC := $(Q)$(CC) $$(CFLAGS) -o $$@ -c $$<
LINK_CC := $(Q)$(CC) $$(CFLAGS) -o $$@ $$(OBJS) $$(LDFLAGS) $$(LDLIBS)

COMP_CXX := $(Q)$(CXX) $$(CXXFLAGS) -o $$@ -c $$<
LINK_CXX := $(Q)$(CXX) $$(CXXFLAGS) -o $$@ $$(OBJS) $$(LDFLAGS) $$(LDLIBS)

COMP_JAVA := $(Q)$(JAVAC) $(JAVA_FLAGS) $$(CLASS_PATH) $$(SOURCES)
LINK_JAVA := $(Q)$(JAR) $(JAR_CREATE_FLAGS) $$(MAIN_CLASS) $$@ $$(CONTENTS)

STATIC := $(Q)$(AR) rcs $$@ $$(OBJS)

ifeq ($(SYSTEM), LINUX)
    SHARED_CC := $(Q)$(CC) $$(CFLAGS) -shared -Wl,-soname,$$(@F) -o $$@ $$(OBJS) $$(LDFLAGS)
    SHARED_CXX := $(Q)$(CXX) $$(CXXFLAGS) -shared -Wl,-soname,$$(@F) -o $$@ $$(OBJS) $$(LDFLAGS)
else ifeq ($(SYSTEM), MACOS)
    SHARED_CC := $(Q)$(CC) $$(CFLAGS) -dynamiclib -o $$@ $$(OBJS) $$(LDFLAGS)
    SHARED_CXX := $(Q)$(CXX) $$(CXXFLAGS) -dynamiclib -o $$@ $$(OBJS) $$(LDFLAGS)
else
    $(error Unrecognized system $(SYSTEM), check compile.mk)
endif

# Link a binary target from a set of object files.
#
# $(1) = The path to the target output binary.
define BIN_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(wildcard $(d)/*.mk)

$(1): OBJS := $$(OBJ_$(t))
$(1): CFLAGS := $(CFLAGS) $(CFLAGS_$(d))
$(1): CXXFLAGS := $(CXXFLAGS) $(CXXFLAGS_$(d))
$(1): LDFLAGS := $(LDFLAGS) $(LDFLAGS_$(d))
$(1): LDLIBS := $(LDLIBS) $(LDLIBS_$(d))

$(1): $$(OBJ_$(t)) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)

	@echo -e "[$(RED)Link$(DEFAULT) $$(subst $(CURDIR)/,,$$@)]"
	$(LINK_CXX)

endef

# Link static and shared library targets from a set of object files.
define LIB_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(wildcard $(d)/*.mk)

%.a %.so.$(VERSION) %.dylib.$(VERSION): OBJS := $$(OBJ_$(t))
%.a %.so.$(VERSION) %.dylib.$(VERSION): CFLAGS := $(CFLAGS_$(d)) $(CFLAGS)
%.a %.so.$(VERSION) %.dylib.$(VERSION): CXXFLAGS := $(CXXFLAGS_$(d)) $(CXXFLAGS)
%.a %.so.$(VERSION) %.dylib.$(VERSION): LDFLAGS := $(LDFLAGS_$(d)) $(LDFLAGS)

%.a: $$(OBJ_$(t)) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(GREEN)Static$(DEFAULT) $$(subst $(CURDIR)/,,$$@)]"
	$(STATIC)

%.so.$(VERSION) %.dylib.$(VERSION): $$(OBJ_$(t)) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(GREEN)Shared$(DEFAULT) $$(subst $(CURDIR)/,,$$@)]"
	$(SHARED_CXX)

endef

# Build an executable JAR file for a Java project.
#
# $(1) = The application entry point.
# $(2) = The path to the target output JAR file.
# $(3) = The path to the target release package.
define JAR_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(wildcard $(d)/*.mk)

$(2): SOURCES := $$(SOURCES_$(t))
$(2): CLASS_PATH := $$(CLASS_PATH_$(t))
$(2): MAIN_CLASS := $(1)
$(2): CONTENTS := $$(CONTENTS_$(t))

$(2): $$(SOURCES_$(t)) $$(MAKEFILES_$(d))
	@mkdir -p $$(@D) $(CLASS_DIR)

	@# Compile the Java source files into class files.
	@echo -e "[$(CYAN)Compile$(DEFAULT) $(strip $(1))]"
	$(COMP_JAVA)

	@# Iterate over every JAR file in the class path and extracts them for inclusion in the target.
	$(Q)for jar in $$(CLASS_PATH_JAR_$(t)) ; do \
		unzip $(ZIP_EXTRACT_FLAGS) -o $$$$jar -d $(CLASS_DIR) "*.class" ; \
	done

	@# Create the JAR archive from the compiled set of class files, the contents of the extracted
	@# dependent JARs, and the contents of any resource directories.
	@echo -e "[$(RED)JAR$(DEFAULT) $$(subst $(CURDIR)/,,$$@)]"
	$(LINK_JAVA)

$(3):

endef

# Build a release package.
#
# $(1) = The path to the target output binary or libraries.
# $(2) = The path to the target release package.
define PKG_RULES

ifeq ($$(REL_CMDS_$(t)),)

$(2):

else

# Force repackaging if any build files change
MAKEFILES_$(d) := $(BUILD_ROOT)/*.mk

$(2): REL_CMDS := $$(REL_CMDS_$(t))
$(2): REL_NAME := $$(REL_NAME_$(t))
$(2): ETC_TMP_DIR := $$(ETC_TMP_DIR_$(t))
$(2): REL_BIN_DIR := $$(REL_BIN_DIR_$(t))
$(2): REL_LIB_DIR := $$(REL_LIB_DIR_$(t))
$(2): REL_INC_DIR := $$(REL_INC_DIR_$(t))
$(2): REL_SRC_DIR := $$(REL_SRC_DIR_$(t))
$(2): REL_UNINSTALL := $$(REL_UNINSTALL_$(t))

$(2): $(1) $$(MAKEFILES_$(d))
	$(Q)$$(BUILD_REL)

endif

endef

# Compile C/C++/Objective-C/Objective-C++ files to object files.
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

# Objective-C files
$(1)/%.o: $(d)/%.m $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(CYAN)Compile$(DEFAULT) $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CC)

# Objective-C++ files
$(1)/%.o: $(d)/%.mm $$(MAKEFILES_$(d))
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
# $(1) = The path to the target root directory.
# $(2) = The path to the target output libraries.
# $(3) = The path to the target release package.
define DEFINE_BIN_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR, $(1)))

# Define source, object, dependency, and binary files
include $$(d)/files.mk
$$(eval $$(call OBJ_OUT_FILES, $$(SRC_$$(d))))

# Include the source directories
$$(foreach dir, $$(SRC_DIRS_$$(d)), $$(eval $$(call DEFINE_SRC_RULES, $$(dir))))

# Define the compile rules
$$(eval $$(call BIN_RULES, $(2)))
$$(eval $$(call PKG_RULES, $(2), $(3)))
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
# $(1) = The path to the target root directory.
# $(2) = The path to the target output libraries.
# $(3) = The path to the target release package.
define DEFINE_LIB_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR, $(1)))

# Define source, object, dependency, and binary files
include $$(d)/files.mk
$$(eval $$(call OBJ_OUT_FILES, $$(SRC_$$(d))))

# Include the source directories
$$(foreach dir, $$(SRC_DIRS_$$(d)), $$(eval $$(call DEFINE_SRC_RULES, $$(dir))))

# Define the compile rules
$$(eval $$(call LIB_RULES))
$$(eval $$(call PKG_RULES, $(2), $(3)))
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))

# Include dependency files
-include $$(DEP_$$(d))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef

# Define the rules to build an executable JAR file for a Java project. The files.mk should define:
#
#     SRC_DIRS_$(d) = The source directories to include in the build.
#     SRC_$(d) = The sources to be built in the target libraries.
#     MAIN_CLASS_$(d) = The application entry point for the executable JAR.
#     CLASS_PATH_$(d) = The paths to any JARs or packages to reference for compilation.
#     RESOURCES_$(d) = The paths to any resources to include in the executable JAR.
#
# $(1) = The path to the target root directory.
# $(2) = The path to the target output libraries.
# $(3) = The path to the target release package.
define DEFINE_JAR_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR, $(1)))

# Define source, class, and generated files
include $$(d)/files.mk
$$(eval $$(call JAVA_OUT_FILES, $$(SRC_$$(d))))
$$(eval $$(call JAVA_JAR_FILES, $$(CLASS_PATH_$$(d)), $$(RESOURCES_$$(d))))

# Include the source directories
$$(foreach dir, $$(SRC_DIRS_$$(d)), $$(eval $$(call DEFINE_JAVA_RULES, $$(dir))))

# Define the compile rules
$$(eval $$(call JAR_RULES, $$(MAIN_CLASS_$$(d)), $(2), $(3)))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef

# Define the rules to build a source directory. The files.mk is optional for
# source directories. If not found, all source files in the directory will be
# built. If found, the files.mk should define:
#
#     SRC_$(d) = The sources to be built in the target.
#
# $(1) = The path to the target root directory.
define DEFINE_SRC_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR, $(1)))

# Define source, object and dependency files
ifeq ($$(wildcard $$(d)/files.mk),)
    SRC_$$(d) := \
        $$(wildcard $$(d)/*.c) \
        $$(wildcard $$(d)/*.cc) \
        $$(wildcard $$(d)/*.cpp) \
        $$(wildcard $$(d)/*.m) \
        $$(wildcard $$(d)/*.mm)
else
    include $$(d)/files.mk
endif

$$(eval $$(call OBJ_OUT_FILES, $$(SRC_$$(d))))

# Define the compile rules
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
# $(1) = The path to the target root directory.
define DEFINE_JAVA_RULES

# Push current dir to stack
$$(eval $$(call PUSH_DIR, $(1)))

# Define source, object and dependency files
ifeq ($$(wildcard $$(d)/files.mk),)
    SRC_$$(d) := \
        $$(wildcard $$(d)/*.java)
else
    include $$(d)/files.mk
endif

$$(eval $$(call JAVA_OUT_FILES, $$(SRC_$$(d))))

# Pop current dir from stack
$$(eval $$(call POP_DIR))

endef
