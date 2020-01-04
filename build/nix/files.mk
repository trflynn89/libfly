# Define functions which will declare object, dependency, and generated files.

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
# $(2) = The Qt user interface files.
# $(3) = The Qt meta-object header files.
# $(4) = The Qt resource collection files.
# $(5) = The Qt modules to link.
define QT_OUT_FILES

OBJ_DIR_$(d) := $(OBJ_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))
GEN_DIR_$(d) := $(GEN_DIR)/$$(subst $(SOURCE_ROOT)/,,$(d))

ifneq ($(5),)
    # Generated UI header files
    gu_$(d) := $$(addsuffix .uic.h, $$(subst $(d),,$$(basename $(2))))
    gu_$(d) := $$(addprefix $$(GEN_DIR_$(d)), $$(gu_$(d)))

    # Generated MOC source files
    gm_$(d) := $$(addsuffix .moc.cpp, $$(subst $(d),,$$(basename $(3))))
    gm_$(d) := $$(addprefix $$(GEN_DIR_$(d)), $$(gm_$(d)))

    # Compiled MOC object files
    om_$(d) := $$(addsuffix .moc.o, $$(subst $(d),,$$(basename $(3))))
    om_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(om_$(d)))

    # Generated QRC source files
    gq_$(d) := $$(addsuffix .rcc.cpp, $$(subst $(d),,$$(basename $(4))))
    gq_$(d) := $$(addprefix $$(GEN_DIR_$(d)), $$(gq_$(d)))

    # Generated QRC object files
    oq_$(d) := $$(addsuffix .rcc.o, $$(subst $(d),,$$(basename $(4))))
    oq_$(d) := $$(addprefix $$(OBJ_DIR_$(d)), $$(oq_$(d)))

    GEN_$$(strip $(1)) += $$(gu_$(d)) $$(gm_$(d)) $$(gq_$(d))
    OBJ_$$(strip $(1)) += $$(om_$(d)) $$(oq_$(d))

    CFLAGS_$(d) += $(QT_CFLAGS) -I$$(GEN_DIR_$(d))
    CXXFLAGS_$(d) += $(QT_CFLAGS) -I$$(GEN_DIR_$(d))
    LDFLAGS_$(d) += $(QT_LDFLAGS)
    LDLIBS_$(d) += $$(addprefix -lQt$(QT_VERSION_MAJOR), $(5))
endif

endef
