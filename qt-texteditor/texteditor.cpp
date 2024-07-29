#include "texteditor.h"
#include "ui_texteditor.h"

#include <QFileDialog>
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
        }
    }
}

// "Save" button in toolbar triggered
void TextEditor::on_actionSave_triggered()
{
    if(textChanged){
        QTimer::singleShot(0, [=](){
            this->m_statusLeft->setText("Saved file" + this->currentFile);
            QTimer::singleShot(4000, [=](){
                this->m_statusLeft->setText("");
            });
        });
        this->textChanged = false;
    }else{
        QTimer::singleShot(0, [=](){
            this->m_statusLeft->setText("File already saved");
            QTimer::singleShot(4000, [=](){
                this->m_statusLeft->setText("");
            });
        });
    }
}

// text in TextEdit changed
void TextEditor::on_plainTextEdit_textChanged()
{
    this->textChanged = true;
}

