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
    : x(0), position(0), anchor(0), adjusted_anchor(0),
      priv(p)
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
    priv->addCursor(this);
}

QTextCursorPrivate::~QTextCursorPrivate()
{
    if (priv)
        priv->removeCursor(this);
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

void QTextCursorPrivate::setX()
{
    QTextBlock block = priv->blocksFind(position);
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

    priv->remove(pos1, pos2-pos1, op);
    anchor = position;
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
    QTextTable *table = qt_cast<QTextTable *>(f_position);
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
}

bool QTextCursorPrivate::movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode)
{
    bool adjustX = true;
    QTextBlock blockIt = block();

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
        newPosition = blockIt.position() + line.from();

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
    case QTextCursor::PreviousWord:
    case QTextCursor::WordLeft:
        newPosition = priv->previousCursorPosition(position, QTextLayout::SkipWords);
        break;
    case QTextCursor::Up: {
        int i = line.line() - 1;
        if (i == -1) {
            if (blockIt == priv->blocksBegin())
                return false;
            int blockPosition = blockIt.position();
            QTextTable *table = qt_cast<QTextTable *>(priv->frameAt(blockPosition));
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
            i = layout->numLines()-1;
        }
        if (layout->numLines()) {
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
        if (!line.isValid() || line.length() == 0)
            break;
        // currently we don't draw the space at the end, so move to the next
        // reasonable position.
        newPosition = blockIt.position() + line.from() + line.length() - 1;

        break;
    }
    case QTextCursor::EndOfWord: {
        const QCharAttributes *attributes = layout->engine()->attributes();

        const int len = layout->text().length();
        if (relativePos >= len)
            return relativePos;
        relativePos++;
        while (relativePos < len && !attributes[relativePos].wordStop && !attributes[relativePos].whiteSpace)
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
        int i = line.line() + 1;

        if (i >= layout->numLines()) {
            int blockPosition = blockIt.position() + blockIt.length() - 1;
            QTextTable *table = qt_cast<QTextTable *>(priv->frameAt(blockPosition));
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
        if (layout->numLines()) {
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
        QTextTable *table = qt_cast<QTextTable *>(priv->frameAt(position));
        if (table && ((op >= QTextCursor::PreviousBlock && op <= QTextCursor::WordLeft)
                      || (op >= QTextCursor::NextBlock && op <= QTextCursor::WordRight))) {
            int oldColumn = table->cellAt(position).column();
            int newColumn = table->cellAt(newPosition).column();
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
    setPosition() and movePosition(); the latter can also be used to
    select text. For selections see selectionStart(), selectionEnd(),
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
    Functions such as movePosition() and deleteChar() limit cursor
    movement to these valid positions.

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
    \value EndOfWord move to the end of the current word
    \value EndOfBlock move to the end of the current block
    \value NextBlock move to the beginning of the next block
    \value NextCharacter move to the next character
    \value NextWord move to the next word
    \value Down move down one line
    \value Right move right one character
    \value WordRight right move one word

    \sa movePosition()
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
    : d(new QTextCursorPrivate(document->docHandle()))
{
}

/*!
    Constructs a cursor pointing to the beginning of the \a frame.
*/
QTextCursor::QTextCursor(QTextFrame *frame)
    : d(new QTextCursorPrivate(frame->document()->docHandle()))
{
}


/*!
    Constructs a cursor pointing to the beginning of the \a block.
*/
QTextCursor::QTextCursor(const QTextBlock &block)
    : d(new QTextCursorPrivate(block.docHandle()))
{
    d->position = block.position();
}


/*!
  \internal
 */
QTextCursor::QTextCursor(QTextDocumentPrivate *p, int pos)
    : d(new QTextCursorPrivate(p))
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
    return !d || !d->priv;
}

/*!
    Moves the cursor to the absolute position \a pos using \c MoveMode
    \a m. The cursor is positioned between characters.

    \sa position() movePosition() anchor()
*/
void QTextCursor::setPosition(int pos, MoveMode m)
{
    if (!d || !d->priv)
        return;
    d->setPosition(pos);
    if (m == MoveAnchor) {
        d->anchor = pos;
        d->adjusted_anchor = pos;
    }
    // ##### adjust anchor for KeepAnchor!
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
    Moves the cursor in accordance with the \c MoveOperation \a op,
    using \c MoveMode \a mode. The move is performed \a n (default 1)
    times.

    If \a mode is \c KeepAnchor, the cursor selects the text it moves
    over; (this is the same effect that the user achieves when they
    move using arrow keys etc., with the Shift key pressed).
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
    if (!d || !d->priv || text.isEmpty())
        return;

    Q_ASSERT(format.isValid());

    d->priv->beginEditBlock();

    d->remove();

    QTextFormatCollection *formats = d->priv->formatCollection();
    int formatIdx = formats->indexForFormat(format);
    Q_ASSERT(formats->format(formatIdx).isCharFormat());

    QTextBlockFormat blockFmt = blockFormat();

    QStringList blocks = text.split(QChar::ParagraphSeparator);
    for (int i = 0; i < blocks.size(); ++i) {
        if (i > 0)
            d->insertBlock(blockFmt, format);
        d->priv->insert(d->position, blocks.at(i), formatIdx);
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
    Returns true if the cursor contains a selection and the selection is simple a range
    from selectionStart() to selectionEnd(); otherwise returns false.

    Complex selections can currently only appear if table cells over
    more than one row are selected. In this case the selected range of table
    cells in the current table can be retrieved with selectedTableCells().
*/
bool QTextCursor::hasComplexSelection() const
{
    if (!d || d->position == d->anchor)
        return false;

    QTextTable *t = qt_cast<QTextTable *>(d->priv->frameAt(d->position));
    if (!t)
        return false;

    QTextTableCell cell_pos = t->cellAt(d->position);
    QTextTableCell cell_anchor = t->cellAt(d->adjusted_anchor);

    Q_ASSERT(cell_anchor.isValid());

    if (cell_pos == cell_anchor
        || cell_pos.row() == cell_anchor.row())
        return false;

    return true;
}

/*!
  If the selection spans over table cells, the arguments are set to the range of cells selected in the
  table. Otherwise all arguments are set to -1.
*/
void QTextCursor::selectedTableCells(int *firstRow, int *numRows, int *firstColumn, int *numColumns) const
{
    *firstRow = -1;
    *firstColumn = -1;
    *numRows = -1;
    *numColumns = -1;

    if (!d || d->position == d->anchor)
        return;

    QTextTable *t = qt_cast<QTextTable *>(d->priv->frameAt(d->position));
    if (!t)
        return;

    QTextTableCell cell_pos = t->cellAt(d->position);
    QTextTableCell cell_anchor = t->cellAt(d->adjusted_anchor);

    Q_ASSERT(cell_anchor.isValid());

    if (cell_pos == cell_anchor)
        return;

    *firstRow = qMin(cell_pos.row(), cell_anchor.row());
    *firstColumn = qMin(cell_pos.column(), cell_anchor.column());
    *numRows = qMax(cell_pos.row() + cell_pos.rowSpan(), cell_anchor.row() + cell_anchor.rowSpan()) - *firstRow;
    *numColumns = qMax(cell_pos.column() + cell_pos.columnSpan(), cell_anchor.column() + cell_anchor.columnSpan()) - *firstColumn;
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

        text += QString::fromRawData(docText.constData() + frag->stringPosition + offsetInFragment, len);
        pos += len;
    }
}

/*!
    Returns the current selection's text (which may be empty). This
    only returns the text, with no formatting information. If you want
    a document fragment use selection() instead.
*/
QString QTextCursor::selectedText() const
{
    if (!d || !d->priv || d->position == d->anchor)
        return QString();

    const QString docText = d->priv->buffer();
    QString text;

    if (hasComplexSelection()) {
        QTextTable *table = currentTable();
        int row_start, col_start, num_rows, num_cols;
        selectedTableCells(&row_start, &num_rows, &col_start,  &num_cols);

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
    Returns the current selection (which may be empty). If you just
    want the selected text, i.e. without formatting information, use
    selectedText() instead.
*/
QTextDocumentFragment QTextCursor::selection() const
{
    return QTextDocumentFragment(*this);
}

/*!
    Returns an iterator for the block that contains the cursor.
*/
QTextBlock QTextCursor::block() const
{
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

    \sa setBlockFormat()
*/
void QTextCursor::setBlockFormat(const QTextBlockFormat &format)
{
    if (!d || !d->priv)
        return;

    int pos1 = d->position;
    int pos2 = d->adjusted_anchor;
    if (pos1 > pos2) {
        pos1 = d->anchor;
        pos2 = d->position;
    }

    QTextBlock from = d->priv->blocksFind(pos1);
    QTextBlock to = d->priv->blocksFind(pos2);
    d->priv->setBlockFormat(from, to, format, QTextDocumentPrivate::SetFormat);
}

/*!
    Modifies the block format of the current block (or all blocks that
    are contained in the selection) with \a modifier.

    \sa setBlockFormat()
*/
void QTextCursor::mergeBlockFormat(const QTextBlockFormat &modifier)
{
    if (!d || !d->priv)
        return;

    int pos1 = d->position;
    int pos2 = d->adjusted_anchor;
    if (pos1 > pos2) {
        pos1 = d->anchor;
        pos2 = d->position;
    }

    QTextBlock from = d->priv->blocksFind(pos1);
    QTextBlock to = d->priv->blocksFind(pos2);
    d->priv->setBlockFormat(from, to, modifier, QTextDocumentPrivate::MergeFormat);
}

/*!
    Returns the format of the character immediately following the
    cursor position().

    \sa insertText(), blockFormat()
 */
QTextCharFormat QTextCursor::charFormat() const
{
    if (!d || !d->priv)
        return QTextCharFormat();

    int pos = d->position - 1;
    if (pos < 0)
        pos = 0;
    Q_ASSERT(pos >= 0 && pos < d->priv->length());


    QTextDocumentPrivate::FragmentIterator it = d->priv->find(pos);
    Q_ASSERT(!it.atEnd());
    int idx = it.value()->format;

    QTextCharFormat cfmt = d->priv->formatCollection()->charFormat(idx);
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
    if (!d || !d->priv || d->position == d->anchor)
        return;

    int pos1 = d->position;
    int pos2 = d->adjusted_anchor;
    if (pos1 > pos2) {
        pos1 = d->adjusted_anchor;
        pos2 = d->position;
    }

    d->priv->setCharFormat(pos1, pos2-pos1, format, QTextDocumentPrivate::SetFormat);
}

/*!
    Applies all the properties set in \a modifier to all the formats
    that are part of the selection. Does nothing if the cursor doesn't
    have a selection.

    \sa hasSelection()
*/
void QTextCursor::mergeCharFormat(const QTextCharFormat &modifier)
{
    if (!d || !d->priv || d->position == d->anchor)
        return;

    int pos1 = d->position;
    int pos2 = d->adjusted_anchor;
    if (pos1 > pos2) {
        pos1 = d->adjusted_anchor;
        pos2 = d->position;
    }

    d->priv->setCharFormat(pos1, pos2-pos1, modifier, QTextDocumentPrivate::MergeFormat);
}

/*!
    Returns true if the cursor is at the start of a block; otherwise
    returns false.

    \sa atEnd()
*/
bool QTextCursor::atBlockStart() const
{
    if (!d || !d->priv)
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
    if (!d || !d->priv)
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
    if (!d || !d->priv)
        return;

    d->priv->beginEditBlock();
    d->remove();
    d->insertBlock(format, charFormat());
    d->priv->endEditBlock();
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
    if (!d || !d->priv)
        return 0;

    QTextBlockFormat b = blockFormat();
    QTextObject *o = d->priv->objectForFormat(b);
    return qt_cast<QTextList *>(o);
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
    if(!d || !d->priv)
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
    Returns a pointer to the current table, if the cursor position()
    is inside a block that is part of a table; otherwise returns a
    null pointer.

    \sa insertTable()
*/
QTextTable *QTextCursor::currentTable() const
{
    if(!d || !d->priv)
        return 0;

    QTextFrame *frame = d->priv->frameAt(d->position);
    while (frame) {
        QTextTable *table = qt_cast<QTextTable *>(frame);
        if (table)
            return table;
        frame = frame->parentFrame();
    }
    return 0;
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
    if (!d || !d->priv)
        return 0;

    return d->priv->insertFrame(selectionStart(), selectionEnd(), format);
}

/*!
    Returns a pointer to the current frame, returns a
    null pointer if the cursor is invalid.

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

    Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator<", "cannot compare cusors attached to different documents");

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

    Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator<=", "cannot compare cusors attached to different documents");

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

    return d->position == rhs.d->position && d->priv == rhs.d->priv;
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

    Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator>=", "cannot compare cusors attached to different documents");

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

    Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator>", "cannot compare cusors attached to different documents");

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

