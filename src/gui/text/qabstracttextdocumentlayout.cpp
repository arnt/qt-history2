#include <qabstracttextdocumentlayout.h>
#include <qtextformat.h>
#include "qtextdocument_p.h"
#include "qtextengine_p.h"

#include "qabstracttextdocumentlayout_p.h"
#define d d_func()
#define q q_func()


/*!
    \class QAbstractTextDocumentLayout qabstracttextdocumentlayout.h
    \brief The QAbstractTextDocumentLayout class is an abstract base
    class used to implement custom layouts for QTextDocuments.

    \ingroup text

    The standard layout provided by Qt can handle simple word
    processing including inline layouts, lists and tables.

    Some applications (e.g. a word processor or a DTP application)
    might need more features than the ones provided by Qt's layout
    engine, in which case you can subclass QAbstractTextDocumentLayout
    to provide your own custom layout behavior for your text
    documents.
*/

/*!
    \fn void QAbstractTextDocumentLayout::update(const QRect &rect)

    This signal is emitted when the rectangle \a rect has been
    updated.
*/

/*!
    \fn void QAbstractTextDocumentLayout::setPageSize(const QSize &size)

    Sets the page size to \a size.

    \sa pageSize() numPages()
*/

/*!
    \fn QSize QAbstractTextDocumentLayout::pageSize() const

    Returns the page size.

    \sa setPageSize() numPages()
*/

/*!
    \fn void QAbstractTextDocumentLayout::draw(QPainter *painter, const PaintContext &context)

    Draws the layout on the given \a painter with the given \a
    context.
*/

/*!
    \fn int QAbstractTextDocumentLayout::hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const

    \internal

    Returns the cursor postion for the given \a point and with the
    given \a accuracy. Returns -1 to indicate failure (i.e. no valid
    cursor position was found.)
*/

// ### DOC: No idea what this does
/*!
    \fn void QAbstractTextDocumentLayout::documentChange(int from, int oldLength, int length)

    \internal
*/

/*!
    \fn int QAbstractTextDocumentLayout::numPages() const

    Returns the number of pages required by this layout; this depends
    in part on the pageSize().
*/

/*!
    Creates a new text document layout for the given \a document.
*/
QAbstractTextDocumentLayout::QAbstractTextDocumentLayout(QTextDocument *document)
    : QObject(*new QAbstractTextDocumentLayoutPrivate, document)
{
}

/*!
    \internal
*/
QAbstractTextDocumentLayout::QAbstractTextDocumentLayout(QAbstractTextDocumentLayoutPrivate &p, QTextDocument *document)
    :QObject(p, document)
{
}

/*!
    \internal

    Registers the given \a component as a handler for items of the
    given \a formatType.
*/
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

/*!
    \internal

    Returns a handler for objects of the given \a objectType.
*/
QTextObjectInterface *QAbstractTextDocumentLayout::handlerForObject(int objectType) const
{
    QTextObjectHandler handler = d->handlers.value(objectType);
    if (!handler.component)
        return 0;

    return handler.iface;
}

/*!
    Sets the size of the inline object \a item in accordance with the
    text \a format.
*/
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

/*!
    Lays out the inline object \a item using the given text \a format.
    The base class implementation does nothing.

    \sa drawObject()
*/
void QAbstractTextDocumentLayout::layoutObject(QTextInlineObject item, const QTextFormat &format)
{
    Q_UNUSED(item);
    Q_UNUSED(format);
}

/*!
    Called to draw the inline object \a item on painter \a p within
    rectangle \a rect using the given text \a format and with the
    selection type \a selType.

    \sa layoutObject()
*/
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
        QBrush brush(item.engine()->pal->highlight(), Qt::Dense4Pattern);
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

/*!
    \internal

    Returns the index of the format at position \a pos.
*/
int QAbstractTextDocumentLayout::formatIndex(int pos)
{
    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    return pieceTable->find(pos).value()->format;
}

/*!
    Returns the character format that is applicable at position \a
    pos.
*/
QTextCharFormat QAbstractTextDocumentLayout::format(int pos)
{
    QTextDocumentPrivate *pieceTable = qt_cast<QTextDocument *>(parent())->docHandle();
    int idx = pieceTable->find(pos).value()->format;
    return pieceTable->formatCollection()->charFormat(idx);
}



/*!
    Returns the text document that this layout is operating on.
*/
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
