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

#ifndef QTEXTDOCUMENTFRAGMENT_H
#define QTEXTDOCUMENTFRAGMENT_H

#include <qstring.h>

class QTextStream;
class QTextDocument;
class QTextDocumentFragmentPrivate;
class QTextCursor;
class QDataStream;

class Q_GUI_EXPORT QTextDocumentFragment
{
    friend class QTextCursor;
    friend QDataStream &operator<<(QDataStream &, const QTextDocumentFragment &fragment);
    friend QDataStream &operator>>(QDataStream &, QTextDocumentFragment &fragment);
public:
    QTextDocumentFragment();
    QTextDocumentFragment(QTextDocument *document);
    QTextDocumentFragment(const QTextCursor &range);
    QTextDocumentFragment(const QTextDocumentFragment &rhs);
    QTextDocumentFragment &operator=(const QTextDocumentFragment &rhs);
    ~QTextDocumentFragment();

    bool isEmpty() const;

    QString toPlainText() const;

    static QTextDocumentFragment fromPlainText(const QString &plainText);
    static QTextDocumentFragment fromHTML(const QString &html);
    static QTextDocumentFragment fromHTML(const QByteArray &html);

private:
    QTextDocumentFragmentPrivate *d;
};

QDataStream &operator<<(QDataStream &, const QTextDocumentFragment &fragment);
QDataStream &operator>>(QDataStream &, QTextDocumentFragment &fragment);

#endif // QTEXTDOCUMENTFRAGMENT_H
