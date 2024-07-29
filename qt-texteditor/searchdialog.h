#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QPlainTextEdit>
#include <QDialog>

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    SearchDialog(QPlainTextEdit *parent = nullptr);
    ~SearchDialog();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    Ui::SearchDialog *ui;
    QPlainTextEdit *edit;
};

#endif // SEARCHDIALOG_H
