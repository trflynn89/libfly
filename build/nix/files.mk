# Define functions which will declare object, dependency, and executable files.

# A list of all object files that have been compiled
OBJS :=

# Set a source directory's source files to all C/C++ files.
define WILDCARD_SOURCES

SRC_$(d) := \
    $(wildcard $(d)/*.c) \
    $(wildcard $(d)/*.cc) \
    $(wildcard $(d)/*.cpp)

endef

# Define output files for compiled C/C++ targets.
# $(1) = The C/C++ files to be compiled.
define OBJ_OUT_FILES

OBJ_DIR_$(d) := $(OBJ_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))

OBJ_$(d) := $$(addsuffix .o, $$(subst $(d),,$$(basename $$(strip $(1)))))
OBJ_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(OBJ_$(d)))
OBJS += $$(OBJ_$(d))

DEP_$(d) := $$(OBJ_$(d):%.o=%.d)

CLEAN_$(d) := $$(OBJ_$(d)) $$(DEP_$(d))

endef

# Define output files for binary targets.
# $(1) = The C/C++ files to be compiled.
define BIN_OUT_FILES

$(call OBJ_OUT_FILES, $(1))

endef

# Define output files for QT5 targets.
# $(1) = The C/C++ files to be compiled.
# $(2) = The QT5 UIC/MOC/RCC source file.
define QT5_OUT_FILES

$(call OBJ_OUT_FILES, $(1))

QT5_UIC_$(d) := $(GEN_DIR)/$$(strip $(2)).uic.h
QT5_MOC_$(d) := $$(OBJ_DIR_$(d))/$$(strip $(2)).moc.o
QT5_RCC_$(d) := $$(OBJ_DIR_$(d))/$$(strip $(2)).rcc.o

CLEAN_$(d) += $$(QT5_UIC_$(d)) $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))
OBJ_$(d) += $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))
OBJS += $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))

endef

# Define output files for library targets.
# $(1) = The C/C++ files to be compiled.
define LIB_OUT_FILES

$(call OBJ_OUT_FILES, $(1))

endef
