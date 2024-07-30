#include "replacedialog.h"
#include "ui_replacedialog.h"

#include <QPlainTextEdit>
#include <QMessageBox>

ReplaceDialog::ReplaceDialog(QPlainTextEdit *parent) :
    QDialog(parent),
    ui(new Ui::ReplaceDialog), edit(parent)
{
    ui->setupUi(this);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

void ReplaceDialog::on_pushButton_clicked()
{
    this->close();
}


void ReplaceDialog::on_cancelButton_clicked()
{
    this->close();
}


void ReplaceDialog::on_replaceButton_clicked()
{
    QTextDocument::FindFlags flag;
    if (this->ui->backward_chk->isChecked()) flag |= QTextDocument::FindBackward;
    if (this->ui->case_sen_chk->isChecked()) flag |= QTextDocument::FindCaseSensitively;
    if (this->ui->whole_words_chk->isChecked()) flag |= QTextDocument::FindWholeWords;
    QTextCursor cursor = this->edit->textCursor();

    if(this->edit->find(this->ui->originalString->text(), flag))
    {
        qDebug() << "FOUND!";
        QTextCursor cursor = this->edit->textCursor();
        cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor, 1);
        this->edit->insertPlainText(this->ui->replaceString->text());
    }else{
        //no match in whole document, use the old cursor!
        QMessageBox msgBox;
        msgBox.setWindowTitle("Search Result");
        msgBox.setText(tr("String not found."));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }
}

