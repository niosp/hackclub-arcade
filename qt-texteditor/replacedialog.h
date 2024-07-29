#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>

namespace Ui {
class ReplaceDialog;
}

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QPlainTextEdit *parent = nullptr);
    ~ReplaceDialog();

private slots:
    void on_pushButton_clicked();

    void on_cancelButton_clicked();

    void on_replaceButton_clicked();

private:
    Ui::ReplaceDialog *ui;
    QPlainTextEdit *edit;
};

#endif // REPLACEDIALOG_H
