#include "qtextobject.h"
#include "qtextobject_p.h"
#include "qtextdocument.h"
#include "qtextformat_p.h"
#include "qtextdocument_p.h"
#include "qtextcursor.h"

#define d d_func()
#define q q_func()

QTextObject::QTextObject(QTextDocument *doc)
    : QObject(*new QTextObjectPrivate, doc)
{
}

QTextObject::QTextObject(QTextObjectPrivate &p, QTextDocument *doc)
    :QObject(p, doc)
{
}

QTextObject::~QTextObject()
{
}


QTextFormat QTextObject::format() const
{
    return d->pieceTable->formatCollection()->objectFormat(d->objectIndex);
}

void QTextObject::setFormat(const QTextFormat &format)
{
    int idx = d->pieceTable->formatCollection()->indexForFormat(format);
    d->pieceTable->changeObjectFormat(this, idx);
}

int QTextObject::objectIndex() const
{
    return d->objectIndex;
}

QTextDocument *QTextObject::document() const
{
    return qt_cast<QTextDocument *>(parent());
}


QTextBlockGroup::QTextBlockGroup(QTextDocument *doc)
    : QTextObject(*new QTextBlockGroupPrivate, doc)
{
}

QTextBlockGroup::QTextBlockGroup(QTextBlockGroupPrivate &p, QTextDocument *doc)
    : QTextObject(p, doc)
{
}

QTextBlockGroup::~QTextBlockGroup()
{
}

void QTextBlockGroup::insertBlock(const QTextBlockIterator &block)
{
    QTextBlockGroupPrivate::BlockList::Iterator it = qLowerBound(d->blocks.begin(), d->blocks.end(), block);
    d->blocks.insert(it, block);
}

void QTextBlockGroup::removeBlock(const QTextBlockIterator &block)
{
    d->blocks.removeAll(block);
}

void QTextBlockGroup::blockFormatChanged(const QTextBlockIterator &)
{
}

QList<QTextBlockIterator> QTextBlockGroup::blockList() const
{
    return d->blocks;
}



QTextFrameLayoutData::~QTextFrameLayoutData()
{
}



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
QTextCursor QTextFrame::first() const
{
    return QTextCursor(d->pieceTable, firstPosition());
}

/*!
  The last cursor position inside the frame
*/
QTextCursor QTextFrame::last() const
{
    return QTextCursor(d->pieceTable, lastPosition());
}

int QTextFrame::firstPosition() const
{
    if (!d->fragment_start)
        return 0;
    return d->pieceTable->fragmentMap().position(d->fragment_start) + 1;
}

int QTextFrame::lastPosition() const
{
    if (!d->fragment_end)
        return d->pieceTable->length();
    return d->pieceTable->fragmentMap().position(d->fragment_end);
}

QTextFrameLayoutData *QTextFrame::layoutData() const
{
    return d->layoutData;
}

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

QTextFrame::iterator QTextFrame::begin() const
{
    const QTextDocumentPrivate *priv = document()->data();
    int b = priv->blockMap().findNode(firstPosition());
    return iterator(this, b);
}

QTextFrame::iterator QTextFrame::end() const
{
    return iterator(this, 0);
}


QTextFrame::iterator::iterator()
{
    f = 0;
    cf = 0;
    cb = 0;
}

QTextFrame::iterator::iterator(const iterator &o)
{
    f = o.f;
    cf = o.cf;
    cb = o.cb;
}

const QTextFrame *QTextFrame::iterator::currentFrame() const
{
    return cf;
}

QTextBlock QTextFrame::iterator::currentBlock() const
{
    const QTextDocumentPrivate *priv = f->document()->data();
    return QTextBlock(priv, cb);
}

QTextFrame::iterator QTextFrame::iterator::operator++()
{
    const QTextDocumentPrivate *priv = f->document()->data();
    const QTextDocumentPrivate::BlockMap &map = priv->blockMap();
    if (cf) {
        int end = cf->lastPosition() + 1;
        cb = map.findNode(end);
        cf = 0;
    } else if (cb) {
        cb = map.next(cb);
        int pos = map.position(cb);
        // check if we entered a frame
        QTextDocumentPrivate::FragmentIterator frag = priv->find(pos-1);
        if (priv->buffer().at(frag->stringPosition) != QChar::ParagraphSeparator) {
            QTextFrame *nf = qt_cast<QTextFrame *>(priv->objectForFormat(frag->format));
            if (nf) {
                if (priv->buffer().at(frag->stringPosition) == QTextBeginningOfFrame && nf != f) {
                    cf = nf;
                    cb = 0;
                } else if (priv->buffer().at(frag->stringPosition) == QTextEndOfFrame) {
                    Q_ASSERT(nf == f);
                    cf = 0;
                    cb = 0;
                }
            }
        }
    }
    return *this;
}

QTextFrame::iterator QTextFrame::iterator::operator--()
{
    const QTextDocumentPrivate *priv = f->document()->data();
    const QTextDocumentPrivate::BlockMap &map = priv->blockMap();
    if (cf) {
        int start = cf->firstPosition() - 1;
        cb = map.findNode(start);
        cf = 0;
    } else if (cb) {
        int pos = map.position(cb);
        if (pos == f->firstPosition())
            goto end;
        // check if we have to enter a frame
        QTextDocumentPrivate::FragmentIterator frag = priv->find(pos-1);
        if (priv->buffer().at(frag->stringPosition) != QChar::ParagraphSeparator) {
            QTextFrame *pf = qt_cast<QTextFrame *>(priv->objectForFormat(frag->format));
            if (pf) {
                if (priv->buffer().at(frag->stringPosition) == QTextBeginningOfFrame) {
                    Q_ASSERT(pf == f);
                } else if (priv->buffer().at(frag->stringPosition) == QTextEndOfFrame) {
                    cf = pf;
                    cb = 0;
                    goto end;
                }
            }
        }
        cb = map.prev(cb);
    } else {
        cb = map.findNode(f->lastPosition());
    }
    end:
    return *this;
}





/*!
    \class QTextBlock qtextblock.h
    \brief The QTextBlock class offers an API to access the block structure of QTextDocuments.

    \ingroup text

    A QTextBlock is an object that provides read-only access to the
    block/paragraph structure of QTextDocuments. It is mainly interesting if you want to
    implement your own layouting for the visual representation of a QTextDocument.
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
    Returns true if the given position is located within the block.
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
        QString text = blockText();
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
    Returns the paragraph of plain text the block holds.
 */
QString QTextBlock::blockText() const
{
    if (!p || !n)
        return QString::null;

    const QString buffer = p->buffer();
    QString text;

    QTextDocumentPrivate::FragmentIterator it = p->find(position());
    QTextDocumentPrivate::FragmentIterator end = p->find(position() + length() - 1); // -1 to omit the block separator char
    for (; it != end; ++it) {
        const QTextFragmentData * const frag = it.value();
        text += QString::fromRawData(buffer.constData() + frag->stringPosition, frag->size);
    }

    return text;
}


const QTextDocument *QTextBlock::document() const
{
    return p ? p->document() : 0;
}
