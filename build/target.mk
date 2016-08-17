# Verify targets added via api.mk, and verify current target.

SUPPORTED_TARGET_TYPES := BIN LIB

# Make sure target is defined
ifeq ($(target),)
    $(error Target must be defined, possibly with SET_DEFAULT_TARGET)
endif

# Make sure target was known
ifeq ($(TARGET_PATH),)
    $(error Unknown target $(target))
endif

# Make sure target type is supported
ifneq ($(TARGET_TYPE), $(filter $(SUPPORTED_TARGET_TYPES), $(TARGET_TYPE)))
    $(error Target type $(TARGET_TYPE) not one of: $(SUPPORTED_TARGET_TYPES))
endif

# Target binary / library
ifeq ($(TARGET_TYPE), BIN)
    TARGET_NAME := $(BIN_DIR)/$(target)
else ifeq ($(TARGET_TYPE), LIB)
    ifeq ($(release), 1)
        TARGET_NAME := $(LIB_DIR)/$(target).so.$(VERSION)
    else
        TARGET_NAME := $(LIB_DIR)/$(target).a
    endif
endif

# Target release package
TARGET_PACKAGE := $(ETC_DIR)/$(target)-$(VERSION).$(arch).tar.bz2
