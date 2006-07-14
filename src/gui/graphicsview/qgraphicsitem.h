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

#ifndef QT_NO_GRAPHICSVIEW

class QBrush;
class QCursor;
class QFocusEvent;
class QGraphicsItemGroup;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneEvent;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;
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

    enum GraphicsItemChange {
        ItemPositionChange,
        ItemMatrixChange,
        ItemVisibleChange,
        ItemEnabledChange,
        ItemSelectedChange,
        ItemParentChange,
        ItemChildAddedChange,
        ItemChildRemovedChange
    };

    QGraphicsItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    virtual ~QGraphicsItem();

    QGraphicsScene *scene() const;

    QGraphicsItem *parentItem() const;
    QGraphicsItem *topLevelItem() const;
    void setParentItem(QGraphicsItem *parent);
    QList<QGraphicsItem *> children() const;

    QGraphicsItemGroup *group() const;
    void setGroup(QGraphicsItemGroup *group);

    GraphicsItemFlags flags() const;
    void setFlag(GraphicsItemFlag flag, bool enabled = true);
    void setFlags(GraphicsItemFlags flags);

#ifndef QT_NO_TOOLTIP
    QString toolTip() const;
    void setToolTip(const QString &toolTip);
#endif

#ifndef QT_NO_CURSOR
    QCursor cursor() const;
    void setCursor(const QCursor &cursor);
    bool hasCursor() const;
    void unsetCursor();
#endif

    bool isVisible() const;
    void setVisible(bool visible);
    inline void hide() { setVisible(false); }
    inline void show() { setVisible(true); }

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isSelected() const;
    void setSelected(bool selected);

    bool acceptDrops() const;
    void setAcceptDrops(bool on);

    Qt::MouseButtons acceptedMouseButtons() const;
    void setAcceptedMouseButtons(Qt::MouseButtons buttons);

    bool acceptsHoverEvents() const;
    void setAcceptsHoverEvents(bool enabled);

    bool handlesChildEvents() const;
    void setHandlesChildEvents(bool enabled);

    bool hasFocus() const;
    void setFocus(Qt::FocusReason focusReason = Qt::OtherFocusReason);
    void clearFocus();

    // Positioning in scene coordinates
    QPointF pos() const;
    inline qreal x() const { return pos().x(); }
    inline qreal y() const { return pos().y(); }
    QPointF scenePos() const;
    void setPos(const QPointF &pos);
    inline void setPos(qreal x, qreal y) { setPos(QPointF(x, y)); }
    inline void moveBy(qreal dx, qreal dy) { setPos(pos().x() + dx, pos().y() + dy); }

    void ensureVisible(const QRectF &rect = QRectF(), int xmargin = 50, int ymargin = 50);
    inline void ensureVisible(qreal x, qreal y, qreal w, qreal h, int xmargin = 50, int ymargin = 50)
    { ensureVisible(QRectF(x, y, w, h), xmargin, ymargin); }

    // Local transformation
    QMatrix matrix() const;
    QMatrix sceneMatrix() const;
    void setMatrix(const QMatrix &matrix, bool combine = false);
    void resetMatrix();
    void rotate(qreal angle);
    void scale(qreal sx, qreal sy);
    void shear(qreal sh, qreal sv);
    void translate(qreal dx, qreal dy);
    virtual void advance(int phase);

    // Stacking order
    qreal zValue() const;
    void setZValue(qreal z);

    // Hit test
    virtual QRectF boundingRect() const = 0;
    QRectF childrenBoundingRect() const;
    QRectF sceneBoundingRect() const;
    virtual QPainterPath shape() const;
    virtual bool contains(const QPointF &point) const;
    virtual bool collidesWithItem(QGraphicsItem *other) const;
    virtual bool collidesWithPath(const QPainterPath &path) const;
    QList<QGraphicsItem *> collidingItems() const;

    // Drawing
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) = 0;
    void update(const QRectF &rect = QRectF());
    inline void update(qreal x, qreal y, qreal width, qreal height)
    { update(QRectF(x, y, width, height)); }

    // Coordinate mapping
    QPointF mapToItem(QGraphicsItem *item, const QPointF &point) const;
    QPointF mapToParent(const QPointF &point) const;
    QPointF mapToScene(const QPointF &point) const;
    QPolygonF mapToItem(QGraphicsItem *item, const QRectF &rect) const;
    QPolygonF mapToParent(const QRectF &rect) const;
    QPolygonF mapToScene(const QRectF &rect) const;
    QPolygonF mapToItem(QGraphicsItem *item, const QPolygonF &polygon) const;
    QPolygonF mapToParent(const QPolygonF &polygon) const;
    QPolygonF mapToScene(const QPolygonF &polygon) const;
    QPainterPath mapToItem(QGraphicsItem *item, const QPainterPath &path) const;
    QPainterPath mapToParent(const QPainterPath &path) const;
    QPainterPath mapToScene(const QPainterPath &path) const;
    QPointF mapFromItem(QGraphicsItem *item, const QPointF &point) const;
    QPointF mapFromParent(const QPointF &point) const;
    QPointF mapFromScene(const QPointF &point) const;
    QPolygonF mapFromItem(QGraphicsItem *item, const QRectF &rect) const;
    QPolygonF mapFromParent(const QRectF &rect) const;
    QPolygonF mapFromScene(const QRectF &rect) const;
    QPolygonF mapFromItem(QGraphicsItem *item, const QPolygonF &polygon) const;
    QPolygonF mapFromParent(const QPolygonF &polygon) const;
    QPolygonF mapFromScene(const QPolygonF &polygon) const;
    QPainterPath mapFromItem(QGraphicsItem *item, const QPainterPath &path) const;
    QPainterPath mapFromParent(const QPainterPath &path) const;
    QPainterPath mapFromScene(const QPainterPath &path) const;

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
        UserType = 65536
    };
    virtual int type() const;

    void installSceneEventFilter(QGraphicsItem *filterItem);
    void removeSceneEventFilter(QGraphicsItem *filterItem);

