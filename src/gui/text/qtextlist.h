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

#ifndef QTEXTLIST_H
#define QTEXTLIST_H

#include <qtextobject.h>
#include <qobject.h>

class QTextListPrivate;
class QTextCursor;

class Q_GUI_EXPORT QTextList : public QTextBlockGroup
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextList)
public:
    QTextList(QTextDocument *doc);
    ~QTextList();

    int count() const;

    inline bool isEmpty() const
    { return count() == 0; }

    QTextBlock item(int i) const;

    int itemNumber(const QTextBlock &) const;
    QString itemText(const QTextBlock &) const;

    void removeItem(int i);
    void remove(const QTextBlock &);

    void setFormat(const QTextListFormat &format) { QTextObject::setFormat(format); }
    QTextListFormat format() const { return QTextObject::format().toListFormat(); }

private:
    Q_DISABLE_COPY(QTextList)
};

#endif // QTEXTLIST_H
