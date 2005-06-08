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

#include <qabstracttextdocumentlayout.h>
#include <qtextformat.h>
#include "qtextdocument_p.h"
#include "qtextengine_p.h"

#include "qabstracttextdocumentlayout_p.h"


/*!
    \class QAbstractTextDocumentLayout
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
    \fn void QAbstractTextDocumentLayout::update(const QRectF &rect)

    This signal is emitted when the rectangle \a rect has been
    updated.
*/

/*!
    \fn void QAbstractTextDocumentLayout::documentSizeChanged(const QSizeF &newSize)

    This signal is emitted when the size of the document changes. The new
    size is specified by \a newSize.

    This information is useful to widgets that display text documents
    since it enables them to update their scroll bars correctly.

    \sa documentSize()
*/

/*!
    \fn void QAbstractTextDocumentLayout::pageCountChanged(int newPages)

    This signal is emitted when the number of pages in the layout
    changes; \a newPages is the updated page count.

    Changes to the page count are due to the changes to the layout or
    the document content itself.

    \sa pageCount()
*/

/*!
    \fn int QAbstractTextDocumentLayout::pageCount() const

    Returns the number of pages required by the layout.

    \sa pageCountChanged()
*/

/*!
    \fn QSizeF QAbstractTextDocumentLayout::documentSize() const

    Returns the total size of the document. This is useful to display widgets
    since they can use to information to update their scroll bars correctly

    \sa documentSizeChanged(), QTextDocument::pageSize
*/

/*!
    \fn void QAbstractTextDocumentLayout::draw(QPainter *painter, const PaintContext &context)

    Draws the layout on the given \a painter with the given \a
    context.
*/

/*!
    \fn int QAbstractTextDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const

    \internal

    Returns the cursor postion for the given \a point and with the
    given \a accuracy. Returns -1 to indicate failure (i.e. no valid
    cursor position was found.)
*/

/*!
    \fn void QAbstractTextDocumentLayout::documentChanged(int position, int charsRemoved, int charsAdded)

    This function is called whenever the contents of the document change.
    A change occurs when text is inserted, removed, or a combination of
    the two types of operation. The change is specified by \a position,
    \a charsRemoved, and \a charsAdded corresponding to the starting
    character position of the change, the number of character removed from
    the document, and the number of characters added.

    For example, when inserting the text "Hello" into an empty document,
    \a charsRemoved would be 0 and \a charsAdded would be 5 (the length of
    the string).

    Replacing text is the combination of removal and insertion. For example,
    if the text "Hello" gets replaced by "Hi", \a charsRemoved would be 5
    and \a charsAdded would be 2.
*/

/*!
    \class QAbstractTextDocumentLayout::PaintContext
    \internal
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
*/
QAbstractTextDocumentLayout::~QAbstractTextDocumentLayout()
{
}

/*!
    \internal

    Registers the given \a component as a handler for items of the
    given \a formatType.
*/
void QAbstractTextDocumentLayout::registerHandler(int formatType, QObject *component)
{
    Q_D(QAbstractTextDocumentLayout);

    QTextObjectInterface *iface = qobject_cast<QTextObjectInterface *>(component);
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
    Q_D(const QAbstractTextDocumentLayout);

    QTextObjectHandler handler = d->handlers.value(objectType);
    if (!handler.component)
        return 0;

    return handler.iface;
}

/*!
    Sets the size of the inline object \a item in accordance with the
    text \a format.
*/
void QAbstractTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format)
{
    Q_D(QAbstractTextDocumentLayout);

    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;

    QSizeF s = handler.iface->intrinsicSize(document(), posInDocument, format);
    item.setWidth(s.width());
    item.setAscent(s.height());
    item.setDescent(0);
}

/*!
    Lays out the inline object \a item using the given text \a format.
    The base class implementation does nothing.

    \sa drawInlineObject()
*/
void QAbstractTextDocumentLayout::positionInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(item);
    Q_UNUSED(posInDocument);
    Q_UNUSED(format);
}

