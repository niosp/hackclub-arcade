#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QMainWindow>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class TextEditor; }
QT_END_NAMESPACE

class TextEditor : public QMainWindow
{
    Q_OBJECT

public:
    TextEditor(QWidget *parent = nullptr);
    ~TextEditor();

private slots:
    void on_actionOpen_File_triggered();

    void on_actionSave_triggered();

    void on_plainTextEdit_textChanged();

    void on_actionExit_triggered();

    void on_actionSave_As_triggered();

    void on_actionZoom_in_triggered();

    void on_actionZoom_out_triggered();

private:
    Ui::TextEditor *ui;
    QString currentFile;
    bool textChanged;
    QLabel * m_statusLeft;
    QLabel * m_statusMiddle;
    QLabel * m_statusRight;
};
#endif // TEXTEDITOR_H
