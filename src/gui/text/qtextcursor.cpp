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

void QTextCursorPrivate::insertBlock(const QTextBlockFormat &format)
{
    if (block().blockFormat().tableCellEndOfRow())
        moveTo(QTextCursor::NextBlock);

    QTextFormatCollection *formats = pieceTable->formatCollection();
    int idx = formats->indexForFormat(format);
    Q_ASSERT(formats->format(idx).isBlockFormat());

    pieceTable->insertBlock(position, idx, formats->indexForFormat(QTextCharFormat()));
}

QTextTable *QTextCursorPrivate::createTable(int rows, int cols, const QTextTableFormat &tableFormat)
{
    QTextFormatCollection *collection = pieceTable->formatCollection();
    QTextTable *table = qt_cast<QTextTable *>(collection->createGroup(tableFormat));
    Q_ASSERT(table);

    int pos = position;

    pieceTable->beginEditBlock();

//     qDebug("---> createTable: rows=%d, cols=%d at %d", rows, cols, pos);
    // add block after table
    QTextCharFormat charFmt;
    charFmt.setNonDeletable(true);
    int charIdx = pieceTable->formatCollection()->indexForFormat(charFmt);
    pieceTable->insertBlock(pos, pieceTable->formatCollection()->indexForFormat(QTextBlockFormat()), charIdx);
//     qDebug("      addBlock at %d", pos);

    // create table formats
    QTextBlockFormat fmt = blockFormat();
    fmt.setGroup(table);
    int cellIdx = collection->indexForFormat(fmt);
    fmt.setTableCellEndOfRow(true);
    int eorIdx = collection->indexForFormat(fmt);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            pieceTable->insertBlock(pos, cellIdx, charIdx);
// 	    qDebug("      addCell at %d", pos);
            ++pos;
        }
        pieceTable->insertBlock(pos, eorIdx, charIdx);
// 	qDebug("      addEOR at %d", pos);
        ++pos;
    }

    pieceTable->endEditBlock();

    return table;
}


QTextTable *QTextCursorPrivate::tableAt(int position) const
{
    const QVector<QTextGroup *> &groups = pieceTable->formatCollection()->formatGroups();
    QTextTable *table = 0;
    int tableStart = -1;
    for (int i = 0; i < groups.size(); ++i) {
	QTextTable *t = qt_cast<QTextTable *>(groups.at(i));
	if (!t)
	    continue;
	int start = t->start().position();
	if (start <= position && start > tableStart && t->end().position() > position)
	    // inside table
	    table = t;
    }
    return table;
}


void QTextCursorPrivate::adjustCursor(int dir)
{
    QTextTable *t_anchor_ = tableAt(anchor);
    QTextTable *t_position_ = tableAt(position);

    QTextTablePrivate *t_anchor = 0;
    QTextTablePrivate *t_position = 0;

    if (t_anchor_)
        t_anchor = t_anchor_->d_func();

    if (t_position_)
        t_position = t_position_->d_func();

    // first adjust position if needed
    if (t_position) {
        int t_position_start = t_position->start().position();
        int t_position_end = t_position->end().position();
        bool anchor_in_table = false;
        bool anchor_in_cell = false;
        if (t_anchor) {
            int t_anchor_start = t_anchor->start().position();
            int t_anchor_end = t_anchor->end().position();
            if (t_anchor_start >= t_position_start && t_anchor_end <= t_position_end) {
                anchor_in_table = true;
                if (t_position->cellStart(anchor) == t_position->cellStart(position))
                    anchor_in_cell = true;
            }
        }
        if (!anchor_in_table) {
            // ### AdjustUp/Down should use x position!
            position = (dir & AdjustPrev) ? t_position_start : t_position_end + 1;
        } else if (!anchor_in_cell) {
            if (dir & AdjustPrev)
                position = t_position->cellStart(position).position() + 1;
            else
                position = t_position->cellEnd(position).position();
        }
    }

    adjusted_anchor = anchor;
    if (t_anchor) {
        int t_anchor_start = t_anchor->start().position();
        int t_anchor_end = t_anchor->end().position();
        bool position_in_table = false;
        bool position_in_cell = false;
        if (t_position) {
            int t_position_start = t_position->start().position();
            int t_position_end = t_position->end().position();
            if (t_position_start >= t_anchor_start && t_position_end <= t_anchor_end) {
                position_in_table = true;
                if (t_anchor->cellStart(anchor) == t_anchor->cellStart(position))
                    position_in_cell = true;
            }
        }
        dir = (position > anchor) ? AdjustPrev : AdjustNext;
        if (!position_in_table) {
            adjusted_anchor = (dir == AdjustPrev) ? t_anchor_start : t_anchor_end + 1;
        } else if (!position_in_cell) {
            if (dir & AdjustPrev)
                adjusted_anchor = t_anchor->cellStart(anchor).position() + 1;
            else
                adjusted_anchor = t_anchor->cellEnd(anchor).position();
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
            --blockIt;
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
            ++blockIt;
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
        // adjust end and position
        int dir = (op >= QTextCursor::Start && op <= QTextCursor::WordLeft)
                  ? AdjustPrev : AdjustNext;
        adjustCursor(dir);
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
void QTextCursor::setPosition(int pos)
{
    if (!d)
        return;
    d->setPosition(pos);
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

void QTextCursor::setAnchor(int anchor)
{
    if (!d)
        return;
    d->anchor = anchor;
    d->adjusted_anchor = anchor;
    d->adjustCursor(anchor > d->position ? AdjustPrev : AdjustNext);
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

    if (blockFormat().tableCellEndOfRow())
        d->moveTo(NextBlock);

    QTextFormatCollection *formats = d->pieceTable->formatCollection();
    int formatIdx = formats->indexForFormat(format);
    Q_ASSERT(formats->format(formatIdx).isCharFormat());

    QTextBlockFormat blockFmt = blockFormat();

    QStringList blocks = text.split(QChar::ParagraphSeparator);
    for (int i = 0; i < blocks.size(); ++i) {
        if (i > 0)
            insertBlock(blockFmt);
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
        pos1 = d->anchor;
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

    Q_ASSERT(d->pieceTable->formatCollection()->charFormat(idx).isValid());
    return d->pieceTable->formatCollection()->charFormat(idx);
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

    QTextBlockFormat bfmt = blockFormat();
    QTextTableFormat table = bfmt.tableFormat();
    if (table.isValid()) {
        bfmt.setGroup(0);
        bfmt.setTableCellEndOfRow(false);
    }
    d->insertBlock(bfmt);
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
    d->insertBlock(format);
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

    if (blockFormat().tableCellEndOfRow())
        d->moveTo(NextBlock);

    int pos = d->position;
    QTextTable *t = d->createTable(rows, cols, format);
    setPosition(pos);
    d->moveTo(NextBlock);
    return t;
}

/*!
    Returns a pointer to the current table, if the cursor is positioned inside
    a block that is part of a table; otherwise returns a null pointer.

    \sa createTable()
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

