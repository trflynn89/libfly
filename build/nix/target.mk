# Verify targets added via api.mk and define make goals for each of the targets.

# List of all target release packages
TARGET_PACKAGES :=

# List of all test target output binaries
TEST_BINARIES :=

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
else ifeq ($$(TARGET_TYPE_$$(t)), LIB)
    TARGET_FILE_$$(t) := $(LIB_DIR)/$$(t).so.$(VERSION)
    TARGET_FILE_$$(t) += $(LIB_DIR)/$$(t).a
else ifeq ($$(TARGET_TYPE_$$(t)), MVN)
    TARGET_FILE_$$(t) := $(MVN_DIR)/$$(t)-$(VERSION).jar
else
    $$(error Target type $$(TARGET_TYPE_$$(t)) not supported)
endif

# Define the path to the target release package
TARGET_PACKAGE_$$(t) := $(ETC_DIR)/$$(t)-nix-$(VERSION).$(arch).tar.bz2
TARGET_PACKAGES += $$(TARGET_PACKAGE_$$(t))

# Define the make goal to build the targets
$$(t): $$(TARGET_FILE_$$(t)) $$(TARGET_PACKAGE_$$(t))

# Define the compilation goals for the target
ifeq ($$(TARGET_TYPE_$$(t)), BIN)
    $(call DEFINE_BIN_RULES, $$(t), $$(TARGET_PATH_$$(t)), \
        $$(TARGET_FILE_$$(t)), $$(TARGET_PACKAGE_$$(t)))
else ifeq ($$(TARGET_TYPE_$$(t)), LIB)
    $(call DEFINE_LIB_RULES, $$(t), $$(TARGET_PATH_$$(t)), \
        $$(TARGET_FILE_$$(t)), $$(TARGET_PACKAGE_$$(t)))
else ifeq ($$(TARGET_TYPE_$$(t)), MVN)
    $(call DEFINE_MVN_RULES, $$(t), $$(TARGET_PATH_$$(t)), \
        $$(TARGET_FILE_$$(t)), $$(TARGET_PACKAGE_$$(t)))
endif

endef

$(foreach target, $(TARGETS), $(eval $(call DEFINE_TARGET, $(target))))
