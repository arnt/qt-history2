#include "qtextobject.h"
#include "qtextobject_p.h"
#include "qtextdocument.h"
#include "qtextformat_p.h"
#include "qtextdocument_p.h"
#include "qtextcursor.h"

#define d d_func()
#define q q_func()

// ### DOC: We ought to explain the CONCEPT of objectIndexes if
// relevant to the public API
/*!
    \class QTextObject
    \brief The QTextObject class is a base class for different kinds
    of objects that can group parts of a QTextDocument together.

    \ingroup text

    The common grouping text objects are lists (QTextList), frames
    (QTextFrame) and tables (QTextTable). A text object has an
    associated format() and document().

    There are essentially two kinds of text objects: Objects used with
    blocks (block formats) and objects used with characters (character
    formats). The first kind are derived from QTextBlockGroup, and the
    second kind from QTextFrame.

    You should rarely need to use this class directly. When creating
    custom text objects, you will also need to reimplement
    QTextDocument::createObject(), that acts as a factory method for
    creating text objects.
*/

/*!
    \fn int QTextObject::formatType() const

    \internal
*/

/*!
    Creates a new QTextObject for the document, \a doc.

    \warning This function should never be called directly, but only
    from QTextDocument::createObject().
*/
QTextObject::QTextObject(QTextDocument *doc)
    : QObject(*new QTextObjectPrivate, doc)
{
}

/*!
  \internal
*/
QTextObject::QTextObject(QTextObjectPrivate &p, QTextDocument *doc)
    :QObject(p, doc)
{
}

/*!
    Destroys the text object.

    \warning Text objects are owned by the document, so you should
    never destroy them yourself.
*/
QTextObject::~QTextObject()
{
}

/*!
    Returns the text object's format.

    \sa setFormat() document()
*/
QTextFormat QTextObject::format() const
{
    return d->pieceTable->formatCollection()->objectFormat(d->objectIndex);
}

/*!
    Sets the text object's format to \a format.

    \sa format()
*/
void QTextObject::setFormat(const QTextFormat &format)
{
    int idx = d->pieceTable->formatCollection()->indexForFormat(format);
    d->pieceTable->changeObjectFormat(this, idx);
}

/*!
    The object index of this object. This can be used together with
    QTextFormat::setObjectIndex().
*/
int QTextObject::objectIndex() const
{
    return d->objectIndex;
}

/*!
    Returns the document this object belongs to.

    \sa format()
*/
QTextDocument *QTextObject::document() const
{
    return qt_cast<QTextDocument *>(parent());
}

/*!
  \internal
*/
QTextDocumentPrivate *QTextObject::docHandle() const
{
    return qt_cast<QTextDocument *>(parent())->docHandle();
}

/*!
    \class QTextBlockGroup
    \brief The QTextBlockGroup class groups together a list of
    QTextBlocks within a document.

    \l{QTextBlock}s can be inserted with blockInserted() and removed
    with blockRemoved(). If a block's format is changed
    blockFormatChanged() is called. The list of blocks in the group is
    returned by blockList().

    The block group maintains an up-to-date list of the blocks which
    belong to the group even when editing takes place.
*/

/*!
    Creates a new new block group for the document \a doc.

    \warning This function should only be called from
    QTextDocument::createObject().
*/
QTextBlockGroup::QTextBlockGroup(QTextDocument *doc)
    : QTextObject(*new QTextBlockGroupPrivate, doc)
{
}

/*!
  \internal
*/
QTextBlockGroup::QTextBlockGroup(QTextBlockGroupPrivate &p, QTextDocument *doc)
    : QTextObject(p, doc)
{
}

/*!
    Destroys this block group; the blocks are not deleted, they simply
    don't belong to this block anymore.
*/
QTextBlockGroup::~QTextBlockGroup()
{
}

