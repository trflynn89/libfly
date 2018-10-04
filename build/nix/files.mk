# Define functions which will declare object, dependency, and executable files.

# Define output files for compiled C/C++ targets.
#
# $(1) = The target's name.
# $(2) = The C/C++ files to be compiled.
define OBJ_OUT_FILES

OBJ_DIR_$(d) := $(OBJ_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))

OBJ_$(d) := $$(addsuffix .o, $$(subst $(d),,$$(basename $(2))))
OBJ_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(OBJ_$(d)))
OBJS_$$(strip $(1)) += $$(OBJ_$(d))

DEP_$(d) := $$(OBJ_$(d):%.o=%.d)

CLEAN_$(d) := $$(OBJ_$(d)) $$(DEP_$(d))

endef

# Define output files for QT5 targets.
#
# $(1) = The target's name.
# $(2) = The C/C++ files to be compiled.
# $(3) = The QT5 UIC source files.
# $(4) = The QT5 MOC source files.
# $(5) = The QT5 RCC source files.
define QT5_OUT_FILES

$(call OBJ_OUT_FILES, $(1), $(2))

QT5_UIC_$(d) := $$(addprefix $(GEN_DIR)/, $$(addsuffix .uic.h, $(3)))
QT5_UICS += $$(QT5_UIC_$(d))

QT5_MOC_$(d) := $$(addprefix $$(OBJ_DIR_$(d))/, $$(addsuffix .moc.o, $(4)))
QT5_MOCS += $$(QT5_MOC_$(d))

QT5_RCC_$(d) := $$(addprefix $$(OBJ_DIR_$(d))/, $$(addsuffix .rcc.o, $(5)))
QT5_RCCS += $$(QT5_RCC_$(d))

CLEAN_$(d) += $$(QT5_UIC_$(d)) $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))
OBJ_$(d) += $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))
OBJS_$$(strip $(1)) += $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))

endef
