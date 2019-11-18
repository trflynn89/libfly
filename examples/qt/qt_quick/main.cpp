#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

#if defined(_WIN32)
// Requires "Qt Visual Studio Tools" extension:
// https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019
//
// Once installed, in VS, open Extensions > Qt VS Tools > Qt Options. Add both
// the installation:
//      Name: Qt_5.13.2, Path: C:\Qt\5.13.2\msvc2017_64
//
// In new Qt projects, change the following project properties:
//      Qt Project Settings > Qt Installation = Qt_5.13.2_$(PlatformTarget)
//      Qt Project Settings > Qt Modules = core;gui;qml
//      Qt Resource Compiler > Output File Name = %(Filename).rcc.cpp
#elif defined(__linux__)
extern "C"
{
    // AddressSanitizer reports leaks from some system libraries. Override the
    // default suppressions to disable leak checking in that library.
    const char *__lsan_default_suppressions()
    {
        return R"(
            leak:libfontconfig
            leak:libQt5Core
            leak:libQt5Qml
            leak:libQt5QuickTemplates2
        )";
    }

    // And do not print suppressions from those libraries.
    const char *__asan_default_options()
    {
        return R"(
            print_suppressions=0
        )";
    }
}
#endif

//==============================================================================
int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication application(argc, argv);
    QQmlApplicationEngine engine(QUrl("qrc:///clocks.qml"));

    return application.exec();
}
