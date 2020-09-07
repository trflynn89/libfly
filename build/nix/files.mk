# Define functions which will declare object, dependency, class, and generated files.

# A literal space.
SPACE :=
SPACE +=

# Define output files for compiled C-family targets.
#
# $(1) = The C-family files to be compiled.
define OBJ_OUT_FILES

OBJ_DIR_$(d) := $(OBJ_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))

o_$(d) := $$(addsuffix .o, $$(subst $(d),,$$(basename $(1))))
o_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(o_$(d)))

OBJ_$(t) += $$(o_$(d))
DEP_$(d) := $$(o_$(d):%.o=%.d)

endef

# Define the source files for compiled Java targets.
#
# $(1) = The Java files to be compiled.
define JAVA_SRC_FILES

SOURCES_$(t) += $(1)

endef

# Define the files and command line arguments for Java compilation and JAR creation.
#
# For now, all Java source files are compiled to a subdirectory of $(CLASS_DIR). The entirety of the
# subdirectory's contents are then added to the JAR. Ideally, the build system could track compiled
# class files for explicit inclusion. But the Jar must also contain class files of anonymous inner
# classes. Including everything is a temporary solution until a way to determine those extra class
# files is implemented.
#
# $(1) = The paths to any JARs or packages to reference for compilation.
# $(2) = The paths to any resources to include in the executable JAR.
define JAVA_JAR_FILES

CLASS_DIR_$(t) := $(CLASS_DIR)/$(t)

# Form the command line argument to include the target's entire class directory.
CONTENTS_$(t) += -C $$(CLASS_DIR_$(t)) '.'

ifneq ($(1),)
    # Form the command line argument to include each provided class path.
    CLASS_PATH_$(t) := -cp $(subst $(SPACE),:,$(strip $(1)))

    # Track each JAR file specified in the class path for extraction.
    CLASS_PATH_JAR_$(t) := $(filter %.jar, $(strip $(1)))
endif

# Form the command line argument to include each provided resource path in the target JAR. The
# format for each resource of the form /path/to/resource will be: -C '/path/to' 'resource'
CONTENTS_$(t) += $$(foreach res, $(2), -C $$(dir $$(res)) $$(notdir $$(res)))

endef
