#include "qtextcursor.h"
#include "qtextcursor_p.h"
#include "qglobal.h"
#include "qtextdocumentfragment.h"
#include "qtextdocumentfragment_p.h"
#include "qtextlistmanager_p.h"
#include "qtexttablemanager_p.h"
#include "qtexttable.h"

#include <private/qtextlayout_p.h>
#include <qdebug.h>

enum {
    AdjustPrev = 0x1,
    AdjustUp = 0x3,
    AdjustNext = 0x4,
    AdjustDown = 0x12
};

QTextCursorPrivate::QTextCursorPrivate(const QTextPieceTable *table)
    : x(0), position(1), anchor(1), adjusted_anchor(1),
      pieceTable(const_cast<QTextPieceTable *>(table))
{
    Q_ASSERT(pieceTable);
    pieceTable->addCursor(this);
}

QTextCursorPrivate::QTextCursorPrivate(const QTextCursorPrivate &rhs)
    : QSharedObject(rhs)
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
    if (charsAddedOrRemoved < 0 && anchor < positionOfChange - charsAddedOrRemoved)
	anchor = positionOfChange;
    else
	anchor += charsAddedOrRemoved;
}

void QTextCursorPrivate::setPosition(int newPosition)
{
    if (newPosition < 0 || newPosition > pieceTable->length() ||
	newPosition == position)
	return;
    Q_ASSERT(pieceTable->length());

    position = newPosition;
}

void QTextCursorPrivate::setX()
{
    QTextPieceTable::BlockIterator block = pieceTable->blocksFind(position-1);
    const QTextLayout *layout = block.layout();
    int pos = position - block.start();

    QTextLine line = layout->findLine(pos);
    if (line.isValid())
	x = line.cursorToX(pos);
}

void QTextCursorPrivate::insertDirect(int strPos, int strLength, int formatIdx)
{
    const QString &buffer = pieceTable->buffer();

    QTextCharFormat format = pieceTable->formatCollection()->charFormat(formatIdx);

    QTextBlockFormat blockFmt = block().blockFormat();
    if (format.isBlockFormat())
	blockFmt = format.toBlockFormat();

    const int endPos = strPos + strLength;
    int pos = strPos;
    int strStart = pos;

    while (pos < endPos) {
	if (buffer[pos] == QTextParagraphSeparator) {
	    if (strStart != pos)
		pieceTable->insert(position, strStart, pos - strStart, formatIdx);

	    insertBlock(blockFmt);

	    ++pos;
	    strStart = pos;
	    continue;
	}
	++pos;
    }

    if (strStart != pos)
	pieceTable->insert(position, strStart, endPos - strStart, formatIdx);
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

    pieceTable->insertBlockSeparator(position, idx);
}


void QTextCursorPrivate::adjustCursor(int dir)
{
    QTextTable *t_anchor_ = pieceTable->tableManager()->tableAt(anchor);
    QTextTable *t_position_ = pieceTable->tableManager()->tableAt(position);

    QTextTablePrivate *t_anchor = 0;
    QTextTablePrivate *t_position = 0;

    if (t_anchor_)
	t_anchor = t_anchor_->d;

    if (t_position_)
	t_position = t_position_->d;

    // first adjust position if needed
    if (t_position) {
	int t_position_start = t_position->start().key();
	int t_position_end = t_position->end().key();
	bool anchor_in_table = false;
	bool anchor_in_cell = false;
	if (t_anchor) {
	    int t_anchor_start = t_anchor->start().key();
	    int t_anchor_end = t_anchor->end().key();
	    if (t_anchor_start >= t_position_start && t_anchor_end <= t_position_end) {
		anchor_in_table = true;
		if (t_position->cellStart(anchor).key() == t_position->cellStart(position).key())
		    anchor_in_cell = true;
	    }
	}
	if (!anchor_in_table) {
	    // ### AdjustUp/Down should use x position!
	    position = (dir & AdjustPrev) ? t_position_start : t_position_end + 1;
	} else if (!anchor_in_cell) {
	    if (dir & AdjustPrev)
		position = t_position->cellStart(position).key() + 1;
	    else
		position = t_position->cellEnd(position).key();
	}
    }

    adjusted_anchor = anchor;
    if (t_anchor) {
	int t_anchor_start = t_anchor->start().key();
	int t_anchor_end = t_anchor->end().key();
	bool position_in_table = false;
	bool position_in_cell = false;
	if (t_position) {
	    int t_position_start = t_position->start().key();
	    int t_position_end = t_position->end().key();
	    if (t_position_start >= t_anchor_start && t_position_end <= t_anchor_end) {
		position_in_table = true;
		if (t_anchor->cellStart(anchor).key() == t_anchor->cellStart(position).key())
		    position_in_cell = true;
	    }
	}
	dir = (position > anchor) ? AdjustPrev : AdjustNext;
	if (!position_in_table) {
	    adjusted_anchor = (dir == AdjustPrev) ? t_anchor_start : t_anchor_end + 1;
	} else if (!position_in_cell) {
	    if (dir & AdjustPrev)
		adjusted_anchor = t_anchor->cellStart(anchor).key() + 1;
	    else
		adjusted_anchor = t_anchor->cellEnd(anchor).key();
	}
    }
}

