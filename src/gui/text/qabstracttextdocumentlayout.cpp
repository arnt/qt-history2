#include <qabstracttextdocumentlayout.h>
#include <qtextformat.h>
#include "qtextdocument_p.h"
#include "qtextengine_p.h"

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


QAbstractTextDocumentLayout::QAbstractTextDocumentLayout(QTextDocument *document)
    : QObject(*new QAbstractTextDocumentLayoutPrivate, document)
{
}

QAbstractTextDocumentLayout::QAbstractTextDocumentLayout(QAbstractTextDocumentLayoutPrivate &p, QTextDocument *document)
    :QObject(p, document)
{
}

void QAbstractTextDocumentLayout::registerHandler(int formatType, QObject *component)
{
    QTextObjectInterface *iface = qt_cast<QTextObjectInterface *>(component);
    if (!iface)
        return; // ### print error message on terminal?

    connect(component, SIGNAL(destroyed(QObject*)), this, SLOT(handlerDestroyed(QObject*)));

    QTextObjectHandler h;
    h.iface = iface;
    h.component = component;
    d->handlers.insert(formatType, h);
}

QTextObjectInterface *QAbstractTextDocumentLayout::handlerForObject(int objectType) const
{
    QTextObjectHandler handler = d->handlers.value(objectType);
    if (!handler.component)
        return 0;

    return handler.iface;
}

void QAbstractTextDocumentLayout::setSize(QTextInlineObject item, const QTextFormat &format)
{
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;

    QSize s = handler.iface->intrinsicSize(format);
    item.setWidth(s.width());
    item.setAscent(s.height());
    item.setDescent(0);
}

void QAbstractTextDocumentLayout::layoutObject(QTextInlineObject item, const QTextFormat &format)
{
    Q_UNUSED(item);
    Q_UNUSED(format);
}

void QAbstractTextDocumentLayout::drawObject(QPainter *p, const QRect &rect, QTextInlineObject item,
                                             const QTextFormat &format, QTextLayout::SelectionType selType)
{
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;
    handler.iface->drawObject(p, rect, format);

    if (selType == QTextLayout::Highlight && item.engine()->pal) {
        QBrush brush(item.engine()->pal->highlight(), QBrush::Dense4Pattern);
        p->fillRect(rect, brush);
    }
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


void QAbstractTextDocumentLayout::invalidate(const QRect & /* r */)
{
}

void QAbstractTextDocumentLayout::invalidate(const QRegion & /* r */)
{
}

QTextBlock QAbstractTextDocumentLayout::findBlock(int pos) const
{
    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    return QTextBlock(pieceTable, pieceTable->blockMap().findNode(pos));
}

QTextBlock QAbstractTextDocumentLayout::begin() const
{
    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    return QTextBlock(pieceTable, pieceTable->blockMap().begin().n);
}

QTextBlock QAbstractTextDocumentLayout::end() const
{
    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    return QTextBlock(pieceTable, 0);
}

QTextFrame *QAbstractTextDocumentLayout::frameAt(int pos) const
{
    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    return pieceTable->frameAt(pos);
}

QTextFrame *QAbstractTextDocumentLayout::rootFrame() const
{
    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    return pieceTable->rootFrame();
}


int QAbstractTextDocumentLayout::formatIndex(int pos)
{
    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    return pieceTable->find(pos).value()->format;
}

QTextCharFormat QAbstractTextDocumentLayout::format(int pos)
{
    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    int idx = pieceTable->find(pos).value()->format;
    return pieceTable->formatCollection()->charFormat(idx);
}


QTextObject *QAbstractTextDocumentLayout::object(int objectIndex) const
{
    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    return pieceTable->objectForIndex(objectIndex);
}

QTextObject *QAbstractTextDocumentLayout::objectForFormat(const QTextFormat &f) const
{
    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    return pieceTable->objectForFormat(f);
}


const QTextDocument *QAbstractTextDocumentLayout::document() const
{
    return qt_cast<QTextDocument *>(parent());
}


/*!
    Returns the name of the anchor at point \a pos, or an empty string
    if there's no anchor at that point.
*/
QString QAbstractTextDocumentLayout::anchorAt(const QPoint& pos) const
{
    int cursorPos = hitTest(pos, QText::ExactHit);
    if (cursorPos == -1)
        return QString();

    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    QTextDocumentPrivate::FragmentIterator it = pieceTable->find(cursorPos);
    QTextCharFormat fmt = pieceTable->formatCollection()->charFormat(it->format);
    return fmt.anchorName();
}
