/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SQLSYNTAXHIGHLIGHTER_H
#define SQLSYNTAXHIGHLIGHTER_H

#include <qsyntaxhighlighter.h>

// This is just an example of a SQL syntax highlighter, it only supports a
// subset of standard SQL

class SqlSyntaxHighlighter: public QSyntaxHighlighter
{
public:
    SqlSyntaxHighlighter(QTextEdit *textEdit);
    int highlightParagraph(const QString &text, int endStateOfLastPara);

private:
    enum States{ invalid = 0, normal = 1, inQuotes = 2, remark = 3 };

    static QColor color(States state);
    bool checkKeyword(const QString& text, const char* word, int& i);
    int highlightKeyword(const QString& text, int i);

private:
    QFont boldFont;
};

#endif