// ### DOC: Shouldn't this be insertBlock()?
/*!
    Appends the given \a block to the end of the group.

    \warning If you reimplement this function you must call the base
    class implementation.
*/
void QTextBlockGroup::blockInserted(const QTextBlock &block)
{
    QTextBlockGroupPrivate::BlockList::Iterator it = qLowerBound(d->blocks.begin(), d->blocks.end(), block);
    d->blocks.insert(it, block);
}

// ### DOC: Shouldn't this be removeBlock()?
/*!
    Removes the given \a block from the group; the block itself is not
    deleted, it simply isn't a member of this group anymore.
*/
void QTextBlockGroup::blockRemoved(const QTextBlock &block)
{
    d->blocks.removeAll(block);
}

/*!
    This function is called whenever the specified \a block of text
    (which is part of this group), is changed. The base class
    implementation does nothing.
*/
void QTextBlockGroup::blockFormatChanged(const QTextBlock &)
{
}

/*!
    Returns a (possibly empty) list of all the blocks that are part of
    the block group.
*/
QList<QTextBlock> QTextBlockGroup::blockList() const
{
    return d->blocks;
}



QTextFrameLayoutData::~QTextFrameLayoutData()
{
}


/*!
    \class QTextFrame
    \brief The QTextFrame class represents a frame in a QTextDocument.

    Each frame in a document consists of a frame start character,
    QChar(0xFDD0), followed by the frame's contents, followed by a
    frame end character, QChar(0xFDD1). The character formats of the
    start and end character contain a reference to the frame object's
    objectIndex.

    Frames can be used to create hierarchical documents. Each document
    has a root frame (QTextDocument::rootFrame()), and each frame
    (except for the root frame), has a parentFrame() and a (possibly
    empty) list of child frames. In addition to containing
    childFrames(), a frame can also contain \l{QTextBlock}s.
    A frame also has a format (setFormat(), format()). The positions
    in a frame are available from firstCursorPosition() and
    lastCursorPosition(), and the frame's position in the document
    from firstPosition() and lastPosition().

    Frames are usually created using QTextCursor::insertFrame().

    You can iterate over frame's contents using the
    QTextFrame::iterator class: this provides read-only access to a
    frame's list of blocks and child frames.
*/

/*!
    \fn void QTextFrame::setFormat(const QTextFrameFormat &format)

    Sets the frame's \a format.

    \sa format()
*/

/*!
    \fn QTextFrameFormat QTextFrame::format() const

    Returns the frame's format.

    \sa setFormat()
*/

/*!
    Creates a new empty frame for the text document, \a doc.
*/
QTextFrame::QTextFrame(QTextDocument *doc)
    : QTextObject(*new QTextFramePrivate, doc)
{
    d->fragment_start = 0;
    d->fragment_end = 0;
    d->parentFrame = 0;
    d->layoutData = 0;
}

// ### DOC: What does this do to child frames?
/*!
    Destroys this frame and removes it from the document's layout.
*/
QTextFrame::~QTextFrame()
{
    delete d->layoutData;
}

/*!
    \internal
*/
QTextFrame::QTextFrame(QTextFramePrivate &p, QTextDocument *doc)
    : QTextObject(p, doc)
{
    d->fragment_start = 0;
    d->fragment_end = 0;
    d->parentFrame = 0;
    d->layoutData = 0;
}

/*!
    Returns a (possibly empty) list of this frame's child frames.

    \sa parentFrame()
*/
QList<QTextFrame *> QTextFrame::childFrames()
{
    return d->childFrames;
}

/*!
    Returns this frame's parent frame (which will be 0 if this frame
    is the QTextDocument::rootFrame()).

    \sa childFrames()
*/
QTextFrame *QTextFrame::parentFrame()
{
    return d->parentFrame;
}


/*!
    Returns the first cursor position inside the frame.

    \sa lastCursorPosition() firstPosition() lastPosition()
*/
QTextCursor QTextFrame::firstCursorPosition() const
{
    return QTextCursor(d->pieceTable, firstPosition());
}

