#include <qabstracttextdocumentlayout.h>
#include <qtextformat.h>
#include "qtextpiecetable_p.h"

#include "qabstracttextdocumentlayout_p.h"
#define d d_func()
#define q q_func()


/*!
    \class QAbstractTextDocumentLayout qabstracttextdocumentlayout.h
    \brief An abstract base class used to implement custom layouts for QTextDocuments.

    \ingroup text

    QAbstractTextDocumentLayout is an abstract base class that can be
    used to implement custom layouts for QTextDocuments. The standard
    layout provided by Qt can handle simple word processing including
    inline layouting, lists and tables.

    Some applications (e.g. a word processor or a DTP application)
    might have requirements that need more features the ones offered
    by Qt's layouting engine. QAbstractTextDocumentLayout can in this
    case be used to implement a custom layouter for the text document.
*/


QAbstractTextDocumentLayout::QAbstractTextDocumentLayout()
    : QObject(*new QAbstractTextDocumentLayoutPrivate, 0)
{
}


void QAbstractTextDocumentLayout::registerHandler(int formatType, QObject *component)
{
    QTextObjectInterface *iface = qt_cast<QTextObjectInterface *>(component);
    if (!iface)
	return; // ### print error message on terminal?

    connect(component, SIGNAL(destroyed(QObject *)), this, SLOT(handlerDestroyed(QObject *)));

    QTextObjectHandler h;
    h.iface = iface;
    h.component = component;
    d->handlers.insert(formatType, h);
}

void QAbstractTextDocumentLayout::layoutObject(QTextObject item, const QTextFormat &format)
{
    QTextObjectHandler handler = d->handlers.value(format.type());
    if (!handler.component)
	return;
    handler.iface->layoutObject(item, format);
}

void QAbstractTextDocumentLayout::drawObject(QPainter *p, const QPoint &position, QTextObject item,
					     const QTextFormat &format, QTextLayout::SelectionType selType)
{
    QTextObjectHandler handler = d->handlers.value(format.type());
    if (!handler.component)
	return;
    handler.iface->drawObject(p, position, item, format, selType);
}

void QAbstractTextDocumentLayout::handlerDestroyed(QObject *obj)
{
    HandlerHash::Iterator it = d->handlers.begin();
    while (it != d->handlers.end())
	if ((*it).component == obj)
	    it = d->handlers.erase(it);
	else
	    ++it;
}


void QAbstractTextDocumentLayout::invalidate(const QRect &r)
{
}

void QAbstractTextDocumentLayout::invalidate(const QRegion &r)
{
}

void QAbstractTextDocumentLayout::setPageSize(const QSize &size)
{
    d->pageSize = size;
}

QSize QAbstractTextDocumentLayout::pageSize() const
{
    return d->pageSize;
}


QTextLayout *QAbstractTextDocumentLayout::BlockIterator::layout() const
{
    const QTextPieceTable::BlockMap &blockMap = pt->blockMap();
    return blockMap.fragment(block)->layout;
}

QTextBlockFormat QAbstractTextDocumentLayout::BlockIterator::format() const
{
    int idx = formatIndex();
    if (idx == -1)
	return QTextBlockFormat();

    return pt->formatCollection()->blockFormat(idx);
}

int QAbstractTextDocumentLayout::BlockIterator::formatIndex() const
{
    if (atEnd())
	return -1;

    QTextPieceTable::FragmentIterator it = pt->find(pt->blockMap().key(block));
    Q_ASSERT(!it.atEnd());

    return it.value()->format;
}

int QAbstractTextDocumentLayout::BlockIterator::position() const
{
    return pt->blockMap().key(block);
}

int QAbstractTextDocumentLayout::BlockIterator::length() const
{
    return pt->blockMap().size(block);
}

bool QAbstractTextDocumentLayout::BlockIterator::operator<(const BlockIterator &it) const
{
    Q_ASSERT(pt == it.pt);
    return pt->blockMap().key(block) < pt->blockMap().key(it.block);
}

QAbstractTextDocumentLayout::BlockIterator& QAbstractTextDocumentLayout::BlockIterator::operator++()
{
    block = pt->blockMap().next(block);
    return *this;
}

QAbstractTextDocumentLayout::BlockIterator& QAbstractTextDocumentLayout::BlockIterator::operator--()
{
    block = pt->blockMap().prev(block);
    return *this;
}


QAbstractTextDocumentLayout::BlockIterator QAbstractTextDocumentLayout::findBlock(int pos) const
{
    QTextPieceTable *pieceTable = qt_cast<QTextPieceTable *>(parent());
    if (!pieceTable)
	return BlockIterator();
    return BlockIterator(pieceTable, pieceTable->blockMap().findNode(pos));
}

QAbstractTextDocumentLayout::BlockIterator QAbstractTextDocumentLayout::begin() const
{
    QTextPieceTable *pieceTable = qt_cast<QTextPieceTable *>(parent());
    if (!pieceTable)
	return BlockIterator();
    return BlockIterator(pieceTable, pieceTable->blockMap().begin().n);
}

QAbstractTextDocumentLayout::BlockIterator QAbstractTextDocumentLayout::end() const
{
    QTextPieceTable *pieceTable = qt_cast<QTextPieceTable *>(parent());
    if (!pieceTable)
	return BlockIterator();
    return BlockIterator(pieceTable, 0);
}

int QAbstractTextDocumentLayout::formatIndex(int pos)
{
    QTextPieceTable *pieceTable = qt_cast<QTextPieceTable *>(parent());
    if (!pieceTable)
	return -1;
    return pieceTable->find(pos).value()->format;
}

QTextFormat QAbstractTextDocumentLayout::format(int pos)
{
    QTextPieceTable *pieceTable = qt_cast<QTextPieceTable *>(parent());
    if (!pieceTable)
	return QTextFormat();
    int idx = pieceTable->find(pos).value()->format;
    return pieceTable->formatCollection()->blockFormat(idx);
}
