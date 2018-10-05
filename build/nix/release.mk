# Define steps for making a release target.

REL_BIN_DIR := $(ETC_DIR)$(INSTALL_BIN_DIR)
REL_LIB_DIR := $(ETC_DIR)$(INSTALL_LIB_DIR)
REL_INC_DIR := $(ETC_DIR)$(INSTALL_INC_DIR)
REL_SRC_DIR := $(ETC_DIR)$(INSTALL_SRC_DIR)

COMMA := ,

# Build the release package
define BUILD_REL

    if [[ -z "$(REL_CMDS)" ]] ; then \
        exit 0; \
    fi; \
    \
    echo "[Package $(subst $(CURDIR)/,,$@)]"; \
    \
    $(RM) -r $(ETC_DIR) && \
    mkdir -p $(REL_BIN_DIR) $(REL_LIB_DIR) $(REL_INC_DIR) $(REL_SRC_DIR) \
    \
    $(REL_CMDS) && \
    cd $(ETC_DIR) && \
    \
    for f in `find .$(INSTALL_LIB_DIR) -type f -name "*\.so\.*\.*\.*"` ; do \
        src=$${f:1}; \
        dst=$${src%.*}; \
        ext=$${src##*.}; \
        \
        while [[ "$$ext" != "so" ]] ; do \
            ln -sf $$src .$$dst; \
            \
            src=$$dst; \
            dst=$${dst%.*}; \
            ext=$${src##*.}; \
        done; \
    done; \
    \
    if [[ $$? -ne 0 ]] ; then \
        exit 1; \
    fi; \
    \
    echo "#!/usr/bin/env bash" > $(REL_BIN_DIR)/uninstall_$(REL_NAME); \
    chmod 755 $(REL_BIN_DIR)/uninstall_$(REL_NAME); \
    \
    for f in `find . ! -type d` ; do \
        echo $(RM) $${f:1} >> $(REL_BIN_DIR)/uninstall_$(REL_NAME); \
    done; \
    \
    tar --remove-files $(TAR_CREATE_FLAGS) $@ *

endef

# Add a command to be run before building the release package.
#
# $(1) = The target's name.
# $(2) = The command to run.
define ADD_REL_CMD

REL_CMDS_$(strip $(1)) := $(REL_CMDS_$(strip $(1))) && $(2)

endef

# Add a binary file to the release package. The file will be made executable and
# have its symbols stripped.
#
# $(1) = The target's name.
define ADD_REL_BIN

t := $(strip $(1))

REL_NAME_$(t) := $(1)
$(eval $(call ADD_REL_CMD, $(1), cp -f $(BIN_DIR)/$(t) $(REL_BIN_DIR)))
$(eval $(call ADD_REL_CMD, $(1), strip -s $(REL_BIN_DIR)/$(t)))
$(eval $(call ADD_REL_CMD, $(1), chmod 755 $(REL_BIN_DIR)/$(t)))

endef

# Add a shared library file to release package. The file will have its symbols
# stripped.
#
# $(1) = The target's name.
define ADD_REL_LIB

t := $(strip $(1))

REL_NAME_$(t) := $(1)
$(eval $(call ADD_REL_CMD, $(1), cp -f $(LIB_DIR)/$(t).* $(REL_LIB_DIR)))
$(eval $(call ADD_REL_CMD, $(1), strip -s $(REL_LIB_DIR)/$(t).*))

endef

# Add all header files under a directory to the release package.
#
# $(1) = The target's name.
# $(2) = The path to the directory.
# $(3) = Header file extension.
define ADD_REL_INC

REL_NAME_$(strip $(1)) := $(1)
$(eval $(call ADD_REL_CMD, $(1), rsync -am --include='$(strip $(3))' -f 'hide$(COMMA)! */' $(2) $(REL_INC_DIR)))

endef

# Add all header files under a directory to the release package.
#
# $(1) = The target's name.
# $(2) = The path to the directory.
# $(3) = Source file extension.
define ADD_REL_SRC

REL_NAME_$(strip $(1)) := $(1)
$(eval $(call ADD_REL_CMD, $(1), rsync -am --include='$(strip $(3))' -f 'hide$(COMMA)! */' $(2) $(REL_SRC_DIR)))

endef
