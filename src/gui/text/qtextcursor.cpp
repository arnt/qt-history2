#include "qtextcursor.h"
#include "qtextcursor_p.h"
#include "qglobal.h"
#include "qtextdocumentfragment.h"
#include "qtextdocumentfragment_p.h"
#include "qtextlist.h"
#include "qtexttable.h"
#include "qtexttable_p.h"

#include <qtextlayout.h>
#include <qdebug.h>

#include "qtextdocument_p.h"

enum {
    AdjustPrev = 0x1,
    AdjustUp = 0x3,
    AdjustNext = 0x4,
    AdjustDown = 0x12
};

QTextCursorPrivate::QTextCursorPrivate(const QTextPieceTable *table)
    : x(0), position(0), anchor(0), adjusted_anchor(0),
      pieceTable(const_cast<QTextPieceTable *>(table))
{
    Q_ASSERT(pieceTable);
    pieceTable->addCursor(this);
}

QTextCursorPrivate::QTextCursorPrivate(const QTextCursorPrivate &rhs)
    : QSharedData(rhs)
{
    position = rhs.position;
    anchor = rhs.anchor;
    adjusted_anchor = rhs.adjusted_anchor;
    pieceTable = rhs.pieceTable;
    x = rhs.x;
    pieceTable->addCursor(this);
}

QTextCursorPrivate::~QTextCursorPrivate()
{
    pieceTable->removeCursor(this);
}

void QTextCursorPrivate::adjustPosition(int positionOfChange, int charsAddedOrRemoved, UndoCommand::Operation op)
{
    // not(!) <= , so that inserting text adjusts the cursor correctly
    if (position < positionOfChange ||
        (position == positionOfChange && op == UndoCommand::KeepCursor))
        return;

    if (charsAddedOrRemoved < 0 && position < positionOfChange - charsAddedOrRemoved)
        position = positionOfChange;
    else
        position += charsAddedOrRemoved;
    if (charsAddedOrRemoved < 0 && anchor < positionOfChange - charsAddedOrRemoved) {
        anchor = positionOfChange;
        adjusted_anchor = positionOfChange;
    } else {
        anchor += charsAddedOrRemoved;
        adjusted_anchor += charsAddedOrRemoved;
    }
}

void QTextCursorPrivate::setPosition(int newPosition)
{
    Q_ASSERT(newPosition >= 0 && newPosition < pieceTable->length());
    position = newPosition;
}

void QTextCursorPrivate::setX()
{
    QTextBlockIterator block = pieceTable->blocksFind(position);
    const QTextLayout *layout = block.layout();
    int pos = position - block.position();

    QTextLine line = layout->findLine(pos);
    if (line.isValid())
        x = line.cursorToX(pos);
}

void QTextCursorPrivate::remove()
{
    if (anchor == position)
        return;
    int pos1 = position;
    int pos2 = adjusted_anchor;
    UndoCommand::Operation op = UndoCommand::KeepCursor;
    if (pos1 > pos2) {
        pos1 = anchor;
        pos2 = position;
        op = UndoCommand::MoveCursor;
    }

    pieceTable->remove(pos1, pos2-pos1, op);
    anchor = position;
}

bool QTextCursorPrivate::canDelete(int pos) const
{
    QTextPieceTable::FragmentIterator fit = pieceTable->find(pos);
    QTextCharFormat fmt = pieceTable->formatCollection()->charFormat((*fit)->format);
    return !fmt.nonDeletable();
}

void QTextCursorPrivate::insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat)
{
    QTextFormatCollection *formats = pieceTable->formatCollection();
    int idx = formats->indexForFormat(format);
    Q_ASSERT(formats->format(idx).isBlockFormat());

    pieceTable->insertBlock(position, idx, formats->indexForFormat(charFormat));
}

QTextTable *QTextCursorPrivate::tableAt(int position) const
{
    QTextFrame *frame = pieceTable->frameAt(position);
    while (frame) {
        QTextTable *table = qt_cast<QTextTable *>(frame);
        if (table)
            return table;
        frame = frame->parent();
    }
    return 0;
}