protected:
    virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
    virtual bool sceneEvent(QEvent *event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dropEvent(QGraphicsSceneDragDropEvent *event);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
    virtual void inputMethodEvent(QInputMethodEvent *event);
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    enum Extension {
        UserExtension = 0x80000000
    };
    virtual bool supportsExtension(Extension extension) const;
    virtual void setExtension(Extension extension, const QVariant &variant);
    virtual QVariant extension(const QVariant &variant) const;

protected:
    QGraphicsItem(QGraphicsItemPrivate &dd,
                  QGraphicsItem *parent, QGraphicsScene *scene);
    QGraphicsItemPrivate *d_ptr;

    void addToIndex();
    void removeFromIndex();
    void prepareGeometryChange();

private:
    Q_DISABLE_COPY(QGraphicsItem)
    Q_DECLARE_PRIVATE(QGraphicsItem)
    friend class QGraphicsItemGroup;
    friend class QGraphicsScene;
    friend class QGraphicsScenePrivate;
    friend class QGraphicsSceneFindItemBspTreeVisitor;
    friend class QGraphicsView;
    friend bool qt_closestLeaf(const QGraphicsItem *, const QGraphicsItem *);
    friend bool qt_closestItemFirst(const QGraphicsItem *, const QGraphicsItem *);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGraphicsItem::GraphicsItemFlags)

class QAbstractGraphicsShapeItemPrivate;
class Q_GUI_EXPORT QAbstractGraphicsShapeItem : public QGraphicsItem
{
public:
    QAbstractGraphicsShapeItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~QAbstractGraphicsShapeItem();

    QPen pen() const;
    void setPen(const QPen &pen);

    QBrush brush() const;
    void setBrush(const QBrush &brush);

protected:
    QAbstractGraphicsShapeItem(QAbstractGraphicsShapeItemPrivate &dd,
                               QGraphicsItem *parent, QGraphicsScene *scene);

private:
    Q_DISABLE_COPY(QAbstractGraphicsShapeItem)
    Q_DECLARE_PRIVATE(QAbstractGraphicsShapeItem)
};

