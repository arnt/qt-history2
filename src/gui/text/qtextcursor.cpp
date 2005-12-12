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

#include "qtextcursor.h"
#include "qtextcursor_p.h"
#include "qglobal.h"
#include "qtextdocumentfragment.h"
#include "qtextdocumentfragment_p.h"
#include "qtextlist.h"
#include "qtexttable.h"
#include "qtexttable_p.h"
#include "qtextengine_p.h"

#include <qtextlayout.h>
#include <qdebug.h>

enum {
    AdjustPrev = 0x1,
    AdjustUp = 0x3,
    AdjustNext = 0x4,
    AdjustDown = 0x12
};

QTextCursorPrivate::QTextCursorPrivate(QTextDocumentPrivate *p)
    : priv(p), x(0), position(0), anchor(0), adjusted_anchor(0),
      currentCharFormat(-1)
{
    priv->addCursor(this);
}

QTextCursorPrivate::QTextCursorPrivate(const QTextCursorPrivate &rhs)
    : QSharedData(rhs)
{
    position = rhs.position;
    anchor = rhs.anchor;
    adjusted_anchor = rhs.adjusted_anchor;
    priv = rhs.priv;
    x = rhs.x;
    currentCharFormat = rhs.currentCharFormat;
    priv->addCursor(this);
}

QTextCursorPrivate::~QTextCursorPrivate()
{
    if (priv)
        priv->removeCursor(this);
}

QTextCursorPrivate::AdjustResult QTextCursorPrivate::adjustPosition(int positionOfChange, int charsAddedOrRemoved, QTextUndoCommand::Operation op)
{
    QTextCursorPrivate::AdjustResult result = QTextCursorPrivate::CursorMoved;
    // not(!) <= , so that inserting text adjusts the cursor correctly
    if (position < positionOfChange ||
        (position == positionOfChange && op == QTextUndoCommand::KeepCursor)) {
        result = CursorUnchanged;
    } else {
        if (charsAddedOrRemoved < 0 && position < positionOfChange - charsAddedOrRemoved)
            position = positionOfChange;
        else
            position += charsAddedOrRemoved;
        
        currentCharFormat = -1;
    }
    
    if (anchor >= positionOfChange
        && (anchor != positionOfChange || op != QTextUndoCommand::KeepCursor)) {
        if (charsAddedOrRemoved < 0 && anchor < positionOfChange - charsAddedOrRemoved)
            anchor = positionOfChange;
        else
            anchor += charsAddedOrRemoved;
    }
    
    if (adjusted_anchor >= positionOfChange
        && (adjusted_anchor != positionOfChange || op != QTextUndoCommand::KeepCursor)) {
        if (charsAddedOrRemoved < 0 && adjusted_anchor < positionOfChange - charsAddedOrRemoved)
            adjusted_anchor = positionOfChange;
        else
            adjusted_anchor += charsAddedOrRemoved;
    }
    
    return result;
}

void QTextCursorPrivate::setX()
{
    QTextBlock block = priv->blocksFind(position);
    const QTextLayout *layout = block.layout();
    int pos = position - block.position();

    QTextLine line = layout->lineForTextPosition(pos);
    if (line.isValid())
        x = line.cursorToX(pos);
}

void QTextCursorPrivate::remove()
{
    if (anchor == position)
        return;
    priv->beginEditBlock();
    currentCharFormat = -1;
    int pos1 = position;
    int pos2 = adjusted_anchor;
    QTextUndoCommand::Operation op = QTextUndoCommand::KeepCursor;
    if (pos1 > pos2) {
        pos1 = adjusted_anchor;
        pos2 = position;
        op = QTextUndoCommand::MoveCursor;
    }

    // deleting inside table? -> delete only content
    QTextTable *table = complexSelectionTable();
    if (table) {
        int startRow, startCol, numRows, numCols;
        selectedTableCells(&startRow, &numRows, &startCol, &numCols);
        clearCells(table, startRow, startCol, numRows, numCols, op);
    } else {
        priv->remove(pos1, pos2-pos1, op);
    }

    adjusted_anchor = anchor = position;
    priv->endEditBlock();
}

void QTextCursorPrivate::clearCells(QTextTable *table, int startRow, int startCol, int numRows, int numCols, QTextUndoCommand::Operation op)
{
    priv->beginEditBlock();

    for (int row = startRow; row < startRow + numRows; ++row)
        for (int col = startCol; col < startCol + numCols; ++col) {
            QTextTableCell cell = table->cellAt(row, col);
            const int startPos = cell.firstPosition();
            const int endPos = cell.lastPosition();
            Q_ASSERT(startPos <= endPos);
            priv->remove(startPos, endPos - startPos, op);
        }

    priv->endEditBlock();
}

bool QTextCursorPrivate::canDelete(int pos) const
{
    QTextDocumentPrivate::FragmentIterator fit = priv->find(pos);
    QTextCharFormat fmt = priv->formatCollection()->charFormat((*fit)->format);
    return (fmt.objectIndex() == -1);
}

void QTextCursorPrivate::insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat)
{
    QTextFormatCollection *formats = priv->formatCollection();
    int idx = formats->indexForFormat(format);
    Q_ASSERT(formats->format(idx).isBlockFormat());

    priv->insertBlock(position, idx, formats->indexForFormat(charFormat));
    currentCharFormat = -1;
}

void QTextCursorPrivate::adjustCursor(QTextCursor::MoveOperation m)
{
    adjusted_anchor = anchor;
    if (position == anchor)
        return;

    QTextFrame *f_position = priv->frameAt(position);
    QTextFrame *f_anchor = priv->frameAt(adjusted_anchor);

    if (f_position != f_anchor) {
        // find common parent frame
        QList<QTextFrame *> positionChain;
        QList<QTextFrame *> anchorChain;
        QTextFrame *f = f_position;
        while (f) {
            positionChain.prepend(f);
            f = f->parentFrame();
        }
        f = f_anchor;
        while (f) {
            anchorChain.prepend(f);
            f = f->parentFrame();
        }
        Q_ASSERT(positionChain.at(0) == anchorChain.at(0));
        int i = 1;
        int l = qMin(positionChain.size(), anchorChain.size());
        for (; i < l; ++i) {
            if (positionChain.at(i) != anchorChain.at(i))
                break;
        }

        if (m <= QTextCursor::WordLeft) {
            if (i < positionChain.size())
                position = positionChain.at(i)->firstPosition() - 1;
        } else {
            if (i < positionChain.size())
                position = positionChain.at(i)->lastPosition() + 1;
        }
        if (position < adjusted_anchor) {
            if (i < anchorChain.size())
                adjusted_anchor = anchorChain.at(i)->lastPosition() + 1;
        } else {
            if (i < anchorChain.size())
                adjusted_anchor = anchorChain.at(i)->firstPosition() - 1;
        }

        f_position = positionChain.at(i-1);
    }

    // same frame, either need to adjust to cell boundaries or return
    QTextTable *table = qobject_cast<QTextTable *>(f_position);
    if (!table)
        return;

    QTextTableCell c_position = table->cellAt(position);
    QTextTableCell c_anchor = table->cellAt(adjusted_anchor);
    if (c_position != c_anchor) {
        bool before;
        int col_position = c_position.column();
        int col_anchor = c_anchor.column();
        if (col_position == col_anchor) {
            before = c_position.row() < c_anchor.row();
        } else {
            before = col_position < col_anchor;
        }

        // adjust to cell boundaries
        if (m <= QTextCursor::WordLeft) {
            position = c_position.firstPosition();
            if (!before)
                --position;
        } else {
            position = c_position.lastPosition();
            if (before)
                ++position;
        }
        if (position < adjusted_anchor)
            adjusted_anchor = c_anchor.lastPosition();
        else
            adjusted_anchor = c_anchor.firstPosition();
    }
    currentCharFormat = -1;
}

