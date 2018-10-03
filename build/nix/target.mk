# Verify targets added via api.mk and define make goals for each of the targets.

# Verify a single target and, if valid, define a make goal to build that target.
#
# $(1) = The target's name.
define DEFINE_TARGET

t := $$(strip $(1))

# Define the path to the target output binary/library
ifeq ($$(TARGET_TYPE_$$(t)), BIN)
    TARGET_FILE_$$(t) := $(BIN_DIR)/$$(t)

    ifneq ($$(filter $(TEST_TARGETS), $$(t)),)
        TEST_BINARIES += $$(TARGET_FILE_$$(t))
    endif
else ifeq ($$(TARGET_TYPE_$$(t)), QT5)
    TARGET_FILE_$$(t) := $(BIN_DIR)/$$(t)
else ifeq ($$(TARGET_TYPE_$$(t)), LIB)
    ifeq ($(release), 1)
        TARGET_FILE_$$(t) := $(LIB_DIR)/$$(t).so.$(VERSION)
    else
        TARGET_FILE_$$(t) := $(LIB_DIR)/$$(t).a
    endif
else
    $$(error Target type $$(TARGET_TYPE_$$(t)) not supported)
endif

# Define the path to the target release package
TARGET_PACKAGE_$$(t) := $(ETC_DIR)/$$(t)-nix-$(VERSION).$(arch).tar.bz2

# Define the make goal to build the target
$$(t): $$(TARGET_FILE_$$(t)) $$(TARGET_PACKAGE_$$(t))

# Include top-level source directory's files.mk file
ifeq ($$(TARGET_TYPE_$$(t)), BIN)
    $(call INCLUDE_BIN_DIR, $$(t), $(SOURCE_ROOT), $$(TARGET_PATH_$$(t)), \
        $$(TARGET_FILE_$$(t)), $$(TARGET_PACKAGE_$$(t)))
else ifeq ($$(TARGET_TYPE_$$(t)), QT5)
    $(call INCLUDE_QT5_DIR, $$(t), $(SOURCE_ROOT), $$(TARGET_PATH_$$(t)), \
        $$(TARGET_FILE_$$(t)), $$(TARGET_PACKAGE_$$(t)))
else ifeq ($$(TARGET_TYPE_$$(t)), LIB)
    $(call INCLUDE_LIB_DIR, $$(t), $(SOURCE_ROOT), $$(TARGET_PATH_$$(t)), \
        $$(TARGET_FILE_$$(t)), $$(TARGET_PACKAGE_$$(t)))
endif

endef

$(foreach target, $(TARGETS), $(eval $(call DEFINE_TARGET, $(target))))
