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

#ifndef QGRAPHICSVIEW_H
#define QGRAPHICSVIEW_H

#include <QtCore/qmetatype.h>
#include <QtGui/qpainter.h>
#include <QtGui/qscrollarea.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QGraphicsItem;
class QGraphicsScene;
class QPainterPath;
class QPolygonF;
class QStyleOptionGraphicsItem;

Q_DECLARE_METATYPE(QPainter::RenderHints)

class QGraphicsViewPrivate;
class Q_GUI_EXPORT QGraphicsView : public QAbstractScrollArea
{
    Q_OBJECT
    Q_PROPERTY(QBrush backgroundBrush READ backgroundBrush WRITE setBackgroundBrush)
    Q_PROPERTY(QBrush foregroundBrush READ foregroundBrush WRITE setForegroundBrush)
    Q_PROPERTY(bool sceneInteractionAllowed READ isSceneInteractionAllowed WRITE setSceneInteractionAllowed)
    Q_PROPERTY(QRectF sceneRect READ sceneRect WRITE setSceneRect)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(QPainter::RenderHints renderHints READ renderHints WRITE setRenderHints)

public:
    enum SelectionMode {
        NoSelection,
        SingleSelection,
        MultiSelection,
        ExtendedSelection
    };
/*
    enum PaintOption {
        StretchContents = 0x1,
        NoBackground = 0x2,
        NoForeground = 0x4,
        IgnoreViewMatrix = 0x8
    };
    Q_DECLARE_FLAGS(PaintOptions, PaintOption)
*/    
    QGraphicsView(QGraphicsScene *scene = 0, QWidget *parent = 0);
    ~QGraphicsView();

    QPainter::RenderHints renderHints() const;
    void setRenderHint(QPainter::RenderHint hint, bool enabled = true);
    void setRenderHints(QPainter::RenderHints hints);

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    // ### Are these necessary?
    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode mode);

    bool isSceneInteractionAllowed() const;    
    void setSceneInteractionAllowed(bool allowed);
    
    QGraphicsScene *scene() const;
    void setScene(QGraphicsScene *scene);

    QRectF sceneRect() const;
    void setSceneRect(const QRectF &rect);
    inline void setSceneRect(qreal x, qreal y, qreal width, qreal height)
    { setSceneRect(QRectF(x, y, width, height)); }
    
    QWidget *renderWidget() const;
    void setRenderWidget(QWidget *widget);
    /*
    void renderToDevice(QPaintDevice *device, const QRect &rect = QRect(), PaintOptions options = 0);
    */
    
    QBrush backgroundBrush() const;
    void setBackgroundBrush(const QBrush &brush);
    
    QBrush foregroundBrush() const;
    void setForegroundBrush(const QBrush &brush);

    QMatrix matrix() const;    
    void setMatrix(const QMatrix &matrix, bool combine = false);
    void resetMatrix();
    void rotate(qreal angle);
    void scale(qreal sx, qreal sy);
    void shear(qreal sh, qreal sv);
    void translate(qreal dx, qreal dy);

    void centerOn(const QPointF &pos);
    inline void centerOn(qreal x, qreal y)
    { centerOn(QPointF(x, y)); }
    void centerOn(const QGraphicsItem *item);
    void ensureVisible(const QPointF &pos, int xmargin = 50, int ymargin = 50);
    inline void ensureVisible(qreal x, qreal y, int xmargin = 50, int ymargin = 50)
    { ensureVisible(QPointF(x, y), xmargin, ymargin); }
    void ensureVisible(const QGraphicsItem *item, int xmargin = 50, int ymargin = 50);
    
    QList<QGraphicsItem *> items() const;
    QList<QGraphicsItem *> items(const QPoint &pos) const;
    inline QList<QGraphicsItem *> items(int x, int y) const
    { return items(QPoint(x, y)); }
    QList<QGraphicsItem *> items(const QRect &rect) const;
    QList<QGraphicsItem *> items(const QPolygon &polygon) const;
    QList<QGraphicsItem *> items(const QPainterPath &path) const;
    QGraphicsItem *itemAt(const QPoint &pos) const;
    inline QGraphicsItem *itemAt(int x, int y) const
    { return itemAt(QPoint(x, y)); }

    QPointF mapToScene(const QPoint &point) const;
    QPolygonF mapToScene(const QRect &rect) const;
    QPolygonF mapToScene(const QPolygon &polygon) const;
    QPainterPath mapToScene(const QPainterPath &path) const;
    QPoint mapFromScene(const QPointF &point) const;
    QPolygon mapFromScene(const QRectF &rect) const;
    QPolygon mapFromScene(const QPolygonF &polygon) const;
    QPainterPath mapFromScene(const QPainterPath &path) const;
    inline QPointF mapToScene(int x, int y) const
    { return mapToScene(QPoint(x, y)); }
    inline QPolygonF mapToScene(int x, int y, int width, int height) const
    { return mapToScene(QRect(x, y, width, height)); }
    inline QPoint mapFromScene(qreal x, qreal y) const
    { return mapFromScene(QPointF(x, y)); }
    inline QPolygon mapFromScene(qreal x, qreal y, qreal width, qreal height) const
    { return mapFromScene(QRectF(x, y, width, height)); }

public Q_SLOTS:
    void update(const QList<QRectF> &rects);

protected:
    bool eventFilter(QObject *receiver, QEvent *event);
    bool event(QEvent *event);
    void scrollContentsBy(int dx, int dy);

    virtual void paintBackground(QPainter *painter, const QRect &rect);
    virtual void paintForeground(QPainter *painter, const QRect &rect);
    virtual void paintItems(QPainter *painter, const QList<QGraphicsItem *> items);
    QStyleOptionGraphicsItem styleOptionForItem(QGraphicsItem *item) const;
    
private:
    QGraphicsViewPrivate *d;
    friend class QGraphicsSceneWidget;
    friend class QGraphicsViewPrivate;
};
/*
Q_DECLARE_OPERATORS_FOR_FLAGS(QGraphicsView::PaintOptions)
*/

QT_END_HEADER

#endif