bool QTextCursorPrivate::movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode)
{
    currentCharFormat = -1;
    bool adjustX = true;
    QTextBlock blockIt = block();

    if (op >= QTextCursor::Left && op <= QTextCursor::WordRight
        && blockIt.blockFormat().layoutDirection() == Qt::RightToLeft) {
        if (op == QTextCursor::Left)
            op = QTextCursor::NextCharacter;
        else if (op == QTextCursor::Right)
            op = QTextCursor::PreviousCharacter;
        else if (op == QTextCursor::WordLeft)
            op = QTextCursor::NextWord;
        else if (op == QTextCursor::WordRight)
            op = QTextCursor::PreviousWord;
    }

    const QTextLayout *layout = blockIt.layout();
    int relativePos = position - blockIt.position();
    QTextLine line = layout->lineForTextPosition(relativePos);

    Q_ASSERT(priv->frameAt(position) == priv->frameAt(adjusted_anchor));

    int newPosition = position;

    switch(op) {
    case QTextCursor::NoMove:
        return true;

    case QTextCursor::Start:
        newPosition = 0;
        break;
    case QTextCursor::StartOfLine: {

        if (!line.isValid())
            break;
        newPosition = blockIt.position() + line.textStart();

        break;
    }
    case QTextCursor::StartOfBlock: {
        newPosition = blockIt.position();
        break;
    }
    case QTextCursor::PreviousBlock: {
        if (blockIt == priv->blocksBegin())
            return false;
        blockIt = blockIt.previous();

        newPosition = blockIt.position();
        break;
    }
    case QTextCursor::PreviousCharacter:
    case QTextCursor::Left:
        newPosition = priv->previousCursorPosition(position, QTextLayout::SkipCharacters);
        break;
    case QTextCursor::StartOfWord: {
        QTextEngine *engine = layout->engine();
        const QCharAttributes *attributes = engine->attributes();

        if (relativePos == 0)
            return false;

        // skip if already at word start
        if (attributes[relativePos - 1].whiteSpace
            && !attributes[relativePos].whiteSpace)
            return false;

        // FALL THROUGH!
    }
    case QTextCursor::PreviousWord:
    case QTextCursor::WordLeft:
        newPosition = priv->previousCursorPosition(position, QTextLayout::SkipWords);
        break;
    case QTextCursor::Up: {
        int i = line.lineNumber() - 1;
        if (i == -1) {
            if (blockIt == priv->blocksBegin())
                return false;
            int blockPosition = blockIt.position();
            QTextTable *table = qobject_cast<QTextTable *>(priv->frameAt(blockPosition));
            if (table) {
                QTextTableCell cell = table->cellAt(blockPosition);
                if (cell.firstPosition() == blockPosition) {
                    int row = cell.row() - 1;
                    if (row >= 0) {
                        blockPosition = table->cellAt(row, cell.column()).lastPosition();
                    } else {
                        // move to line above the table
                        blockPosition = table->firstPosition() - 1;
                    }
                    blockIt = priv->blocksFind(blockPosition);
                } else {
                    blockIt = blockIt.previous();
                }
            } else {
                blockIt = blockIt.previous();
            }
            layout = blockIt.layout();
            i = layout->lineCount()-1;
        }
        if (layout->lineCount()) {
            QTextLine line = layout->lineAt(i);
            newPosition = line.xToCursor(x) + blockIt.position();
        } else {
            newPosition = blockIt.position();
        }
        adjustX = false;
        break;
    }

    case QTextCursor::End:
        newPosition = priv->length() - 1;
        break;
    case QTextCursor::EndOfLine: {
        if (!line.isValid() || line.textLength() == 0)
            break;
        newPosition = blockIt.position() + line.textStart() + line.textLength();
        if (line.lineNumber() < layout->lineCount() - 1) {
            const QString text = blockIt.text();
            // ###### this relies on spaces being the cause for linebreaks.
            // this doesn't work with japanese
            if (text.at(line.textStart() + line.textLength() - 1).isSpace())
                --newPosition;
        }
        break;
    }
    case QTextCursor::EndOfWord: {
        QTextEngine *engine = layout->engine();
        const QCharAttributes *attributes = engine->attributes();
        const QString string = engine->layoutData->string;

        const int len = layout->engine()->layoutData->string.length();
        if (relativePos >= len)
            return false;
        relativePos++;
        while (relativePos < len
               && !attributes[relativePos].whiteSpace
               && !engine->atWordSeparator(relativePos))
            relativePos++;

        newPosition = blockIt.position() + relativePos;
        break;
    }
    case QTextCursor::EndOfBlock:
        if (blockIt.length() >= 1)
            // position right before the block separator
            newPosition = blockIt.position() + blockIt.length() - 1;
        break;
    case QTextCursor::NextBlock: {
        blockIt = blockIt.next();
        if (!blockIt.isValid())
            return false;

        newPosition = blockIt.position();
        break;
    }
    case QTextCursor::NextCharacter:
    case QTextCursor::Right:
        newPosition = priv->nextCursorPosition(position, QTextLayout::SkipCharacters);
        break;
    case QTextCursor::NextWord:
    case QTextCursor::WordRight:
        newPosition = priv->nextCursorPosition(position, QTextLayout::SkipWords);
        break;

    case QTextCursor::Down: {
        int i = line.lineNumber() + 1;

        if (i >= layout->lineCount()) {
            int blockPosition = blockIt.position() + blockIt.length() - 1;
            QTextTable *table = qobject_cast<QTextTable *>(priv->frameAt(blockPosition));
            if (table) {
                QTextTableCell cell = table->cellAt(blockPosition);
                if (cell.lastPosition() == blockPosition) {
                    int row = cell.row() + cell.rowSpan();
                    if (row < table->rows()) {
                        blockPosition = table->cellAt(row, cell.column()).firstPosition();
                    } else {
                        // move to line below the table
                        blockPosition = table->lastPosition() + 1;
                    }
                    blockIt = priv->blocksFind(blockPosition);
                } else {
                    blockIt = blockIt.next();
                }
            } else {
                blockIt = blockIt.next();
            }

            if (blockIt == priv->blocksEnd())
                return false;
            layout = blockIt.layout();
            i = 0;
        }
        if (layout->lineCount()) {
            QTextLine line = layout->lineAt(i);
            newPosition = line.xToCursor(x) + blockIt.position();
        } else {
            newPosition = blockIt.position();
        }
        adjustX = false;
        break;
    }
    }

    if (mode == QTextCursor::KeepAnchor) {
        QTextTable *table = qobject_cast<QTextTable *>(priv->frameAt(position));
        if (table && ((op >= QTextCursor::PreviousBlock && op <= QTextCursor::WordLeft)
                      || (op >= QTextCursor::NextBlock && op <= QTextCursor::WordRight))) {
            int oldColumn = table->cellAt(position).column();

            const QTextTableCell otherCell = table->cellAt(newPosition);
            if (!otherCell.isValid())
                return false;

            int newColumn = otherCell.column();
            if ((oldColumn > newColumn && op >= QTextCursor::End)
                || (oldColumn < newColumn && op <= QTextCursor::WordLeft))
                return false;
        }
    }
    setPosition(newPosition);

    if (mode == QTextCursor::MoveAnchor) {
        anchor = position;
        adjusted_anchor = position;
    } else {
        adjustCursor(op);
    }

    if (adjustX)
        setX();

    return true;
}

