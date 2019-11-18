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

# Define output files for Qt targets.
#
# $(1) = The target's name.
# $(2) = The Qt user interface files.
# $(3) = The Qt meta-object header files.
# $(4) = The Qt resource collection files.
# $(5) = The Qt modules to link.
define QT_OUT_FILES

OBJ_DIR_$(d) := $(OBJ_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))
GEN_DIR_$(d) := $(GEN_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))

ifneq ($(5),)
    u_$(d) := $$(addsuffix .uic.h, $$(subst $(d),,$$(basename $(2))))
    u_$(d) := $$(addprefix $$(GEN_DIR_$(d)), $$(u_$(d)))

    m_$(d) := $$(addsuffix .moc.o, $$(subst $(d),,$$(basename $(3))))
    m_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(m_$(d)))

    q_$(d) := $$(addsuffix .rcc.o, $$(subst $(d),,$$(basename $(4))))
    q_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(q_$(d)))

    OBJ_$$(strip $(1)) += $$(u_$(d)) $$(m_$(d)) $$(q_$(d))

    CFLAGS_$$(d) += $(QT_CFLAGS)
    CXXFLAGS_$$(d) += $(QT_CFLAGS)
    LDFLAGS_$$(d) += $(QT_LDFLAGS)
    LDLIBS_$(d) += $$(addprefix -lQt$(QT_VERSION_MAJOR), $(5))
endif

endef
