# Define functions which will declare object, dependency, and generated files.

# Define output files for compiled C/C++ targets.
#
# $(1) = The target's name.
# $(2) = The C/C++ files to be compiled.
define OBJ_OUT_FILES

OBJ_DIR_$(d) := $(OBJ_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))
GEN_DIR_$(d) := $(GEN_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))

o_$(d) := $$(addsuffix .o, $$(subst $(d),,$$(basename $(2))))
o_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(o_$(d)))

OBJ_$$(strip $(1)) += $$(o_$(d))
DEP_$(d) := $$(o_$(d):%.o=%.d)

endef

# Define output files for Qt targets. Should be called after $(OBJ_OUT_FILES).
#
# $(1) = The target's name.
# $(2) = The Qt resource collection files.
# $(3) = The Qt modules to link.
define QT_OUT_FILES

g_$(d) := $$(addsuffix .o, $$(subst $(d),,$(2)))
g_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(g_$(d)))

m_$(d) := $$(addprefix -lQt$(QT_VERSION_MAJOR), $(3))

OBJ_$$(strip $(1)) += $$(g_$(d))

CFLAGS_$$(d) += $(QT_CFLAGS)
CXXFLAGS_$$(d) += $(QT_CFLAGS)
LDFLAGS_$$(d) += $(QT_LDFLAGS)
LDLIBS_$(d) += $$(m_$(d))

endef
