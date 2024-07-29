#include "searchdialog.h"
#include "ui_searchdialog.h"
#include <QPlainTextEdit>
#include <QMessageBox>

SearchDialog::SearchDialog(QPlainTextEdit *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog), edit(parent)
{
    ui->setupUi(this);
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

// search button
void SearchDialog::on_pushButton_2_clicked()
{
    QTextDocument::FindFlags flag;
    bool reverse_state = this->ui->backward_chk->isChecked() ? true : false;
    if (this->ui->backward_chk->isChecked()) flag |= QTextDocument::FindBackward;
    if (this->ui->case_sen_chk->isChecked()) flag |= QTextDocument::FindCaseSensitively;
    if (this->ui->whole_words_chk->isChecked()) flag |= QTextDocument::FindWholeWords;
    QTextCursor cursor = this->edit->textCursor();
    QTextCursor cursor_saved = cursor;
    qDebug() << "1";
    if (!this->edit->find(ui->lineEdit->text(), flag))
    {
        qDebug() << "2";
        cursor.movePosition(reverse_state?QTextCursor::End:QTextCursor::Start);
        this->edit->setTextCursor(cursor);
        if (!this->edit->find(ui->lineEdit->text(), flag))
        {
            qDebug() << "3";
            //no match in whole document, use the old cursor!
            QMessageBox msgBox;
            msgBox.setWindowTitle("Search Result");
            msgBox.setText(tr("String not found."));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            this->edit->setTextCursor(cursor_saved);
        }
    }
    qDebug() << "4";
}
// cancel button
void SearchDialog::on_pushButton_clicked()
{
    this->close();
}