QTextTable *QTextCursorPrivate::complexSelectionTable() const
{
    if (position == anchor)
        return 0;

    QTextTable *t = qobject_cast<QTextTable *>(priv->frameAt(position));
    if (t) {
        QTextTableCell cell_pos = t->cellAt(position);
        QTextTableCell cell_anchor = t->cellAt(adjusted_anchor);

        Q_ASSERT(cell_anchor.isValid());

        if (cell_pos == cell_anchor)
            t = 0;
    }
    return t;
}

void QTextCursorPrivate::selectedTableCells(int *firstRow, int *numRows, int *firstColumn, int *numColumns) const
{
    *firstRow = -1;
    *firstColumn = -1;
    *numRows = -1;
    *numColumns = -1;

    if (position == anchor)
        return;

    QTextTable *t = qobject_cast<QTextTable *>(priv->frameAt(position));
    if (!t)
        return;

    QTextTableCell cell_pos = t->cellAt(position);
    QTextTableCell cell_anchor = t->cellAt(adjusted_anchor);

    Q_ASSERT(cell_anchor.isValid());

    if (cell_pos == cell_anchor)
        return;

    *firstRow = qMin(cell_pos.row(), cell_anchor.row());
    *firstColumn = qMin(cell_pos.column(), cell_anchor.column());
    *numRows = qMax(cell_pos.row() + cell_pos.rowSpan(), cell_anchor.row() + cell_anchor.rowSpan()) - *firstRow;
    *numColumns = qMax(cell_pos.column() + cell_pos.columnSpan(), cell_anchor.column() + cell_anchor.columnSpan()) - *firstColumn;
}

static void setBlockCharFormat(QTextDocumentPrivate *priv, int pos1, int pos2,
                               const QTextCharFormat &format, QTextDocumentPrivate::FormatChangeMode changeMode)
{
    QTextBlock it = priv->blocksFind(pos1);
    QTextBlock end = priv->blocksFind(pos2);
    if (end.isValid())
        end = end.next();

    for (; it != end; it = it.next()) {
        priv->setCharFormat(it.position() - 1, 1, format, changeMode);
    }
}

void QTextCursorPrivate::setBlockCharFormat(const QTextCharFormat &format, QTextDocumentPrivate::FormatChangeMode changeMode)
{
    priv->beginEditBlock();

    QTextTable *table = complexSelectionTable();
    if (table) {
        int row_start, col_start, num_rows, num_cols;
        selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

        Q_ASSERT(row_start != -1);
        for (int r = row_start; r < row_start + num_rows; ++r) {
            for (int c = col_start; c < col_start + num_cols; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                int rspan = cell.rowSpan();
                int cspan = cell.columnSpan();
                if (rspan != 1) {
                    int cr = cell.row();
                    if (cr != r)
                        continue;
                }
                if (cspan != 1) {
                    int cc = cell.column();
                    if (cc != c)
                        continue;
                }

                int pos1 = cell.firstPosition();
                int pos2 = cell.lastPosition();
                ::setBlockCharFormat(priv, pos1, pos2, format, changeMode);
            }
        }
    } else {
        int pos1 = position;
        int pos2 = adjusted_anchor;
        if (pos1 > pos2) {
            pos1 = adjusted_anchor;
            pos2 = position;
        }

        ::setBlockCharFormat(priv, pos1, pos2, format, changeMode);
    }
    priv->endEditBlock();
}


void QTextCursorPrivate::setBlockFormat(const QTextBlockFormat &format, QTextDocumentPrivate::FormatChangeMode changeMode)
{
    QTextTable *table = complexSelectionTable();
    if (table) {
        priv->beginEditBlock();
        int row_start, col_start, num_rows, num_cols;
        selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

        Q_ASSERT(row_start != -1);
        for (int r = row_start; r < row_start + num_rows; ++r) {
            for (int c = col_start; c < col_start + num_cols; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                int rspan = cell.rowSpan();
                int cspan = cell.columnSpan();
                if (rspan != 1) {
                    int cr = cell.row();
                    if (cr != r)
                        continue;
                }
                if (cspan != 1) {
                    int cc = cell.column();
                    if (cc != c)
                        continue;
                }

                int pos1 = cell.firstPosition();
                int pos2 = cell.lastPosition();
                priv->setBlockFormat(priv->blocksFind(pos1), priv->blocksFind(pos2), format, changeMode);
            }
        }
        priv->endEditBlock();
    } else {
        int pos1 = position;
        int pos2 = adjusted_anchor;
        if (pos1 > pos2) {
            pos1 = adjusted_anchor;
            pos2 = position;
        }

        priv->setBlockFormat(priv->blocksFind(pos1), priv->blocksFind(pos2), format, changeMode);
    }
}

void QTextCursorPrivate::setCharFormat(const QTextCharFormat &format, QTextDocumentPrivate::FormatChangeMode changeMode)
{
    Q_ASSERT(position != anchor);

    QTextTable *table = complexSelectionTable();
    if (table) {
        priv->beginEditBlock();
        int row_start, col_start, num_rows, num_cols;
        selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

        Q_ASSERT(row_start != -1);
        for (int r = row_start; r < row_start + num_rows; ++r) {
            for (int c = col_start; c < col_start + num_cols; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                int rspan = cell.rowSpan();
                int cspan = cell.columnSpan();
                if (rspan != 1) {
                    int cr = cell.row();
                    if (cr != r)
                        continue;
                }
                if (cspan != 1) {
                    int cc = cell.column();
                    if (cc != c)
                        continue;
                }

                int pos1 = cell.firstPosition();
                int pos2 = cell.lastPosition();
                priv->setCharFormat(pos1, pos2-pos1, format, changeMode);
            }
        }
        priv->endEditBlock();
    } else {
        int pos1 = position;
        int pos2 = adjusted_anchor;
        if (pos1 > pos2) {
            pos1 = adjusted_anchor;
            pos2 = position;
        }

        priv->setCharFormat(pos1, pos2-pos1, format, changeMode);
    }
}