/*!
    Returns the last cursor position inside the frame.

    \sa firstCursorPosition() firstPosition() lastPosition()
*/
QTextCursor QTextFrame::lastCursorPosition() const
{
    return QTextCursor(d->pieceTable, lastPosition());
}

/*!
    Returns the first document position inside the frame.

    \sa lastPosition() firstCursorPosition() lastCursorPosition()
*/
int QTextFrame::firstPosition() const
{
    if (!d->fragment_start)
        return 0;
    return d->pieceTable->fragmentMap().position(d->fragment_start) + 1;
}

/*!
    Returns the last document position inside the frame.

    \sa firstPosition() firstCursorPosition() lastCursorPosition()
*/
int QTextFrame::lastPosition() const
{
    if (!d->fragment_end)
        return d->pieceTable->length() - 1;
    return d->pieceTable->fragmentMap().position(d->fragment_end);
}

/*!
  \internal
*/
QTextFrameLayoutData *QTextFrame::layoutData() const
{
    return d->layoutData;
}

/*!
  \internal
*/
void QTextFrame::setLayoutData(QTextFrameLayoutData *data)
{
    delete d->layoutData;
    d->layoutData = data;
}



void QTextFramePrivate::fragmentAdded(const QChar &type, uint fragment)
{
    if (type == QTextBeginningOfFrame) {
        Q_ASSERT(!fragment_start);
        fragment_start = fragment;
    } else if (type == QTextEndOfFrame) {
        Q_ASSERT(!fragment_end);
        fragment_end = fragment;
    } else if (type == QChar::ObjectReplacementCharacter) {
        Q_ASSERT(!fragment_start);
        Q_ASSERT(!fragment_end);
        fragment_start = fragment;
        fragment_end = fragment;
    } else {
        Q_ASSERT(false);
    }
}

void QTextFramePrivate::fragmentRemoved(const QChar &type, uint fragment)
{
    if (type == QTextBeginningOfFrame) {
        Q_ASSERT(fragment_start == fragment);
        fragment_start = 0;
    } else if (type == QTextEndOfFrame) {
        Q_ASSERT(fragment_end == fragment);
        fragment_end = 0;
    } else if (type == QChar::ObjectReplacementCharacter) {
        Q_ASSERT(fragment_start == fragment);
        Q_ASSERT(fragment_end == fragment);
        fragment_start = 0;
        fragment_end = 0;
    } else {
        Q_ASSERT(false);
    }
    remove_me();
}


void QTextFramePrivate::remove_me()
{
    if (!parentFrame)
        return;

    int index = parentFrame->d->childFrames.indexOf(q);

    // iterator over all children and move them to the parent
    for (int i = 0; i < childFrames.size(); ++i) {
        QTextFrame *c = childFrames.at(i);
        parentFrame->d->childFrames.insert(index, c);
        c->d->parentFrame = parentFrame;
        ++index;
    }
    Q_ASSERT(parentFrame->d->childFrames.at(index) == q);
    parentFrame->d->childFrames.removeAt(index);

    childFrames.clear();
    parentFrame = 0;
}

/*!
    Returns an iterator pointing to the first block inside the frame.

    \sa end()
*/
QTextFrame::iterator QTextFrame::begin() const
{
    const QTextDocumentPrivate *priv = docHandle();
    int b = priv->blockMap().findNode(firstPosition());
    int e = priv->blockMap().findNode(lastPosition()+1);
    return iterator(const_cast<QTextFrame *>(this), b, b, e);
}

/*!
    Returns an iterator pointing to the last block inside the frame.

    \sa begin()
*/
QTextFrame::iterator QTextFrame::end() const
{
    const QTextDocumentPrivate *priv = docHandle();
    int b = priv->blockMap().findNode(firstPosition());
    int e = priv->blockMap().findNode(lastPosition()+1);
    return iterator(const_cast<QTextFrame *>(this), e, b, e);
}

