#include <QtWidgets/QApplication>

#include "notepad.h"

#if defined(_WIN32)
// Requires "Qt Visual Studio Tools" extension:
// https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019
//
// Once installed, in VS, open Extensions > Qt VS Tools > Qt Options. Add both
// the installation:
//      Name: Qt_5.13.2, Path: C:\Qt\5.13.2\msvc2017_64
//
// In new Qt projects, change the following project properties:
//      Qt Project Settings > Qt Installation = Qt_5.13.2
//      Qt Project Settings > Qt Modules = core;gui;printsupport;widgets
//      Qt Meta-Object Compiler > Output File Name = %(Filename).moc.cpp
//      Qt Resource Compiler > Output File Name = %(Filename).rcc.cpp
//      Qt User Interface Compiler > Output File Name = %(Filename).uic.h
#elif defined(__linux__)
extern "C"
{
    // AddressSanitizer reports leaks from some system libraries. Override the
    // default suppressions to disable leak checking in that library.
    const char *__lsan_default_suppressions()
    {
        return R"(
            leak:libfontconfig
            leak:libglib
            leak:libGLX_mesa
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
    QApplication application(argc, argv);

    Notepad notepad;
    notepad.show();

    return application.exec();
}
