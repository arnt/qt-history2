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
                position = positionChain.at(i)->endPosition() + 1;
            if (i < anchorChain.size())
                adjusted_anchor = anchorChain.at(i)->startPosition() - 1;
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

bool QTextCursorPrivate::movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode)
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
    manipulate a QTextDocument. It embodies both a cursor position and
    optionally a selection.

    QTextCursor is modelled on how a text cursor behaves in a text
    editor, providing a programmatic means of doing what users do
    through the user interface. A document can be thought of as a
    single string of characters with the cursor's position() being \e
    between any two characters (or at the very beginning or very end
    of the document). Documents can also contain tables, lists,
    images, etc., in addition to text, but from the APIs point of view
    the document is just one long string, with some portions of that
    string considered to be within particular blocks (e.g.
    paragraphs), or within a table's cell, or a list's item, etc. When
    we refer to "current character" we mean the character immediately
    after the cursor position() in the document; similarly the
    "current block" is the block that contains the cursor position().

    A QTextCursor also has an anchor() position. The text that is
    between the anchor() and the position() is the selection. If
    anchor() == position() there is no selection.

    The cursor position can be changed programmatically using
    setPosition() and moveTo(); the latter can also be used to select
    text. For selections see selectionStart(), selectionEnd(),
    hasSelection(), clearSelection(), and removeSelectedText().

    If the position() is at the start of a block atBlockStart()
    returns true; and if it is at the end of a block atEnd() returns
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
    Insertions are done using insertText(), insertBlock(),
    insertList(), insertTable(), insertImage(), insertFrame(), and
    insertFragment().

    Actions can be grouped (i.e. treated as a single action for
    undo/redo) using beginEditBlock() and endEditBlock().

    Cursor movements are limited to valid cursor positions. In Latin
    writing this is usually after every character in the text. In some
    other writing systems cursor movements are limited to "clusters"
    (e.g. a syllable in Devanagari, or a base letter plus diacritics).
    Functions such as moveTo() and deleteChar() limit cursor movement
    to these valid positions.

*/

/*!
    \enum QTextCursor::MoveOperation

    \value NoMove Keep the cursor where it is

    \value Start Move to the start of the document
    \value StartOfLine Move to the start of the current line
    \value PreviousBlock move to the start of the previous block
    \value PreviousCharacter move to the previous character
    \value PreviousWord move to the beginning of the previous word
    \value Up move up one line
    \value Left move left one character
    \value WordLeft move left one word

    \value End move to the end of the document
    \value EndOfLine move to the end of the current line
    \value NextBlock move to the beginning of the next block
    \value NextCharacter move to the next character
    \value NextWord move to the next word
    \value Down move down one line
    \value Right move right one character
    \value WordRight right move one word

    \sa moveTo()
*/

/*!
    \enum QTextCursor::MoveMode

    \value MoveAnchor Moves the anchor to the same position as the cursor itself.
    \value KeepAnchor Keeps the anchor where it is.

    If the anchor() is kept where it is and the position() is moved,
    the text in-between will be selected.
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
    : d(new QTextCursorPrivate(const_cast<const QTextDocument*>(document)->d_func()->pieceTable))
{
}


/*!
    Constructs a cursor pointing to the beginning of the \a block.
*/
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
    Moves the cursor to the absolute position \a pos using \c MoveMode
    \a m. The cursor is positioned between characters.

    \sa position() moveTo() anchor()
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
    The cursor is positioned between characters.

    \sa setPosition() moveTo() anchor()
*/
int QTextCursor::position() const
{
    if (!d)
        return -1;
    return d->position;
}

/*!
    Returns the anchor position; this is the same as position() unless
    there is a selection in which case position() marks one end of the
    selection and anchor() marks the other end. Just like the cursor
    position, the anchor position is between characters.

    \sa position() setPosition() moveTo() selectionStart() selectionEnd()
*/
int QTextCursor::anchor() const
{
    if (!d)
        return -1;
    return d->anchor;
}

/*!
    Moves the cursor in accordance with the \c MoveOperation \a op,
    using \c MoveMode \a mode. The move is performed \a n (default 1)
    times.

    If \a mode is \c KeepAnchor, the cursor selects the text it moves
    over; (this is the same effect that the user achieves when they
    move using arrow keys etc., with the Shift key pressed).
*/
bool QTextCursor::movePosition(MoveOperation op, MoveMode mode, int n)
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

    \sa charFormat() hasSelection()
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
    If there is no selected text, deletes the character \e at the
    current cursor position; otherwise deletes the selected text.

    \sa deletePreviousChar() hasSelection() clearSelection()
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
    If there is no selected text, deletes the character \e before the
    current cursor position; otherwise deletes the selected text.

    \sa deleteChar() hasSelection() clearSelection()
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
    Returns true if the cursor contains a selection; otherwise returns false.
*/
bool QTextCursor::hasSelection() const
{
    return d && d->position != d->anchor;
}

