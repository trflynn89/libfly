#include <QtWidgets/QApplication>

#include "notepad.h"

#if defined(__linux__)
extern "C"
{
    const char *__lsan_default_suppressions();
    const char *__asan_default_options();

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
