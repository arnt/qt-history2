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

#include <QtCore/qstring.h>

QT_MODULE(Gui)

class QTextStream;
class QTextDocument;
class QTextDocumentFragmentPrivate;
class QTextCursor;

class Q_GUI_EXPORT QTextDocumentFragment
{
public:
    QTextDocumentFragment();
    explicit QTextDocumentFragment(const QTextDocument *document);
    explicit QTextDocumentFragment(const QTextCursor &range);
    QTextDocumentFragment(const QTextDocumentFragment &rhs);
    QTextDocumentFragment &operator=(const QTextDocumentFragment &rhs);
    ~QTextDocumentFragment();

    bool isEmpty() const;

    QString toPlainText() const;
    QString toHtml() const;

    static QTextDocumentFragment fromPlainText(const QString &plainText);
    static QTextDocumentFragment fromHtml(const QString &html);

private:
    QTextDocumentFragmentPrivate *d;
    friend class QTextCursor;
};

#endif // QTEXTDOCUMENTFRAGMENT_H
