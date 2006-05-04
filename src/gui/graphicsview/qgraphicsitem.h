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

#ifndef QGRAPHICSITEM_H
#define QGRAPHICSITEM_H

#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>
#include <QtCore/qrect.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qpixmap.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QBrush;
class QFocusEvent;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneEvent;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsScene;
class QInputMethodEvent;
class QKeyEvent;
class QMatrix;
class QMenu;
class QPainter;
class QPen;
class QPointF;
class QRectF;
class QStyleOptionGraphicsItem;

class QGraphicsItemPrivate;
class Q_GUI_EXPORT QGraphicsItem
{
public:
    enum GraphicsItemFlag {
        ItemIsMovable = 0x1,
        ItemIsSelectable = 0x2,
        ItemIsFocusable = 0x4
    };
    Q_DECLARE_FLAGS(GraphicsItemFlags, GraphicsItemFlag)

    QGraphicsItem(QGraphicsItem *parent = 0);
    virtual ~QGraphicsItem();

    QGraphicsScene *scene() const;

    QGraphicsItem *parentItem() const;
    void setParentItem(QGraphicsItem *parent);
    QList<QGraphicsItem *> children() const;

    GraphicsItemFlags flags() const;
    void setFlag(GraphicsItemFlag flag, bool enabled = true);
    void setFlags(GraphicsItemFlags flags);

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    bool isVisible() const;
    void setVisible(bool visible);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isSelected() const;
    void setSelected(bool selected);

    bool acceptsMouseEvents() const;
    void setAcceptsMouseEvents(bool enabled);

    bool acceptsHoverEvents() const;
    void setAcceptsHoverEvents(bool enabled);

    bool hasFocus() const;
    void setFocus(Qt::FocusReason focusReason = Qt::OtherFocusReason);
    void clearFocus();

    // Positioning in scene coordinates
    QPointF pos() const;
    QPointF scenePos() const;
    void setPos(const QPointF &pos);
    inline void setPos(qreal x, qreal y) { setPos(QPointF(x, y)); }
    inline void moveBy(qreal dx, qreal dy) { setPos(pos().x() + dx, pos().y() + dy); }

    // Local transformation
    QMatrix matrix() const;
    QMatrix sceneMatrix() const;
    void setMatrix(const QMatrix &matrix, bool combine = false);
    void resetMatrix();
    void rotate(qreal angle);
    void scale(qreal sx, qreal sy);
    void shear(qreal sh, qreal sv);
    void translate(qreal dx, qreal dy);

    // Stacking order
    qreal zValue() const;
    void setZValue(qreal z);

    // Hit test
    virtual QRectF boundingRect() const = 0;
    QRectF sceneBoundingRect() const;
    virtual QPainterPath shape() const;
    virtual bool contains(const QPointF &point) const;
    virtual bool collidesWith(QGraphicsItem *other) const;
    virtual bool collidesWith(const QPainterPath &path) const;

    // Drawing
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) = 0;
    void update(const QRectF &rect = QRectF());
    inline void update(qreal x, qreal y, qreal width, qreal height)
    { update(QRectF(x, y, width, height)); }

    // Coordinate mapping
    QPointF mapToItem(QGraphicsItem *item, const QPointF &point) const;
    QPointF mapToParent(const QPointF &point) const;
    QPointF mapToScene(const QPointF &point) const;
    QPointF mapFromItem(QGraphicsItem *item, const QPointF &point) const;
    QPointF mapFromParent(const QPointF &point) const;
    QPointF mapFromScene(const QPointF &point) const;
    inline QPointF mapToItem(QGraphicsItem *item, qreal x, qreal y) const
    { return mapToItem(item, QPointF(x, y)); }
    inline QPointF mapToParent(qreal x, qreal y) const
    { return mapToParent(QPointF(x, y)); }
    inline QPointF mapToScene(qreal x, qreal y) const
    { return mapToScene(QPointF(x, y));  }
    inline QPointF mapFromItem(QGraphicsItem *item, qreal x, qreal y) const
    { return mapFromItem(item, QPointF(x, y)); }
    inline QPointF mapFromParent(qreal x, qreal y) const
    { return mapFromParent(QPointF(x, y));  }
    inline QPointF mapFromScene(qreal x, qreal y) const
    { return mapFromScene(QPointF(x, y));  }

    bool isAncestorOf(const QGraphicsItem *child) const;

    // Custom data
    QVariant data(int key) const;
    void setData(int key, const QVariant &value);

    enum {
        Type = 1,
        UserType = 0x80000000
    };
    virtual int type() const;

    void installEventFilter(QGraphicsItem *filterItem);
    void removeEventFilter(QGraphicsItem *filterItem);

