#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

#if defined(__linux__)
extern "C"
{
    // AddressSanitizer reports leaks from some system libraries. Override the
    // default suppressions to disable leak checking in that library.
    const char *__lsan_default_suppressions()
    {
        return R"(
            leak:libfontconfig
            leak:libGLX_mesa
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
