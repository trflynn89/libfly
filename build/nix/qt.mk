# Define build flags for Qt projects and define a script to automate installing
# the Qt SDK.

ifeq ($(wildcard $(BUILD_ROOT)/qt.js),)
    QT_SCRIPT := $(BUILD_ROOT)/../common/qt.js
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
    -I$(QT_INC)

QT_LDFLAGS := -Wl,-rpath,$(QT_LIB) -L$(QT_LIB)
QT_LDLIBS := \
    -lQt$(QT_VERSION_MAJOR)Widgets \
    -lQt$(QT_VERSION_MAJOR)Gui \
    -lQt$(QT_VERSION_MAJOR)Core

ifeq ($(qt), 1)

# The x86 Qt installer has not been updated since 2017, and only supports up to
# Qt 5.5. But Qt 5.13 is needed for C++17, so only x64 Qt projects are allowed.
ifeq ($(arch), x86)
    $(error Architecture $(arch) not supported, check qt.mk)
endif

QT_INSTALLER := qt-unified-linux-$(arch)-online.run
QT_INSTALLER_URL := http://qt.mirror.constant.com/official_releases/online_installers/$(QT_INSTALLER)

ifeq ($(verbose), 1)
    QT_INSTALL_FLAGS := --verbose
else
    QT_INSTALL_FLAGS := --platform minimal
endif

# Script to install Qt without any user interaction.
define QT_INSTALL

    (cd /tmp && curl -O $(QT_INSTALLER_URL)); \
    if [[ $$? -ne 0 ]] ; then \
        $(RM) /tmp/$(QT_INSTALLER); \
        exit 1; \
    fi; \
    \
    chmod +x /tmp/$(QT_INSTALLER); \
    $(SUDO) $(RM) -r $(QT_INSTALL_POINT); \
    $(SUDO) /tmp/$(QT_INSTALLER) $(QT_INSTALL_FLAGS) --script $(QT_SCRIPT); \
    \
    $(RM) /tmp/$(QT_INSTALLER)

endef

endif