protected:
    virtual bool sceneEventFilter(QGraphicsItem *watched, QGraphicsSceneEvent *event);
    virtual void sceneEvent(QEvent *event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void focusEvent(QFocusEvent *event);
    virtual void hoverEvent(QGraphicsSceneHoverEvent *event);
    virtual void keyEvent(QKeyEvent *event);
    virtual void mouseEvent(QGraphicsSceneMouseEvent *event);
    virtual void inputMethodEvent(QInputMethodEvent *event);
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    enum Extension {
        UserExtension = 0x80000000
    };
    virtual bool supportsExtension(Extension extension) const;
    virtual void setExtension(Extension extension, const QVariant &variant);
    virtual QVariant extension(const QVariant &variant) const;

protected:
    QGraphicsItem(QGraphicsItemPrivate &dd, QGraphicsItem *parent);
    QGraphicsItemPrivate *d_ptr;

private:
    void addToIndex();
    void removeFromIndex();

    Q_DISABLE_COPY(QGraphicsItem)
    Q_DECLARE_PRIVATE(QGraphicsItem)
    friend class QGraphicsScene;
    friend class QGraphicsScenePrivate;
    friend class QGraphicsView;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGraphicsItem::GraphicsItemFlags)

class QAbstractGraphicsPathItemPrivate;
class Q_GUI_EXPORT QAbstractGraphicsPathItem : public QGraphicsItem
{
public:
    QAbstractGraphicsPathItem(QGraphicsItem *parent = 0);
    ~QAbstractGraphicsPathItem();

    QPen pen() const;
    void setPen(const QPen &pen);

    QBrush brush() const;
    void setBrush(const QBrush &brush);

protected:
    QAbstractGraphicsPathItem(QAbstractGraphicsPathItemPrivate &dd, QGraphicsItem *parent);

private:
    Q_DISABLE_COPY(QAbstractGraphicsPathItem)
    Q_DECLARE_PRIVATE(QAbstractGraphicsPathItem)
};

class QGraphicsPathItemPrivate;
class Q_GUI_EXPORT QGraphicsPathItem : public QAbstractGraphicsPathItem
{
public:
    QGraphicsPathItem(const QPainterPath &path = QPainterPath(), QGraphicsItem *parent = 0);
    ~QGraphicsPathItem();

    QPainterPath path() const;
    void setPath(const QPainterPath &path);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 2 };
    int type() const;

protected:
    bool supportsExtension(Extension extension) const;
    void setExtension(Extension extension, const QVariant &variant);
    QVariant extension(const QVariant &variant) const;

private:
    Q_DISABLE_COPY(QGraphicsPathItem)
    Q_DECLARE_PRIVATE(QGraphicsPathItem)
};

class QGraphicsRectItemPrivate;
class Q_GUI_EXPORT QGraphicsRectItem : public QAbstractGraphicsPathItem
{
public:
    QGraphicsRectItem(const QRectF &rect = QRect(), QGraphicsItem *parent = 0);
    ~QGraphicsRectItem();

    QRectF rect() const;
    void setRect(const QRectF &rect);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 3 };
    int type() const;

protected:
    bool supportsExtension(Extension extension) const;
    void setExtension(Extension extension, const QVariant &variant);
    QVariant extension(const QVariant &variant) const;

private:
    Q_DISABLE_COPY(QGraphicsRectItem)
    Q_DECLARE_PRIVATE(QGraphicsRectItem)
};

class QGraphicsEllipseItemPrivate;
class Q_GUI_EXPORT QGraphicsEllipseItem : public QAbstractGraphicsPathItem
{
public:
    QGraphicsEllipseItem(const QRectF &rect = QRect(), QGraphicsItem *parent = 0);
    ~QGraphicsEllipseItem();

