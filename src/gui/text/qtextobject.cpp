#include "qtextobject.h"
#include "qtextobject_p.h"
#include "qtextdocument.h"
#include "qtextformat_p.h"
#include "qtextdocument_p.h"
#include "qtextcursor.h"

#define d d_func()
#define q q_func()

/*! \class QTextObject
  \ingroup text

  A QTextObject is a base class for different kind of objects that can
  group parts of a QTextDocument together.  The common text objects
  provided by Qt are lists (QTextList), frames (QTextFrame) and tables
  (QTextTable).

  There are basically two kinds of text objects. Objects used together
  with blocks (blockformats) and objects used together with characters
  (charformats). The first group derived from QTextBlockGroup, the
  other one from QTextFrame.

  You should rarely need to use this class directly. When creating
  custom text objects, you will also need to reimplement
  QTextDocument::createObject(), that acts as a factory method for
  creating objects.
*/

/*!
  Creates a new QTextObject on the document.

  Should never be called directly, but only from QTextDocument::createObject().
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
  Destroys the text object. Text objects belong to the document, so
  you should never destroy them yourself.
*/
QTextObject::~QTextObject()
{
}

/*!
  The format associated with the text object.
*/
QTextFormat QTextObject::format() const
{
    return d->pieceTable->formatCollection()->objectFormat(d->objectIndex);
}

/*!
  associate a new format with the text object
*/
void QTextObject::setFormat(const QTextFormat &format)
{
    int idx = d->pieceTable->formatCollection()->indexForFormat(format);
    d->pieceTable->changeObjectFormat(this, idx);
}

/*!
  The object index of this object. This can be used together with QTextFormat::setObjectIndex().
*/
int QTextObject::objectIndex() const
{
    return d->objectIndex;
}

/*!
  The document this object belongs to.
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

  These types of text objects can be used to group together a set of
  blocks inside the document, e.g. to form a list.

  Adding blocks of text to the block group can be done by setting the
  object index of the block format to the objectIndex() of this
  object, or more conveniently by calling insertBlock().

  The block group will always have an up to date list of which blocks
  belong to the group independent of editing operations.
 */

/*!
  Create new new block group. Should only get called from QTextDocument::createObject().
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

QTextBlockGroup::~QTextBlockGroup()
{
}

/*!
  Notification when a new block got added to the group. If you
  reimplement this method, always call the parent implementation.
*/
void QTextBlockGroup::blockInserted(const QTextBlock &block)
{
    QTextBlockGroupPrivate::BlockList::Iterator it = qLowerBound(d->blocks.begin(), d->blocks.end(), block);
    d->blocks.insert(it, block);
}

/*!
  Notification when a block got removed from the group. If you
  reimplement this method, always call the parent implementation.
*/
void QTextBlockGroup::blockRemoved(const QTextBlock &block)
{
    d->blocks.removeAll(block);
}

/*!
  Notification when the format of a block of text (that is part of the group) has changed.
*/
void QTextBlockGroup::blockFormatChanged(const QTextBlock &)
{
}

/*!
  returns a list of all the blocks that are part of the block group.
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

  This class represents a frame in a text document.

 A frame is defined by a start and end point inside the document, the
 start point having a QChar(0xfdd0) as text, the end point a
 QChar(0xfdd1), and the char formats of start and end containing a
 reference to the objectIndex of this object.

 Frames can be used to form a hierachical structure on top of the
 otherwise flat document. Each document has a rootFrame
 (s.a. QTextDocument::rootFrame()). Each frame (except for the
 rootFrame has a parent), and a list of children. In addition frames
 can contain QTextBlock's.

 Frames are usually created using QTextCursor::insertFrame().


 You can iterate over a text frame using the QTextFrame::iterator
 class, giving you read only access to the list of blocks and child
 frames contained inside the frame.
*/

QTextFrame::QTextFrame(QTextDocument *doc)
    : QTextObject(*new QTextFramePrivate, doc)
{
    d->fragment_start = 0;
    d->fragment_end = 0;
    d->parentFrame = 0;
    d->layoutData = 0;
}

