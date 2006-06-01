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

#ifndef QGRAPHICSVIEW_H
#define QGRAPHICSVIEW_H

#include <QtCore/qmetatype.h>
#include <QtGui/qpainter.h>
#include <QtGui/qscrollarea.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_GRAPHICSVIEW

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
    Q_PROPERTY(bool interactive READ isInteractive WRITE setInteractive)
    Q_PROPERTY(QRectF sceneRect READ sceneRect WRITE setSceneRect)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(QPainter::RenderHints renderHints READ renderHints WRITE setRenderHints)
    Q_PROPERTY(DragMode dragMode READ dragMode WRITE setDragMode)

public:
    enum DragMode {
        NoDrag,
        ScrollHandDrag,
        RubberBandDrag
    };

    QGraphicsView(QGraphicsScene *scene = 0, QWidget *parent = 0);
    ~QGraphicsView();

    QSize sizeHint() const;

    QPainter::RenderHints renderHints() const;
    void setRenderHint(QPainter::RenderHint hint, bool enabled = true);
    void setRenderHints(QPainter::RenderHints hints);

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    DragMode dragMode() const;
    void setDragMode(DragMode mode);

    bool isInteractive() const;
    void setInteractive(bool allowed);

    QGraphicsScene *scene() const;
    void setScene(QGraphicsScene *scene);

    QRectF sceneRect() const;
    void setSceneRect(const QRectF &rect);
    inline void setSceneRect(qreal x, qreal y, qreal w, qreal h);

    QMatrix matrix() const;
    void setMatrix(const QMatrix &matrix, bool combine = false);
    void resetMatrix();
    void rotate(qreal angle);
    void scale(qreal sx, qreal sy);
    void shear(qreal sh, qreal sv);
    void translate(qreal dx, qreal dy);

    void centerOn(const QPointF &pos);
    inline void centerOn(qreal x, qreal y);
    void centerOn(const QGraphicsItem *item);
    void ensureVisible(const QRectF &rect, int xmargin = 50, int ymargin = 50);
    inline void ensureVisible(qreal x, qreal y, qreal w, qreal h, int xmargin = 50, int ymargin = 50);
    void ensureVisible(const QGraphicsItem *item, int xmargin = 50, int ymargin = 50);
    void fitInView(const QRectF &rect, Qt::AspectRatioMode aspectRadioMode = Qt::IgnoreAspectRatio);
    inline void fitInView(qreal x, qreal y, qreal w, qreal h,
                          Qt::AspectRatioMode aspectRadioMode = Qt::IgnoreAspectRatio);
    void fitInView(const QGraphicsItem *item,
                   Qt::AspectRatioMode aspectRadioMode = Qt::IgnoreAspectRatio);

    void render(QPainter *painter, const QRectF &target = QRectF(), const QRect &source = QRect(),
                Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio);

    QList<QGraphicsItem *> items() const;
    QList<QGraphicsItem *> items(const QPoint &pos) const;
    inline QList<QGraphicsItem *> items(int x, int y) const;
    QList<QGraphicsItem *> items(const QRect &rect) const;
    QList<QGraphicsItem *> items(const QPolygon &polygon) const;
    QList<QGraphicsItem *> items(const QPainterPath &path) const;
    QGraphicsItem *itemAt(const QPoint &pos) const;
    inline QGraphicsItem *itemAt(int x, int y) const;

    QPointF mapToScene(const QPoint &point) const;
    QPolygonF mapToScene(const QRect &rect) const;
    QPolygonF mapToScene(const QPolygon &polygon) const;
    QPainterPath mapToScene(const QPainterPath &path) const;
    QPoint mapFromScene(const QPointF &point) const;
    QPolygon mapFromScene(const QRectF &rect) const;
    QPolygon mapFromScene(const QPolygonF &polygon) const;
    QPainterPath mapFromScene(const QPainterPath &path) const;
    inline QPointF mapToScene(int x, int y) const;
    inline QPolygonF mapToScene(int x, int y, int w, int h) const;
    inline QPoint mapFromScene(qreal x, qreal y) const;
    inline QPolygon mapFromScene(qreal x, qreal y, qreal w, qreal h) const;

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    QBrush backgroundBrush() const;
    void setBackgroundBrush(const QBrush &brush);

    QBrush foregroundBrush() const;
    void setForegroundBrush(const QBrush &brush);

public Q_SLOTS:
    void updateScene(const QList<QRectF> &rects);
    void updateSceneRect(const QRectF &rect);

protected Q_SLOTS:
    void setupViewport(QWidget *widget);

protected:
    bool event(QEvent *event);
    bool viewportEvent(QEvent *event);

    void contextMenuEvent(QContextMenuEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void scrollContentsBy(int dx, int dy);
    void showEvent(QShowEvent *event);

    virtual void drawBackground(QPainter *painter, const QRectF &rect);
    virtual void drawForeground(QPainter *painter, const QRectF &rect);
    virtual void drawItems(QPainter *painter, const QList<QGraphicsItem *> &items,
                           const QList<QStyleOptionGraphicsItem> &options);
    
private:
    Q_DECLARE_PRIVATE(QGraphicsView)
    friend class QGraphicsSceneWidget;
};

inline void QGraphicsView::setSceneRect(qreal ax, qreal ay, qreal aw, qreal ah)
{ setSceneRect(QRectF(ax, ay, aw, ah)); }
inline void QGraphicsView::centerOn(qreal ax, qreal ay)
{ centerOn(QPointF(ax, ay)); }
inline void QGraphicsView::ensureVisible(qreal ax, qreal ay, qreal aw, qreal ah, int xmargin, int ymargin)
{ ensureVisible(QRectF(ax, ay, aw, ah), xmargin, ymargin); }
inline void QGraphicsView::fitInView(qreal x, qreal y, qreal w, qreal h, Qt::AspectRatioMode mode)
{ fitInView(QRectF(x, y, w, h), mode); }
inline QList<QGraphicsItem *> QGraphicsView::items(int ax, int ay) const
{ return items(QPoint(ax, ay)); }
inline QGraphicsItem *QGraphicsView::itemAt(int ax, int ay) const
{ return itemAt(QPoint(ax, ay)); }
inline QPointF QGraphicsView::mapToScene(int ax, int ay) const
{ return mapToScene(QPoint(ax, ay)); }
inline QPolygonF QGraphicsView::mapToScene(int ax, int ay, int w, int h) const
{ return mapToScene(QRect(ax, ay, w, h)); }
inline QPoint QGraphicsView::mapFromScene(qreal ax, qreal ay) const
{ return mapFromScene(QPointF(ax, ay)); }
inline QPolygon QGraphicsView::mapFromScene(qreal ax, qreal ay, qreal w, qreal h) const
{ return mapFromScene(QRectF(ax, ay, w, h)); }

QT_END_HEADER

#endif // QT_NO_GRAPHICSVIEW

#endif
