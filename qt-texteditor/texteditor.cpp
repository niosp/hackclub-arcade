#include "texteditor.h"
#include "ui_texteditor.h"
#include "searchdialog.h"
#include "replacedialog.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTimer>

#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>

TextEditor::TextEditor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TextEditor)
{
    ui->setupUi(this);
    ui->statusBar->showMessage("asdasdas");
    // Create multipart status bar
    m_statusLeft = new QLabel("", this);
    m_statusMiddle = new QLabel("Line 0 Col 0", this);
    m_statusRight = new QLabel("", this);
    statusBar()->addPermanentWidget(m_statusLeft, 90);
    statusBar()->addPermanentWidget(m_statusMiddle, 20);
    statusBar()->addPermanentWidget(m_statusRight, 5);
    ui->actionZoom_in->setShortcut(Qt::Key_Plus | Qt::CTRL);
    ui->plainTextEdit->setUndoRedoEnabled(true);
}

TextEditor::~TextEditor()
{
    delete ui;
}

void TextEditor::on_actionOpen_File_triggered()
{

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                                                    tr("All Files (*);;Text Files (*.txt)"));
    if (!fileName.isEmpty()) {
        std::ifstream file(fileName.toStdString(), std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            qDebug() << "Could not open file: " << fileName << "\n";
        }else{
            std::string contents;
            file.seekg(0, std::ios::end);
            contents.resize(file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(&contents[0], contents.size());
            file.close();
            QString str = QString::fromStdString(contents);
            ui->plainTextEdit->setPlainText(str);
            this->currentFile = fileName;
            this->m_statusLeft->setText("Editing " + fileName);
        }
    }
}

// "Save" button in toolbar triggered
void TextEditor::on_actionSave_triggered()
{
    if(textChanged && currentFile != nullptr){
        QFile file(this->currentFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("Could not open file for writing.");
            return;
        }
        QTextStream out(&file);
        out << ui->plainTextEdit->toPlainText();
        file.close();
        QTimer::singleShot(0, [=](){
            this->m_statusLeft->setText("Saved file: " + this->currentFile);
            QTimer::singleShot(4000, [=](){
                this->m_statusLeft->setText("");
            });
        });
        this->textChanged = false;
    }else if(!textChanged && currentFile != nullptr){
        QTimer::singleShot(0, [=](){
            this->m_statusLeft->setText("File already saved");
            QTimer::singleShot(4000, [=](){
                this->m_statusLeft->setText("");
            });
        });
    }else{
        // trigger save as
        TextEditor::on_actionSave_As_triggered();
    }
}

void TextEditor::on_plainTextEdit_textChanged()
{
    this->textChanged = true;
}


void TextEditor::on_actionExit_triggered()
{
    QApplication::quit();
}


void TextEditor::on_actionSave_As_triggered()
{
    QString userDirectory = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save File"),
        userDirectory,
        tr("Text Files (*.txt);;All Files (*)")
    );
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning("Could not open file for writing.");
        return;
    }
    QTextStream out(&file);
    out << ui->plainTextEdit->toPlainText();
    file.close();
    this->currentFile = fileName;
    QTimer::singleShot(0, [=](){
        this->m_statusLeft->setText("Created and saved file: " + this->currentFile);
        QTimer::singleShot(4000, [=](){
            this->m_statusLeft->setText("");
        });
    });
}


void TextEditor::on_actionZoom_in_triggered()
{
    qDebug() << "zoom in called";
    ui->plainTextEdit->zoomIn(2);
}


void TextEditor::on_actionZoom_out_triggered()
{
    qDebug() << "zoom out called";
    ui->plainTextEdit->zoomOut(2);
}

void TextEditor::find_string(QString s, bool reverse, bool casesens, bool words)
{
    QTextDocument::FindFlags flag;
    if (reverse) flag |= QTextDocument::FindBackward;
    if (casesens) flag |= QTextDocument::FindCaseSensitively;
    if (words) flag |= QTextDocument::FindWholeWords;

    QTextCursor cursor = ui->plainTextEdit->textCursor();
    QTextCursor cursor_saved = cursor;

    if (!ui->plainTextEdit->find(s, flag))
    {
        cursor.movePosition(reverse?QTextCursor::End:QTextCursor::Start);
        ui->plainTextEdit->setTextCursor(cursor);
        if (!ui->plainTextEdit->find(s, flag))
        {
            //no match in whole document? use the old cursor!
            QMessageBox msgBox;
            msgBox.setText(tr("String not found."));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            ui->plainTextEdit->setTextCursor(cursor_saved);
        }
    }else{

    }
}


void TextEditor::on_plainTextEdit_cursorPositionChanged()
{
    // update cursor position
    QTextCursor text_cursor = ui->plainTextEdit->textCursor();
    this->m_statusMiddle->setText("Line: " + QString::number(text_cursor.blockNumber()) + " Column: " + QString::number(text_cursor.columnNumber()));
}


void TextEditor::on_actionFind_triggered()
{
    SearchDialog sd(ui->plainTextEdit);
    sd.show();
    sd.exec();
}


void TextEditor::on_actionUndo_triggered()
{
}


void TextEditor::on_actionRedo_triggered()
{
}


void TextEditor::on_actionSelect_all_triggered()
{
    this->ui->plainTextEdit->selectAll();
}


void TextEditor::on_actionCut_triggered()
{
    this->ui->plainTextEdit->cut();
}


void TextEditor::on_actionReplace_triggered()
{
    ReplaceDialog rp(ui->plainTextEdit);
    rp.show();
    rp.exec();
}

