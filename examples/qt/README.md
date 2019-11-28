# Qt Build Integration Examples

These examples serve to demonstrate Qt integration with the libfly build system.
Though really for Linux, a Visual Studio solution is provided for Windows.

Both Qt Quick and Qt Widgets projects are supported and demonstrated.

## Installing Qt

Automated installers are provided using a [Qt Controller Script](https://doc.qt.io/qtinstallerframework/noninteractive.html)
located [here](../build/ci/qt.js).

### Linux

Automated installation of Qt on Linux is provided by the command:

    make -C libfly/build/nix qt=1 setup

By default, Qt is not installed with `make setup`. Adding `qt=1` to the `setup`
target enables Qt.

### Windows

Automated installation of Qt on Windows is provided by the command:

    libfly\build\win\qt.ps1

Once Qt is installed, also install the [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019)
extension. Once installed, open Extensions > Qt VS Tools > Qt Options and add
the Qt installation with the following options (adapted for the version actually
installed):

    Name: Qt_5.13.2
    Path: C:\Qt\5.13.2\msvc2017_64

## qt_quick

This Qt Quick example is adapted from Qt's clocks QML example. It depends on the
following components:

    Qt5Core
    Qt5Gui
    Qt5Qml

## qt_widget

This Qt Widgets example is adapted from Qt's notepad widget example. It depends
on the following components:

    Qt5Core
    Qt5Gui
    Qt5PrintSupport
    Qt5Widgets