/*!
    Clears the current selection.

    \sa removeSelectedText() hasSelection()
*/
void QTextCursor::clearSelection()
{
    if (!d)
        return;
    d->adjusted_anchor = d->anchor = d->position;
}

/*!
    If there is a selection, its content is deleted; otherwise does
    nothing.

    \sa hasSelection()
*/
void QTextCursor::removeSelectedText()
{
    if (!d || d->position == d->anchor)
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
    if (!d)
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
    if (!d)
        return -1;
    return qMax(d->position, d->adjusted_anchor);
}


QString QTextCursor::selectedText() const
{
    // ###########
    return QString();
}

QTextDocumentFragment QTextCursor::selection() const
{
    return QTextDocumentFragment(*this);
}

/*!
    Returns an iterator for the block that contains the cursor.
*/
QTextBlockIterator QTextCursor::block() const
{
    return d->block();
}

/*!
    Returns the block format of the block the cursor is in.

    \sa setBlockFormat() charFormat()
 */
QTextBlockFormat QTextCursor::blockFormat() const
{
    if (!d)
        return QTextBlockFormat();

    return d->block().blockFormat();
}

/*!
    Sets the block format of the current block (or all blocks that
    are contained in the selection) to \a format.

    \sa setBlockFormat()
*/
void QTextCursor::setBlockFormat(const QTextBlockFormat &format)
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
    d->pieceTable->setBlockFormat(from, to, format, QTextPieceTable::SetFormat);
}

/*!
    Modifies the block format of the current block (or all blocks that
    are contained in the selection) with \a modifier.

    \sa setBlockFormat()
*/
void QTextCursor::mergeBlockFormat(const QTextBlockFormat &modifier)
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
    Returns the format of the character immediately following the
    cursor position().

    \sa insertText(), blockFormat()
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
    // ##### we miss a clearProperty here
    if (cfmt.objectIndex() != -1)
        cfmt.setObjectIndex(-1);
    Q_ASSERT(cfmt.isValid());
    return cfmt;
}

/*!
    Set the format \a format as char format for the selection. Does
    nothing if the cursor doesn't have a selection.

    \sa hasSelection()
*/
void QTextCursor::setCharFormat(const QTextCharFormat &format)
{
    if (!d || d->position == d->anchor)
        return;

    int pos1 = d->position;
    int pos2 = d->adjusted_anchor;
    if (pos1 > pos2) {
        pos1 = d->adjusted_anchor;
        pos2 = d->position;
    }

    d->pieceTable->setCharFormat(pos1, pos2-pos1, format, QTextPieceTable::SetFormat);
}

/*!
    Applies all the properties set in \a modifier to all the formats
    that are part of the selection. Does nothing if the cursor doesn't
    have a selection.

    \sa hasSelection()
*/
void QTextCursor::mergeCharFormat(const QTextCharFormat &modifier)
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
    Returns true if the cursor is at the start of a block; otherwise
    returns false.

    \sa atEnd()
*/
bool QTextCursor::atBlockStart() const
{
    if (!d)
        return false;

    return d->position == d->block().position();
}

/*!
    Returns true if the cursor is at the end of the document;
    otherwise returns false.

    \sa atBlockStart()
*/
bool QTextCursor::atEnd() const
{
    if (!d)
        return false;

    return d->position == d->pieceTable->length() - 1;
}

/*!
    Inserts a new empty block at the cursor position() with the
    current blockFormat() and charFormat().

    \sa setBlockFormat()
*/
void QTextCursor::insertBlock()
{
    if (!d)
        return;

    d->insertBlock(blockFormat(), charFormat());
}

/*!
    \overload

    Inserts a new empty block at the cursor position() with block
    format \a format and the current charFormat().

    \sa setBlockFormat()
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

    \sa insertList() currentList()
 */
QTextList *QTextCursor::createList(const QTextListFormat &format)
{
    if (!d)
        return 0;

    QTextList *list = static_cast<QTextList *>(d->pieceTable->createObject(format));
    QTextBlockFormat modifier;
    modifier.setObjectIndex(list->objectIndex());
    mergeBlockFormat(modifier);
    return list;
}

/*!
    \overload

    Creates and returns a new list with the given \a style and makes the
    current paragraph the cursor is in the first list item.

    \sa insertList() currentList()
 */
QTextList *QTextCursor::createList(int style)
{
    QTextListFormat fmt;
    fmt.setStyle(style);
    return createList(fmt);
}

/*!
    Returns the current list, if the cursor position() is inside a
    block that is part of a list; otherwise returns a null pointer.

    \sa insertList() createList()
 */
QTextList *QTextCursor::currentList() const
{
    if (!d)
        return 0;

    QTextBlockFormat b = blockFormat();
    QTextFormatObject *o = d->pieceTable->objectForFormat(b);
    return qt_cast<QTextList *>(o);
}