void QTextCursorPrivate::adjustCursor()
{
    adjusted_anchor = anchor;
    if (position == anchor)
        return;

    QTextFrame *f_position = pieceTable->frameAt(position);
    QTextFrame *f_anchor = pieceTable->frameAt(adjusted_anchor);

    if (f_position != f_anchor) {
        // find common parent frame
        QList<QTextFrame *> positionChain;
        QList<QTextFrame *> anchorChain;
        QTextFrame *f = f_position;
        while (f) {
            positionChain.prepend(f);
            f = f->parent();
        }
        f = f_anchor;
        while (f) {
            anchorChain.prepend(f);
            f = f->parent();
        }
        Q_ASSERT(positionChain.at(0) == anchorChain.at(0));
        int i = 1;
        int l = qMin(positionChain.size(), anchorChain.size());
        for (; i < l; ++i) {
            if (positionChain.at(i) != anchorChain.at(i))
                break;
        }

        if (position < adjusted_anchor) {
            if (i < positionChain.size())
                position = positionChain.at(i)->startPosition() - 1;
            if (i < anchorChain.size())
                adjusted_anchor = anchorChain.at(i)->endPosition() + 1;
        } else {
            if (i < positionChain.size())
                position = positionChain.at(i)->endPosition() - 1;
            if (i < anchorChain.size())
                adjusted_anchor = anchorChain.at(i)->startPosition() + 1;
        }

        f_position = positionChain.at(i-1);
    }

    // same frame, either need to adjust to cell boundaries or return
    QTextTable *table = qt_cast<QTextTable *>(f_position);
    if (!table)
        return;

    QTextTableCell c_position = table->cellAt(position);
    QTextTableCell c_anchor = table->cellAt(adjusted_anchor);
    if (c_position != c_anchor) {
        // adjust to cell boundaries
        if (position < adjusted_anchor) {
            position = c_position.startPosition();
            adjusted_anchor = c_anchor.endPosition();
        } else {
            position = c_position.endPosition();
            adjusted_anchor = c_anchor.startPosition();
        }
    }
}

