#include "notepad.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QFont>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFontDialog>
#include <QtWidgets/QMessageBox>

#include "notepad.uic.h"

//==============================================================================
Notepad::Notepad(QWidget *parent) :
    QMainWindow(parent),
    m_upUi(std::make_unique<Ui::Notepad>())
{
    m_upUi->setupUi(this);
    setCentralWidget(m_upUi->textEdit);

    connect(
        m_upUi->actionNew,
        &QAction::triggered,
        this,
        &Notepad::newDocument);
    connect(m_upUi->actionOpen, &QAction::triggered, this, &Notepad::open);
    connect(m_upUi->actionSave, &QAction::triggered, this, &Notepad::save);
    connect(m_upUi->actionSaveAs, &QAction::triggered, this, &Notepad::saveAs);
    connect(m_upUi->actionPrint, &QAction::triggered, this, &Notepad::print);
    connect(m_upUi->actionExit, &QAction::triggered, this, &Notepad::exit);
    connect(m_upUi->actionCopy, &QAction::triggered, this, &Notepad::copy);
    connect(m_upUi->actionCut, &QAction::triggered, this, &Notepad::cut);
    connect(m_upUi->actionPaste, &QAction::triggered, this, &Notepad::paste);
    connect(m_upUi->actionUndo, &QAction::triggered, this, &Notepad::undo);
    connect(m_upUi->actionRedo, &QAction::triggered, this, &Notepad::redo);
    connect(
        m_upUi->actionSelectFont,
        &QAction::triggered,
        this,
        &Notepad::selectFont);
    connect(
        m_upUi->actionSetFontBold,
        &QAction::triggered,
        this,
        &Notepad::setFontBold);
    connect(
        m_upUi->actionSetFontUnderline,
        &QAction::triggered,
        this,
        &Notepad::setFontUnderline);
    connect(
        m_upUi->actionSetFontItalic,
        &QAction::triggered,
        this,
        &Notepad::setFontItalic);
    connect(m_upUi->actionAbout, &QAction::triggered, this, &Notepad::about);
}

//==============================================================================
Notepad::~Notepad()
{
}

//==============================================================================
void Notepad::newDocument()
{
    m_currentFile.clear();
    m_upUi->textEdit->setText(QString());
}

//==============================================================================
void Notepad::open()
{
    m_currentFile = QFileDialog::getOpenFileName(this, "Open the file");
    QFile file(m_currentFile);

    if (!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(
            this,
            "Warning",
            "Cannot open file: " + file.errorString());
        return;
    }

    setWindowTitle(m_currentFile);

    QTextStream in(&file);
    QString text = in.readAll();
    m_upUi->textEdit->setText(text);

    file.close();
}

//==============================================================================
void Notepad::save()
{
    // If we don't have a filename from before, get one.
    if (m_currentFile.isEmpty())
    {
        m_currentFile = QFileDialog::getSaveFileName(this, "Save");
    }

    QFile file(m_currentFile);

    if (!file.open(QIODevice::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(
            this,
            "Warning",
            "Cannot save file: " + file.errorString());
        return;
    }

    setWindowTitle(m_currentFile);

    QTextStream out(&file);
    QString text = m_upUi->textEdit->toPlainText();
    out << text;

    file.close();
}

//==============================================================================
void Notepad::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save as");
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(
            this,
            "Warning",
            "Cannot save file: " + file.errorString());
        return;
    }

    m_currentFile = fileName;
    setWindowTitle(fileName);

    QTextStream out(&file);
    QString text = m_upUi->textEdit->toPlainText();
    out << text;

    file.close();
}

//==============================================================================
void Notepad::print()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    m_upUi->textEdit->print(&printer);
}

//==============================================================================
void Notepad::exit()
{
    QCoreApplication::quit();
}

//==============================================================================
void Notepad::copy()
{
    m_upUi->textEdit->copy();
}

//==============================================================================
void Notepad::cut()
{
    m_upUi->textEdit->cut();
}

//==============================================================================
void Notepad::paste()
{
    m_upUi->textEdit->paste();
}

//==============================================================================
void Notepad::undo()
{
    m_upUi->textEdit->undo();
}

//==============================================================================
void Notepad::redo()
{
    m_upUi->textEdit->redo();
}

//==============================================================================
void Notepad::selectFont()
{
    bool fontSelected;
    QFont font = QFontDialog::getFont(&fontSelected, this);

    if (fontSelected)
    {
        m_upUi->textEdit->setFont(font);
    }
}

//==============================================================================
void Notepad::setFontUnderline(bool underline)
{
    m_upUi->textEdit->setFontUnderline(underline);
}

//==============================================================================
void Notepad::setFontItalic(bool italic)
{
    m_upUi->textEdit->setFontItalic(italic);
}

//==============================================================================
void Notepad::setFontBold(bool bold)
{
    bold ? m_upUi->textEdit->setFontWeight(QFont::Bold) :
           m_upUi->textEdit->setFontWeight(QFont::Normal);
}

//==============================================================================
void Notepad::about()
{
    QMessageBox::about(
        this,
        tr("About Notepad"),
        tr("The <b>Notepad</b> example demonstrates how to code a basic "
           "text editor using QtWidgets"));
}
