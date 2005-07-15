/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSYNTAXHIGHLIGHTER_H
#define QSYNTAXHIGHLIGHTER_H

#include <QtCore/qobject.h>
#include <QtGui/qtextobject.h>

class QTextDocument;
class QSyntaxHighlighterPrivate;
class QTextCharFormat;
class QFont;
class QColor;
class QTextBlockUserData;

class Q_GUI_EXPORT QSyntaxHighlighter : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSyntaxHighlighter)
public:
    QSyntaxHighlighter(QObject *parent);
    QSyntaxHighlighter(QTextDocument *parent);
    virtual ~QSyntaxHighlighter();

    void setDocument(QTextDocument *doc);
    QTextDocument *document() const;

protected:
    virtual void highlightBlock(const QString &text) = 0;

    void setFormat(int start, int count, const QTextCharFormat &format);
    void setFormat(int start, int count, const QColor &color);
    void setFormat(int start, int count, const QFont &font);
    QTextCharFormat format(int pos) const;

    int previousBlockState() const;
    int currentBlockState() const;
    void setCurrentBlockState(int newState);

    void setCurrentBlockUserData(QTextBlockUserData *data);
    QTextBlockUserData *currentBlockUserData() const;

private:
    Q_DISABLE_COPY(QSyntaxHighlighter)
    Q_PRIVATE_SLOT(d_func(), void reformatBlocks(int from, int charsRemoved, int charsAdded));
};

#endif // QSYNTAXHIGHLIGHTER_H