class QGraphicsPathItemPrivate;
class Q_GUI_EXPORT QGraphicsPathItem : public QAbstractGraphicsShapeItem
{
public:
    QGraphicsPathItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QGraphicsPathItem(const QPainterPath &path, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
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
class Q_GUI_EXPORT QGraphicsRectItem : public QAbstractGraphicsShapeItem
{
public:
    QGraphicsRectItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QGraphicsRectItem(const QRectF &rect, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QGraphicsRectItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~QGraphicsRectItem();

    QRectF rect() const;
    void setRect(const QRectF &rect);
    inline void setRect(qreal x, qreal y, qreal w, qreal h)
    { setRect(QRectF(x, y, w, h)); }

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
class Q_GUI_EXPORT QGraphicsEllipseItem : public QAbstractGraphicsShapeItem
{
public:
    QGraphicsEllipseItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QGraphicsEllipseItem(const QRectF &rect, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QGraphicsEllipseItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~QGraphicsEllipseItem();

    QRectF rect() const;
    void setRect(const QRectF &rect);
    inline void setRect(qreal x, qreal y, qreal w, qreal h)
    { setRect(QRectF(x, y, w, h)); }

    int startAngle() const;
    void setStartAngle(int angle);

    int spanAngle() const;
    void setSpanAngle(int angle);

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
class Q_GUI_EXPORT QGraphicsPolygonItem : public QAbstractGraphicsShapeItem
{
public:
    QGraphicsPolygonItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QGraphicsPolygonItem(const QPolygonF &polygon,
                         QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~QGraphicsPolygonItem();

    QPolygonF polygon() const;
    void setPolygon(const QPolygonF &polygon);

    Qt::FillRule fillRule() const;
    void setFillRule(Qt::FillRule rule);

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
    QGraphicsLineItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QGraphicsLineItem(const QLineF &line, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QGraphicsLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~QGraphicsLineItem();

    QPen pen() const;
    void setPen(const QPen &pen);

    QLineF line() const;
    void setLine(const QLineF &line);
    inline void setLine(qreal x1, qreal y1, qreal x2, qreal y2)
    { setLine(QLineF(x1, y1, x2, y2)); }

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
    QGraphicsPixmapItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QGraphicsPixmapItem(const QPixmap &pixmap, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~QGraphicsPixmapItem();

    QPixmap pixmap() const;
    void setPixmap(const QPixmap &pixmap);

    Qt::TransformationMode transformationMode() const;
    void setTransformationMode(Qt::TransformationMode mode);

    QPointF offset() const;
    void setOffset(const QPointF &offset);

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
class QTextDocument;
class QTextCursor;
class Q_GUI_EXPORT QGraphicsTextItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    QDOC_PROPERTY(bool openExternalLinks READ openExternalLinks() WRITE setOpenExternalLinks)
    QDOC_PROPERTY(QTextCursor textCursor READ textCursor WRITE setTextCursor)

public:
    QGraphicsTextItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QGraphicsTextItem(const QString &text, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~QGraphicsTextItem();

    QString toHtml() const;
    void setHtml(const QString &html);

    QString toPlainText() const;
    void setPlainText(const QString &text);

    QFont font() const;
    void setFont(const QFont &font);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    enum { Type = 8 };
    int type() const;

    void setTextWidth(qreal width);
    qreal textWidth() const;

    void adjustSize();

    void setDocument(QTextDocument *document);
    QTextDocument *document() const;

    void setTextInteractionFlags(Qt::TextInteractionFlags flags);
    Qt::TextInteractionFlags textInteractionFlags() const;

    void setOpenExternalLinks(bool open);
    bool openExternalLinks() const;

    void setTextCursor(const QTextCursor &cursor);
    QTextCursor textCursor() const;

Q_SIGNALS:
    void linkActivated(const QString &);
    void linkHovered(const QString &);

protected:
    bool sceneEvent(QEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void inputMethodEvent(QInputMethodEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    bool supportsExtension(Extension extension) const;
    void setExtension(Extension extension, const QVariant &variant);
    QVariant extension(const QVariant &variant) const;

private:
    Q_DISABLE_COPY(QGraphicsTextItem)
    Q_PRIVATE_SLOT(dd, void _q_updateBoundingRect(const QSizeF &))
    Q_PRIVATE_SLOT(dd, void _q_update(QRectF))
    Q_PRIVATE_SLOT(dd, void _q_ensureVisible(QRectF))
    QGraphicsTextItemPrivate *dd;
    friend class QGraphicsTextItemPrivate;
};

class QGraphicsSimpleTextItemPrivate;
class Q_GUI_EXPORT QGraphicsSimpleTextItem : public QGraphicsItem
{
public:
    QGraphicsSimpleTextItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QGraphicsSimpleTextItem(const QString &text, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~QGraphicsSimpleTextItem();

    void setText(const QString &text);
    QString text() const;

    void setFont(const QFont &font);
    QFont font() const;

    void setPen(const QPen &pen);
    QPen pen() const;

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    enum { Type = 9 };
    int type() const;

protected:
    bool supportsExtension(Extension extension) const;
    void setExtension(Extension extension, const QVariant &variant);
    QVariant extension(const QVariant &variant) const;

private:
    Q_DISABLE_COPY(QGraphicsSimpleTextItem)
    Q_DECLARE_PRIVATE(QGraphicsSimpleTextItem)
};

class QGraphicsItemGroupPrivate;
class Q_GUI_EXPORT QGraphicsItemGroup : public QGraphicsItem
{
public:
    QGraphicsItemGroup(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~QGraphicsItemGroup();

    void addToGroup(QGraphicsItem *item);
    void removeFromGroup(QGraphicsItem *item);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 10 };
    int type() const;

private:
    Q_DISABLE_COPY(QGraphicsItemGroup)
    Q_DECLARE_PRIVATE(QGraphicsItemGroup)
};

template <class T> inline T qgraphicsitem_cast(QGraphicsItem *item)
{
    return int(static_cast<T>(0)->Type) == int(QGraphicsItem::Type)
        || int(static_cast<T>(0)->Type) == item->type() ? static_cast<T>(item) : 0;
}

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug debug, QGraphicsItem *item);
#endif

QT_END_HEADER

#endif

#endif // QT_NO_GRAPHICSVIEW
