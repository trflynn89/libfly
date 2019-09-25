# Define functions which will declare object, dependency, and executable files.

# Define output files for compiled C/C++ targets.
#
# $(1) = The target's name.
# $(2) = The C/C++ files to be compiled.
define OBJ_OUT_FILES

OBJ_DIR_$(d) := $(OBJ_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))

o_$(d) := $$(addsuffix .o, $$(subst $(d),,$$(basename $(2))))
o_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(o_$(d)))

OBJ_$$(strip $(1)) += $$(o_$(d))
DEP_$(d) := $$(o_$(d):%.o=%.d)

endef

# Define output files for Qt targets.
#
# $(1) = The target's name.
# $(2) = The C/C++ files to be compiled.
# $(3) = The Qt UIC source files.
# $(4) = The Qt MOC source files.
# $(5) = The Qt RCC source files.
define QT_OUT_FILES

t := $$(strip $(1))

OBJ_DIR_$(d) := $(OBJ_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))
GEN_DIR_$(d) := $(GEN_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))

OBJ_$$(t) += $$(addprefix $$(GEN_DIR_$(d))/, $$(addsuffix .uic.h, $(3)))
OBJ_$$(t) += $$(addprefix $$(OBJ_DIR_$(d))/, $$(addsuffix .moc.o, $(4)))
OBJ_$$(t) += $$(addprefix $$(OBJ_DIR_$(d))/, $$(addsuffix .rcc.o, $(5)))

$(call OBJ_OUT_FILES, $(1), $(2))

endef