/*!
    \class QTextFrame::iterator
    \brief the QTextFrame::iterator class provides a means of reading
    the contents of a QTextFrame.

    A frame consists of an arbitrary sequence of \l{QTextBlock}s and
    child \c{QTextFrame}s. This class provides a read-only means of
    iterating over the contents of a frame.

*/

/*!
    Constructs an invalid iterator.
*/
QTextFrame::iterator::iterator()
{
    f = 0;
    cf = 0;
    cb = 0;
}

/*!
  \internal
*/
QTextFrame::iterator::iterator(QTextFrame *frame, int block, int begin, int end)
{
    f = frame;
    b = begin;
    e = end;
    cf = 0;
    cb = block;
}

QTextFrame::iterator::iterator(const iterator &o)
{
    f = o.f;
    b = o.b;
    e = o.e;
    cf = o.cf;
    cb = o.cb;
}

/*!
    Returns the current frame the iterator points to or 0 if the
    iterator currently points to a block.

    \sa currentBlock()
*/
QTextFrame *QTextFrame::iterator::currentFrame() const
{
    return cf;
}

/*!
    Returns the current block the iterator points to. If the iterator
    points to a child frame, the returned block is invalid.

    \sa currentFrame()
*/
QTextBlock QTextFrame::iterator::currentBlock() const
{
    return QTextBlock(f->docHandle(), cb);
}

/*!
    Moves the iterator to the next frame or block.

    \sa currentBlock() currentFrame()
*/
QTextFrame::iterator QTextFrame::iterator::operator++()
{
    const QTextDocumentPrivate *priv = f->docHandle();
    const QTextDocumentPrivate::BlockMap &map = priv->blockMap();
    if (cf) {
        int end = cf->lastPosition() + 1;
        cb = map.findNode(end);
        cf = 0;
    } else if (cb) {
        cb = map.next(cb);
        if (cb == e)
            return *this;

        int pos = map.position(cb);
        // check if we entered a frame
        QTextDocumentPrivate::FragmentIterator frag = priv->find(pos-1);
        if (priv->buffer().at(frag->stringPosition) != QChar::ParagraphSeparator) {
            QTextFrame *nf = qt_cast<QTextFrame *>(priv->objectForFormat(frag->format));
            if (nf) {
                if (priv->buffer().at(frag->stringPosition) == QTextBeginningOfFrame && nf != f) {
                    cf = nf;
                    cb = 0;
                } else {
                    Q_ASSERT(priv->buffer().at(frag->stringPosition) != QTextEndOfFrame);
                }
            }
        }
    }
    return *this;
}

/*!
    Moves the iterator to the previous frame or block.

    \sa currentBlock() currentFrame()
*/
QTextFrame::iterator QTextFrame::iterator::operator--()
{
    const QTextDocumentPrivate *priv = f->docHandle();
    const QTextDocumentPrivate::BlockMap &map = priv->blockMap();
    if (cf) {
        int start = cf->firstPosition() - 1;
        cb = map.findNode(start);
        cf = 0;
    } else {
        if (cb == b)
            goto end;
        if (cb != e) {
            int pos = map.position(cb);
            // check if we have to enter a frame
            QTextDocumentPrivate::FragmentIterator frag = priv->find(pos-1);
            if (priv->buffer().at(frag->stringPosition) != QChar::ParagraphSeparator) {
                QTextFrame *pf = qt_cast<QTextFrame *>(priv->objectForFormat(frag->format));
                if (pf) {
                    if (priv->buffer().at(frag->stringPosition) == QTextBeginningOfFrame) {
                        Q_ASSERT(pf == f);
                    } else if (priv->buffer().at(frag->stringPosition) == QTextEndOfFrame) {
                        Q_ASSERT(pf != f);
                        cf = pf;
                        cb = 0;
                        goto end;
                    }
                }
            }
        }
        cb = map.previous(cb);
    }
 end:
    return *this;
}





