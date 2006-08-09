/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qgraphicssvgitem.h"

#include "qpainter.h"
#include "qstyleoption.h"
#include "qsvgrenderer.h"
#include "qdebug.h"

#include "private/qobject_p.h"

class QGraphicsSvgItemPrivate : public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(QGraphicsSvgItem)

    QGraphicsSvgItemPrivate()
        : renderer(0), shared(false), dirty(true),
          cached(true)
    { }
    
    void init()
    {
        Q_Q(QGraphicsSvgItem);
        renderer = new QSvgRenderer(q);
        QObject::connect(renderer, SIGNAL(repaintNeeded()),
                         q, SLOT(_q_repaintItem()));
    }

    void _q_repaintItem()
    {
        q_func()->update();
    }

    QSvgRenderer *renderer;
    QPixmap pixmap;
    QRectF boundingRect;
    QSize  pixmapSize;
    bool shared;
    bool dirty;
    QString elemId;
    bool cached;
};

/*!
    \class QGraphicsSvgItem
    \ingroup multimedia

    \brief The QGraphicsSvgItem class is a QGraphicsItem that can be used to render
           the contents of SVG files.
    
    \since 4.2

    QGraphicsSvgItem provides a way of rendering SVG files onto QGraphicsView.
    QGraphicsSvgItem can be created by passing the SVG file to be rendered to
    its constructor or by explicit setting a shared QSvgRenderer on it.

    Note that setting QSvgRenderer on a QGraphicsSvgItem doesn't make the item take
    ownership of the renderer, therefore if using setSharedRenderer() method one has
    to make sure that the lifetime of the QSvgRenderer object will be at least as long
    as that of the QGraphicsSvgItem.

    QGraphicsSvgItem provides a way of rendering only parts of the SVG files via
    the setElementId. If setElementId() method is called, only the SVG element
    (and its children) with the passed id will be renderer. This provides a convenient
    way of selectively rendering large SVG files that contain a number of discrete
    elements. For example the following code renders only jokers from a SVG file
    containing a whole card deck:

    \code
    QSvgRenderer *renderer = new QSvgRenderer(QLatin1String("SvgCardDeck.svg"));
    QGraphicsSvgItem *black = new QGraphicsSvgItem();
    QGraphicsSvgItem *red   = new QGraphicsSvgItem();

    black->setSharedRenderer(renderer);
    black->setElementId(QLatin1String("black_joker"));
    
    red->setSharedRenderer(renderer);
    red->setElementId(QLatin1String("black_joker"));
    \endcode

    Size of the item can be set via the setSize() method or via
    direct manipulation of the items transformation matrix.

    By default the SVG rendering is cached to speedup
    the display of items. Caching can be disabled by passing false
    to the setCachingEnabled() method.

    \sa QSvgWidget, {QtSvg Module}, QGraphicsItem, QGraphicsView
*/


/*!
    Constructs a new svg item with the given \a parent.
*/
QGraphicsSvgItem::QGraphicsSvgItem(QGraphicsItem *parentItem)
    : QObject(*new QGraphicsSvgItemPrivate(), 0), QGraphicsItem(parentItem)
{
    Q_D(QGraphicsSvgItem);
    d->init();
}

/*!
    Constructs a new item with the given \a parent and loads the contents of the
    SVG file with the specified \a filename.
*/
QGraphicsSvgItem::QGraphicsSvgItem(const QString &fileName, QGraphicsItem *parentItem)
    : QObject(*new QGraphicsSvgItemPrivate(), 0), QGraphicsItem(parentItem)
{
    Q_D(QGraphicsSvgItem);
    d->init();
    d->renderer->load(fileName);
}

/*!
    Returns the currently use QSvgRenderer.
*/
QSvgRenderer *QGraphicsSvgItem::renderer() const
{
    return d_func()->renderer;
}


/*!
    Returns the bounding rectangle of this item.
*/
QRectF QGraphicsSvgItem::boundingRect() const
{
    Q_D(const QGraphicsSvgItem);
    return d->boundingRect;
}

