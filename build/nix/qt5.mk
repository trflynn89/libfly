# Define build flags for Qt5 projects and define a script to automate installing
# the Qt5 SDK.

# Qt5 flags
QT5_DIR := /opt/Qt5
QT5_VERSION := 5.11.2

QT5_BIN := $(QT5_DIR)/$(QT5_VERSION)/gcc_64/bin
QT5_INC := $(QT5_DIR)/$(QT5_VERSION)/gcc_64/include
QT5_LIB := $(QT5_DIR)/$(QT5_VERSION)/gcc_64/lib

QT5_UIC := $(QT5_BIN)/uic
QT5_MOC := $(QT5_BIN)/moc
QT5_RCC := $(QT5_BIN)/rcc

QT5_CFLAGS := -fPIC -I$(QT5_INC) -I$(QT5_INC)/QtWidgets -I$(QT5_INC)/QtGui -I$(QT5_INC)/QtCore
QT5_LDFLAGS := -Wl,-rpath,$(QT5_LIB) -L$(QT5_LIB)
QT5_LDLIBS := -lQt5Widgets -lQt5Gui -lQt5Core

QT5_INSTALLER := qt-unified-linux-x64-online.run
QT5_INSTALLER_URL := http://qt.mirror.constant.com/official_releases/online_installers/$(QT5_INSTALLER)

# Script to install Qt5 without any user interaction. Downloads the Qt5
# installation binary and creates a javascript file to facilitate desired
# selections in the installer.
define QT5_INSTALL

    curl -O $(QT5_INSTALLER_URL); \
    if [[ $$? -ne 0 ]] ; then \
        $(RM) $(QT5_INSTALLER); \
        exit 1; \
    fi; \
    \
    QT5_SCRIPT="$$(mktemp /tmp/qt5.XXXXX)"; \
    \
    echo " \
    var LOCATION = '$(QT5_DIR)'; \
    var VERSION = '$(QT5_VERSION)'.replace(/\./g, ''); \
    \
    function Controller() \
    { \
        installer.autoRejectMessageBoxes(); \
        installer.installationFinished.connect(function() \
        { \
            gui.clickButton(buttons.NextButton); \
        }); \
    } \
    \
    Controller.prototype.WelcomePageCallback = function() \
    { \
        gui.clickButton(buttons.NextButton, 3000); \
    } \
    \
    Controller.prototype.CredentialsPageCallback = function() \
    { \
        gui.clickButton(buttons.NextButton); \
    } \
    \
    Controller.prototype.IntroductionPageCallback = function() \
    { \
        gui.clickButton(buttons.NextButton); \
    } \
    \
    Controller.prototype.LicenseAgreementPageCallback = function() \
    { \
        var widget = gui.currentPageWidget(); \
    \
        if (widget != null) \
        { \
            widget.AcceptLicenseRadioButton.setChecked(true); \
        } \
    \
        gui.clickButton(buttons.NextButton); \
    } \
    \
    Controller.prototype.TargetDirectoryPageCallback = function() \
    { \
        gui.currentPageWidget().TargetDirectoryLineEdit.setText(LOCATION); \
        gui.clickButton(buttons.NextButton); \
    } \
    \
    Controller.prototype.ComponentSelectionPageCallback = function() \
    { \
        var widget = gui.currentPageWidget(); \
    \
        widget.deselectAll(); \
        widget.selectComponent('qt.qt5.' + VERSION + '.gcc_64'); \
    \
        gui.clickButton(buttons.NextButton); \
    } \
    \
    Controller.prototype.StartMenuDirectoryPageCallback = function() \
    { \
        gui.clickButton(buttons.NextButton); \
    } \
    \
    Controller.prototype.ReadyForInstallationPageCallback = function() \
    { \
        gui.clickButton(buttons.NextButton); \
    } \
    \
    Controller.prototype.FinishedPageCallback = function() \
    { \
        var checkBoxForm = gui.currentPageWidget().LaunchQtCreatorCheckBoxForm; \
    \
        if (checkBoxForm && checkBoxForm.launchQtCreatorCheckBox) \
        { \
            checkBoxForm.launchQtCreatorCheckBox.checked = false; \
        } \
    \
        gui.clickButton(buttons.FinishButton); \
    } \
    " >> $$QT5_SCRIPT; \
    \
    chmod +x $(QT5_INSTALLER); \
    sudo ./$(QT5_INSTALLER) --script $$QT5_SCRIPT; \
    \
    $(RM) $(QT5_INSTALLER) $$QT5_SCRIPT

endef
