/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qtextedit.h>

#include "sqlsyntaxhighlighter.h"

// This is just an example of a SQL syntax highlighter, it only supports a
// subset of standard SQL

SqlSyntaxHighlighter::SqlSyntaxHighlighter(QTextEdit *textEdit): QSyntaxHighlighter(textEdit)
{
    if (!textEdit)
        return;

    boldFont = textEdit->font();
    boldFont.setBold(TRUE);
}

int SqlSyntaxHighlighter::highlightParagraph(const QString &text, int endStateOfLastPara)
{
    static QChar tick('\'');
    static QChar dash('-');
    States state = endStateOfLastPara < 0 ? normal : static_cast<States>(endStateOfLastPara);
    States oldState = invalid;
    int i = 0;
    int lastI = 0;
    QChar nextChar;
    if (state == remark)
        state = normal; // sql remarks don't span lines
    while (i < text.length()) {
        oldState = state;
        if (i != text.length() - 1)
            nextChar = text.at(i + 1);

        if (text[ i ] == tick ) {
            if (state == inQuotes) {
                ++i; // include the tick in highlighting
                if (nextChar == tick)
                    ++i; // skip the escaped char
                else
                    state = normal;
            } else if (state == normal) {
                state = inQuotes;
            }
        } else if (text[ i ] == dash) {
            if (state == normal && nextChar == dash)
                state = remark;
        } else if (state == normal) {
            int ci = highlightKeyword(text, i);
            if (ci) {
                setFormat(lastI, i - lastI, color(oldState));
                lastI = ci;
                i = ci - 1;
            }
        }
        if (oldState != state) {
            setFormat(lastI, i - lastI, color(oldState));
            lastI = i;
            oldState = state;
        }
        ++i;
    }
    if (i > 0)
        setFormat(lastI, i - lastI, color(state));
    return static_cast<int>(state);
}

// returns the color for a paragraph for the state
QColor SqlSyntaxHighlighter::color(States state)
{
    switch (state) {
    case inQuotes:
        return QColor(0, 0, 200);
    case remark:
        return QColor(50, 50, 50);
    case invalid:
    case normal:
    default:
        return QColor(0, 0, 0);
    }
    return QColor();
}

// checks whether we found a keyword and highlights it
bool SqlSyntaxHighlighter::checkKeyword(const QString& text, const char* word, int& i)
{
    int len = strlen(word);
    if (qstrcmp(text.mid(i, len).lower().ascii(), word) == 0) {
        setFormat(i, len, boldFont);
        i += len;
        return TRUE;
    }
    return FALSE;
}

int SqlSyntaxHighlighter::highlightKeyword(const QString& text, int i)
{
    // yes, not complete, but works for 95% of my queries
    static const char* keywords[] = { "alter", "and", "between", "by", "call", "create",
	    "delete", "distinct", "drop", "from", "group", "having", "inner", "insert",
	    "into", "in", "join", "keys", "key", "left", "like", "limit", "natural",
	    "not", "null", "on", "or", "order", "outer", "primary", "right", "select",
	    "table", "update", "values", "view", "where", 0 };
    char c = text[i].lower().latin1();
    int count = 0;
    const char *word = keywords[ 0 ];
    // speed hack - compare the first character only and if matches,
    // compare the whole string
    while (word) {
        if (word[ 0 ] == c && checkKeyword(text, word, i))
            return i;
        word = keywords[ ++count ];
    }
    return 0;
}
