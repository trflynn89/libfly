# Define make goals for compiling for all supported target types and the intermediate files they
# require. Each target source directory added via $(ADD_TARGET) must contain a file called files.mk.
# The contents expected of that file depend on the target type. This files.mk file is also where
# the APIs defined in release.mk may be used to create an archived release package.
#
# All variables defined in a files.mk file should be defined in terms of the special variable $(d):
#
#     d = The path to the directory containing the current files.mk file.
#
# This variable is defined and maintained by stack.mk. It is used to define variables of the same
# meaning on a per-source-directory basis.
#
# The files.mk for all target types may contain:
#
#     SRC_DIRS_$(d) = The source directories relative to $(SOURCE_ROOT) to include in the build.
#     SRC_$(d) = The sources in this directory to build.
#
# The files.mk for target type JAR may additionally contain:
#
#     MAIN_CLASS_$(d) = (Required) The application entry point for the executable JAR.
#     CLASS_PATH_$(d) = The paths to any JARs or packages to reference for compilation.
#     RESOURCES_$(d) = The paths to any runtime resources to include in the executable JAR.
#
# Each directory added to $(SRC_DIRS_$(d)) may optionally contain a files.mk file to define
# variables specific to that directory:
#
#     SRC_$(d) = The sources in this directory to build.
#
# If a directory in $(SRC_DIRS_$(d)) does not contain a files.mk file, then $(SRC_$(d)) defaults to
# every source file in that directory.
#
# Any of the files.mk files may contain the per-directory compiler/linker flag extensions described
# in flags.mk.

# Define helper aliases for compiler/linker invocations.
COMP_CC := $(Q)$(CC) $$(CFLAGS) -o $$@ -c $$<
LINK_CC := $(Q)$(CC) $$(CFLAGS) -o $$@ $$(OBJS) $$(LDFLAGS) $$(LDLIBS)

COMP_CXX := $(Q)$(CXX) $$(CXXFLAGS) -o $$@ -c $$<
LINK_CXX := $(Q)$(CXX) $$(CXXFLAGS) -o $$@ $$(OBJS) $$(LDFLAGS) $$(LDLIBS)

COMP_JAVA := $(Q)$(JAVAC) $$(JFLAGS) -d $$(CLASS_DIR) $$(CLASS_PATH) $$(SOURCES)
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

# Define the make goal to link a binary target from a set of object files.
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

# Define the make goal to link static and shared targets from a set of object files.
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

