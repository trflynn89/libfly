// Control script to install Qt without any user interaction into /opt/Qt.
// https://doc.qt.io/qtinstallerframework/noninteractive.html
var QT_VERSION_MAJOR = '5';
var QT_VERSION_MINOR = '13';
var QT_VERSION_PATCH = '1';

var QT_VERSION = QT_VERSION_MAJOR + QT_VERSION_MINOR + QT_VERSION_PATCH;

function Controller()
{
    installer.autoRejectMessageBoxes();
    installer.installationFinished.connect(function()
    {
        gui.clickButton(buttons.NextButton);
    });
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
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ComponentSelectionPageCallback = function()
{
    // First enable installation of the latest Qt versions.
    var page = gui.pageWidgetByObjectName('ComponentSelectionPage');

    gui.findChild(page, 'Archive').click();
    gui.findChild(page, 'Latest releases').click();
    gui.findChild(page, 'FetchCategoryButton').click();

    // Then select the desired version.
    var component = 'qt.qt' + QT_VERSION_MAJOR + '.' + QT_VERSION + '.gcc_64';
    var widget = gui.currentPageWidget();

    if (widget)
    {
        widget.deselectAll();
        widget.selectComponent(component);
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