    QRectF rect() const;
    void setRect(const QRectF &rect);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 4 };
    int type() const;

protected:
    bool supportsExtension(Extension extension) const;
    void setExtension(Extension extension, const QVariant &variant);
    QVariant extension(const QVariant &variant) const;

private:
    Q_DISABLE_COPY(QGraphicsEllipseItem)
    Q_DECLARE_PRIVATE(QGraphicsEllipseItem)
};

class QGraphicsPolygonItemPrivate;
class Q_GUI_EXPORT QGraphicsPolygonItem : public QAbstractGraphicsPathItem
{
public:
    QGraphicsPolygonItem(const QPolygonF &polygon = QPolygonF(), QGraphicsItem *parent = 0);
    ~QGraphicsPolygonItem();

    QPolygonF polygon() const;
    void setPolygon(const QPolygonF &polygon);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 5 };
    int type() const;

protected:
    bool supportsExtension(Extension extension) const;
    void setExtension(Extension extension, const QVariant &variant);
    QVariant extension(const QVariant &variant) const;

private:
    Q_DISABLE_COPY(QGraphicsPolygonItem)
    Q_DECLARE_PRIVATE(QGraphicsPolygonItem)
};

class QGraphicsLineItemPrivate;
class Q_GUI_EXPORT QGraphicsLineItem : public QGraphicsItem
{
public:
    QGraphicsLineItem(const QLineF &line = QLineF(), QGraphicsItem *parent = 0);
    ~QGraphicsLineItem();

    QPen pen() const;
    void setPen(const QPen &pen);

    QLineF line() const;
    void setLine(const QLineF &line);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 6 };
    int type() const;

protected:
    bool supportsExtension(Extension extension) const;
    void setExtension(Extension extension, const QVariant &variant);
    QVariant extension(const QVariant &variant) const;

private:
    Q_DISABLE_COPY(QGraphicsLineItem)
    Q_DECLARE_PRIVATE(QGraphicsLineItem)
};

class QGraphicsPixmapItemPrivate;
class Q_GUI_EXPORT QGraphicsPixmapItem : public QGraphicsItem
{
public:
    QGraphicsPixmapItem(const QPixmap &pixmap = QPixmap(), QGraphicsItem *parent = 0);
    ~QGraphicsPixmapItem();

    QPixmap pixmap() const;
    void setPixmap(const QPixmap &pixmap);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    enum { Type = 7 };
    int type() const;

protected:
    bool supportsExtension(Extension extension) const;
    void setExtension(Extension extension, const QVariant &variant);
    QVariant extension(const QVariant &variant) const;

private:
    Q_DISABLE_COPY(QGraphicsPixmapItem)
    Q_DECLARE_PRIVATE(QGraphicsPixmapItem)
};

class QGraphicsTextItemPrivate;
class Q_GUI_EXPORT QGraphicsTextItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    QGraphicsTextItem(const QString &text = QString(), QGraphicsItem *parent = 0);
    ~QGraphicsTextItem();

    QString text() const;
    void setText(const QString &text);

    QFont font() const;
    void setFont(const QFont &font);

    QPen pen() const;
    void setPen(const QPen &pen);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    enum { Type = 8 };
    int type() const;

protected:
    void mouseEvent(QGraphicsSceneMouseEvent *event);
    void keyEvent(QKeyEvent *event);
    void focusEvent(QFocusEvent *event);

    bool supportsExtension(Extension extension) const;
    void setExtension(Extension extension, const QVariant &variant);
    QVariant extension(const QVariant &variant) const;

private Q_SLOTS:
    void viewportUpdate(const QRectF &rect);
    void textChanged();

private:
    Q_DISABLE_COPY(QGraphicsTextItem)
    QGraphicsTextItemPrivate *dd;
    friend class QGraphicsTextItemPrivate;
};

template <class T> inline T qgraphicsitem_cast(QGraphicsItem *item)
{
    return int(static_cast<T>(0)->Type) == int(QGraphicsItem::Type)
	|| int(static_cast<T>(0)->Type) == item->type() ? static_cast<T>(item) : 0;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug debug, QGraphicsItem *item);
#endif

QT_END_HEADER

#endif
