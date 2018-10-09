#include <QApplication>
#include <QLabel>
#include <QtGui>

#include "test/qt5/main_window.h"

#if defined(FLY_USE_SANITIZER)

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * AddressSanitizer catches SIGSEGV by default. Override the default options to
 * allow a user-specified handler.
 */
const char *__asan_default_options()
{
    return R"(
        allow_user_segv_handler=1
        print_suppressions=0
    )";
}

/**
 * AddressSanitizer reports leaks in several external libraries. Override the
 * default suppressions to disable leak checking in those libraries.
 */
const char *__lsan_default_suppressions()
{
    return R"(
        leak:libdbus-1
        leak:libfontconfig
        leak:vmwgfx_dri
    )";
}

#ifdef __cplusplus
}
#endif

#endif

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