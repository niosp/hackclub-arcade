#ifndef HIGHLIGHTERC_H
#define HIGHLIGHTERC_H

#include <QSyntaxHighlighter>
#include <QObject>
#include <QRegularExpression>
#include <QString>

class HighlighterC : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    HighlighterC(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<HighlightingRule> highlighting_rules;

    QRegularExpression comment_start_expression;
    QRegularExpression comment_end_expression;

    QTextCharFormat keyword_format;
    QTextCharFormat class_format;
    QTextCharFormat single_line_comment_format;
    QTextCharFormat multi_line_comment_format;
    QTextCharFormat quotation_format;
    QTextCharFormat function_format;
};

#endif // HIGHLIGHTERC_H
