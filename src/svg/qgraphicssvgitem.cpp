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

#ifndef QT_NO_GRAPHICSVIEW

#include "qpainter.h"
#include "qstyleoption.h"
#include "qsvgrenderer.h"
#include "qpixmapcache.h"
#include "qdebug.h"

#include "private/qobject_p.h"

class QGraphicsSvgItemPrivate : public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(QGraphicsSvgItem)

    QGraphicsSvgItemPrivate()
        : renderer(0), maximumCacheSize(1024, 768), shared(false), 
          dirty(true), cached(true)
    {
    }

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

    inline void updateDefaultSize()
    {
        QRectF bounds;
        if (elemId.isEmpty()) {
            bounds = QRectF(QPointF(0, 0), renderer->defaultSize());
        } else {
            bounds = renderer->boundsOnElement(elemId);
        }
        boundingRect.setSize(bounds.size());
    }

    QSvgRenderer *renderer;
    QRectF boundingRect;
    QSize  maximumCacheSize;
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
    Constructs a new SVG item with the given \a parent.
*/
QGraphicsSvgItem::QGraphicsSvgItem(QGraphicsItem *parent)
    : QObject(*new QGraphicsSvgItemPrivate(), 0), QGraphicsItem(parent)
{
    Q_D(QGraphicsSvgItem);
    d->init();
}

/*!
    Constructs a new item with the given \a parent and loads the contents of the
    SVG file with the specified \a fileName.
*/
QGraphicsSvgItem::QGraphicsSvgItem(const QString &fileName, QGraphicsItem *parent)
    : QObject(*new QGraphicsSvgItemPrivate(), 0), QGraphicsItem(parent)
{
    Q_D(QGraphicsSvgItem);
    d->init();
    d->renderer->load(fileName);
    d->updateDefaultSize();
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

/*!
    \reimp
*/
void QGraphicsSvgItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                             QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    Q_D(QGraphicsSvgItem);
    if (!d->renderer->isValid())
        return;

    QMatrix m = painter->worldMatrix();
    QRect deviceRect = m.mapRect(d->boundingRect).toRect();
    
    if (!d->cached ||
        deviceRect.size().width() > d->maximumCacheSize.width() || 
        deviceRect.size().height() > d->maximumCacheSize.height()) {
        if (d->elemId.isEmpty())
            d->renderer->render(painter, d->boundingRect);
        else
            d->renderer->render(painter, d->elemId, d->boundingRect);
        return;
    }

    QString uniqueId = QString::fromLatin1("%1_%2_%3_%4_%5").arg((long)this)
                       .arg(m.m11()).arg(m.m12()).arg(m.m21()).arg(m.m22());

    QPixmap pix;
    if (!QPixmapCache::find(uniqueId, pix)) {
        pix = QPixmap(deviceRect.size());
        d->dirty = true;
#if 0
        qDebug()<<"Cache doesn't contain item "<<uniqueId
                <<", size = "<<QPixmapCache::cacheLimit();
#endif
    }

    QPointF viewPoint = m.mapRect(d->boundingRect).topLeft();
    QPointF viewOrigo = m.map(QPointF(0,  0));

    if (d->dirty) {
        pix.fill(Qt::transparent);
        QPainter p(&pix);

        QPointF offset = viewOrigo - viewPoint;
        p.translate(offset);
        p.setWorldMatrix(m, true);
        p.translate(m.inverted().map(QPointF(0, 0)));

        if (d->elemId.isEmpty())
            d->renderer->render(&p, d->boundingRect);
        else
            d->renderer->render(&p, d->elemId, d->boundingRect);

        p.end();
        QPixmapCache::insert(uniqueId,  pix);
        d->dirty = false;
    }

    painter->setWorldMatrix(QMatrix());
    painter->drawPixmap(viewPoint, pix);
}

/*!
    \reimp
*/
int QGraphicsSvgItem::type() const
{
    return Type;
}


/*!
    Sets the maximum cache size of the item to \a size.

    This function doesn't take the current transformation matrix into
    account and sets the untransformed size.
    The cache correspods to the QPixmap which is used to cache the
    results of the rendering.
    Use QPixmap::setCacheLimit() to set limitations on the whole cache
    and use setMaximumCacheSize when setting cache size for individual
    items.

*/
void QGraphicsSvgItem::setMaximumCacheSize(const QSize &size)
{
    Q_D(QGraphicsSvgItem);

    if (size.isEmpty()) {
        qWarning("Can't set the size of a QGraphicsSvgItem cache to an empty rectangle");
        return;
    }

    d->maximumCacheSize = size;
    d->dirty = true;
    update();
}

/*!
    Returns the current maximum size of the cache for this item.

    This function doesn't take the current transformation matrix into
    account and returns the untransformed size.
    The default maximum cache size is 1024x768.
    QPixmapCache::cacheLimit() sets the
    cumulative bounds on the whole cache, maximumCacheSize refers
    to a maximum cache size for this particular item.
*/
QSize QGraphicsSvgItem::maximumCacheSize() const
{
    Q_D(const QGraphicsSvgItem);
    return d->maximumCacheSize;
}

/*!
    Sets the XML ID of the element that this item should render to \a
    id.
*/
void QGraphicsSvgItem::setElementId(const QString &id)
{
    Q_D(QGraphicsSvgItem);
    d->elemId = id;
    d->dirty = true;
    d->updateDefaultSize();
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
    Sets \a renderer to be a shared QSvgRenderer on the item. By
    using this method one can share the same QSvgRenderer on a number
    of items. This means that the SVG file will be parsed only once.
    QSvgRenderer passed to this method has to exist for as long as
    this item is used.
*/
void QGraphicsSvgItem::setSharedRenderer(QSvgRenderer *renderer)
{
    Q_D(QGraphicsSvgItem);
    if (!d->shared)
        delete d->renderer;

    d->renderer = renderer;
    d->shared = true;
    d->dirty = true;

    d->updateDefaultSize();

    update();
}

/*!
    If \a caching is true, enables caching on the item; otherwise
    disables it.

    By default, caching is on. For performance reasons, it is advised
    to keep the caching enabled.
    Note that caching will not work if either the amount of cached
    items exceeded QPixmapCache::cacheLimit() or if the current
    item on the given view is greater than the
    QGraphicsSvgItem::maximumCacheSize().
*/
void QGraphicsSvgItem::setCachingEnabled(bool caching)
{
    Q_D(QGraphicsSvgItem);

    d->cached = caching;
    d->dirty = true;
    update();
}

/*!
    Returns true if the contents of the SVG file to be
    renderer is cached.

    Note that caching will not work if either the amount of cached
    items exceeded QPixmapCache::cacheLimit() or if the current
    item on the given view is greater than the
    QGraphicsSvgItem::maximumCacheSize().
*/
bool QGraphicsSvgItem::isCachingEnabled() const
{
    Q_D(const QGraphicsSvgItem);
    return d->cached;
}


#include "moc_qgraphicssvgitem.cpp"

#endif // QT_NO_GRAPHICSVIEW
