#ifndef QTEXTCURSOR_P_H
#define QTEXTCURSOR_P_H

#ifndef QT_H
#include "qtextcursor.h"
#include "qtextdocument.h"
#include "qtextpiecetable_p.h"
#include <private/qtextformat_p.h>
#endif // QT_H


class QTextCursorPrivate : public QSharedData
{
public:
    QTextCursorPrivate(const QTextPieceTable *table);
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

    inline QTextBlockIterator block() const
    { return pieceTable->blocksFind(position); }
    inline QTextBlockFormat blockFormat() const
    { return block().blockFormat(); }

    QTextTable *tableAt(int position) const;


    int x;
    int position;
    int anchor;
    int adjusted_anchor;
    QTextPieceTable *pieceTable;
};

#endif // QTEXTCURSOR_P_H