/*!
    \class QTextCursor qtextcursor.h
    \brief The QTextCursor class offers an API to access and modify QTextDocuments.

    \ingroup text
    \mainclass

    Text cursors are objects that are used to access and modify the contents
    and underlying structure of text documents via a programming interface
    that mimics the behavior of a cursor in a text editor. QTextCursor contains
    information about both the cursor's position within a QTextDocument and any
    selection that it has made.

    QTextCursor is modeled on the way a text cursor behaves in a text
    editor, providing a programmatic means of performing standard actions
    through the user interface. A document can be thought of as a
    single string of characters with the cursor's position() being \e
    between any two characters (or at the very beginning or very end
    of the document). Documents can also contain tables, lists,
    images, and other objects in addition to text but, from the developer's
    point of view, the document can be treated as one long string.
    Some portions of that string can be considered to lie within particular
    blocks (e.g. paragraphs), or within a table's cell, or a list's item,
    or other structural elements. When we refer to "current character" we
    mean the character immediately after the cursor position() in the
    document; similarly the "current block" is the block that contains the
    cursor position().

    A QTextCursor also has an anchor() position. The text that is
    between the anchor() and the position() is the selection. If
    anchor() == position() there is no selection.

    The cursor position can be changed programmatically using
    setPosition() and movePosition(); the latter can also be used to
    select text. For selections see selectionStart(), selectionEnd(),
    hasSelection(), clearSelection(), and removeSelectedText().

    If the position() is at the start of a block atBlockStart()
    returns true; and if it is at the end of a block atBlockEnd() returns
    true. The format of the current character is returned by
    charFormat(), and the format of the current block is returned by
    blockFormat().

    Formatting can be applied to the current character (the character
    immedately after position()) using applyCharFormatModifier(), and
    to the current block (the block that contains position()) using
    setBlockFormat() and applyBlockFormatModifier(). The text at the
    current character position can be turned into a list using
    createList().

    Deletions can be achieved using deleteChar(),
    deletePreviousChar(), and removeSelectedText().

    Text strings can be inserted into the document with the insertText()
    function, blocks (representing new paragraphs) can be inserted with
    insertBlock().

    Existing fragments of text can be inserted with insertFragment() but,
    if you want to insert pieces of text in various formats, it is usually
    still easier to use insertText() and supply a character format.

    Various types of higher-level structure can also be inserted into the
    document with the cursor:

    \list
    \i Lists are ordered sequences of block elements that are decorated with
       bullet points or symbols. These are inserted in a specified format
       with insertList().
    \i Tables are inserted with the insertTable() function, and can be
       given an optional format. These contain an array of cells that can
       be traversed using the cursor.
    \i Inline images are inserted with insertImage(). The image to be
       used can be specified in an image format, or by name.
    \i Frames are inserted by calling insertFrame() with a specified format.
    \endlist

    Actions can be grouped (i.e. treated as a single action for
    undo/redo) using beginEditBlock() and endEditBlock().

    Cursor movements are limited to valid cursor positions. In Latin
    writing this is usually after every character in the text. In some
    other writing systems cursor movements are limited to "clusters"
    (e.g. a syllable in Devanagari, or a base letter plus diacritics).
    Functions such as movePosition() and deleteChar() limit cursor
    movement to these valid positions.

    \sa \link richtext.html Rich Text Processing\endlink

*/

/*!
    \enum QTextCursor::MoveOperation

    \value NoMove Keep the cursor where it is

    \value Start Move to the start of the document.
    \value StartOfLine Move to the start of the current line.
    \value StartOfBlock Move to the start of the current block.
    \value StartOfWord Move to the start of the current word.
    \value PreviousBlock Move to the start of the previous block.
    \value PreviousCharacter Move to the previous character.
    \value PreviousWord Move to the beginning of the previous word.
    \value Up Move up one line.
    \value Left Move left one character.
    \value WordLeft Move left one word.

    \value End Move to the end of the document.
    \value EndOfLine Move to the end of the current line.
    \value EndOfWord Move to the end of the current word.
    \value EndOfBlock Move to the end of the current block.
    \value NextBlock Move to the beginning of the next block.
    \value NextCharacter Move to the next character.
    \value NextWord Move to the next word.
    \value Down Move down one line.
    \value Right Move right one character.
    \value WordRight Move right one word.

    \sa movePosition()
*/

/*!
    \enum QTextCursor::MoveMode

    \value MoveAnchor Moves the anchor to the same position as the cursor itself.
    \value KeepAnchor Keeps the anchor where it is.

    If the anchor() is kept where it is and the position() is moved,
    the text in between will be selected.
*/

/*!
    \enum QTextCursor::SelectionType

    \value WordUnderCursor Selects the word under the cursor. If the cursor
           is not positioned within a string of selectable characters, no
           text is selected.
    \value LineUnderCursor Selects the line of text under the cursor.
    \value BlockUnderCursor Selects the block of text under the cursor.
*/

/*!
    Constructs a null cursor.
 */
QTextCursor::QTextCursor()
    : d(0)
{
}

/*!
    Constructs a cursor pointing to the beginning of the \a document.
 */
QTextCursor::QTextCursor(QTextDocument *document)
    : d(new QTextCursorPrivate(document->docHandle()))
{
}

/*!
    Constructs a cursor pointing to the beginning of the \a frame.
*/
QTextCursor::QTextCursor(QTextFrame *frame)
    : d(new QTextCursorPrivate(frame->document()->docHandle()))
{
    d->adjusted_anchor = d->anchor = d->position = frame->firstPosition();
}


/*!
    Constructs a cursor pointing to the beginning of the \a block.
*/
QTextCursor::QTextCursor(const QTextBlock &block)
    : d(new QTextCursorPrivate(block.docHandle()))
{
    d->adjusted_anchor = d->anchor = d->position = block.position();
}


/*!
  \internal
 */
QTextCursor::QTextCursor(QTextDocumentPrivate *p, int pos)
    : d(new QTextCursorPrivate(p))
{
    d->adjusted_anchor = d->anchor = d->position = pos;

    d->setX();
}

/*!
    \internal
*/
QTextCursor::QTextCursor(QTextCursorPrivate *d)
{
    Q_ASSERT(d);
    this->d = d;
}

/*!
    Constructs a new cursor that is a copy of \a cursor.
 */
QTextCursor::QTextCursor(const QTextCursor &cursor)
{
    d = cursor.d;
}

/*!
    Makes a copy of \a cursor and assigns it to this QTextCursor.
 */
QTextCursor &QTextCursor::operator=(const QTextCursor &cursor)
{
    d = cursor.d;
    return *this;
}

/*!
    Destroys the QTextCursor.
 */
QTextCursor::~QTextCursor()
{
}