bool QTextCursorPrivate::moveTo(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode)
{
    bool adjustX = true;
    QTextBlockIterator blockIt = block();

    if (op >= QTextCursor::Left && op <= QTextCursor::WordRight
        && blockIt.blockFormat().direction() == QTextBlockFormat::RightToLeft) {
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
    QTextLine line = layout->findLine(relativePos);

    Q_ASSERT(pieceTable->frameAt(position) == pieceTable->frameAt(adjusted_anchor));

    switch(op) {
    case QTextCursor::NoMove:
        return true;

    case QTextCursor::Start:
        position = 0;
        break;
    case QTextCursor::StartOfLine: {

        if (!line.isValid())
            break;
        setPosition(blockIt.position() + line.from());

        break;
    }
    case QTextCursor::PreviousBlock: {
        if (blockIt == pieceTable->blocksBegin())
            return false;
        --blockIt;

        setPosition(blockIt.position());
        break;
    }
    case QTextCursor::PreviousCharacter:
    case QTextCursor::Left:
        setPosition(pieceTable->previousCursorPosition(position, QTextLayout::SkipCharacters));
        break;
    case QTextCursor::PreviousWord:
    case QTextCursor::WordLeft:
        setPosition(pieceTable->previousCursorPosition(position, QTextLayout::SkipWords));
        break;
    case QTextCursor::Up: {
        int i = line.line() - 1;
        if (i == -1) {
            if (blockIt == pieceTable->blocksBegin())
                return false;
            int blockPosition = blockIt.position();
            QTextTable *table = qt_cast<QTextTable *>(pieceTable->frameAt(blockPosition));
            if (table) {
                QTextTableCell cell = table->cellAt(blockPosition);
                if (cell.startPosition() == blockPosition) {
                    int row = cell.row() - 1;
                    if (row >= 0) {
                        blockPosition = table->cellAt(row, cell.column()).endPosition();
                    } else {
                        // move to line above the table
                        blockPosition = table->startPosition() - 1;
                    }
                    blockIt = pieceTable->blocksFind(blockPosition);
                } else {
                    --blockIt;
                }
            } else {
                --blockIt;
            }
            layout = blockIt.layout();
            i = layout->numLines()-1;
        }
        if (layout->numLines()) {
            QTextLine line = layout->lineAt(i);
            setPosition(line.xToCursor(x) + blockIt.position());
        } else {
            setPosition(blockIt.position());
        }
        adjustX = false;
        break;
    }

    case QTextCursor::End:
        position = pieceTable->length() - 1;
        break;
    case QTextCursor::EndOfLine: {
        if (!line.isValid())
            break;
        // currently we don't draw the space at the end, so move to the next
        // reasonable position.
        setPosition(blockIt.position() + line.from() + line.length() - 1);

        break;
    }
    case QTextCursor::NextBlock: {
        ++blockIt;
        if (blockIt.atEnd())
            return false;

        setPosition(blockIt.position());
        break;
    }
    case QTextCursor::NextCharacter:
    case QTextCursor::Right:
        setPosition(pieceTable->nextCursorPosition(position, QTextLayout::SkipCharacters));
        break;
    case QTextCursor::NextWord:
    case QTextCursor::WordRight:
        setPosition(pieceTable->nextCursorPosition(position, QTextLayout::SkipWords));
        break;

    case QTextCursor::Down: {
        int i = line.line() + 1;

        if (i >= layout->numLines()) {
            int blockPosition = blockIt.position() + blockIt.length() - 1;
            QTextTable *table = qt_cast<QTextTable *>(pieceTable->frameAt(blockPosition));
            if (table) {
                QTextTableCell cell = table->cellAt(blockPosition);
                if (cell.endPosition() == blockPosition) {
                    int row = cell.row() + cell.rowSpan();
                    if (row < table->rows()) {
                        blockPosition = table->cellAt(row, cell.column()).startPosition();
                    } else {
                        // move to line below the table
                        blockPosition = table->endPosition() + 1;
                    }
                    blockIt = pieceTable->blocksFind(blockPosition);
                } else {
                    ++blockIt;
                }
            } else {
                ++blockIt;
            }

            if (blockIt == pieceTable->blocksEnd())
                return false;
            layout = blockIt.layout();
            i = 0;
        }
        if (layout->numLines()) {
            QTextLine line = layout->lineAt(i);
            setPosition(line.xToCursor(x) + blockIt.position());
        } else {
            setPosition(blockIt.position());
        }
        adjustX = false;
        break;
    }
    }
    if (mode == QTextCursor::MoveAnchor) {
        anchor = position;
        adjusted_anchor = position;
    } else {
        adjustCursor();
    }

    if (adjustX)
        setX();

    return true;
}

/*!
    \class QTextCursor qtextcursor.h
    \brief The QTextCursor class offers an API to access and modify QTextDocuments.

    \ingroup text

    A QTextCursor is an object that can be used to access and
    manipulate a QTextDocument. It is a combination of what is usually
    seen as a cursor and a selection.

    A QTextCursor has an \a anchor and a current position. The range
    between anchor and position is the current selection. When there
    is no selection both anchor and position point to the same
    position in the document.

    A cursor can be used to access, modify and set the current
    character and block formats. It canbe used to insert lists, list
    items, tables or fragments of a document at it's current position.

    You can move a cursor using \a moveTo, creating selections if
    needed, insert and remove characters and selections.

    Movements of the cursor are limited to valid cursor positions. In
    Latin writing this is usually after every character in the
    text. In other writing systems however cursor movements is limited
    to so called clusters (e.g. a syllable in Devanagari, or a base
    letter plus diacritics). Functions as \a moveTo and \a deleteChar
    limit cursor movement to these valid positions.

*/

/*!
    \enum QTextCursor::MoveOperation
    \value NoMove Keep the cursor where it is

    \value Start Move to the start of the document
    \value StartOfLine Move to the start of the current line
    \value PreviousBlock move to the start of the previous block
    \value PreviousCharacter move to the previous character
    \value PreviousWord move to the beginning of the previous word
    \value Up move one line up
    \value Left move one character to the left
    \value WordLeft move one word to the left

    \value End move to the end of the document
    \value EndOfLine move to the end of the current line
    \value NextBlock move to the beginning of the next block
    \value NextCharacter move to the next character
    \value NextWord move to the next word
    \value Down move one line down
    \value Right move one character to the right
    \value WordRight move one word to the right

    \sa moveTo()
*/

/*!
    \enum QTextCursor::MoveMode

    \value MoveAnchor Moves the anchor to the same position as the cursor itself.
    \value KeepAnchor Keeps the anchor where it was.
*/

/*!
    Constructs a null cursor.
 */
QTextCursor::QTextCursor()
    : d(0)
{
}

/*!
    Constructs a cursor pointing to the beginning of \a document.
 */
QTextCursor::QTextCursor(QTextDocument *document)
    : d(new QTextCursorPrivate(const_cast<const QTextDocument*>(document)->d_func()->pieceTable))
{
}


QTextCursor::QTextCursor(const QTextBlockIterator &block)
    : d(new QTextCursorPrivate(block.pieceTable()))
{
    d->position = block.position();
}


/*!
  \internal
 */
QTextCursor::QTextCursor(const QTextPieceTable *pt, int pos)
    : d(new QTextCursorPrivate(pt))
{
    d->anchor = d->position = pos;

    d->setX();
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
    cursor gets created when using the default constructor.
 */
bool QTextCursor::isNull() const
{
    return !d;
}

/*!
    Moves the cursor to the absolute position \a pos.

    \sa position()
 */
void QTextCursor::setPosition(int pos, MoveMode m)
{
    if (!d)
        return;
    d->setPosition(pos);
    if (m == MoveAnchor) {
        d->anchor = pos;
        d->adjusted_anchor = pos;
    }
    d->setX();
}

/*!
    Returns the absolute position of the cursor within the document.
 */
int QTextCursor::position() const
{
    if (!d)
        return -1;
    return d->position;
}

int QTextCursor::anchor() const
{
    if (!d)
        return -1;
    return d->anchor;
}

/*!
  Moves the cursor \a n times by MoveOperation \op, using MoveMode \a mode.

  Using KeepAnchor as the \a mode, makes it possible to have a
  selection in the cursor. It corresponds to moving the cursor with a
  pressed Shift Key in a widget.

  \sa MoveOperation, MoveMode
*/
bool QTextCursor::moveTo(MoveOperation op, MoveMode mode, int n)
{
    if (!d)
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
        if (!d->moveTo(op, mode))
            return false;
    }
    return true;
}

/*!
    Inserts \a text at the current position, using the current char format.

    \sa charFormat()
 */
void QTextCursor::insertText(const QString &text)
{
    insertText(text, charFormat());
}

/*!
    \overload

    Inserts \a text at the current position with the given \a format.
 */
void QTextCursor::insertText(const QString &text, const QTextCharFormat &format)
{
    if (!d || text.isEmpty())
        return;

    Q_ASSERT(format.isValid());

    d->pieceTable->beginEditBlock();

    d->remove();

    QTextFormatCollection *formats = d->pieceTable->formatCollection();
    int formatIdx = formats->indexForFormat(format);
    Q_ASSERT(formats->format(formatIdx).isCharFormat());

    QTextBlockFormat blockFmt = blockFormat();

    QStringList blocks = text.split(QChar::ParagraphSeparator);
    for (int i = 0; i < blocks.size(); ++i) {
        if (i > 0)
            d->insertBlock(blockFmt, format);
        d->pieceTable->insert(d->position, blocks.at(i), formatIdx);
    }

    d->pieceTable->endEditBlock();
}

/*!
  Deletes the next character after the current cursor position. If
  the cursor contains a selection, the selection is deleted instead.


*/
void QTextCursor::deleteChar() {
    if (!d)
        return;

    if (d->position == d->anchor) {
        if (!d->canDelete(d->position))
            return;
        d->adjusted_anchor = d->anchor =
                             d->pieceTable->nextCursorPosition(d->anchor, QTextLayout::SkipCharacters);
    }
    d->remove();
    d->setX();
}

/*!
  Deletes the last character before the current cursor position. If
  the cursor contains a selection, the selection is deleted instead.
*/
void QTextCursor::deletePreviousChar()
{
    if (!d)
        return;

    if (d->position == d->anchor) {
        if (d->anchor <= 1 || !d->canDelete(d->anchor-1))
            return;
        d->anchor--;
        d->adjusted_anchor = d->anchor;
    }

    d->remove();
    d->setX();
}

/*!
  Returns true if the cursor contains a selection.
*/
bool QTextCursor::hasSelection() const
{
    return d && d->position != d->anchor;
}

/*!
  Clears the current selection.
*/
void QTextCursor::clearSelection()
{
    if (!d)
        return;
    d->adjusted_anchor = d->anchor = d->position;
}

/*!
  Removes the content of current selection. Does nothing if the cursor doesn't have a selection.
*/
void QTextCursor::removeSelectedText()
{
    if (!d || d->position == d->anchor)
        return;

    d->remove();
    d->setX();
}

/*!
  Returns the start of the selection or the current position if the cursor doesn't have a selection.
*/
int QTextCursor::selectionStart() const
{
    if (!d)
        return -1;
    return qMin(d->position, d->adjusted_anchor);
}

/*!
  Returns the end of the selection or the current position if the cursor doesn't have a selection.
*/
int QTextCursor::selectionEnd() const
{
    if (!d)
        return -1;
    return qMax(d->position, d->adjusted_anchor);
}

/*!
    Sets the block format of the block the cursor is in to \a format.

    \sa blockFormat()
 */
void QTextCursor::setBlockFormat(const QTextBlockFormat &format)
{
    if (!d)
        return;

    QTextBlockIterator it = d->block();
    d->pieceTable->setBlockFormat(it, it, format, QTextPieceTable::SetFormat);
}

QTextBlockIterator QTextCursor::block() const
{
    return d->block();
}

/*!
    Returns the block format of the block the cursor is in.

    \sa setBlockFormat()
 */
QTextBlockFormat QTextCursor::blockFormat() const
{
    if (!d)
        return QTextBlockFormat();

    return d->block().blockFormat();
}

/*!
  Applies all the properties set in \a modifier to all the formats that are part
  of the selection. Does nothing if the cursor doesn't have a selection.
*/
void QTextCursor::applyCharFormatModifier(const QTextCharFormat &modifier)
{
    if (!d || d->position == d->anchor)
        return;

    int pos1 = d->position;
    int pos2 = d->adjusted_anchor;
    if (pos1 > pos2) {
        pos1 = d->adjusted_anchor;
        pos2 = d->position;
    }

    d->pieceTable->setCharFormat(pos1, pos2-pos1, modifier, QTextPieceTable::MergeFormat);
}

/*!
  Modifies the block format of the current block (or all blocks
  that are contained in the selection) with \a modifier.
*/
void QTextCursor::applyBlockFormatModifier(const QTextBlockFormat &modifier)
{
    if (!d)
        return;

    int pos1 = d->position;
    int pos2 = d->adjusted_anchor;
    if (pos1 > pos2) {
        pos1 = d->anchor;
        pos2 = d->position;
    }

    QTextBlockIterator from = d->pieceTable->blocksFind(pos1);
    QTextBlockIterator to = d->pieceTable->blocksFind(pos2);
    d->pieceTable->setBlockFormat(from, to, modifier, QTextPieceTable::MergeFormat);
}

/*!
    Returns the format of the character the cursor points to.

    \sa insertText(), position()
 */
QTextCharFormat QTextCursor::charFormat() const
{
    if (!d)
        return QTextCharFormat();

    int pos = d->position - 1;
    if (pos < 0)
        pos = 0;
    Q_ASSERT(pos >= 0 && pos < d->pieceTable->length());


    QTextPieceTable::FragmentIterator it = d->pieceTable->find(pos);
    Q_ASSERT(!it.atEnd());
    int idx = it.value()->format;

    QTextCharFormat cfmt = d->pieceTable->formatCollection()->charFormat(idx);
    cfmt.setGroup(0);
    Q_ASSERT(cfmt.isValid());
    return cfmt;
}

/*!
  Returns true if the cursor is at the start of a block
*/
bool QTextCursor::atBlockStart() const
{
    if (!d)
        return false;

    return d->position == d->block().position();
}

/*!
  Returns true if the cursor is at the end of a block
*/
bool QTextCursor::atEnd() const
{
    if (!d)
        return false;

    return d->position == d->pieceTable->length() - 1;
}

/*!
  Inserts a new block at the current position with the block format of
  the previous block into the document.
*/
void QTextCursor::insertBlock()
{
    if (!d)
        return;

    d->insertBlock(blockFormat(), charFormat());
}

/*!
  \overload
  Inserts a new block at the current position with block format \a format
  into the document.
*/
void QTextCursor::insertBlock(const QTextBlockFormat &format)
{
    if (!d)
        return;

    d->pieceTable->beginEditBlock();
    d->remove();
    d->insertBlock(format, charFormat());
    d->pieceTable->endEditBlock();
}

/*!
    Inserts a new block at the current position and makes it the first
    list item of a newly created list with the given \a format. Returns
    the created list.

    \sa currentList(), createList(), insertBlock()
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
QTextList *QTextCursor::insertList(int style)
{
    insertBlock();
    return createList(style);
}

/*!
    Creates and returns a new list with the given \a format and makes the
    current paragraph the cursor is in the first list item.

    \sa currentList()
 */
QTextList *QTextCursor::createList(const QTextListFormat &format)
{
    if (!d)
        return 0;

    QTextFormatCollection *c = d->pieceTable->formatCollection();
    QTextList *list = static_cast<QTextList *>(c->createGroup(format));
    QTextBlockFormat modifier;
    modifier.setGroup(list);
    applyBlockFormatModifier(modifier);
    return list;
}

/*!
    \overload

    Creates and returns a new list with the given \a style and makes the
    current paragraph the cursor is in the first list item.

    \sa currentList()
 */
QTextList *QTextCursor::createList(int style)
{
    QTextListFormat fmt;
    fmt.setStyle(style);
    return createList(fmt);
}

/*!
    Returns a pointer to the current list, if the cursor is positioned inside
    a block that is part of a list; otherwise returns a null pointer.

    \sa createList()
 */
QTextList *QTextCursor::currentList() const
{
    if (!d)
        return 0;

    QTextBlockFormat b = blockFormat();
    QTextGroup *g = b.group();
    return qt_cast<QTextList *>(g);
}

int QTextCursor::listItemNumber() const
{
    if (!d)
        return -1;
    QTextList *l = currentList();
    if (!l)
        return -1;

    return l->itemNumber(d->block());
}

QString QTextCursor::listItemText() const
{
    if (!d)
        return QString();

    QTextList *l = currentList();
    if (!l)
        return QString();

    return l->itemText(d->block());
}

/*!
    \overload

    Creates a new table with the given dimension (\a rows and \a cols), inserts
    it at the current position and returns the table object.
    The cursor is positioned at the beginning of the first cell.
 */
QTextTable *QTextCursor::insertTable(int rows, int cols)
{
    return insertTable(rows, cols, QTextTableFormat());
}

/*!
    Creates a new table with the given dimension (\a rows and \a cols)
    and the given \a format, inserts it at the current position and
    returns the table object. The cursor is positioned at the
    beginning of the first cell.
 */
QTextTable *QTextCursor::insertTable(int rows, int cols, const QTextTableFormat &format)
{
    if(!d)
        return 0;

    int pos = d->position;
    QTextTable *t = QTextTablePrivate::createTable(d->pieceTable,d->position, rows, cols, format);
    setPosition(pos+1);
    return t;
}

/*!
    Returns a pointer to the current table, if the cursor is positioned inside
    a block that is part of a table; otherwise returns a null pointer.

    \sa insertTable()
 */
QTextTable *QTextCursor::currentTable() const
{
    if(!d)
        return 0;

    return d->tableAt(d->position);
}

/*!
  Inserts a frame at the current cursor position and places the cursor inside the frame.

  If the cursor holds a selection the whole selection is moved inside the frame.
*/
QTextFrame *QTextCursor::insertFrame(const QTextFrameFormat &format)
{
    if (!d)
        return 0;

    return d->pieceTable->insertFrame(selectionStart(), selectionEnd(), format);
}


/*!
    Inserts \a fragments at the current position.

    \sa QTextFragment
 */
void QTextCursor::insertFragment(const QTextDocumentFragment &fragment)
{
    if (!d || fragment.isEmpty())
        return;

    d->pieceTable->beginEditBlock();
    d->remove();
    fragment.d->insert(*this);
    d->pieceTable->endEditBlock();
}

/*!
  Inserts the image defined by \a format at the current position.
*/
void QTextCursor::insertImage(const QTextImageFormat &format)
{
    insertText(QString(QChar::ObjectReplacementCharacter), format);
}

bool QTextCursor::operator!=(const QTextCursor &rhs) const
{
    return !operator==(rhs);
}

bool QTextCursor::operator<(const QTextCursor &rhs) const
{
    if (!d)
        return rhs.d != 0;

    if (!rhs.d)
        return false;

    Q_ASSERT_X(d->pieceTable == rhs.d->pieceTable, "QTextCursor::operator<", "cannot compare cusors attached to different documents");

    return d->position < rhs.d->position;
}

bool QTextCursor::operator<=(const QTextCursor &rhs) const
{
    if (!d)
        return true;

    if (!rhs.d)
        return false;

    Q_ASSERT_X(d->pieceTable == rhs.d->pieceTable, "QTextCursor::operator<=", "cannot compare cusors attached to different documents");

    return d->position <= rhs.d->position;
}

bool QTextCursor::operator==(const QTextCursor &rhs) const
{
    if (!d)
        return rhs.d == 0;

    if (!rhs.d)
        return false;

    return d->position == rhs.d->position && d->pieceTable == rhs.d->pieceTable;
}

bool QTextCursor::operator>=(const QTextCursor &rhs) const
{
    if (!d)
        return false;

    if (!rhs.d)
        return true;

    Q_ASSERT_X(d->pieceTable == rhs.d->pieceTable, "QTextCursor::operator>=", "cannot compare cusors attached to different documents");

    return d->position >= rhs.d->position;
}

bool QTextCursor::operator>(const QTextCursor &rhs) const
{
    if (!d)
        return false;

    if (!rhs.d)
        return true;

    Q_ASSERT_X(d->pieceTable == rhs.d->pieceTable, "QTextCursor::operator>", "cannot compare cusors attached to different documents");

    return d->position > rhs.d->position;
}

/*!
    Indicates the start of a block of editor operations on the document that should
    appear as one single operation from an undo point of view.

    For example:

    \code
    QTextCursor cursor(textDocument);
    cursor.beginEditBlock();
    cursor.insertText("Hello");
    cursor.insertText("World");
    cursor.endEditBlock();

    textDocument->undo();
    \endcode

    The call to undo() will cause both insertions to be undone and therefore
    causing "World" and "Hello" to be removed.

    \sa endEditBlock()
 */
void QTextCursor::beginEditBlock()
{
    if (!d)
        return;

    d->pieceTable->beginEditBlock();
}

/*!
    Indicates the end of a block of editor operations on the document that should
    appear as one single operation from an undo point of view.

    \sa beginEditBlock()
 */

void QTextCursor::endEditBlock()
{
    if (!d)
        return;

    d->pieceTable->endEditBlock();
}

