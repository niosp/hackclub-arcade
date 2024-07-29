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

private:
    Ui::SearchDialog *ui;
};

#endif // SEARCHDIALOG_H