bool QTextCursorPrivate::moveTo(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode)
{
    bool adjustX = true;

    if (op >= QTextCursor::Left && op <= QTextCursor::WordRight
	&& blockFormat().direction() == QTextBlockFormat::RightToLeft) {
	if (op == QTextCursor::Left)
	    op = QTextCursor::NextCharacter;
	else if (op == QTextCursor::Right)
	    op = QTextCursor::PreviousCharacter;
	else if (op == QTextCursor::WordLeft)
	    op = QTextCursor::NextWord;
	else if (op == QTextCursor::WordRight)
	    op = QTextCursor::PreviousWord;
    }

    QTextPieceTable::BlockIterator blockIt = block();
    const QTextLayout *layout = blockIt.layout();
    int relativePos = position - blockIt.start();
    QTextLine line = layout->findLine(relativePos);

    switch(op) {
    case QTextCursor::NoMove:
	return true;

    case QTextCursor::Start:
	position = 1;
	break;
    case QTextCursor::StartOfLine: {

	if (!line.isValid())
	    break;
	setPosition(blockIt.start() + line.from());

	break;
    }
    case QTextCursor::PreviousBlock: {
	if (blockIt == pieceTable->blocksBegin())
	    return false;
	--blockIt;

	setPosition(blockIt.start());
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
	    setPosition(line.xToCursor(x) + blockIt.start());
	} else {
	    setPosition(blockIt.start());
	}
	adjustX = false;
	break;
    }

    case QTextCursor::End:
	position = pieceTable->length();
	break;
    case QTextCursor::EndOfLine: {
	if (!line.isValid())
	    break;
	// currently we don't draw the space at the end, so move to the next
	// reasonable position.
	setPosition(blockIt.start() + line.from() + line.length() - 1);

	break;
    }
    case QTextCursor::NextBlock: {
	++blockIt;
	if (blockIt.atEnd())
	    return false;

	setPosition(blockIt.start());
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
	    setPosition(line.xToCursor(x) + blockIt.start());
	} else {
	    setPosition(blockIt.start());
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
    : d(new QTextCursorPrivate(const_cast<const QTextDocument*>(document)->pieceTable))
{
    Q_ASSERT(document->pieceTable->blocksBegin() != document->pieceTable->blocksEnd());
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
    Returns TRUE if the cursor is null; otherwise returns FALSE. A null
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

int QTextCursor::adjustedAnchor() const
{
    if (!d)
	return -1;
    return d->adjusted_anchor;
}

bool QTextCursor::moveTo(MoveOperation op, MoveMode mode)
{
    if (!d)
	return false;
    return d->moveTo(op, mode);
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

    d->pieceTable->beginUndoBlock();

    d->remove();

    if (blockFormat().tableCellEndOfRow())
	d->moveTo(NextBlock);

    QTextFormatCollection *formats = d->pieceTable->formatCollection();
    int formatIdx = formats->indexForFormat(format);
    Q_ASSERT(formats->format(formatIdx).isCharFormat());

    QTextBlockFormat blockFmt = blockFormat();
    if (format.isBlockFormat())
	blockFmt = format.toBlockFormat();

    QStringList blocks = text.split(QTextParagraphSeparator);
    for (int i = 0; i < blocks.size(); ++i) {
	if (i > 0)
	    insertBlock(blockFmt);
	d->pieceTable->insert(d->position, blocks.at(i), formatIdx);
    }

    d->pieceTable->endUndoBlock();
}

void QTextCursor::deleteChar()
{
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

bool QTextCursor::hasSelection() const
{
    return d && d->position != d->anchor;
}

void QTextCursor::clearSelection()
{
    if (!d)
	return;
    d->adjusted_anchor = d->anchor = d->position;
}

void QTextCursor::removeSelection()
{
    if (!d || d->position == d->anchor)
	return;

    d->remove();
    d->setX();
}

int QTextCursor::selectionStart() const
{
    if (!d)
	return -1;
    return qMin(d->position, d->adjusted_anchor);
}

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

    d->block().setBlockFormat(format);
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

void QTextCursor::applyFormatModifier(const QTextFormat &modifier) const
{
    if (!d || d->position == d->anchor)
	return;

    int pos1 = d->position;
    int pos2 = d->adjusted_anchor;
    if (pos1 > pos2) {
	pos1 = d->anchor;
	pos2 = d->position;
    }

    const_cast<QTextPieceTable *>((const QTextPieceTable *)d->pieceTable)
	->setFormat(pos1, pos2-pos1, modifier, QTextPieceTable::MergeFormat);
}

void QTextCursor::applyBlockFormatModifier(const QTextBlockFormat &modifier) const
{
    if (!d)
	return;

    int pos1 = d->position;
    int pos2 = d->adjusted_anchor;
    if (pos1 > pos2) {
	pos1 = d->anchor;
	pos2 = d->position;
    }

    if (pos1 < d->pieceTable->length())
	pos1 = d->pieceTable->blocksFind(pos1).key();
    else
	pos1 = d->pieceTable->length() - 1;

    QTextPieceTable::BlockIterator endBlock = d->pieceTable->blocksFind(pos2);
    if (!endBlock.atEnd())
	++endBlock;

    if (endBlock.atEnd())
	pos2 = d->pieceTable->length();
    else
	pos2 = endBlock.key();

    const_cast<QTextPieceTable *>((const QTextPieceTable *)d->pieceTable)
	->setFormat(pos1, pos2-pos1, modifier, QTextPieceTable::MergeFormat);
}

/*!
    Returns the format of the character the cursor points to.

    \sa insertText(), position()
 */
QTextCharFormat QTextCursor::charFormat() const
{
    if (!d)
	return QTextCharFormat();

    int pos = d->position;
    Q_ASSERT(pos >= 0 && pos <= d->pieceTable->length());

    QTextPieceTable::BlockIterator bi = d->block();

    if (pos != bi.start())
	--pos;

    int idx;
    if (pos == bi.end()) {
	idx = -1;
    } else {
	QTextPieceTable::FragmentIterator it = d->pieceTable->find(pos);
	Q_ASSERT(!it.atEnd());
	idx = it.value()->format;
    }

    return d->pieceTable->formatCollection()->charFormat(idx);
}

bool QTextCursor::atBlockStart() const
{
    if (!d)
	return false;

    return d->position == d->block().start();
}

bool QTextCursor::atEnd() const
{
    if (!d)
	return false;

    return d->position == d->pieceTable->length() - 1;
}

void QTextCursor::insertBlock()
{
    if (!d)
	return;

    d->insertBlock(blockFormat());
}

void QTextCursor::insertBlock(const QTextBlockFormat &format)
{
    if (!d)
	return;

    d->pieceTable->beginUndoBlock();
    d->remove();
    d->insertBlock(format);
    d->pieceTable->endUndoBlock();
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

    int listIdx = d->pieceTable->formatCollection()->createReferenceIndex(format);
    QTextBlockFormat modifier;
    modifier.setListFormatIndex(listIdx);
    applyBlockFormatModifier(modifier);

    QTextList *list = d->pieceTable->listManager()->list(listIdx);
    Q_ASSERT(list);
    return list;
}

/*!
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

    return d->pieceTable->listManager()->list(blockFormat().listFormatIndex());
}

int QTextCursor::listItemNumber() const
{
    if (!d)
	return -1;

    return QTextListItem(d->block()).itemNumber();
}

/*!
    Creates a new table with the given dimension (\a rows and \a cols), inserts
    it at the current position and returns the table object.
    The cursor is positioned at the beginning of the first cell.
 */
QTextTable *QTextCursor::insertTable(int rows, int cols)
{
    return insertTable(rows, cols, QTextTableFormat());
}

/*!
    Creates a new table with the given dimension (\a rows and \a cols) and the
    given \format, inserts it at the current position and returns the table object.
    The cursor is positioned at the beginning of the first cell.
 */
QTextTable *QTextCursor::insertTable(int rows, int cols, const QTextTableFormat &format)
{
    if(!d)
	return 0;

    if (blockFormat().tableCellEndOfRow())
	d->moveTo(NextBlock);

    int pos = d->position;
    QTextTable *t = d->pieceTable->tableManager()->createTable(*this, rows, cols, format);
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

    return d->pieceTable->tableManager()->tableAt(d->position);
}

void QTextCursor::insertFragment(const QTextDocumentFragment &fragment)
{
    if (!d || fragment.isNull())
	return;

    d->pieceTable->beginUndoBlock();
    d->remove();
    fragment.d->insert(*this);
    d->pieceTable->endUndoBlock();
}

void QTextCursor::insertImage(const QTextImageFormat &format)
{
    insertText(QString(QTextObjectReplacementChar), format);
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

    return d->position < rhs.d->position;
}

bool QTextCursor::operator<=(const QTextCursor &rhs) const
{
    if (!d)
	return true;

    if (!rhs.d)
	return false;

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

    return d->position >= rhs.d->position;
}

bool QTextCursor::operator>(const QTextCursor &rhs) const
{
    if (!d)
	return false;

    if (!rhs.d)
	return true;

    return d->position > rhs.d->position;
}