/*!
    Returns true if the cursor is null; otherwise returns false. A null
    cursor is created by the default constructor.
 */
bool QTextCursor::isNull() const
{
    return !d || !d->priv;
}

/*!
    Moves the cursor to the absolute position in the document specified by
    \a pos using a \c MoveMode specified by \a m. The cursor is positioned
    between characters.

    \sa position() movePosition() anchor()
*/
void QTextCursor::setPosition(int pos, MoveMode m)
{
    if (!d || !d->priv)
        return;

    if (pos < 0 || pos >= d->priv->length()) {
        qWarning("QTextCursor::setPosition: position '%d' out of range", pos);
        return;
    }

    d->setPosition(pos);
    if (m == MoveAnchor) {
        d->anchor = pos;
        d->adjusted_anchor = pos;
    } else { // keep anchor
        QTextCursor::MoveOperation op;
        if (pos < d->anchor)
            op = QTextCursor::Left;
        else
            op = QTextCursor::Right;
        d->adjustCursor(op);
    }
    d->setX();
}

/*!
    Returns the absolute position of the cursor within the document.
    The cursor is positioned between characters.

    \sa setPosition() movePosition() anchor()
*/
int QTextCursor::position() const
{
    if (!d || !d->priv)
        return -1;
    return d->position;
}

/*!
    Returns the anchor position; this is the same as position() unless
    there is a selection in which case position() marks one end of the
    selection and anchor() marks the other end. Just like the cursor
    position, the anchor position is between characters.

    \sa position() setPosition() movePosition() selectionStart() selectionEnd()
*/
int QTextCursor::anchor() const
{
    if (!d || !d->priv)
        return -1;
    return d->anchor;
}

/*!
    \fn bool QTextCursor::movePosition(MoveOperation operation, MoveMode mode, int n)

    Moves the cursor by performing the given \a operation \a n times, using the specified
    \a mode, and returns true if all operations were completed successfully; otherwise
    returns false.

    For example, if this function is repeatedly used to seek to the end of the next
    word, it will eventually fail when the end of the document is reached.

    By default, the move operation is performed once (\a n = 1).

    If \a mode is \c KeepAnchor, the cursor selects the text it moves
    over. This is the same effect that the user achieves when they
    hold down the Shift key and move the cursor with the cursor keys.
*/
bool QTextCursor::movePosition(MoveOperation op, MoveMode mode, int n)
{
    if (!d || !d->priv)
        return false;
    switch (op) {
        case Start:
        case StartOfLine:
        case End:
        case EndOfLine:
            n = 1;
            break;
        default: break;
    }
    for (; n > 0; --n) {
        if (!d->movePosition(op, mode))
            return false;
    }
    return true;
}

/*!
    Inserts \a text at the current position, using the current
    character format.

    If there is a selection, the selection is deleted and replaced by
    \a text, for example:
    \code
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
    cursor.insertText("Hello World");
    \endcode
    This clears any existing selection, selects the word at the cursor
    (i.e. from position() forward), and replaces the selection with
    the phrase "Hello World".

    Any ASCII linefeed characters (\\n) in the inserted text are transformed
    into unicode block separators, corresponding to insertBlock() calls.

    \sa charFormat() hasSelection()
*/
void QTextCursor::insertText(const QString &text)
{
    QTextCharFormat fmt = charFormat();
    fmt.clearProperty(QTextFormat::ObjectType);
    insertText(text, fmt);
}

/*!
    \overload

    Inserts \a text at the current position with the given \a format.
*/
void QTextCursor::insertText(const QString &text, const QTextCharFormat &format)
{
    if (!d || !d->priv)
        return;

    Q_ASSERT(format.isValid());

    d->priv->beginEditBlock();

    d->remove();
    if (!text.isEmpty()) {
        QTextFormatCollection *formats = d->priv->formatCollection();
        int formatIdx = formats->indexForFormat(format);
        Q_ASSERT(formats->format(formatIdx).isCharFormat());

        QTextBlockFormat blockFmt = blockFormat();

        bool seenCRLF = false;

        int textStart = 0;
        for (int i = 0; i < text.length(); ++i) {
            QChar ch = text.at(i);
            if (ch == QLatin1Char('\n')
                || ch == QChar::ParagraphSeparator) {

                const int textEnd = (seenCRLF ? i - 1 : i);

                if (textEnd > textStart)
                    d->priv->insert(d->position, QString(text.unicode() + textStart, textEnd - textStart), formatIdx);

                textStart = i + 1;
                d->insertBlock(blockFmt, format);

                seenCRLF = false;
            } else if (ch == QLatin1Char('\r')
                       && (i + 1) < text.length()
                       && text.at(i + 1) == QLatin1Char('\n')) {
                seenCRLF = true;
            }
        }
        if (textStart < text.length())
            d->priv->insert(d->position, QString(text.unicode() + textStart, text.length() - textStart), formatIdx);
    }
    d->priv->endEditBlock();
}

/*!
    If there is no selected text, deletes the character \e at the
    current cursor position; otherwise deletes the selected text.

    \sa deletePreviousChar() hasSelection() clearSelection()
*/
void QTextCursor::deleteChar()
{
    if (!d || !d->priv)
        return;

    if (d->position == d->anchor) {
        if (!d->canDelete(d->position))
            return;
        d->adjusted_anchor = d->anchor =
                             d->priv->nextCursorPosition(d->anchor, QTextLayout::SkipCharacters);
    }
    d->remove();
    d->setX();
}

/*!
    If there is no selected text, deletes the character \e before the
    current cursor position; otherwise deletes the selected text.

    \sa deleteChar() hasSelection() clearSelection()
*/
void QTextCursor::deletePreviousChar()
{
    if (!d || !d->priv)
        return;

    if (d->position == d->anchor) {
        if (d->anchor < 1 || !d->canDelete(d->anchor-1))
            return;
        d->anchor--;

        QTextDocumentPrivate::FragmentIterator fragIt = d->priv->find(d->anchor);
        const QTextFragmentData * const frag = fragIt.value();
        int fpos = fragIt.position();
        QChar uc = d->priv->buffer().at(d->anchor - fpos + frag->stringPosition);
        if (d->anchor > fpos && uc.unicode() >= 0xdc00 && uc.unicode() < 0xe000) {
            // second half of a surrogate, check if we have the first half as well,
            // if yes delete both at once
            uc = d->priv->buffer().at(d->anchor - 1 - fpos + frag->stringPosition);
            if (uc.unicode() >= 0xd800 && uc.unicode() < 0xdc00)
                --d->anchor;
        }

        d->adjusted_anchor = d->anchor;
    }

    d->remove();
    d->setX();
}