/*!
    \class QTextBlock qtextblock.h
    \brief The QTextBlock class provides an API for accessing the block
    structure of QTextDocuments.

    \ingroup text

    A QTextBlock is an object that provides read-only access to the
    block/paragraph structure of QTextDocuments. It is mainly
    of use if you want to implement your own layouts for the
    visual representation of a QTextDocument, or if you want to
    iterate over a document and output its contents in your own custom
    format.

    The text block has a position() in the document, a length(), a
    text layout(), a charFormat(), a blockFormat(), and a text(). And
    it belongs to a document(). Navigation between blocks can be done
    using next() and previous().
 */

/*!
    \fn QTextBlock::QTextBlock(QTextDocumentPrivate *priv, int b)

    \internal
*/

/*!
    \fn QTextBlock::QTextBlock()

    \internal
*/

/*!
    \fn QTextBlock::QTextBlock(const QTextBlock &other)

    Copies the \a other text block's attributes to this text block.
*/

/*!
    \fn bool QTextBlock::isValid() const

    Returns true if this text block is valid; otherwise returns false.
*/

/*!
    \fn QTextBlock &QTextBlock::operator=(const QTextBlock &other)

    Assigns the \a other text block to this text block.
*/

/*!
    \fn bool QTextBlock::operator==(const QTextBlock &other) const

    Returns true if this text block is the same as the \a other text
    block.
*/

/*!
    \fn bool QTextBlock::operator!=(const QTextBlock &other) const

    Returns true if this text block is different from the \a other
    text block.
*/

/*!
    \fn bool QTextBlock::operator<(const QTextBlock &other) const

    Returns true if this text block occurs before the \a other text
    block in the document.
*/

/*!
    \fn QTextDocumentPrivate *QTextBlock::docHandle() const

    \internal
*/


/*!
    Returns the starting position of the block within the document.
 */
int QTextBlock::position() const
{
    if (!p || !n)
        return 0;

    return p->blockMap().position(n);
}

/*!
    Returns the length of the block in characters.

    \sa text() charFormat() blockFormat()
 */
int QTextBlock::length() const
{
    if (!p || !n)
        return 0;

    return p->blockMap().size(n);
}

/*!
    Returns true if the given \a position is located within the text
    block; otherwise returns false.
 */
bool QTextBlock::contains(int position) const
{
    if (!p || !n)
        return false;

    int pos = p->blockMap().position(n);
    int len = p->blockMap().size(n);
    return position >= pos && position < pos + len;
}

/*!
    Returns the QTextLayout that is used to lay out and display the
    block's contents.
 */
QTextLayout *QTextBlock::layout() const
{
    if (!p || !n)
        return 0;

    const QTextBlockData *b = p->blockMap().fragment(n);
    if (!b->layout) {
        b->layout = new QTextLayout();
        b->layout->setFormatCollection(p->formatCollection());
        b->layout->setDocumentLayout(p->layout());
    }
    if (b->textDirty) {
        QString text = this->text();
        b->layout->setText(text, charFormat().font());

        if (!text.isEmpty()) {
            int lastTextPosition = 0;
            int textLength = 0;

            QTextDocumentPrivate::FragmentIterator it = p->find(position());
            QTextDocumentPrivate::FragmentIterator end = p->find(position() + length() - 1); // -1 to omit the block separator char
            int lastFormatIdx = it.value()->format;

            for (; it != end; ++it) {
                const QTextFragmentData * const frag = it.value();

                const int formatIndex = frag->format;
                if (formatIndex != lastFormatIdx) {
                    Q_ASSERT(lastFormatIdx != -1);
                    b->layout->setFormat(lastTextPosition, textLength, lastFormatIdx);

                    lastFormatIdx = formatIndex;
                    lastTextPosition += textLength;
                    textLength = 0;
                }

                textLength += frag->size;
            }

            Q_ASSERT(lastFormatIdx != -1);
            b->layout->setFormat(lastTextPosition, textLength, lastFormatIdx);
        }
        b->textDirty = false;
    }
    return b->layout;
}