QTextFrame::~QTextFrame()
{
    delete d->layoutData;
}

QTextFrame::QTextFrame(QTextFramePrivate &p, QTextDocument *doc)
    : QTextObject(p, doc)
{
    d->fragment_start = 0;
    d->fragment_end = 0;
    d->parentFrame = 0;
    d->layoutData = 0;
}

/*!
 */
QList<QTextFrame *> QTextFrame::childFrames()
{
    return d->childFrames;
}

QTextFrame *QTextFrame::parentFrame()
{
    return d->parentFrame;
}


/*!
  The first cursor position inside the frame
*/
QTextCursor QTextFrame::firstCursorPosition() const
{
    return QTextCursor(d->pieceTable, firstPosition());
}

/*!
  The last cursor position inside the frame
*/
QTextCursor QTextFrame::lastCursorPosition() const
{
    return QTextCursor(d->pieceTable, lastPosition());
}

/*!
  The first document position inside the frame
*/
int QTextFrame::firstPosition() const
{
    if (!d->fragment_start)
        return 0;
    return d->pieceTable->fragmentMap().position(d->fragment_start) + 1;
}

/*!
  The last document position inside the frame
*/
int QTextFrame::lastPosition() const
{
    if (!d->fragment_end)
        return d->pieceTable->length();
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
  returns an iterator pointing to the first block inside the frame
*/
QTextFrame::iterator QTextFrame::begin() const
{
    const QTextDocumentPrivate *priv = docHandle();
    int b = priv->blockMap().findNode(firstPosition());
    int e = priv->blockMap().findNode(lastPosition()+1);
    return iterator(const_cast<QTextFrame *>(this), b, b, e);
}

/*!
  returns an iterator pointing to the last block inside the frame
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

  Makes it possible to iterate in a read only way over the frame.
  Every frame is built up by a list of blocks and child frames (freely mixed).

*/

/*!
  Constructs an invalid iterator
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
  returns the current frame the iterator points to or 0 if the iterator currently
  points to a block
*/
QTextFrame *QTextFrame::iterator::currentFrame() const
{
    return cf;
}

/*!  returns the current block the iterator points to. If the iterator
  points to a child frame, the returned block is invalid.
*/
QTextBlock QTextFrame::iterator::currentBlock() const
{
    return QTextBlock(f->docHandle(), cb);
}

/*!
  moves the iterator to the next frame or block
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
  moves the iterator to the previous frame or block
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
    \brief The QTextBlock class offers an API to access the block
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
    Returns a pointer to the QTextLayout that is used to layout and display the
    block contents.
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
        b->layout->setText(text);

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
    Returns the QTextBlockFormat that describes block specific properties.
 */
QTextBlockFormat QTextBlock::blockFormat() const
{
    if (!p || !n)
        return QTextFormat().toBlockFormat();

    return p->formatCollection()->blockFormat(p->blockMap().fragment(n)->format);
}


/*!
    Returns the QTextCharFormat that describes the character format for the block.
    This is mainly used to draw block specific additions as e.g. list markers.
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
    Returns the paragraph of plain text the block holds.
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

    \sa begin()
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

    \sa previous()
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

    \sa next()
*/
QTextBlock QTextBlock::previous() const
{
    if (!p)
        return QTextBlock();

    return QTextBlock(p, p->blockMap().previous(n));
}


/*!
  returns the text fragment the iterator currently points to.
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

  A text fragment is a continious piece of text in the document that carries the same
  format.
*/


int QTextFragment::position() const
{
    if (!p || !n)
        return 0; // ### -1 instead?

    return p->fragmentMap().position(n);
}

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

bool QTextFragment::contains(int position) const
{
    if (!p || !n)
        return false;
    int pos = this->position();
    return position >= pos && position < pos + length();
}

QTextCharFormat QTextFragment::charFormat() const
{
    if (!p || !n)
        return QTextCharFormat();
    const QTextFragmentData *data = p->fragmentMap().fragment(n);
    return p->formatCollection()->charFormat(data->format);
}

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
