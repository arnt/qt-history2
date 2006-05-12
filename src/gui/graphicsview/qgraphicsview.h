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
    inline void setSceneRect(qreal x, qreal y, qreal w, qreal h);
    
    QWidget *renderWidget() const;
    void setRenderWidget(QWidget *widget);
    
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
    inline void centerOn(qreal x, qreal y);
    void centerOn(const QGraphicsItem *item);
    void ensureVisible(const QPointF &pos, int xmargin = 50, int ymargin = 50);
    inline void ensureVisible(qreal x, qreal y, int xmargin = 50, int ymargin = 50);
    void ensureVisible(const QGraphicsItem *item, int xmargin = 50, int ymargin = 50);
    void fitInView(const QRectF &rect, Qt::AspectRatioMode aspectRadioMode = Qt::IgnoreAspectRatio);
    inline void fitInView(qreal x, qreal y, qreal w, qreal h,
                          Qt::AspectRatioMode aspectRadioMode = Qt::IgnoreAspectRatio);
    void fitInView(const QGraphicsItem *item,
                   Qt::AspectRatioMode aspectRadioMode = Qt::IgnoreAspectRatio);

    void drawScene(QPainter *painter, const QRectF &target = QRectF(), const QRect &source = QRect(),
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

public Q_SLOTS:
    void update(const QList<QRectF> &rects);

protected:
    bool eventFilter(QObject *receiver, QEvent *event);
    bool event(QEvent *event);
    void scrollContentsBy(int dx, int dy);

    virtual void paintBackground(QPainter *painter, const QRectF &rect);
    virtual void paintForeground(QPainter *painter, const QRectF &rect);
    virtual void paintItems(QPainter *painter, const QList<QGraphicsItem *> &items,
                            const QList<QStyleOptionGraphicsItem> &options);

private:
    Q_DECLARE_PRIVATE(QGraphicsView)
    friend class QGraphicsSceneWidget;
};

inline void QGraphicsView::setSceneRect(qreal ax, qreal ay, qreal aw, qreal ah)
{ setSceneRect(QRectF(ax, ay, aw, ah)); }
inline void QGraphicsView::centerOn(qreal ax, qreal ay)
{ centerOn(QPointF(ax, ay)); }
inline void QGraphicsView::ensureVisible(qreal ax, qreal ay, int xmargin, int ymargin)
{ ensureVisible(QPointF(ax, ay), xmargin, ymargin); }
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