/*!
    Selects text in the document according to the given \a selection.
*/
void QTextCursor::select(SelectionType selection)
{
    if (!d || !d->priv)
        return;

    clearSelection();

    const QTextBlock block = d->block();
    const int relativePos = d->position - block.position();

    switch (selection) {
        case LineUnderCursor:
            movePosition(StartOfLine);
            movePosition(EndOfLine, KeepAnchor);
            break;
        case WordUnderCursor:
            movePosition(StartOfWord);
            movePosition(EndOfWord, KeepAnchor);
            break;
        case BlockUnderCursor:
            movePosition(StartOfBlock);
            // also select the paragraph separator
            if (movePosition(PreviousBlock)) {
                movePosition(EndOfBlock);
                movePosition(NextBlock, KeepAnchor);
            }
            movePosition(EndOfBlock, KeepAnchor);
            break;
    }
}

/*!
    Returns true if the cursor contains a selection; otherwise returns false.
*/
bool QTextCursor::hasSelection() const
{
    return !!d && d->position != d->anchor;
}


/*!
    Returns true if the cursor contains a selection that is not simply a
    range from selectionStart() to selectionEnd(); otherwise returns false.

    Complex selections are ones that span at least two cells in a table;
    their extent is specified by selectedTableCells().
*/
bool QTextCursor::hasComplexSelection() const
{
    if (!d)
        return false;

    return d->complexSelectionTable() != 0;
}

/*!
    If the selection spans over table cells, \a firstRow is populated
    with the number of the first row in the selection, \a firstColumn
    with the number of the first column in the selection, and \a
    numRows and \a numColumns with the number of rows and columns in
    the selection. If the selection does not span any table cells the
    results are harmless but undefined.
*/
void QTextCursor::selectedTableCells(int *firstRow, int *numRows, int *firstColumn, int *numColumns) const
{
    *firstRow = -1;
    *firstColumn = -1;
    *numRows = -1;
    *numColumns = -1;

    if (!d || d->position == d->anchor)
        return;

    d->selectedTableCells(firstRow, numRows, firstColumn, numColumns);
}


/*!
    Clears the current selection.

    Note that it does \bold{not} delete the text of the selection.

    \sa removeSelectedText() hasSelection()
*/
void QTextCursor::clearSelection()
{
    if (!d)
        return;
    d->adjusted_anchor = d->anchor = d->position;
    d->currentCharFormat = -1;
}

/*!
    If there is a selection, its content is deleted; otherwise does
    nothing.

    \sa hasSelection()
*/
void QTextCursor::removeSelectedText()
{
    if (!d || !d->priv || d->position == d->anchor)
        return;

    d->remove();
    d->setX();
}

/*!
    Returns the start of the selection or position() if the
    cursor doesn't have a selection.

    \sa selectionEnd() position() anchor()
*/
int QTextCursor::selectionStart() const
{
    if (!d || !d->priv)
        return -1;
    return qMin(d->position, d->adjusted_anchor);
}

/*!
    Returns the end of the selection or position() if the cursor
    doesn't have a selection.

    \sa selectionStart() position() anchor()
*/
int QTextCursor::selectionEnd() const
{
    if (!d || !d->priv)
        return -1;
    return qMax(d->position, d->adjusted_anchor);
}

static void getText(QString &text, QTextDocumentPrivate *priv, const QString &docText, int pos, int end)
{
    while (pos < end) {
        QTextDocumentPrivate::FragmentIterator fragIt = priv->find(pos);
        const QTextFragmentData * const frag = fragIt.value();

        const int offsetInFragment = qMax(0, pos - fragIt.position());
        const int len = qMin(int(frag->size - offsetInFragment), end - pos);

        text += QString(docText.constData() + frag->stringPosition + offsetInFragment, len);
        pos += len;
    }
}

/*!
    Returns the current selection's text (which may be empty). This
    only returns the text, with no rich text formatting information.
    If you want a document fragment (i.e. formatted rich text) use
    selection() instead.
*/
QString QTextCursor::selectedText() const
{
    if (!d || !d->priv || d->position == d->anchor)
        return QString();

    const QString docText = d->priv->buffer();
    QString text;

    QTextTable *table = d->complexSelectionTable();
    if (table) {
        int row_start, col_start, num_rows, num_cols;
        selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

        Q_ASSERT(row_start != -1);
        for (int r = row_start; r < row_start + num_rows; ++r) {
            for (int c = col_start; c < col_start + num_cols; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                int rspan = cell.rowSpan();
                int cspan = cell.columnSpan();
                if (rspan != 1) {
                    int cr = cell.row();
                    if (cr != r)
                        continue;
                }
                if (cspan != 1) {
                    int cc = cell.column();
                    if (cc != c)
                        continue;
                }

                getText(text, d->priv, docText, cell.firstPosition(), cell.lastPosition());
            }
        }
    } else {
        getText(text, d->priv, docText, selectionStart(), selectionEnd());
    }

    return text;
}

/*!
    Returns the current selection (which may be empty) with all its
    formatting information. If you just want the selected text (i.e.
    plain text) use selectedText() instead.
*/
QTextDocumentFragment QTextCursor::selection() const
{
    return QTextDocumentFragment(*this);
}

/*!
    Returns the block that contains the cursor.
*/
QTextBlock QTextCursor::block() const
{
    if (!d || !d->priv)
        return QTextBlock();
    return d->block();
}

/*!
    Returns the block format of the block the cursor is in.

    \sa setBlockFormat() charFormat()
 */
QTextBlockFormat QTextCursor::blockFormat() const
{
    if (!d || !d->priv)
        return QTextBlockFormat();

    return d->block().blockFormat();
}

/*!
    Sets the block format of the current block (or all blocks that
    are contained in the selection) to \a format.

    \sa blockFormat()
*/
void QTextCursor::setBlockFormat(const QTextBlockFormat &format)
{
    if (!d || !d->priv)
        return;

    d->setBlockFormat(format, QTextDocumentPrivate::SetFormat);
}

/*!
    Modifies the block format of the current block (or all blocks that
    are contained in the selection) with the block format specified by
    \a modifier.

    \sa setBlockFormat()
*/
void QTextCursor::mergeBlockFormat(const QTextBlockFormat &modifier)
{
    if (!d || !d->priv)
        return;

    d->setBlockFormat(modifier, QTextDocumentPrivate::MergeFormat);
}

/*!
    Returns the block character format of the block the cursor is in.

    The block char format is the format used when inserting text at the
    beginning of a block.

    \sa setBlockCharFormat()
 */
QTextCharFormat QTextCursor::blockCharFormat() const
{
    if (!d || !d->priv)
        return QTextCharFormat();

    return d->block().charFormat();
}

/*!
    Sets the block char format of the current block (or all blocks that
    are contained in the selection) to \a format.

    \sa blockCharFormat()
*/
void QTextCursor::setBlockCharFormat(const QTextCharFormat &format)
{
    if (!d || !d->priv)
        return;

    d->setBlockCharFormat(format, QTextDocumentPrivate::SetFormat);
}

/*!
    Modifies the block char format of the current block (or all blocks that
    are contained in the selection) with the block format specified by
    \a modifier.

    \sa setBlockCharFormat()
*/
void QTextCursor::mergeBlockCharFormat(const QTextCharFormat &modifier)
{
    if (!d || !d->priv)
        return;

    d->setBlockCharFormat(modifier, QTextDocumentPrivate::MergeFormat);
}
/*!
    Returns the format of the character immediately before the
    cursor position().

    \sa insertText(), blockFormat()
 */
