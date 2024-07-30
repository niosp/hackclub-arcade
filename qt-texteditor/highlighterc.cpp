#include "highlighterc.h"

#include <QRegularExpression>

HighlighterC::HighlighterC()
{
    highlightingRules.append({ QRegularExpression(QStringLiteral("\\bint\\b|\\bchar\\b|\\bvoid\\b|\\bfloat\\b|\\bdouble\\b|\\bif\\b|\\belse\\b|\\bwhile\\b|\\bfor\\b|\\breturn\\b|\\bswitch\\b|\\bcase\\b|\\bdefault\\b|\\bstruct\\b|\\bunion\\b|\\benum\\b")), keywordFormat });
    highlightingRules.append({ QRegularExpression(QStringLiteral("//[^\n]*")), commentFormat });
    highlightingRules.append({ QRegularExpression(QStringLiteral("/\\*.*?\\*/")), commentFormat });
    highlightingRules.append({ QRegularExpression(QStringLiteral("\"[^\"\\n]*\"")), stringFormat });
    highlightingRules.append({ QRegularExpression(QStringLiteral("'[^'\\n]*'")), stringFormat });
    highlightingRules.append({ QRegularExpression(QStringLiteral("#[^\n]*")), preprocessorFormat });
}
