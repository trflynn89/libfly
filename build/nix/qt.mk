# Define build flags for Qt projects and define a script to automate installing
# the Qt SDK.

ifeq ($(wildcard $(BUILD_ROOT)/qt.js),)
    QT_SCRIPT := $(BUILD_ROOT)/../ci/qt.js
else
    QT_SCRIPT := $(BUILD_ROOT)/qt.js
endif

QT_VERSION_MAJOR := $(shell grep -oP "(?<=MAJOR = ')(\d+)(?=')" $(QT_SCRIPT))
QT_VERSION_MINOR := $(shell grep -oP "(?<=MINOR = ')(\d+)(?=')" $(QT_SCRIPT))
QT_VERSION_PATCH := $(shell grep -oP "(?<=PATCH = ')(\d+)(?=')" $(QT_SCRIPT))
QT_INSTALL_POINT := $(shell grep -oP "(?<=POINT_LINUX = ')(\S+)(?=')" $(QT_SCRIPT))

QT_VERSION := $(QT_VERSION_MAJOR).$(QT_VERSION_MINOR).$(QT_VERSION_PATCH)

QT_BIN := $(QT_INSTALL_POINT)/$(QT_VERSION)/gcc_64/bin
QT_INC := $(QT_INSTALL_POINT)/$(QT_VERSION)/gcc_64/include
QT_LIB := $(QT_INSTALL_POINT)/$(QT_VERSION)/gcc_64/lib

QT_UIC := $(QT_BIN)/uic
QT_MOC := $(QT_BIN)/moc
QT_RCC := $(QT_BIN)/rcc

QT_CFLAGS := \
    -fPIC \
    -isystem $(QT_INC)

QT_LDFLAGS := \
    -Wl,-rpath,$(QT_LIB) \
    -L$(QT_LIB)

QT_INSTALLER_URL := http://qt.mirror.constant.com/official_releases/online_installers
QT_INSTALLER_EXE := qt-unified-linux-$(arch)-online.run

ifeq ($(verbose), 1)
    QT_INSTALL_FLAGS := --verbose
else
    QT_INSTALL_FLAGS := --platform minimal
endif

# Script to install Qt without any user interaction.
define QT_INSTALL

    (cd /tmp && curl -O $(QT_INSTALLER_URL)/$(QT_INSTALLER_EXE)); \
    if [[ $$? -ne 0 ]] ; then \
        $(RM) /tmp/$(QT_INSTALLER_EXE); \
        exit 1; \
    fi; \
    \
    chmod +x /tmp/$(QT_INSTALLER_EXE); \
    $(SUDO) $(RM) -r $(QT_INSTALL_POINT); \
    $(SUDO) /tmp/$(QT_INSTALLER_EXE) $(QT_INSTALL_FLAGS) --script $(QT_SCRIPT); \
    \
    $(RM) /tmp/$(QT_INSTALLER_EXE)

endef