QTextCharFormat QTextCursor::charFormat() const
{
    if (!d || !d->priv)
        return QTextCharFormat();

    int idx = d->currentCharFormat;
    if (idx == -1) {
        int pos = d->position - 1;
        if (pos == -1) {
            idx = d->priv->blockCharFormatIndex(d->priv->blockMap().firstNode());
        } else {
            Q_ASSERT(pos >= 0 && pos < d->priv->length());


            QTextDocumentPrivate::FragmentIterator it = d->priv->find(pos);
            Q_ASSERT(!it.atEnd());
            idx = it.value()->format;
        }
    }

    QTextCharFormat cfmt = d->priv->formatCollection()->charFormat(idx);
    cfmt.clearProperty(QTextFormat::ObjectIndex);

    Q_ASSERT(cfmt.isValid());
    return cfmt;
}

/*!
    Set the character format to the given \a format for the current selection.
    Does nothing if the cursor does not have a selection.

    \sa hasSelection()
*/
void QTextCursor::setCharFormat(const QTextCharFormat &format)
{
    if (!d || !d->priv)
        return;
    if (d->position == d->anchor) {
        d->currentCharFormat = d->priv->formatCollection()->indexForFormat(format);
        return;
    }
    d->setCharFormat(format, QTextDocumentPrivate::SetFormat);
}

/*!
    Applies all the properties set in \a modifier to all the character formats
    that are part of the selection. Does nothing if the cursor does not
    have a selection.

    \sa hasSelection()
*/
void QTextCursor::mergeCharFormat(const QTextCharFormat &modifier)
{
    if (!d || !d->priv)
        return;
    if (d->position == d->anchor) {
        QTextCharFormat format = charFormat();
        format.merge(modifier);
        d->currentCharFormat = d->priv->formatCollection()->indexForFormat(format);
        return;
    }

    d->setCharFormat(modifier, QTextDocumentPrivate::MergeFormat);
}

/*!
    Returns true if the cursor is at the start of a block; otherwise
    returns false.

    \sa atBlockEnd(), atStart()
*/
bool QTextCursor::atBlockStart() const
{
    if (!d || !d->priv)
        return false;

    return d->position == d->block().position();
}

/*!
    Returns true if the cursor is at the end of a block; otherwise
    returns false.

    \sa atBlockStart(), atEnd()
*/
bool QTextCursor::atBlockEnd() const
{
    if (!d || !d->priv)
        return false;

    return d->position == d->block().position() + d->block().length() - 1;
}

/*!
    Returns true if the cursor is at the start of the document;
    otherwise returns false.

    \sa atBlockStart(), atEnd()
*/
bool QTextCursor::atStart() const
{
    if (!d || !d->priv)
        return false;

    return d->position == 0;
}

/*!
    Returns true if the cursor is at the end of the document;
    otherwise returns false.

    \sa atStart(), atBlockEnd()
*/
bool QTextCursor::atEnd() const
{
    if (!d || !d->priv)
        return false;

    return d->position == d->priv->length() - 1;
}

/*!
    Inserts a new empty block at the cursor position() with the
    current blockFormat() and charFormat().

    \sa setBlockFormat()
*/
void QTextCursor::insertBlock()
{
    insertBlock(blockFormat());
}

/*!
    \overload

    Inserts a new empty block at the cursor position() with block
    format \a format and the current charFormat() as block char format.

    \sa setBlockFormat()
*/
void QTextCursor::insertBlock(const QTextBlockFormat &format)
{
    QTextCharFormat charFmt = charFormat();
    charFmt.clearProperty(QTextFormat::ObjectType);
    insertBlock(format, charFmt);
}

/*!
    \overload

    Inserts a new empty block at the cursor position() with block
    format \a format and \a charFormat as block char format.

    \sa setBlockFormat()
*/
void QTextCursor::insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat)
{
    if (!d || !d->priv)
        return;

    d->priv->beginEditBlock();
    d->remove();
    d->insertBlock(format, charFormat);
    d->priv->endEditBlock();
}

/*!
    Inserts a new block at the current position and makes it the first
    list item of a newly created list with the given \a format. Returns
    the created list.

    \sa currentList() createList() insertBlock()
 */
QTextList *QTextCursor::insertList(const QTextListFormat &format)
{
    insertBlock();
    return createList(format);
}

/*!
    \overload

    Inserts a new block at the current position and makes it the first
    list item of a newly created list with the given \a style. Returns
    the created list.

    \sa currentList(), createList(), insertBlock()
 */
QTextList *QTextCursor::insertList(QTextListFormat::Style style)
{
    insertBlock();
    return createList(style);
}

/*!
    Creates and returns a new list with the given \a format, and makes the
    current paragraph the cursor is in the first list item.

    \sa insertList() currentList()
 */
QTextList *QTextCursor::createList(const QTextListFormat &format)
{
    if (!d || !d->priv)
        return 0;

    QTextList *list = static_cast<QTextList *>(d->priv->createObject(format));
    QTextBlockFormat modifier;
    modifier.setObjectIndex(list->objectIndex());
    mergeBlockFormat(modifier);
    return list;
}

/*!
    \overload

    Creates and returns a new list with the given \a style, making the
    cursor's current paragraph the first list item.

    The style to be used is defined by the QTextListFormat::Style enum.

    \sa insertList() currentList()
 */
QTextList *QTextCursor::createList(QTextListFormat::Style style)
{
    QTextListFormat fmt;
    fmt.setStyle(style);
    return createList(fmt);
}

/*!
    Returns the current list if the cursor position() is inside a
    block that is part of a list; otherwise returns 0.

    \sa insertList() createList()
 */
QTextList *QTextCursor::currentList() const
{
    if (!d || !d->priv)
        return 0;

    QTextBlockFormat b = blockFormat();
    QTextObject *o = d->priv->objectForFormat(b);
    return qobject_cast<QTextList *>(o);
}

/*!
    \fn QTextTable *QTextCursor::insertTable(int rows, int columns)

    \overload

    Creates a new table with the given number of \a rows and \a columns,
    inserts it at the current cursor position() in the document, and returns
    the table object. The cursor is moved to the beginning of the first cell.

    There must be at least one row and one column in the table.

    \sa currentTable()
 */
QTextTable *QTextCursor::insertTable(int rows, int cols)
{
    return insertTable(rows, cols, QTextTableFormat());
}

/*!
    \fn QTextTable *QTextCursor::insertTable(int rows, int columns, const QTextTableFormat &format)

    Creates a new table with the given number of \a rows and \a columns
    in the specified \a format, inserts it at the current cursor position()
    in the document, and returns the table object. The cursor is moved to
    the beginning of the first cell.

    There must be at least one row and one column in the table.

    \sa currentTable()
*/
QTextTable *QTextCursor::insertTable(int rows, int cols, const QTextTableFormat &format)
{
    if(!d || !d->priv || rows == 0 || cols == 0)
        return 0;

    int pos = d->position;
    QTextTable *t = QTextTablePrivate::createTable(d->priv, d->position, rows, cols, format);
    d->setPosition(pos+1);
    // ##### what should we do if we have a selection?
    d->anchor = d->position;
    d->adjusted_anchor = d->anchor;
    return t;
}