/*!
    Returns the QTextBlockFormat that describes block-specific properties.

    \sa charFormat()
 */
QTextBlockFormat QTextBlock::blockFormat() const
{
    if (!p || !n)
        return QTextFormat().toBlockFormat();

    return p->formatCollection()->blockFormat(p->blockMap().fragment(n)->format);
}


/*!
    Returns the QTextCharFormat that describes the block's character
    format. This is mainly used to draw block-specific additions such
    as e.g. list markers.

    \sa blockFormat()
 */
QTextCharFormat QTextBlock::charFormat() const
{
    if (!p || !n)
        return QTextFormat().toCharFormat();

    const QTextDocumentPrivate::FragmentMap &fm = p->fragmentMap();
    int pos = p->blockMap().position(n);
    if (pos > 0)
        --pos;
    return p->formatCollection()->charFormat(fm.find(pos)->format);
}

/*!
    Returns the block's plain text.

    \sa length() charFormat() blockFormat()
 */
QString QTextBlock::text() const
{
    if (!p || !n)
        return QString::null;

    const QString buffer = p->buffer();
    QString text;

    int pos = position();
    QTextDocumentPrivate::FragmentIterator it = p->find(pos);
    QTextDocumentPrivate::FragmentIterator end = p->find(pos + length() - 1); // -1 to omit the block separator char
    for (; it != end; ++it) {
        const QTextFragmentData * const frag = it.value();
        text += QString::fromRawData(buffer.constData() + frag->stringPosition, frag->size);
    }

    return text;
}


/*!
    Returns the text document this text block belongs to, or 0 if the
    text block doesn't belong to any document.
*/
const QTextDocument *QTextBlock::document() const
{
    return p ? p->document() : 0;
}


/*!
    Returns a text block iterator pointing to the beginning of the
    text block.

    \sa end()
*/
QTextBlock::iterator QTextBlock::begin() const
{
    if (!p || !n)
        return iterator();

    int pos = position();
    int len = length();
    int b = p->fragmentMap().findNode(pos);
    int e = p->fragmentMap().findNode(pos+len);
    return iterator(p, b, e, b);
}

/*!
    Returns a text block iterator pointing to the end of the text
    block.

    \sa begin() next() previous()
*/
QTextBlock::iterator QTextBlock::end() const
{
    if (!p || !n)
        return iterator();

    int pos = position();
    int len = length();
    int b = p->fragmentMap().findNode(pos);
    int e = p->fragmentMap().findNode(pos+len);
    return iterator(p, b, e, e);
}


/*!
    Returns the text block after this one, or an empty text block if
    this is the last one.

    \sa previous() begin() end() next()
*/
QTextBlock QTextBlock::next() const
{
    if (!p)
        return QTextBlock();

    return QTextBlock(p, p->blockMap().next(n));
}

/*!
    Returns the text block before this one, or an empty text block if
    this is the first one.

    \sa next() begin() end() previous()
*/
QTextBlock QTextBlock::previous() const
{
    if (!p)
        return QTextBlock();

    return QTextBlock(p, p->blockMap().previous(n));
}


/*!
    Returns the text fragment the iterator currently points to.
*/
QTextFragment QTextBlock::iterator::fragment() const
{
    int ne = n;
    int formatIndex = p->fragmentMap().fragment(n)->format;
    do {
        ne = p->fragmentMap().next(ne);
    } while (ne != e && p->fragmentMap().fragment(ne)->format == formatIndex);
    return QTextFragment(p, n, ne);
}

QTextBlock::iterator QTextBlock::iterator::operator++()
{
    int ne = n;
    int formatIndex = p->fragmentMap().fragment(n)->format;
    do {
        ne = p->fragmentMap().next(ne);
    } while (ne != e && p->fragmentMap().fragment(ne)->format == formatIndex);
    n = ne;
    return *this;
}

