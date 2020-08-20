# Define functions which will declare object, dependency, class, and generated files.

# A literal space.
SPACE :=
SPACE +=

# Define output files for compiled C/C++/Objective-C/Objective-C++ targets.
#
# $(1) = The C/C++/Objective-C/Objective-C++ files to be compiled.
define OBJ_OUT_FILES

OBJ_DIR_$(d) := $(OBJ_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))

o_$(d) := $$(addsuffix .o, $$(subst $(d),,$$(basename $(1))))
o_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(o_$(d)))

OBJ_$(t) += $$(o_$(d))
DEP_$(d) := $$(o_$(d):%.o=%.d)

endef

# Define the output files for compiled Java targets.
#
# $(1) = The Java files to be compiled.
define JAVA_OUT_FILES

c_$(d) := $$(subst $(SOURCE_ROOT)/$$(TARGET_PATH_$(t))/,,$(1))
c_$(d) := $$(addsuffix .class, $$(basename $$(c_$(d))))

SOURCES_$(t) += $(1)

# For each Java class file, form the command line argument to include those class files in the
# target JAR. The format for each class file will be: -C '$(CLASS_DIR)' '$(class file)'
#
# Note the file paths are surrounded in single quotes. If a file path contains the $ symbol,
# this will prevent Make/Bash from evaluating the file name as a variable.
CONTENTS_$(t) += $$(foreach class, $$(c_$(d)), -C '$(CLASS_DIR)' '$$(class)')

endef

# Define the files and command line arguments for Java compilation and JAR creation.
#
# $(1) = The paths to any JARs or packages to reference for compilation.
# $(2) = The paths to any resources to include in the executable JAR.
define JAVA_JAR_FILES

ifneq ($(1),)
    # Form the command line argument to include each provided class path.
    CLASS_PATH_$(t) := -cp $(subst $(SPACE),:,$(strip $(1)))

    # For all JAR files specified in the class path, determine the classes stored in the class path
    # JAR and form the command line argument to include those classes in the target JAR. The format
    # for each class will be: -C '$(CLASS_DIR)' '$(class file)'
    CLASS_PATH_JAR_$(t) := $(filter %.jar, $(strip $(1)))

    ifneq ($$(CLASS_PATH_JAR_$(t)),)
        j_$(t) := $$(foreach jar, $$(CLASS_PATH_JAR_$(t)), $$(shell unzip -Z1 $$(jar) "*.class"))
        CONTENTS_$(t) += $$(foreach class, $$(j_$(t)), -C '$(CLASS_DIR)' '$$(class)')
    endif
endif

# Form the command line argument to include each provided resource path in the target JAR. The
# format for each resource of the form /path/to/resource will be: -C '/path/to' 'resource'
CONTENTS_$(t) += $$(foreach res, $(2), -C '$$(dir $$(res))' '$$(notdir $$(res))')

endef