/*!
    Returns the index of the list item containing the cursor
    position(), or -1 if there is no list item.
*/
int QTextCursor::listItemNumber() const
{
    if (!d)
        return -1;
    QTextList *l = currentList();
    if (!l)
        return -1;

    return l->itemNumber(d->block());
}

/*!
    Returns the text in the list item containing the cursor
    position(), or an empty string if there is no list item.
*/
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

    Creates a new table with \a rows rows and \a cols columns, inserts
    it at the current position(), and returns the table object. The
    cursor position() is moved to the beginning of the first cell.

    \sa currentTable()
 */
QTextTable *QTextCursor::insertTable(int rows, int cols)
{
    return insertTable(rows, cols, QTextTableFormat());
}

/*!
    Creates a new table with \a rows rows and \a cols columns, using
    the given \a format, inserts it at the current position(), and
    returns the table object. The cursor position() is moved to the
    beginning of the first cell.

    \sa currentTable()
*/
QTextTable *QTextCursor::insertTable(int rows, int cols, const QTextTableFormat &format)
{
    if(!d)
        return 0;

    int pos = d->position;
    QTextTable *t = QTextTablePrivate::createTable(d->pieceTable, d->position, rows, cols, format);
    setPosition(pos+1);
    return t;
}

/*!
    Returns a pointer to the current table, if the cursor position()
    is inside a block that is part of a table; otherwise returns a
    null pointer.

    \sa insertTable()
*/
QTextTable *QTextCursor::currentTable() const
{
    if(!d)
        return 0;

    return d->tableAt(d->position);
}

/*!
    Inserts the frame, \a format, at the current cursor position() and
    moves the cursor position() inside the frame.

    If the cursor holds a selection the whole selection is moved
    inside the frame.

    \sa hasSelection()
*/
QTextFrame *QTextCursor::insertFrame(const QTextFrameFormat &format)
{
    if (!d)
        return 0;

    return d->pieceTable->insertFrame(selectionStart(), selectionEnd(), format);
}

/*!
    Returns a pointer to the current frame, returns a
    null pointer if the cursor is invalid.

    \sa insertFrame()
*/
QTextFrame *QTextCursor::currentFrame() const
{
    if(!d)
        return 0;

    return d->pieceTable->frameAt(d->position);
}


/*!
    Inserts the text \a fragment at the current position().
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
    Inserts the image defined by \a format at the current position().
*/
void QTextCursor::insertImage(const QTextImageFormat &format)
{
    insertText(QString(QChar::ObjectReplacementCharacter), format);
}

/*!
    Returns true if the \a rhs cursor is at a different position in
    the document as this cursor; otherwise returns false.
*/
bool QTextCursor::operator!=(const QTextCursor &rhs) const
{
    return !operator==(rhs);
}

/*!
    Returns true if the \a rhs cursor is positioned later in the
    document than this cursor; otherwise returns false.
*/
bool QTextCursor::operator<(const QTextCursor &rhs) const
{
    if (!d)
        return rhs.d != 0;

    if (!rhs.d)
        return false;

    Q_ASSERT_X(d->pieceTable == rhs.d->pieceTable, "QTextCursor::operator<", "cannot compare cusors attached to different documents");

    return d->position < rhs.d->position;
}

/*!
    Returns true if the \a rhs cursor is positioned later or at the
    same position in the document as this cursor; otherwise returns
    false.
*/
bool QTextCursor::operator<=(const QTextCursor &rhs) const
{
    if (!d)
        return true;

    if (!rhs.d)
        return false;

    Q_ASSERT_X(d->pieceTable == rhs.d->pieceTable, "QTextCursor::operator<=", "cannot compare cusors attached to different documents");

    return d->position <= rhs.d->position;
}

/*!
    Returns true if the \a rhs cursor is at the same position in the
    document as this cursor; otherwise returns false.
*/
bool QTextCursor::operator==(const QTextCursor &rhs) const
{
    if (!d)
        return rhs.d == 0;

    if (!rhs.d)
        return false;

    return d->position == rhs.d->position && d->pieceTable == rhs.d->pieceTable;
}

/*!
    Returns true if the \a rhs cursor is positioned earlier or at the
    same position in the document as this cursor; otherwise returns
    false.
*/
bool QTextCursor::operator>=(const QTextCursor &rhs) const
{
    if (!d)
        return false;

    if (!rhs.d)
        return true;

    Q_ASSERT_X(d->pieceTable == rhs.d->pieceTable, "QTextCursor::operator>=", "cannot compare cusors attached to different documents");

    return d->position >= rhs.d->position;
}

/*!
    Returns true if the \a rhs cursor is positioned earlier in the
    document than this cursor; otherwise returns false.
*/
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
    if (!d)
        return;

    d->pieceTable->beginEditBlock();
}

/*!
    Indicates the end of a block of editing operations on the document
    that should appear as a single operation from an undo/redo point
    of view.

    \sa beginEditBlock()
 */

void QTextCursor::endEditBlock()
{
    if (!d)
        return;

    d->pieceTable->endEditBlock();
}

