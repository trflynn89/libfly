#include "test/qt5/main_window.h"

namespace fly {

//==============================================================================
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_upUi(std::make_unique<Ui::MainWindow>())
{
    m_upUi->setupUi(this);
}

} // namespace fly