# Define the make goal to compile Java files and link an executable JAR file from the compiled class
# files.
#
# $(1) = The path to the target output JAR file.
# $(2) = The application entry point.
define JAR_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(wildcard $(d)/*.mk)

$(1): SOURCES := $$(SOURCES_$(t))
$(1): JFLAGS := $(JFLAGS) $(JFLAGS_$(d))
$(1): CLASS_DIR := $$(CLASS_DIR_$(t))
$(1): CLASS_PATH := $$(CLASS_PATH_$(t))
$(1): MAIN_CLASS := $(2)
$(1): CONTENTS := $$(CONTENTS_$(t))

$(1): $$(SOURCES_$(t)) $$(MAKEFILES_$(d))
	@# Remove the target's class directory as a safe measure in case Java files have been deleted.
	@$(RM) -r $$(CLASS_DIR)

	@mkdir -p $$(@D) $$(CLASS_DIR)

	@# Compile the Java source files into class files.
	@echo -e "[$(CYAN)Compile$(DEFAULT) $(strip $(2))]"
	$(COMP_JAVA)

	@# Iterate over every JAR file in the class path and extracts them for inclusion in the target.
	$(Q)for jar in $$(CLASS_PATH_JAR_$(t)) ; do \
		unzip $(ZIP_EXTRACT_FLAGS) -o $$$$jar -d $$(CLASS_DIR) "*.class" ; \
	done

	@# Create the JAR archive from the compiled set of class files, the contents of the extracted
	@# dependent JARs, and the contents of any resource directories.
	@echo -e "[$(RED)JAR$(DEFAULT) $$(subst $(CURDIR)/,,$$@)]"
	$(LINK_JAVA)

endef

# Define the make goal to generate an archived release package.
#
# $(1) = The path to the target output files.
# $(2) = The path to the target release package.
define PKG_RULES

ifeq ($$(REL_CMDS_$(t)),)

$(2):

else

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

# Define the make goal to compile C-family files to object files.
#
# $(1) = Path to directory where object files should be created.
define OBJ_RULES

MAKEFILES_$(d) := $(BUILD_ROOT)/flags.mk $(wildcard $(d)/*.mk)

# Use = instead of := because $$(@) would become an empty string if expanded now.
$(1)/%.o: CFLAGS = $(CFLAGS) $(CFLAGS_$(d)) -MF $$(@:%.o=%.d)
$(1)/%.o: CXXFLAGS = $(CXXFLAGS) $(CXXFLAGS_$(d)) -MF $$(@:%.o=%.d)

# C files.
$(1)/%.o: $(d)/%.c $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(CYAN)Compile$(DEFAULT) $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CC)

# CC files.
$(1)/%.o: $(d)/%.cc $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(CYAN)Compile$(DEFAULT) $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CXX)

# C++ files.
$(1)/%.o: $(d)/%.cpp $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(CYAN)Compile$(DEFAULT) $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CXX)

# Objective-C files.
$(1)/%.o: $(d)/%.m $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(CYAN)Compile$(DEFAULT) $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CC)

# Objective-C++ files.
$(1)/%.o: $(d)/%.mm $$(MAKEFILES_$(d))
	@mkdir -p $$(@D)
	@echo -e "[$(CYAN)Compile$(DEFAULT) $$(subst $(SOURCE_ROOT)/,,$$<)]"
	$(COMP_CXX)

endef

# Define all make goals required to build a target of type BIN (or TEST).
#
# $(1) = The path to the target root directory.
# $(2) = The path to the target output binary.
# $(3) = The path to the target release package.
define DEFINE_BIN_RULES

# Push the current directory to the stack.
$$(eval $$(call PUSH_DIR, $(1)))

# Define source, object, dependency, and binary files.
include $$(d)/files.mk
$$(eval $$(call OBJ_OUT_FILES, $$(SRC_$$(d))))

# Include the source directories.
$$(foreach dir, $$(SRC_DIRS_$$(d)), $$(eval $$(call DEFINE_OBJ_RULES, $$(dir))))

# Define the compile rules.
$$(eval $$(call BIN_RULES, $(2)))
$$(eval $$(call PKG_RULES, $(2), $(3)))
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))

# Include dependency files.
-include $$(DEP_$$(d))

# Pop the current directory from the stack.
$$(eval $$(call POP_DIR))

endef

# Define all make goals required to build a target of type LIB.
#
# $(1) = The path to the target root directory.
# $(2) = The path to the target output libraries.
# $(3) = The path to the target release package.
define DEFINE_LIB_RULES

# Push the current directory to the stack.
$$(eval $$(call PUSH_DIR, $(1)))

# Define source, object, dependency, and binary files.
include $$(d)/files.mk
$$(eval $$(call OBJ_OUT_FILES, $$(SRC_$$(d))))

# Include the source directories.
$$(foreach dir, $$(SRC_DIRS_$$(d)), $$(eval $$(call DEFINE_OBJ_RULES, $$(dir))))

# Define the compile rules.
$$(eval $$(call LIB_RULES))
$$(eval $$(call PKG_RULES, $(2), $(3)))
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))

# Include dependency files.
-include $$(DEP_$$(d))

# Pop the current directory from the stack.
$$(eval $$(call POP_DIR))

endef

# Define all make goals required to build a target of type JAR.
#
# $(1) = The path to the target root directory.
# $(2) = The path to the target output JAR file.
# $(3) = The path to the target release package.
define DEFINE_JAR_RULES

# Push the current directory to the stack.
$$(eval $$(call PUSH_DIR, $(1)))

# Define source, class, and generated files.
include $$(d)/files.mk
$$(eval $$(call JAVA_SRC_FILES, $$(SRC_$$(d))))
$$(eval $$(call JAVA_JAR_FILES, $$(CLASS_PATH_$$(d)), $$(RESOURCES_$$(d))))

# Include the source directories.
$$(foreach dir, $$(SRC_DIRS_$$(d)), $$(eval $$(call DEFINE_JAVA_RULES, $$(dir))))

# Define the compile rules.
$$(eval $$(call JAR_RULES, $(2), $$(MAIN_CLASS_$$(d))))
$$(eval $$(call PKG_RULES, $(2), $(3)))

# Pop the current directory from the stack.
$$(eval $$(call POP_DIR))

endef

# Define all make goals and intermediate files required to compile C-family files.
#
# $(1) = The path to the target root directory.
define DEFINE_OBJ_RULES

# Push the current directory to the stack.
$$(eval $$(call PUSH_DIR, $(1)))

# Define source, object and dependency files.
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

# Define the compile rules.
$$(eval $$(call OBJ_RULES, $$(OBJ_DIR_$$(d))))

# Include dependency files.
-include $$(DEP_$$(d))

# Pop the current directory from the stack.
$$(eval $$(call POP_DIR))

endef

# Define all make goals and intermediate files required to compile Java files.
#
# $(1) = The path to the target root directory.
define DEFINE_JAVA_RULES

# Push the current directory to the stack.
$$(eval $$(call PUSH_DIR, $(1)))

# Define source, object and dependency files.
ifeq ($$(wildcard $$(d)/files.mk),)
    SRC_$$(d) := \
        $$(wildcard $$(d)/*.java)
else
    include $$(d)/files.mk
endif

$$(eval $$(call JAVA_SRC_FILES, $$(SRC_$$(d))))

# Pop the current directory from the stack.
$$(eval $$(call POP_DIR))

endef
