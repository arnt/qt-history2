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
    QTextCursorPrivate(const QTextDocumentPrivate *table);
    QTextCursorPrivate(const QTextCursorPrivate &rhs);
    ~QTextCursorPrivate();

    void adjustPosition(int positionOfChange, int charsAddedOrRemoved, UndoCommand::Operation op);
    void adjustCursor();

    void remove();
    void setPosition(int newPosition);
    void setX();
    bool canDelete(int pos) const;

    void insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat);
    bool movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    inline QTextBlock block() const
    { return QTextBlock(pieceTable, pieceTable->blockMap().findNode(position)); }
    inline QTextBlockFormat blockFormat() const
    { return block().blockFormat(); }

    QTextTable *tableAt(int position) const;


    int x;
    int position;
    int anchor;
    int adjusted_anchor;
    QTextDocumentPrivate *pieceTable;
};

#endif // QTEXTCURSOR_P_H
