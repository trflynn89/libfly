#include <QApplication>
#include <QLabel>
#include <QtGui>

#include "test/qt5/main_window.h"

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
