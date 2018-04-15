# Define functions which will declare object, dependency, and executable files.

# A list of all object files that have been compiled
OBJS :=

# A list of all UIC/MOC/QT5 files
QT5_UICS :=
QT5_MOCS :=
QT5_RCCS :=

# A list of system calls to mock
MOCK_SYSTEM_CALLS :=

# Set a source directory's source files to all C/C++ files.
define WILDCARD_SOURCES

SRC_$(d) := \
    $(wildcard $(d)/*.c) \
    $(wildcard $(d)/*.cc) \
    $(wildcard $(d)/*.cpp)

endef

# Add a system call to be mocked for unit tests.
# $(1) = The system call to mock.
define MOCK_SYSTEM_CALL

MOCK_SYSTEM_CALLS += $$(strip $(1))

LDFLAGS_$(d) += \
    -Wl,--wrap=$$(strip $(1))

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
# $(2) = The QT5 UIC source files.
# $(3) = The QT5 MOC source files.
# $(4) = The QT5 RCC source files.
define QT5_OUT_FILES

$(call OBJ_OUT_FILES, $(1))

QT5_UIC_$(d) := $$(addprefix $(GEN_DIR)/, $$(addsuffix .uic.h, $$(strip $(2))))
QT5_UICS += $$(QT5_UIC_$(d))

QT5_MOC_$(d) := $$(addprefix $$(OBJ_DIR_$(d))/, $$(addsuffix .moc.o, $$(strip $(3))))
QT5_MOCS += $$(QT5_MOC_$(d))

QT5_RCC_$(d) := $$(addprefix $$(OBJ_DIR_$(d))/, $$(addsuffix .rcc.o, $$(strip $(4))))
QT5_RCCS += $$(QT5_RCC_$(d))

CLEAN_$(d) += $$(QT5_UIC_$(d)) $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))
OBJ_$(d) += $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))
OBJS += $$(QT5_MOC_$(d)) $$(QT5_RCC_$(d))

endef

# Define output files for library targets.
# $(1) = The C/C++ files to be compiled.
define LIB_OUT_FILES

$(call OBJ_OUT_FILES, $(1))

endef