void QGraphicsSvgItem::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem *option,
                             QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    Q_D(QGraphicsSvgItem);
    if (!d->renderer->isValid())
        return;

    if (!d->cached) {
        if (d->elemId.isEmpty())
            d->renderer->render(painter, d->boundingRect);
        else
            d->renderer->render(painter, d->elemId, d->boundingRect);
        return;
    }
    
    if (d->dirty) {
        if (d->pixmap.isNull())
            d->pixmap = QPixmap(d->pixmapSize);
        d->pixmap.fill(Qt::transparent);
        QPainter p(&d->pixmap);

        if (d->elemId.isEmpty())
            d->renderer->render(&p);
        else
            d->renderer->render(&p, d->elemId);
        p.end();
        d->dirty = false;
    }
    if (d->pixmapSize != d->boundingRect.size().toSize()) {
        //transform the painter - we adjust the pixmapSize on
        //matrix changes so our pixmap is already of the correct
        //size. now we need to readjust the painter to not combine
        //the adjusted pixmap size with the scaling factor of the
        //matrix
        QMatrix mat = painter->worldMatrix();
        mat.scale(d->boundingRect.width()/d->pixmapSize.width(),
                  d->boundingRect.height()/d->pixmapSize.height());
        painter->setWorldMatrix(mat);
    }
                                                             
    painter->drawPixmap(0, 0, d->pixmap);
}

int QGraphicsSvgItem::type() const
{
    return Type;
}


/*!
    Sets the size of the item.
*/
void QGraphicsSvgItem::setSize(const QSize &size)
{
    Q_D(QGraphicsSvgItem);
    d->boundingRect.setSize(size);
    d->pixmapSize = size;
    if (d->cached)
        d->pixmap = QPixmap(d->pixmapSize);
    d->dirty = true;
    update();
}


/*!
    Returns the current size of the item. Method
    doesn't take the current transformation matrix
    into account and return untransformed size.
*/
QSize QGraphicsSvgItem::size() const
{
    Q_D(const QGraphicsSvgItem);
    return d->pixmapSize;
}


/*!
    Sets the XML ID of the element that this item should
    render. 
*/
void QGraphicsSvgItem::setElementId(const QString &id)
{
    Q_D(QGraphicsSvgItem);
    d->elemId = id;
    d->dirty = true;
    update();
}


/*!
    Returns the XML ID the element that is currently
    being renderer. Returns an empty string if the whole
    file is being rendered.
*/
QString QGraphicsSvgItem::elementId() const
{
    Q_D(const QGraphicsSvgItem);
    return d->elemId;
}

/*!
    Sets a shared QSvgRenderer on the item. By using this method
    one can share the same QSvgRenderer on a number of items. This
    means that the SVG file will be parsed only once.
    QSvgRenderer passed to this method has to exist for as long
    as this item is used.
*/
void QGraphicsSvgItem::setSharedRenderer(QSvgRenderer *renderer)
{
    Q_D(QGraphicsSvgItem);
    if (!d->shared)
        delete d->renderer;
    
    d->renderer = renderer;
    d->shared = true;
    d->dirty = true;
    update();
}

QVariant QGraphicsSvgItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_D(QGraphicsSvgItem);
    if (change == QGraphicsItem::ItemMatrixChange) {
        QRectF rect = qvariant_cast<QMatrix>(value).mapRect(d->boundingRect);
        //qDebug()<<"New coords are "<<rect;
        d->pixmapSize = rect.size().toSize();
        if (d->cached)
            d->pixmap = QPixmap(d->pixmapSize);
        d->dirty = true;
        return value;
    }
    return QGraphicsItem::itemChange(change, value);
}


/*!
    Sets caching strategy on the item. For performance
    reasons it is advised to always keep the caching on.
*/
void QGraphicsSvgItem::setCachingEnabled(bool b)
{
    Q_D(QGraphicsSvgItem);
    if (b) {
        d->pixmap = QPixmap(d->pixmapSize);
    }
    d->cached = b;
    d->dirty = true;
    update();
}

/*!
   Returns true if the contents of the SVG file to be
   renderer is cached.
*/
bool QGraphicsSvgItem::isCachingEnabled() const
{
    Q_D(const QGraphicsSvgItem);
    return d->cached;
}

/*!
  If the item is caching the contents of the SVG file
  to be renderer then this method returns its contents
  in the pixmap of the size of this item. The size of the
  pixmap is adjusted by the current transformation matrix
  of this item.
  If the item isn't cached the method returns a null pixmap.
 */
QPixmap QGraphicsSvgItem::cache() const
{
    Q_D(const QGraphicsSvgItem);
    if (d->cached)
        return d->pixmap;
    else
        return QPixmap();
    
}


#include "moc_qgraphicssvgitem.cpp"
