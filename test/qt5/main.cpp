#include "test/qt5/main_window.h"

#include <QApplication>
#include <QLabel>
#include <QtGui>

extern "C"
{
    // AddressSanitizer reports leaks in libfontconfig. Override the default
    // suppressions to disable leak checking in that library.
    const char *__lsan_default_suppressions()
    {
        return R"(
            leak:libfontconfig
        )";
    }

    // And do not print that suppressions.
    const char *__asan_default_options()
    {
        return R"(
            print_suppressions=0
        )";
    }
}

//==============================================================================
int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    fly::MainWindow window;

    QLabel label(&window);
    label.setPixmap(QPixmap(":/green.png"));

    window.show();

    return application.exec();
}
