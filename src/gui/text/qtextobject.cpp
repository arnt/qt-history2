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
    (QTextFrame), and tables (QTextTable). A text object has an
    associated format() and document().

    There are essentially two kinds of text objects: those that are used
    with blocks (block formats), and those that are used with characters
    (character formats). The first kind are derived from QTextBlockGroup,
    and the second kind from QTextFrame.

    You rarely need to use this class directly. When creating custom text
    objects, you will also need to reimplement QTextDocument::createObject()
    which acts as a factory method for creating text objects.

    \sa QTextDocument
*/

/*!
    \fn int QTextObject::formatType() const

    \internal
*/

/*!
    \fn QTextObject::QTextObject(QTextDocument *document)

    Creates a new QTextObject for the given \a document.

    \warning This function should never be called directly, but only
    from QTextDocument::createObject().
*/
QTextObject::QTextObject(QTextDocument *doc)
    : QObject(*new QTextObjectPrivate, doc)
{
}

/*!
  \fn QTextObject::QTextObject(QTextObjectPrivate &p, QTextDocument *document)

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
    Sets the text object's \a format.

    \sa format()
*/
void QTextObject::setFormat(const QTextFormat &format)
{
    int idx = d->pieceTable->formatCollection()->indexForFormat(format);
    d->pieceTable->changeObjectFormat(this, idx);
}

/*!
    Returns the object index of this object. This can be used together with
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
    return qt_cast<const QTextDocument *>(parent())->docHandle();
}

/*!
    \class QTextBlockGroup
    \brief The QTextBlockGroup class provides a container for text blocks within
    a QTextDocument.

    Block groups can be used to organize blocks of text within a document.
    They maintain an up-to-date list of the text blocks that belong to
    them, even when text blocks are being edited.

    Each group has a parent document which is specified when the group is
    constructed.

    Text blocks can be inserted into a group with blockInserted(), and removed
    with blockRemoved(). If a block's format is changed, blockFormatChanged()
    is called. 

    The list of blocks in the group is returned by blockList().

    \sa QTextBlock QTextDocument
*/

/*!
    \fn QTextBlockGroup::QTextBlockGroup(QTextDocument *document)

    Creates a new new block group for the given \a document.

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
    This function is called whenever the specified \a block of text is changed.
    The text block is a member of this group.

    The base class implementation does nothing.
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

    \ingroup text

    Text frames provide structure for the text in a document.

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

    \sa QTextCursor QTextDocument
*/

/*!
    \typedef QTextFrame::Iterator

    Qt-style synonym for QTextFrame::iterator.
*/

/*!
    \fn QTextFrame *QTextFrame::iterator::parentFrame() const

    Returns the parent frame of the current frame.

    \sa currentFrame() QTextFrame::parentFrame()
*/

/*!
    \fn bool QTextFrame::iterator::operator==(const iterator &other) const

    Retuns true if this iterator is the same as the \a other iterator;
    otherwise returns false.
*/

/*!
    \fn bool QTextFrame::iterator::operator!=(const iterator &other) const

    Retuns true if this iterator is different from the \a other iterator;
    otherwise returns false.
*/

/*!
    \fn QTextFrame::iterator QTextFrame::iterator::operator++(int)

    The postfix ++ operator (\c{i++}) advances the iterator to the
    next item in the text frame and returns an iterator to the old text
    frame.
*/

/*!
    \fn QTextFrame::iterator QTextFrame::iterator::operator--(int)

    The postfix -- operator (\c{i--}) makes the preceding text frame current
    and returns an iterator to the old text frame.
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
    \fn QTextFrame::QTextFrame(QTextDocument *document)

    Creates a new empty frame for the text \a document.
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
    Q_UNUSED(fragment); // --release warning
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
    \class QTextFrame::iterator
    \brief The QTextFrame::iterator class provides an iterator for reading
    the contents of a QTextFrame.

    A frame consists of an arbitrary sequence of \l{QTextBlock}s and
    child \l{QTextFrame}s. This class provides a way to iterate over the
    child objects of a frame, and read their contents. It does not provide
    a way to modify the contents of the frame.

*/

/*!
    \fn bool QTextFrame::iterator::atEnd() const

    Returns true if the current item is the last item in the text frame.
*/

/*!
    Returns an iterator pointing to the first child frame inside this frame.

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
    Returns an iterator pointing to the last child frame inside this frame.

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

/*!
    \fn QTextFrame::iterator::iterator(const iterator &other)

    Copy constructor. Constructs a copy of the \a other iterator.*/
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
    \brief The QTextBlock class provides a container for text fragments in a
    QTextDocument.

    \ingroup text

    A QTextBlock is an object that provides read-only access to the
    block/paragraph structure of QTextDocuments. It is mainly
    of use if you want to implement your own layouts for the
    visual representation of a QTextDocument, or if you want to
    iterate over a document and write the contents in your own custom
    format.

    Text blocks are created by their parent documents. If you need to create
    a new text block, use the cursor-based interface provided by QTextCursor.

    Each text block is located at a specific position() in a document().
    The contents of the block can be obtained by using the text() function.
    The length() function determines the block's size within the document
    (including formatting characters).
    The visual properties of the block are determined by its text layout(),
    its charFormat(), and its blockFormat().

    The next() and previous() functions allow navigation between blocks
    within the document. Note that blocks are returned in sequence, so
    adjacent blocks may come from different places in the document structure.

    \img qtextblock-sequence.png

    \sa QTextBlockFormat QTextCharFormat QTextFragment
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
    \class QTextBlock::iterator
    \brief The QTextBlock::iterator class provides an iterator for reading
    the contents of a QTextBlock.

    A block consists of a sequence of text fragments. This class provides
    a way to iterate over these, and read their contents. It does not provide
    a way to modify the internal structure or contents of the block.

    An iterator can be constructed and used to access the fragments within
    a text block in the following way:

    \quotefile snippets/textblock-fragments/xmlwriter.cpp
    \skipto QTextBlock::iterator
    \printuntil if (fragment.isValid())
    \skipto ...
    \printuntil ...
    \skipto }
    \printuntil }
    \skipto }
    \printuntil }

    \sa QTextFragment
