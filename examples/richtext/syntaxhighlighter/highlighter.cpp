/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    QTextCharFormat variableFormat;
    variableFormat.setFontWeight(QFont::Bold);
    variableFormat.setForeground(Qt::blue);
    mappings["\\b[A-Z_]+\\b"] = variableFormat;

    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setBackground(QColor("#77ff77"));
    mappings["#[^\n]*"] = singleLineCommentFormat;

    QTextCharFormat quotationFormat;
    quotationFormat.setBackground(Qt::cyan);
    quotationFormat.setForeground(Qt::blue);
    mappings["\".*\""] = quotationFormat;

    QTextCharFormat functionFormat;
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    mappings["\\b[a-z0-9_]+\\(.*\\)"] = functionFormat;
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (QString pattern, mappings.keys()) {
        QRegExp expression(pattern);
        int index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, mappings[pattern]);
            index = text.indexOf(expression, index + length);
        }
    }
}