/*!
    \fn void QAbstractTextDocumentLayout::drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject item, const QTextFormat &format)

    Called to draw the inline object \a item on the given \a painter within
    the rectangle specified by \a rect using the text format specified by
    \a format.

    \sa draw()
*/
void QAbstractTextDocumentLayout::drawInlineObject(QPainter *p, const QRectF &rect, QTextInlineObject item,
                                                   int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(item);
    Q_D(QAbstractTextDocumentLayout);

    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;

    handler.iface->drawObject(p, rect, document(), posInDocument, format);

#if 0
    if (selType == QTextLayout::Highlight && item.engine()->pal) {
#if defined (Q_WS_WIN)
        static QPixmap tile;
        if (tile.isNull()) {
            QImage image(128, 128, 32);
            image.fill((item.engine()->pal->highlight().color().rgb() & 0x00ffffff) | 0x7f000000);
            image.setAlphaBuffer(true);
            tile = QPixmap(image);
        }
        p->drawTiledPixmap(rect, tile);
#elif defined (Q_WS_MAC)
        QColor hl = item.engine()->pal->highlight().color();
        QBrush brush(QColor(hl.red(), hl.green(), hl.blue(), 127));
        QPixmap texture = item.engine()->pal->highlight().texture();
        if(!texture.isNull())
            brush.setTexture(texture);
        p->fillRect(rect, brush);
#else
        QBrush brush(item.engine()->pal->highlight().color(), Qt::Dense4Pattern);
        p->fillRect(rect, brush);
#endif
    }
#endif
}

void QAbstractTextDocumentLayoutPrivate::handlerDestroyed(QObject *obj)
{
    HandlerHash::Iterator it = handlers.begin();
    while (it != handlers.end())
        if ((*it).component == obj)
            it = handlers.erase(it);
        else
            ++it;
}

/*!
    \internal

    Returns the index of the format at position \a pos.
*/
int QAbstractTextDocumentLayout::formatIndex(int pos)
{
    QTextDocumentPrivate *pieceTable = qobject_cast<QTextDocument *>(parent())->docHandle();
    return pieceTable->find(pos).value()->format;
}

/*!
    \fn QTextCharFormat QAbstractTextDocumentLayout::format(int position)

    Returns the character format that is applicable at the given \a position.
*/
QTextCharFormat QAbstractTextDocumentLayout::format(int pos)
{
    QTextDocumentPrivate *pieceTable = qobject_cast<QTextDocument *>(parent())->docHandle();
    int idx = pieceTable->find(pos).value()->format;
    return pieceTable->formatCollection()->charFormat(idx);
}



/*!
    Returns the text document that this layout is operating on.
*/
QTextDocument *QAbstractTextDocumentLayout::document() const
{
    return qobject_cast<QTextDocument *>(parent());
}

/*!
    \fn QString QAbstractTextDocumentLayout::anchorAt(const QPointF &position) const

    Returns the reference of the anchor at the given \a position, or an empty
    string if no anchor exists at that point.
*/
QString QAbstractTextDocumentLayout::anchorAt(const QPointF& pos) const
{
    int cursorPos = hitTest(pos, Qt::ExactHit);
    if (cursorPos == -1)
        return QString();

    QTextDocumentPrivate *pieceTable = qobject_cast<const QTextDocument *>(parent())->docHandle();
    QTextDocumentPrivate::FragmentIterator it = pieceTable->find(cursorPos);
    QTextCharFormat fmt = pieceTable->formatCollection()->charFormat(it->format);
    return fmt.anchorHref();
}

/*!
    Returns the bounding rectacle of \a frame.
    \fn QRectF QAbstractTextDocumentLayout::frameBoundingRect(QTextFrame *frame) const
    Returns the bounding rectangle of \a frame.
*/

/*!
    \fn QRectF QAbstractTextDocumentLayout::blockBoundingRect(const QTextBlock &block) const
    Returns the bounding rectangle of \a block.
*/

/*!
    Sets the paint device used for rendering the document's layout to the
    given \a device.

    \sa paintDevice()
*/
void QAbstractTextDocumentLayout::setPaintDevice(QPaintDevice *device)
{
    Q_D(QAbstractTextDocumentLayout);
    d->paintDevice = device;
}

/*!
    Returns the paint device used to render the document's layout.

    \sa setPaintDevice()
*/
QPaintDevice *QAbstractTextDocumentLayout::paintDevice() const
{
    Q_D(const QAbstractTextDocumentLayout);
    return d->paintDevice;
}

#include "moc_qabstracttextdocumentlayout.cpp"