*/

/*!
    \typedef QTextBlock::Iterator

    Qt-style synonym for QTextBlock::iterator.
*/

/*!
    \fn QTextBlock::iterator::iterator()

    Constructs an iterator for this text block.
*/

/*!
    \fn QTextBlock::iterator::iterator(const iterator &other)

    Copy constructor. Constructs a copy of the \a other iterator.
*/

/*!
    \fn bool QTextBlock::iterator::atEnd() const

    Returns true if the current item is the last item in the text block.
*/

/*!
    \fn bool QTextBlock::iterator::operator==(const iterator &other) const

    Retuns true if this iterator is the same as the \a other iterator;
    otherwise returns false.
*/

/*!
    \fn bool QTextBlock::iterator::operator!=(const iterator &other) const

    Retuns true if this iterator is different from the \a other iterator;
    otherwise returns false.
*/

/*!
    \fn QTextBlock::iterator QTextBlock::iterator::operator++(int)

    The postfix ++ operator (\c{i++}) advances the iterator to the
    next item in the text block and returns an iterator to the old current
    item.
*/

/*!
    \fn QTextBlock::iterator QTextBlock::iterator::operator--(int)

    The postfix -- operator (\c{i--}) makes the preceding item current and
    returns an iterator to the old current item.
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
    if (!b->layout || b->textDirty) {
        delete b->layout;
        b->layout = new QTextLayout(*this);
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
    Returns the block's contents as plain text.

    \sa length() charFormat() blockFormat()
 */
QString QTextBlock::text() const
{
    if (!p || !n)
        return QString::null;

    const QString buffer = p->buffer();
    QString text;

    const int pos = position();
    QTextDocumentPrivate::FragmentIterator it = p->find(pos);
    QTextDocumentPrivate::FragmentIterator end = p->find(pos + length() - 1); // -1 to omit the block separator char
    for (; it != end; ++it) {
        const QTextFragmentData * const frag = it.value();
        text += QString(buffer.constData() + frag->stringPosition, frag->size);
    }

    return text;
}


/*!
    Returns the text document this text block belongs to, or 0 if the
    text block does not belong to any document.
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
    int len = length() - 1; // exclude the fragment that holds the paragraph separator
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
    int len = length() - 1; // exclude the fragment that holds the paragraph separator
    int b = p->fragmentMap().findNode(pos);
    int e = p->fragmentMap().findNode(pos+len);
    return iterator(p, b, e, e);
}


/*!
    Returns the text block in the document after this block, or an empty
    text block if this is the last one.

    Note that the next block may be in a different frame or table to this block.

    \sa previous() begin() end()
*/
QTextBlock QTextBlock::next() const
{
    if (!p)
        return QTextBlock();

    return QTextBlock(p, p->blockMap().next(n));
}

/*!
    Returns the text block in the document before this block, or an empty text
    block if this is the first one.

    Note that the next block may be in a different frame or table to this block.

    \sa next() begin() end()
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

/*!
    The prefix ++ operator (\c{++i}) advances the iterator to the
    next item in the hash and returns an iterator to the new current
    item.
*/

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

/*!
    The prefix -- operator (\c{--i}) makes the preceding item
    current and returns an iterator pointing to the new current item.
*/

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
    QTextDocument with a single QTextCharFormat.

    A text fragment describes a piece of text that is stored with a single
    character format. Text in which the character format changes can be
    represented by sequences of text fragments with different formats.

    If the user edits the text in a fragment and introduces a different
    character format, the fragment's text will be split at each point where
    the format changes, and new fragments will be created.
    For example, changing the style of some text in the middle of a
    sentence will cause the fragment to be broken into three separate fragments:
    the first and third with the same format as before, and the second with
    the new style. The first fragment will contain the text from the beginning
    of the sentence, the second will contain the text from the middle, and the
    third takes the text from the end of the sentence.

    \img qtextfragment-split.png

    A fragment's text and character format can be obtained with the text()
    and charFormat() functions. The length() function gives the length of
    the text in the fragment. position() gives the position in the document
    of the start of the fragment. To determine whether the fragment contains
    a particular position within the document, use the contains() function.

    \sa QTextDocument
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
    Returns the text fragment's as plain text.

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
        const QTextFragmentData * const frag = p->fragmentMap().fragment(f);
        result += QString(buffer.constData() + frag->stringPosition, frag->size);
        f = p->fragmentMap().next(f);
    }
    return result;
}
