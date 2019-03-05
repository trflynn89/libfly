#pragma once

#include "test/qt5/main_window.uic.h"

#include <QMainWindow>
#include <memory>

namespace fly {

/**
 * Simple QMainWindow implementation to create an empty window.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version October 4, 2018
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT;

public:
    explicit MainWindow(QWidget * = 0);

private:
    std::unique_ptr<Ui::MainWindow> m_upUi;
};

} // namespace fly
