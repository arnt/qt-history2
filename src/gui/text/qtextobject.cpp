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


QList<QTextFrame *> QTextFrame::children()
{
    return d->childFrames;
}

QTextFrame *QTextFrame::parent()
{
    return d->parentFrame;
}


/*!
  The first cursor position inside the frame
*/
QTextCursor QTextFrame::start()
{
    return QTextCursor(d->pieceTable, startPosition());
}

/*!
  The last cursor position inside the frame
*/
QTextCursor QTextFrame::end()
{
    return QTextCursor(d->pieceTable, endPosition());
}

int QTextFrame::startPosition()
{
    if (!d->fragment_start)
        return 0;
    return d->pieceTable->fragmentMap().position(d->fragment_start) + 1;
}

int QTextFrame::endPosition()
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
