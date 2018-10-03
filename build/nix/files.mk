# Define functions which will declare object, dependency, and executable files.

# Define output files for compiled C/C++ targets.
#
# $(1) = The target's name.
# $(2) = The C/C++ files to be compiled.
define OBJ_OUT_FILES

t := $$(strip $(1))

OBJ_DIR_$(d) := $(OBJ_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))

OBJ_$(d) := $$(addsuffix .o, $$(subst $(d),,$$(basename $$(strip $(2)))))
OBJ_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(OBJ_$(d)))
OBJS_$$(t) += $$(OBJ_$(d))

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

t := $$(strip $(1))

$(call OBJ_OUT_FILES, $$(t), $(2))

QT5_UIC_$(d) := $$(addprefix $(GEN_DIR)/, $$(addsuffix .uic.h, $$(strip $(3))))
QT5_UICS += $$(QT5_UIC_$(d))

QT5_MOC_$(d) := $$(addprefix $$(OBJ_DIR_$(d))/, $$(addsuffix .moc.o, $$(strip $(4))))
QT5_MOCS += $$(QT5_MOC_$(d))

QT5_RCC_$(d) := $$(addprefix $$(OBJ_DIR_$(d))/, $$(addsuffix .rcc.o, $$(strip $(5))))
QT5_RCCS += $$(QT5_RCC_$(d))

CLEAN_$(d) += $$(QT5_UIC_$(d)) $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))
OBJ_$(d) += $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))
OBJS_$$(t) += $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))

endef
