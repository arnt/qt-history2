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

#ifndef QTEXTCURSOR_P_H
#define QTEXTCURSOR_P_H

#ifndef QT_H
#include "qtextcursor.h"
#include "qtextdocument.h"
#include "qtextdocument_p.h"
#include <private/qtextformat_p.h>
#include "qtextobject.h"
#endif // QT_H


class QTextCursorPrivate : public QSharedData
{
public:
    QTextCursorPrivate(QTextDocumentPrivate *p);
    QTextCursorPrivate(const QTextCursorPrivate &rhs);
    ~QTextCursorPrivate();

    void adjustPosition(int positionOfChange, int charsAddedOrRemoved, QTextUndoCommand::Operation op);

    void adjustCursor(QTextCursor::MoveOperation m);

    void remove();
    inline void setPosition(int newPosition) {
        Q_ASSERT(newPosition >= 0 && newPosition < priv->length());
        position = newPosition;
    }
    void setX();
    bool canDelete(int pos) const;

    void insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat);
    bool movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    inline QTextBlock block() const
        { return QTextBlock(priv, priv->blockMap().findNode(position)); }
    inline QTextBlockFormat blockFormat() const
        { return block().blockFormat(); }

    int x;
    int position;
    int anchor;
    int adjusted_anchor;
    QTextDocumentPrivate *priv;
};

#endif // QTEXTCURSOR_P_H