QTextBlock::iterator QTextBlock::iterator::operator--()
{
    int ne = p->fragmentMap().previous(n);
    int formatIndex = p->fragmentMap().fragment(n)->format;
    int prev = ne;
    do {
        ne = prev;
        prev = p->fragmentMap().previous(ne);
    } while (ne != b && p->fragmentMap().fragment(prev)->format == formatIndex);
    n = ne;
    return *this;
}


/*!
    \class QTextFragment
    \brief The QTextFragment class holds a piece of text in a
    QTextDocument that has a single QTextCharFormat.

    If the user edits the text in a fragment, for example, makes a
    word in the middle bold, the fragment will be broken into three
    separate fragments, the first and third with the same format as
    before and containing the text before and after the emboldened
    word, and the second with a bold font and the emboldened word.

    A text fragment can be queried for its text(), charFormat(),
    length() and position(), and for whether it contains() a
    particular position in the document as a whole.
*/

/*!
    \fn QTextFragment::QTextFragment(const QTextDocumentPrivate *priv, int f, int fe)
    \internal
*/

/*!
    \fn QTextFragment::QTextFragment()

    Creates a new empty text fragment.
*/

/*!
    \fn QTextFragment::QTextFragment(const QTextFragment &other)

    Copies the content (text and format) of the \a other text fragment
    to this text fragment.
*/

/*!
    \fn QTextFragment &QTextFragment::operator=(const QTextFragment
    &other)

    Assigns the content (text and format) of the \a other text fragment
    to this text fragment.
*/

/*!
    \fn bool QTextFragment::isValid() const

    Returns true if this is a valid text fragment (i.e. has a valid
    position in a document); otherwise returns false.
*/

/*!
    \fn bool QTextFragment::operator==(const QTextFragment &other) const

    Returns true if this text fragment is the same (at the same
    position) as the \a other text fragment; otherwise returns false.
*/

/*!
    \fn bool QTextFragment::operator!=(const QTextFragment &other) const

    Returns true if this text fragment is different (at a different
    position) from the \a other text fragment; otherwise returns
    false.
*/

/*!
    \fn bool QTextFragment::operator<(const QTextFragment &other) const

    Returns true if this text fragment appears earlier in the document
    than the \a other text fragment; otherwise returns false.
*/


/*!
    Returns the position of this text fragment in the document.
*/
int QTextFragment::position() const
{
    if (!p || !n)
        return 0; // ### -1 instead?

    return p->fragmentMap().position(n);
}

/*!
    Returns the number of characters in the text fragment.

    \sa text()
*/
int QTextFragment::length() const
{
    if (!p || !n)
        return 0;

    int len = 0;
    int f = n;
    while (f != ne) {
        len += p->fragmentMap().size(f);
        f = p->fragmentMap().next(f);
    }
    return len;
}

/*!
    Returns true if the text fragment contains the text at the given
    \a position in the document; otherwise returns false.
*/
bool QTextFragment::contains(int position) const
{
    if (!p || !n)
        return false;
    int pos = this->position();
    return position >= pos && position < pos + length();
}

/*!
    Returns the text fragment's character format.

    \sa text()
*/
QTextCharFormat QTextFragment::charFormat() const
{
    if (!p || !n)
        return QTextCharFormat();
    const QTextFragmentData *data = p->fragmentMap().fragment(n);
    return p->formatCollection()->charFormat(data->format);
}

/*!
    Returns the text fragment's text.

    \sa length() charFormat()
*/
QString QTextFragment::text() const
{
    if (!p || !n)
        return QString();

    QString result;
    QString buffer = p->buffer();
    int f = n;
    while (f != ne) {
        result += QString::fromRawData(buffer.constData() + p->fragmentMap().position(f), p->fragmentMap().size(f));
        f = p->fragmentMap().next(f);
    }
    return result;
}