/*!
    Returns a pointer to the current table if the cursor position()
    is inside a block that is part of a table; otherwise returns 0.

    \sa insertTable()
*/
QTextTable *QTextCursor::currentTable() const
{
    if(!d || !d->priv)
        return 0;

    QTextFrame *frame = d->priv->frameAt(d->position);
    while (frame) {
        QTextTable *table = qobject_cast<QTextTable *>(frame);
        if (table)
            return table;
        frame = frame->parentFrame();
    }
    return 0;
}

/*!
    Inserts a frame with the given \a format at the current cursor position(),
    moves the cursor position() inside the frame, and returns the frame.

    If the cursor holds a selection, the whole selection is moved inside the
    frame.

    \sa hasSelection()
*/
QTextFrame *QTextCursor::insertFrame(const QTextFrameFormat &format)
{
    if (!d || !d->priv)
        return 0;

    return d->priv->insertFrame(selectionStart(), selectionEnd(), format);
}

/*!
    Returns a pointer to the current frame. Returns 0 if the cursor is invalid.

    \sa insertFrame()
*/
QTextFrame *QTextCursor::currentFrame() const
{
    if(!d || !d->priv)
        return 0;

    return d->priv->frameAt(d->position);
}


/*!
    Inserts the text \a fragment at the current position().
*/
void QTextCursor::insertFragment(const QTextDocumentFragment &fragment)
{
    if (!d || !d->priv || fragment.isEmpty())
        return;

    d->priv->beginEditBlock();
    d->remove();
    fragment.d->insert(*this);
    d->priv->endEditBlock();
}

/*!
    Inserts the image defined by \a format at the current position().
*/
void QTextCursor::insertImage(const QTextImageFormat &format)
{
    insertText(QString(QChar::ObjectReplacementCharacter), format);
}

/*!
    \overload

    Convenience method for inserting the image with the given \a name at the
    current position().
*/
void QTextCursor::insertImage(const QString &name)
{
    QTextImageFormat format;
    format.setName(name);
    insertImage(format);
}

/*!
    \fn bool QTextCursor::operator!=(const QTextCursor &other) const

    Returns true if the \a other cursor is at a different position in
    the document as this cursor; otherwise returns false.
*/
bool QTextCursor::operator!=(const QTextCursor &rhs) const
{
    return !operator==(rhs);
}

/*!
    \fn bool QTextCursor::operator<(const QTextCursor &other) const

    Returns true if the \a other cursor is positioned later in the
    document than this cursor; otherwise returns false.
*/
bool QTextCursor::operator<(const QTextCursor &rhs) const
{
    if (!d)
        return !!rhs.d;

    if (!rhs.d)
        return false;

    Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator<", "cannot compare cursors attached to different documents");

    return d->position < rhs.d->position;
}

/*!
    \fn bool QTextCursor::operator<=(const QTextCursor &other) const

    Returns true if the \a other cursor is positioned later or at the
    same position in the document as this cursor; otherwise returns
    false.
*/
bool QTextCursor::operator<=(const QTextCursor &rhs) const
{
    if (!d)
        return true;

    if (!rhs.d)
        return false;

    Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator<=", "cannot compare cursors attached to different documents");

    return d->position <= rhs.d->position;
}

/*!
    \fn bool QTextCursor::operator==(const QTextCursor &other) const

    Returns true if the \a other cursor is at the same position in the
    document as this cursor; otherwise returns false.
*/
bool QTextCursor::operator==(const QTextCursor &rhs) const
{
    if (!d)
        return !rhs.d;

    if (!rhs.d)
        return false;

    return d->position == rhs.d->position && d->priv == rhs.d->priv;
}

/*!
    \fn bool QTextCursor::operator>=(const QTextCursor &other) const

    Returns true if the \a other cursor is positioned earlier or at the
    same position in the document as this cursor; otherwise returns
    false.
*/
bool QTextCursor::operator>=(const QTextCursor &rhs) const
{
    if (!d)
        return false;

    if (!rhs.d)
        return true;

    Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator>=", "cannot compare cursors attached to different documents");

    return d->position >= rhs.d->position;
}

/*!
    \fn bool QTextCursor::operator>(const QTextCursor &other) const

    Returns true if the \a other cursor is positioned earlier in the
    document than this cursor; otherwise returns false.
*/
bool QTextCursor::operator>(const QTextCursor &rhs) const
{
    if (!d)
        return false;

    if (!rhs.d)
        return true;

    Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator>", "cannot compare cursors attached to different documents");

    return d->position > rhs.d->position;
}

/*!
    Indicates the start of a block of editing operations on the
    document that should appear as a single operation from an
    undo/redo point of view.

    For example:

    \code
    QTextCursor cursor(textDocument);
    cursor.beginEditBlock();
    cursor.insertText("Hello");
    cursor.insertText("World");
    cursor.endEditBlock();

    textDocument->undo();
    \endcode

    The call to undo() will cause both insertions to be undone,
    causing both "World" and "Hello" to be removed.

    \sa endEditBlock()
 */
void QTextCursor::beginEditBlock()
{
    if (!d || !d->priv)
        return;

    d->priv->beginEditBlock();
}

/*!
    Like beginEditBlock() indicates the start of a block of editing operations
    that should appear as a single operation for undo/redo. However unlike
    beginEditBlock() it does not start a new block but reverses the previous call to
    endEditBlock() and therefore makes following operations part of the previous edit block created.

    For example:

    \code
    QTextCursor cursor(textDocument);
    cursor.beginEditBlock();
    cursor.insertText("Hello");
    cursor.insertText("World");
    cursor.endEditBlock();

    ...

    cursor.joinPreviousEditBlock();
    cursor.insertText("Hey");
    cursor.endEditBlock();

    textDocument->undo();
    \endcode

    The call to undo() will cause all three insertions to be undone.

    \sa beginEditBlock(), endEditBlock()
 */
void QTextCursor::joinPreviousEditBlock()
{
    if (!d || !d->priv)
        return;

    d->priv->joinPreviousEditBlock();
}

/*!
    Indicates the end of a block of editing operations on the document
    that should appear as a single operation from an undo/redo point
    of view.

    \sa beginEditBlock()
 */

void QTextCursor::endEditBlock()
{
    if (!d || !d->priv)
        return;

    d->priv->endEditBlock();
}

/*!
    Returns true if this cursor and \a other are copies of each other, i.e.
    one of them was created as a copy of the other and neither has moved since.
    This is much stricter than equality.

    \sa operator=() operator==()
*/
bool QTextCursor::isCopyOf(const QTextCursor &other) const
{
    return d == other.d;
}
