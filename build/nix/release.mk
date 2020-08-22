# Define steps for generating an archived release package for a target. The APIs defined below may
# be used to add a file to the release package, or execute a command while creating the package. If
# none of these APIs are used, no release package will be created.
#
# Any shared libraries added to the release package will also include a chain of symbolic links to
# which resolve to the real, versioned library defined by the application's Makefile. For example,
# if the Makefile defines $(VERSION) as "1.0.0" and invokes $(ADD_TARGET) for a LIB target with the
# name "libfly", the following symbolic link chain will be created under $(INSTALL_LIB_DIR):
#
#     libfly.so -> libfly.so.1 -> libfly.so.1.0 -> libfly.so.1.0.0 (the real file)
#
# The release package will also include an uninstall script with a simple $(RM) command to remove
# all files included in the release package.

# A literal comma.
COMMA := ,

# Build the release package.
define BUILD_REL

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
    echo $(SUDO) $(RM) -r $$files >> $(REL_BIN_DIR)/uninstall_$(REL_NAME); \
    \
    tar $(TAR_CREATE_FLAGS) $@ *; \
    $(RM) -r $(ETC_TMP_DIR)

endef

# Set the path and target variables for the release package.
define SET_REL_VAR

REL_NAME_$(t) := $(t)
ETC_TMP_DIR_$(t) := $(ETC_DIR)/$(t)
REL_BIN_DIR_$(t) := $$(ETC_TMP_DIR_$(t))$(INSTALL_BIN_DIR)
REL_LIB_DIR_$(t) := $$(ETC_TMP_DIR_$(t))$(INSTALL_LIB_DIR)
REL_INC_DIR_$(t) := $$(ETC_TMP_DIR_$(t))$(INSTALL_INC_DIR)
REL_SRC_DIR_$(t) := $$(ETC_TMP_DIR_$(t))$(INSTALL_SRC_DIR)

endef

# Add a command to be run while building the release package.
#
# $(1) = The command to run.
define ADD_REL_CMD

REL_CMDS_$(t) := $(REL_CMDS_$(t)) && $(1)

endef

# Add a binary file to the release package. The file will be made executable and have its symbols
# stripped.
define ADD_REL_BIN

$(eval $(call SET_REL_VAR, $(t)))
$(eval $(call ADD_REL_CMD, cp -f $(BIN_DIR)/$(t) $(REL_BIN_DIR_$(t))))
$(eval $(call ADD_REL_CMD, $(STRIP) $(STRIP_FLAGS) $(REL_BIN_DIR_$(t))/$(t)))
$(eval $(call ADD_REL_CMD, chmod 755 $(REL_BIN_DIR_$(t))/$(t)))

REL_UNINSTALL_$(t) += $(REL_BIN_DIR_$(t))/$(t)

endef

# Add a shared library file to release package. The file will have its symbols stripped.
define ADD_REL_LIB

$(eval $(call SET_REL_VAR, $(t)))
$(eval $(call ADD_REL_CMD, cp -f $(LIB_DIR)/$(t).* $(REL_LIB_DIR_$(t))))
$(eval $(call ADD_REL_CMD, $(STRIP) $(STRIP_FLAGS) $(REL_LIB_DIR_$(t))/$(t).*))

REL_UNINSTALL_$(t) += $(REL_LIB_DIR_$(t))/$(t)*

endef

# Add all header files under a directory to the release package.
#
# $(1) = The path to the directory.
# $(2) = Header file extension.
# $(3) = Header file destination under $(INSTALL_INC_DIR).
define ADD_REL_INC

$(eval $(call SET_REL_VAR, $(t)))
$(eval $(call ADD_REL_CMD, \
    rsync -am --include='$(strip $(2))' -f 'hide$(COMMA)! */' \
    $(1)/ $(REL_INC_DIR_$(t))/$(strip $(3))))

REL_UNINSTALL_$(t) += $(REL_INC_DIR_$(t))/$(strip $(3))

endef

# Add all source files under a directory to the release package.
#
# $(1) = The path to the directory.
# $(2) = Source file extension.
# $(3) = Source file destination under $(INSTALL_SRC_DIR).
define ADD_REL_SRC

$(eval $(call SET_REL_VAR, $(t)))
$(eval $(call ADD_REL_CMD, \
    rsync -am --include='$(strip $(2))' -f 'hide$(COMMA)! */' \
    $(1)/ $(REL_SRC_DIR_$(t))/$(strip $(3))))

REL_UNINSTALL_$(t) += $(REL_SRC_DIR_$(t))/$(strip $(3))

endef
