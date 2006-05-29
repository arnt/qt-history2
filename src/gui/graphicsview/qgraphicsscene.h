/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGRAPHICSSCENE_H
#define QGRAPHICSSCENE_H

#include <QtCore/qobject.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtGui/qbrush.h>
#include <QtGui/qfont.h>
#include <QtGui/qmatrix.h>
#include <QtGui/qpen.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_GRAPHICSVIEW

template<typename T> class QList;
class QFocusEvent;
class QKeyEvent;
class QGraphicsItem;
class QGraphicsItemGroup;
class QGraphicsEllipseItem;
class QGraphicsLineItem;
class QGraphicsPathItem;
class QGraphicsPixmapItem;
class QGraphicsPolygonItem;
class QGraphicsRectItem;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneEvent;
class QGraphicsSceneHelpEvent;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsTextItem;
class QHelpEvent;
class QLineF;
class QPainterPath;
class QPixmap;
class QPointF;
class QPolygonF;
class QRectF;
class QSizeF;

class QGraphicsScenePrivate;
class Q_GUI_EXPORT QGraphicsScene : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ItemIndexMethod itemIndexMethod READ itemIndexMethod WRITE setItemIndexMethod)
    Q_PROPERTY(QRectF sceneRect READ sceneRect WRITE setSceneRect)
public:
    enum ItemIndexMethod {
        BspTreeIndex,
        NoIndex = -1
    };

    QGraphicsScene(QObject *parent = 0);
    virtual ~QGraphicsScene();

    QRectF sceneRect() const;
    void setSceneRect(const QRectF &rect);
    inline void setSceneRect(qreal x, qreal y, qreal w, qreal h)
    { setSceneRect(QRectF(x, y, w, h)); }

    void drawScene(QPainter *painter, const QRectF &target = QRectF(),
                   const QRectF &source = QRectF(),
                   Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio,
                   const QMatrix &matrix = QMatrix());

    ItemIndexMethod itemIndexMethod() const;
    void setItemIndexMethod(ItemIndexMethod method);

    QRectF itemsBoundingRect() const;

    QList<QGraphicsItem *> items() const;
    QList<QGraphicsItem *> items(const QPointF &pos) const;
    QList<QGraphicsItem *> items(const QRectF &rect) const;
    QList<QGraphicsItem *> items(const QPolygonF &polygon) const;
    QList<QGraphicsItem *> items(const QPainterPath &path) const;
    QList<QGraphicsItem *> collidingItems(QGraphicsItem *item) const;
    QGraphicsItem *itemAt(const QPointF &pos) const;
    inline QGraphicsItem *itemAt(qreal x, qreal y) const
    { return itemAt(QPointF(x, y)); }

    QList<QGraphicsItem *> selectedItems() const;
    void setSelectionArea(const QPainterPath &path);
    void clearSelection();

    QGraphicsItemGroup *createItemGroup(const QList<QGraphicsItem *> &items);
    void destroyItemGroup(QGraphicsItemGroup *group);

    void addItem(QGraphicsItem *item);
    QGraphicsEllipseItem *addEllipse(const QRectF &rect, const QPen &pen = QPen(), const QBrush &brush = QBrush());
    QGraphicsLineItem *addLine(const QLineF &line, const QPen &pen = QPen());
    QGraphicsPathItem *addPath(const QPainterPath &path, const QPen &pen = QPen(), const QBrush &brush = QBrush());
    QGraphicsPixmapItem *addPixmap(const QPixmap &pixmap);
    QGraphicsPolygonItem *addPolygon(const QPolygonF &polygon, const QPen &pen = QPen(), const QBrush &brush = QBrush());
    QGraphicsRectItem *addRect(const QRectF &rect, const QPen &pen = QPen(), const QBrush &brush = QBrush());
    QGraphicsTextItem *addText(const QString &text, const QFont &font = QFont());
    void removeItem(QGraphicsItem *item);

    QGraphicsItem *focusItem() const;
    void setFocusItem(QGraphicsItem *item, Qt::FocusReason focusReason = Qt::OtherFocusReason);
    bool hasFocus() const;
    void setFocus(Qt::FocusReason focusReason = Qt::OtherFocusReason);
    void clearFocus();

    QGraphicsItem *mouseGrabberItem() const;

protected:
    bool event(QEvent *event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void helpEvent(QGraphicsSceneHelpEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

Q_SIGNALS:
    void changed(const QList<QRectF> &region);
    void sceneRectChanged(const QRectF &rect);

private:
    void itemUpdated(QGraphicsItem *item, const QRectF &rect);

    Q_DECLARE_PRIVATE(QGraphicsScene)
    Q_PRIVATE_SLOT(d_func(), void generateBspTree())
    Q_PRIVATE_SLOT(d_func(), void emitUpdated())
    Q_PRIVATE_SLOT(d_func(), void removeItemLater(QGraphicsItem *item))
    friend class QGraphicsItem;
    friend class QGraphicsView;
    friend class QGraphicsViewPrivate;
};

QT_END_HEADER

#endif // QT_NO_GRAPHICSVIEW

#endif
