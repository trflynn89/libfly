# Define steps for making a release target.

COMMA := ,

# Build the release package
define BUILD_REL

    if [[ -z "$(REL_CMDS)" ]] ; then \
        exit 0; \
    fi; \
    \
    echo -e "[$(YELLOW)Package$(DEFAULT) $(subst $(CURDIR)/,,$@)]"; \
    \
    $(RM) -r $(ETC_TMP_DIR) && \
    mkdir -p $(REL_BIN_DIR) $(REL_LIB_DIR) $(REL_INC_DIR) $(REL_SRC_DIR) \
    \
    $(REL_CMDS) && \
    cd $(ETC_TMP_DIR) && \
    \
    for f in `find .$(INSTALL_LIB_DIR) -type f -name "*\.so\.*\.*\.*" -o -name "*\.dylib\.*\.*\.*"` ; do \
        src=$${f:1}; \
        dst=$${src%.*}; \
        ext=$${src##*.}; \
        \
        while [[ "$$ext" != "so" ]] && [[ "$$ext" != "dylib" ]] ; do \
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
    files="$(subst $(ETC_TMP_DIR),,$(REL_UNINSTALL))"; \
    files="$$files $(INSTALL_BIN_DIR)/uninstall_$(REL_NAME)"; \
    echo $(RM) -r $$files >> $(REL_BIN_DIR)/uninstall_$(REL_NAME); \
    \
    tar $(TAR_CREATE_FLAGS) $@ *; \
    $(RM) -r $(ETC_TMP_DIR)

endef

# Set the path and target variables for the release package.
#
# $(1) = The target's name.
define SET_REL_VAR

t := $(strip $(1))

REL_NAME_$$(t) := $$(t)
ETC_TMP_DIR_$$(t) := $(ETC_DIR)/$$(t)
REL_BIN_DIR_$$(t) := $$(ETC_TMP_DIR_$$(t))$(INSTALL_BIN_DIR)
REL_LIB_DIR_$$(t) := $$(ETC_TMP_DIR_$$(t))$(INSTALL_LIB_DIR)
REL_INC_DIR_$$(t) := $$(ETC_TMP_DIR_$$(t))$(INSTALL_INC_DIR)
REL_SRC_DIR_$$(t) := $$(ETC_TMP_DIR_$$(t))$(INSTALL_SRC_DIR)

endef

# Add a command to be run before building the release package.
#
# $(1) = The target's name.
# $(2) = The command to run.
define ADD_REL_CMD

t := $(strip $(1))

REL_CMDS_$(t) := $(REL_CMDS_$(t)) && $(2)

endef

# Add a binary file to the release package. The file will be made executable and
# have its symbols stripped.
#
# $(1) = The target's name.
define ADD_REL_BIN

t := $(strip $(1))

$(eval $(call SET_REL_VAR, $(1)))
$(eval $(call ADD_REL_CMD, $(1), cp -f $(BIN_DIR)/$(t) $(REL_BIN_DIR_$(t))))
$(eval $(call ADD_REL_CMD, $(1), $(STRIP) $(STRIP_FLAGS) $(REL_BIN_DIR_$(t))/$(t)))
$(eval $(call ADD_REL_CMD, $(1), chmod 755 $(REL_BIN_DIR_$(t))/$(t)))

REL_UNINSTALL_$(t) += $(REL_BIN_DIR_$(t))/$(t)

endef

# Add a shared library file to release package. The file will have its symbols
# stripped.
#
# $(1) = The target's name.
define ADD_REL_LIB

t := $(strip $(1))

$(eval $(call SET_REL_VAR, $(1)))
$(eval $(call ADD_REL_CMD, $(1), cp -f $(LIB_DIR)/$(t).* $(REL_LIB_DIR_$(t))))
$(eval $(call ADD_REL_CMD, $(1), $(STRIP) $(STRIP_FLAGS) $(REL_LIB_DIR_$(t))/$(t).*))

REL_UNINSTALL_$(t) += $(REL_LIB_DIR_$(t))/$(t)*

endef

# Add all header files under a directory to the release package.
#
# $(1) = The target's name.
# $(2) = The path to the directory.
# $(3) = Header file extension.
# $(4) = Header file destination.
define ADD_REL_INC

t := $(strip $(1))

$(eval $(call SET_REL_VAR, $(1)))
$(eval $(call ADD_REL_CMD, $(1), \
    rsync -am --include='$(strip $(3))' -f 'hide$(COMMA)! */' \
    $(2)/ $(REL_INC_DIR_$(t))/$(strip $(4))))

REL_UNINSTALL_$(t) += $(REL_INC_DIR_$(t))/$(strip $(4))

endef

# Add all source files under a directory to the release package.
#
# $(1) = The target's name.
# $(2) = The path to the directory.
# $(3) = Source file extension.
# $(4) = Source file destination.
define ADD_REL_SRC

t := $(strip $(1))

$(eval $(call SET_REL_VAR, $(1)))
$(eval $(call ADD_REL_CMD, $(1), \
    rsync -am --include='$(strip $(3))' -f 'hide$(COMMA)! */' \
    $(2)/ $(REL_SRC_DIR_$(t))/$(strip $(4))))

REL_UNINSTALL_$(t) += $(REL_SRC_DIR_$(t))/$(strip $(4))

endef
