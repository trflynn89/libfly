// Control script to install Qt without any user interaction.
// https://doc.qt.io/qtinstallerframework/noninteractive.html
var QT_VERSION_MAJOR = '5';
var QT_VERSION_MINOR = '13';
var QT_VERSION_PATCH = '1';
var QT_VERSION = QT_VERSION_MAJOR + QT_VERSION_MINOR + QT_VERSION_PATCH;

var QT_INSTALL_POINT_WINDOWS = 'C:\\Qt';
var QT_INSTALL_POINT_LINUX = '/opt/Qt';
var QT_INSTALL_POINT = null;

var QT_COMPONENTS = null;

function Controller()
{
    installer.autoRejectMessageBoxes();
    installer.installationFinished.connect(function()
    {
        gui.clickButton(buttons.NextButton);
    });

    if (installer.fileExists('C:'))
    {
        QT_INSTALL_POINT = QT_INSTALL_POINT_WINDOWS;
        QT_COMPONENTS = ['win32_msvc2017', 'win64_msvc2017_64'];
    }
    else
    {
        QT_INSTALL_POINT = QT_INSTALL_POINT_LINUX;
        QT_COMPONENTS = ['gcc_64'];
    }
}

Controller.prototype.WelcomePageCallback = function()
{
    gui.clickButton(buttons.NextButton, 3000);
}

Controller.prototype.CredentialsPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.IntroductionPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.DynamicTelemetryPluginFormCallback = function() {
    var widget = gui.currentPageWidget();

    if (widget)
    {
        var group = widget.TelemetryPluginForm.statisticGroupBox;
        group.disableStatisticRadioButton.setChecked(true);
    }

    gui.clickButton(buttons.NextButton);
}

Controller.prototype.TargetDirectoryPageCallback = function()
{
    gui.currentPageWidget().TargetDirectoryLineEdit.setText(QT_INSTALL_POINT);
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ComponentSelectionPageCallback = function()
{
    // First enable installation of the latest Qt versions.
    var page = gui.pageWidgetByObjectName('ComponentSelectionPage');

    gui.findChild(page, 'Archive').click();
    gui.findChild(page, 'Latest releases').click();
    gui.findChild(page, 'FetchCategoryButton').click();

    // Then select the desired components.
    var widget = gui.currentPageWidget();

    if (widget)
    {
        var component_filter = 'qt.qt' + QT_VERSION_MAJOR + '.' + QT_VERSION;
        widget.deselectAll();

        QT_COMPONENTS.forEach(function(component)
        {
            component = component_filter + '.' + component;
            widget.selectComponent(component);
        });
    }

    gui.clickButton(buttons.NextButton);
}

Controller.prototype.LicenseAgreementPageCallback = function()
{
    var widget = gui.currentPageWidget();

    if (widget)
    {
        widget.AcceptLicenseRadioButton.setChecked(true);
    }

    gui.clickButton(buttons.NextButton);
}

Controller.prototype.StartMenuDirectoryPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ReadyForInstallationPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.FinishedPageCallback = function()
{
    var widget = gui.currentPageWidget();

    if (widget)
    {
        var checkBoxForm = widget.LaunchQtCreatorCheckBoxForm;

        if (checkBoxForm && checkBoxForm.launchQtCreatorCheckBox)
        {
            checkBoxForm.launchQtCreatorCheckBox.checked = false;
        }
    }

    gui.clickButton(buttons.FinishButton);
}
