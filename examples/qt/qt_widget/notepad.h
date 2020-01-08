#pragma once

#include <QtCore/QString>
#include <QtWidgets/QMainWindow>

#include <memory>

namespace Ui {
class Notepad;
} // namespace Ui

class Notepad : public QMainWindow
{
    Q_OBJECT

public:
    explicit Notepad(QWidget *parent = nullptr);
    ~Notepad();

private slots:
    void newDocument();
    void open();
    void save();
    void saveAs();
    void print();
    void exit();
    void copy();
    void cut();
    void paste();
    void undo();
    void redo();
    void selectFont();
    void setFontBold(bool bold);
    void setFontUnderline(bool underline);
    void setFontItalic(bool italic);
    void about();

private:
    std::unique_ptr<Ui::Notepad> m_upUi;
    QString m_currentFile;
};
