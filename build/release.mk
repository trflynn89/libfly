# Define steps for making a release target.

REL_ROOT_DIR := $(ETC_DIR)/root

REL_BIN_DIR := $(REL_ROOT_DIR)$(INSTALL_BIN_DIR)
REL_LIB_DIR := $(REL_ROOT_DIR)$(INSTALL_LIB_DIR)
REL_INC_DIR := $(REL_ROOT_DIR)$(INSTALL_INC_DIR)
REL_SRC_DIR := $(REL_ROOT_DIR)$(INSTALL_SRC_DIR)

REL_CONF :=

COMMA := ,

# Build the OS-specific release package
ifeq ($(OS), redhat)
    define BUILD_REL
        $(REL_CMDS)

        rpmbuild -bb --quiet \
            --define "_topdir $(ETC_DIR)/rpm" \
            --define "version $(VERSION)" \
            --buildroot $(REL_ROOT_DIR) \
            --target $(arch) \
            $(REL_CONF)

        mv -f $(ETC_DIR)/rpm/RPMS/$(arch)/* $(ETC_DIR)

        $(RM) -r $(ETC_DIR)/rpm $(REL_ROOT_DIR)
    endef
else ifeq ($(OS), debian)
    define BUILD_REL
        echo "Debian release packaging not yet supported"
        exit 1
    endef
endif

# Variable to store the list of files to be added to the release package
REL_CMDS := $(RM) -r $(ETC_DIR) && mkdir -p $(REL_BIN_DIR) $(REL_LIB_DIR) $(REL_INC_DIR)

# Define the release config file to use.
# $(1) = The path to the release config spec.
define SET_REL_CONF

REL_CONF := $(1)

endef

# Add a command to be run before building the release package.
# $(1) = The command to run.
define ADD_REL_CMD

REL_CMDS := $(REL_CMDS) && $(1)

endef

# Add a binary file to the release package. The file will be made executable and
# have its symbols stripped.
# $(1) = The path to the binary.
define ADD_REL_BIN

$(eval $(call ADD_REL_CMD, cp -f $(1) $(REL_BIN_DIR)))
$(eval $(call ADD_REL_CMD, strip -s $(REL_BIN_DIR)/$(notdir $(1))))
$(eval $(call ADD_REL_CMD, chmod 755 $(REL_BIN_DIR)/$(notdir $(1))))

endef

# Add a shared library file to release package. The file will have its symbols
# stripped.
# $(1) = The path to the library.
define ADD_REL_LIB

$(eval $(call ADD_REL_CMD, cp -f $(1) $(REL_LIB_DIR)))
$(eval $(call ADD_REL_CMD, strip -s $(REL_LIB_DIR)/$(notdir $(1))))

endef

# Add all header files under a directory to the release package.
# $(1) = The path to the directory.
# $(2) = Header file extension.
define ADD_REL_INC

$(eval $(call ADD_REL_CMD, rsync -am --include='$(strip $(2))' -f 'hide$(COMMA)! */' $(1) $(REL_INC_DIR)))

endef

# Add all header files under a directory to the release package.
# $(1) = The path to the directory.
# $(2) = Source file extension.
define ADD_REL_SRC

$(eval $(call ADD_REL_CMD, rsync -am --include='$(strip $(2))' -f 'hide$(COMMA)! */' $(1) $(REL_SRC_DIR)))

endef
