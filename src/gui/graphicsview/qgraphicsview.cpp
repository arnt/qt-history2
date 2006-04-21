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

//#define QGRAPHICSVIEW_DEBUG

/*!
    \class QGraphicsView
    \brief The QGraphicsView class provides a widget for displaying the contents of a QGraphicsScene.

    QGraphicsView visualizes the contents of a QGraphicsScene in a scrollable
    viewport. To create a scene with geometrical items, see QGraphicsScene's
    documentation.
    
    To visualize a scene, you start by constructing a QGraphicsView object,
    passing the address of the scene you want to visualize to QGraphicsView's
    constructor. Alternatively, you can call setScene() to set the scene at a
    later point. After you call show(), the view will by default scroll to the
    center of the scene and display any items that are visible at this
    point. For example:

    \code
        QGraphicsScene scene;
        scene.addText("Hello, world!");

        QGraphicsView view(&scene);
        view.show();
    \endcode

    You can explicitly scroll to any position on the scene by using the
    scrollbars, or by calling centerOn(). By passing a point to centerOn(),
    QGraphicsView will scroll its viewport to ensure that the point is
    centered in the view. An overload is provided for scrolling to a
    QGraphicsItem, in which case QGraphicsView will see to that the center of
    the item is centered in the view. If all you want is to ensure that a
    certain point is visible, (but not necessarily centered,) you can call
    ensureVisible() instead.

    QGraphicsView can be used to visualize a whole scene, or only parts of it.
    The visualized area is by default detected automatically when the view is
    displayed for the first time (by calling
    QGraphicsScene::itemsBoundingRect()). To set the visualized area rectangle
    yourself, you can call setSceneRect(). This will adjust the scrollbars'
    ranges appropriately.

    QGraphicsView visualizes the scene by first painting an optional
    background, then the items, and finally an optional foreground. Call
    setBackgroundBrush() or setForegroundBrush() to set the background and
    foreground to a color, pattern, gradient or texture. You can also provide
    custom background- or foreground painting yourself in a subclass of
    QGraphicsView, by reimplementing paintBackground() or
    paintForeground(). It is also possible to provide custom item drawing by
    reimplementing paintItems(). By default, the items are drawing by using a
    regular QPainter, and standard render hints. To change the default render
    hints that QGraphicsView passes to QPainter when painting items, you can
    call setRenderHints().
    
    By default, QGraphicsView renders its contents onto a regular widget. You
    can access this widget by calling renderWidget(), or you can replace it by
    calling setRenderWidget(). To render using OpenGL, simply call
    setRenderWidget(new QGLWidget). QGraphicsView takes ownership of the
    provided renderwidget.

    QGraphicsView supports affine transformations, using QMatrix. You can
    either pass a matrix to setMatrix(), or you can call one of the
    convenience functions rotate(), scale(), translate() or shear(). The most
    two common transformations are scaling, which is used to implement
    zooming, and rotation. QGraphicsView keeps the center of the view fixed
    during a transformation.
    
    You can interact with the items on the scene by using the mouse and
    keyboard. QGraphicsView translates the mouse and key events into \e scene
    events, (events that inherit QGraphicsSceneEvent,), and forward them to
    the visualized scene. In the end, it's the individual item that handles
    the events and reacts to them. For example, if you click on a selectable
    item, the item will typically let the scene know that it has been
    selected, and it will also redraw itself to display a selection
    rectangle. Similiary, if you click and drag the mouse to move a movable
    item, it's the item that handles the mouse moves and moves itself.  Item
    interaction is enabled by default, and you can disable it by calling
    setSceneInteractionEnabled().
    
    You can also provide your own custom scene interaction, by creating a
    subclass of QGraphicsView, and reimplementing the mouse and key event
    handlers. To simplify how you programmatically interact with items in the
    view, QGraphicsView provides the mapping functions mapToScene() and
    mapFromScene(), and the item accessors items() and itemAt(). These
    functions allow you to map points, rectangles, polygons and paths between
    view coordinates and scene coordinates, and to find items on the scene
    using view coordinates.

    \sa QGraphicsScene, QGraphicsItem, QGraphicsSceneEvent
*/

/*!
    \enum QGraphicsView::SelectionMode

    \value NoSelection
    \value SingleSelection
    \value MultiSelection
    \value ExtendedSelection
*/

#include "qgraphicsitem.h"
#include "qgraphicsscene.h"
#include "qgraphicsscene_p.h"
#include "qgraphicssceneevent.h"
#include "qgraphicsview.h"

#include <QtCore/qdatetime.h>
#include <QtCore/qdebug.h>
#include <QtGui/qapplication.h>
#include <QtGui/qevent.h>
#include <QtGui/qlayout.h>
#include <QtGui/qmatrix.h>
#include <QtGui/qpainter.h>
#include <QtGui/qrubberband.h>
#include <QtGui/qscrollbar.h>
#include <QtGui/qstyleoption.h>
#include <private/qabstractscrollarea_p.h>

static bool qt_closestItemLast(const QGraphicsItem *item1, const QGraphicsItem *item2)
{
    if (item2->isAncestorOf(item1))
        return false;
    if (item2->zValue() == item1->zValue())
        return item2 > item1;
    return item2->zValue() > item1->zValue();
}

class QGraphicsViewPrivate : public QAbstractScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsView)
public:
    QGraphicsViewPrivate();

    void recalculateContentSize();

    // Event forwarding
    void paintEvent(QPainter *painter, const QRegion &region /*, QGraphicsView::PaintOptions options */);
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);

    void setupRenderWidget(QWidget *widget);

    QPainter::RenderHints renderHints;

    QGraphicsView::SelectionMode selectionMode;
    bool sceneInteractionAllowed;
    QRectF sceneRect;
    void updateLastCenterPoint();

    QPointF mousePressItemPoint;
    QPointF mousePressScenePoint;
    QPoint mousePressViewPoint;
    QPoint mousePressScreenPoint;
    QPointF lastMouseMoveScenePoint;
    Qt::MouseButton mousePressButton;
    QMatrix matrix;
    bool accelerateScrolling;
    qreal leftIndent;
    qreal topIndent;

    // Replaying mouse events
    QMouseEvent lastMouseEvent;
    bool useLastMouseEvent;
    void replayLastMouseEvent();
    void storeMouseEvent(QMouseEvent *event);
    
    QPointF lastCenterPoint;
    bool useLastCenterPoint;
    Qt::Alignment alignment;

    QGraphicsScene *scene;
    QWidget *renderWidget;
    QBrush backgroundBrush;
    QBrush foregroundBrush;
    QRubberBand *rubberBand;
    bool rubberBanding;
};

/*!
    \internal
*/
QGraphicsViewPrivate::QGraphicsViewPrivate()
    : renderHints(QPainter::TextAntialiasing),
      selectionMode(QGraphicsView::ExtendedSelection),
      sceneInteractionAllowed(true), accelerateScrolling(true),
      leftIndent(0), topIndent(0),
      lastMouseEvent(QEvent::None, QPoint(), Qt::NoButton, 0, 0),
      useLastMouseEvent(false), useLastCenterPoint(false), alignment(Qt::AlignCenter),
      scene(0), renderWidget(0), rubberBand(0), rubberBanding(false)
{
}

/*!
    \internal
*/
void QGraphicsViewPrivate::recalculateContentSize()
{
    Q_Q(QGraphicsView);
    if (sceneRect.isNull() && scene)
        sceneRect = scene->itemsBoundingRect();
    
    int width = renderWidget->width();
    int height = renderWidget->height();
    int margin = 4;
    QRectF viewRect = matrix.mapRect(sceneRect);

    // Setting the ranges of these scroll bars can/will cause the values to
    // change, and scrollContentsBy() will be called correspondingly. This
    // will reset the last center point.
    QPointF savedLastCenterPoint = lastCenterPoint;

    // If the whole scene fits horizontally, we center the scene horizontally,
    // and ignore the horizontal scrollbars.
    if (viewRect.width() < width) {
        q->horizontalScrollBar()->setRange(0, 0);

        switch (alignment & Qt::AlignHorizontal_Mask) {
        case Qt::AlignLeft:
            leftIndent = -viewRect.left() + margin;
            break;
        case Qt::AlignRight:
            leftIndent = width - viewRect.width() - viewRect.left() - margin;
            break;
        case Qt::AlignHCenter:
        default:
            leftIndent = width / 2 - (viewRect.left() + viewRect.right()) / 2;
            break;
        }
        useLastCenterPoint = false;
    } else {
        q->horizontalScrollBar()->setRange(int(viewRect.left() - margin),
                                           int(viewRect.right() - width + margin));
        q->horizontalScrollBar()->setPageStep(width);
        q->horizontalScrollBar()->setSingleStep(width / 20);
        leftIndent = 0;
    }

    // If the whole scene fits vertical, we center the scene vertically, and
    // ignore the vertical scrollbars.
    if (viewRect.height() < height) {
        q->verticalScrollBar()->setRange(0, 0);

        switch (alignment & Qt::AlignVertical_Mask) {
        case Qt::AlignTop:
            topIndent = -viewRect.top() + margin;
            break;
        case Qt::AlignBottom:
            topIndent = height - viewRect.height() - viewRect.top() - margin;
            break;
        case Qt::AlignVCenter:
        default:
            topIndent = height / 2 - (viewRect.top() + viewRect.bottom()) / 2;
            break;
        }

        useLastCenterPoint = false;
    } else {
        q->verticalScrollBar()->setRange(int(viewRect.top() - margin),
                                         int(viewRect.bottom() - height + margin));
        q->verticalScrollBar()->setPageStep(height);
        q->verticalScrollBar()->setSingleStep(height / 20);
        topIndent = 0;
    }
    
    // Restore the center point from before the ranges changed.
    lastCenterPoint = savedLastCenterPoint;

    renderWidget->update();
}

/*!
    \internal
*/
void QGraphicsViewPrivate::paintEvent(QPainter *painter, const QRegion &region
                                      /*, QGraphicsView::PaintOptions options*/)
{
    Q_Q(QGraphicsView);

#if defined QGRAPHICSVIEW_DEBUG
    QTime stopWatch;
    stopWatch.start();
#endif
    painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, renderHints & QPainter::SmoothPixmapTransform);

    QMatrix moveMatrix;
    moveMatrix.translate(-q->horizontalScrollBar()->value() + leftIndent,
                         -q->verticalScrollBar()->value() + topIndent);
    QMatrix painterMatrix = matrix * moveMatrix;

    // Set up the painter matrix
    painter->setMatrix(painterMatrix, true);

    // Determine the exposed region
    QRegion exposedRegion = region;
    if (!accelerateScrolling || !scene)
        exposedRegion = renderWidget->rect();

    // Draw background
    //if ((options & QGraphicsView::NoBackground) == 0) {
    foreach (QRect rect, exposedRegion.rects())
        q->paintBackground(painter, q->mapToScene(rect.adjusted(-1, -1, 1, 1)).boundingRect());
    //}

    // Find all visible items
    QList<QGraphicsItem *> visibleItems;
    QSet<QGraphicsItem *> visibleItemSet;
    if (scene) {
        foreach (QRect rect, exposedRegion.rects()) {
            QList<QGraphicsItem *> itemsInArea = scene->items(q->mapToScene(rect));
            foreach (QGraphicsItem *item, itemsInArea) {
                if (!visibleItemSet.contains(item)) {
                    visibleItemSet << item;
                    visibleItems << item;
                }
            }
        }
    }

#if defined QGRAPHICSVIEW_DEBUG
    qDebug() << "**** QGraphicsSceneWidget::paintEvent()";
    qDebug() << "\tExposed view region:" << region;
    qDebug() << "\tScene bounding rect:" << (scene ? scene->itemsBoundingRect() : QRectF());
    qDebug() << "\tExposed items: " << visibleItems.size();
    qDebug() << "\tExposed rects: ";
    foreach (QRect rect, exposedRegion.rects()) {
        qDebug() << "\t\t" << q->mapToScene(rect);
    }
    qDebug() << "\tTime spent searching for items:"
             << (stopWatch.elapsed() / 1000.0) << "("
             << (1000 / qMax(1, stopWatch.elapsed())) << " iterations/s )";
    bool doo = false;
#endif

    qSort(visibleItems.begin(), visibleItems.end(), qt_closestItemLast);

    q->paintItems(painter, visibleItems);

#if defined QGRAPHICSVIEW_DEBUG
    qDebug() << "\tTime spent drawing:" << (stopWatch.elapsed() / 1000.0) << "("
             << ((visibleItems.size() * 1000) / (stopWatch.elapsed() + 1.0)) << "items/sec )";
    qDebug() << "\tEstimated FPS:" << (1000.0 / qMax<double>(1.0, stopWatch.elapsed()));
#endif

    // Draw foreground
    //if ((options & QGraphicsView::NoForeground) == 0) {
    foreach (QRect rect, exposedRegion.rects())
        q->paintForeground(painter, q->mapToScene(rect.adjusted(-1, -1, 1, 1)).boundingRect());
    //}
}

/*!
    \internal
*/
void QGraphicsViewPrivate::contextMenuEvent(QContextMenuEvent *event)
{
    Q_Q(QGraphicsView);
    if (!scene || !sceneInteractionAllowed)
        return;

    mousePressViewPoint = event->pos();
    mousePressScenePoint = q->mapToScene(mousePressViewPoint);
    mousePressScreenPoint = event->globalPos();
    lastMouseMoveScenePoint = mousePressScenePoint;

    QGraphicsSceneContextMenuEvent contextEvent(QEvent::GraphicsSceneContextMenu);
    contextEvent.setWidget(q);
    contextEvent.setScenePos(mousePressScenePoint);
    contextEvent.setScreenPos(mousePressScreenPoint);
    contextEvent.setModifiers(event->modifiers());
    QApplication::sendEvent(scene, &contextEvent);
}

/*!
    \internal
*/
void QGraphicsViewPrivate::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_Q(QGraphicsView);
    if (!scene || !sceneInteractionAllowed)
        return;

    storeMouseEvent(event);

    mousePressViewPoint = event->pos();
    mousePressScenePoint = q->mapToScene(mousePressViewPoint);
    mousePressScreenPoint = event->globalPos();
    lastMouseMoveScenePoint = mousePressScenePoint;
    mousePressButton = event->button();

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseDoubleClick);
    mouseEvent.setWidget(q);
    mouseEvent.setButtonDownScenePos(mousePressButton, mousePressScenePoint);
    mouseEvent.setButtonDownScreenPos(mousePressButton, mousePressScreenPoint);
    mouseEvent.setScenePos(q->mapToScene(mousePressViewPoint));
    mouseEvent.setScreenPos(mousePressScreenPoint);
    mouseEvent.setLastScenePos(lastMouseMoveScenePoint);
    mouseEvent.setLastScreenPos(q->mapFromScene(lastMouseMoveScenePoint));
    mouseEvent.setButtons(event->buttons());
    mouseEvent.setButtons(event->buttons());
    
    mouseEvent.setButton(event->button());
    mouseEvent.setModifiers(event->modifiers());
    QApplication::sendEvent(scene, &mouseEvent);
}

/*!
    \internal
*/
void QGraphicsViewPrivate::mousePressEvent(QMouseEvent *event)
{
    Q_Q(QGraphicsView);
    if (!scene || !sceneInteractionAllowed)
        return;

    mousePressViewPoint = event->pos();
    mousePressScenePoint = q->mapToScene(mousePressViewPoint);
    mousePressScreenPoint = event->globalPos();
    lastMouseMoveScenePoint = mousePressScenePoint;
    mousePressButton = event->button();

    if ((rubberBanding = !scene->itemAt(mousePressScenePoint)))
        scene->clearSelection();        
    
    storeMouseEvent(event);

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
    mouseEvent.setWidget(q);
    mouseEvent.setButtonDownScenePos(mousePressButton, mousePressScenePoint);
    mouseEvent.setButtonDownScreenPos(mousePressButton, mousePressScreenPoint);
    mouseEvent.setScenePos(mousePressScenePoint);
    mouseEvent.setScreenPos(mousePressScreenPoint);
    mouseEvent.setLastScenePos(lastMouseMoveScenePoint);
    mouseEvent.setLastScreenPos(q->mapFromScene(lastMouseMoveScenePoint));
    mouseEvent.setButtons(event->buttons());
    mouseEvent.setButton(event->button());
    mouseEvent.setModifiers(event->modifiers());
    QApplication::sendEvent(scene, &mouseEvent);
}

/*!
    \internal
*/
void QGraphicsViewPrivate::mouseMoveEvent(QMouseEvent *event)
{
    Q_Q(QGraphicsView);
    if (!scene || !sceneInteractionAllowed)
        return;

    if (rubberBanding) {
        // Invoke El Rubber
        if (!rubberBand) {
            if ((mousePressViewPoint - event->pos()).manhattanLength()
                < QApplication::startDragDistance())
                return;
            
            rubberBand = new QRubberBand(QRubberBand::Rectangle, q);
        }
        QRect selectionRect = QRect(mousePressViewPoint, event->pos()).normalized();
        rubberBand->setGeometry(selectionRect);
        QPainterPath selectionArea;
        selectionArea.addPolygon(q->mapToScene(selectionRect));
        scene->setSelectionArea(selectionArea);
        if (!rubberBand->isVisible())
            rubberBand->show();
        return;
    }

    if (!useLastMouseEvent)
        storeMouseEvent(event);
    
    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
    mouseEvent.setWidget(q);
    mouseEvent.setButtonDownScenePos(mousePressButton, mousePressScenePoint);
    mouseEvent.setButtonDownScreenPos(mousePressButton, mousePressScreenPoint);
    mouseEvent.setScenePos(q->mapToScene(event->pos()));
    mouseEvent.setScreenPos(event->globalPos());
    mouseEvent.setLastScenePos(lastMouseMoveScenePoint);
    mouseEvent.setLastScreenPos(q->mapFromScene(lastMouseMoveScenePoint));
    mouseEvent.setButtons(event->buttons());
    mouseEvent.setButton(event->button());
    mouseEvent.setModifiers(event->modifiers());
    lastMouseMoveScenePoint = mouseEvent.scenePos();
    QApplication::sendEvent(scene, &mouseEvent);
}

/*!
    \internal
*/
void QGraphicsViewPrivate::mouseReleaseEvent(QMouseEvent *event)
{
    Q_Q(QGraphicsView);
    if (!scene || !sceneInteractionAllowed)
        return;

    if (rubberBanding) {
        // Withdraw El Rubber
        delete rubberBand;
        rubberBand = 0;
        rubberBanding = false;
        return;
    }
    
    storeMouseEvent(event);
 
    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
    mouseEvent.setWidget(q);
    mouseEvent.setButtonDownScenePos(mousePressButton, mousePressScenePoint);
    mouseEvent.setButtonDownScreenPos(mousePressButton, mousePressScreenPoint);
    mouseEvent.setScenePos(q->mapToScene(event->pos()));
    mouseEvent.setScreenPos(event->globalPos());
    mouseEvent.setLastScenePos(lastMouseMoveScenePoint);
    mouseEvent.setLastScreenPos(q->mapFromScene(lastMouseMoveScenePoint));
    mouseEvent.setButtons(event->buttons());
    mouseEvent.setButton(event->button());
    mouseEvent.setModifiers(event->modifiers());
    QApplication::sendEvent(scene, &mouseEvent);
}

/*!
    \internal
*/
void QGraphicsViewPrivate::keyPressEvent(QKeyEvent *event)
{
    if (!scene || !sceneInteractionAllowed)
        return;
    QApplication::sendEvent(scene, event);
}

/*!
    \internal
*/
void QGraphicsViewPrivate::keyReleaseEvent(QKeyEvent *event)
{
    if (!scene || !sceneInteractionAllowed)
        return;
    QApplication::sendEvent(scene, event);
}

/*!
    \internal
*/
void QGraphicsViewPrivate::focusInEvent(QFocusEvent *event)
{
    QApplication::sendEvent(scene, event);
}

/*!
    \internal
*/
void QGraphicsViewPrivate::focusOutEvent(QFocusEvent *event)
{
    QApplication::sendEvent(scene, event);
}

/*!
    \internal
*/
void QGraphicsViewPrivate::setupRenderWidget(QWidget *widget)
{
    Q_Q(QGraphicsView);
    if (!widget)
        widget = new QWidget(q->viewport());
    
    accelerateScrolling = !widget->inherits("QGLWidget");

    if (!renderWidget) {
        // If we didn't have a render widget already, we set the center
        // point to what corresponds to the center of the render widget.       
        lastCenterPoint = q->mapToScene(q->mapFromScene(q->mapToScene(widget->rect().center())));
        useLastCenterPoint = true;
    }

    QWidget *oldRenderWidget = renderWidget;
    renderWidget = widget;
    renderWidget->setFocusPolicy(Qt::StrongFocus);

    // autoFillBackground enables scroll acceleration.
    renderWidget->setAutoFillBackground(true);

    QLayout *layout = q->viewport()->layout();
    layout->addWidget(renderWidget);
    layout->removeWidget(oldRenderWidget);

    if (oldRenderWidget) {
        renderWidget->resize(oldRenderWidget->size());
        recalculateContentSize();

        // Comment
        q->centerOn(QPointF(lastCenterPoint));
    } else {
        updateLastCenterPoint();
    }

    if (oldRenderWidget) {
        oldRenderWidget->hide();
        delete oldRenderWidget;
    }
    renderWidget->installEventFilter(q);
    renderWidget->setMouseTracking(true);
}

/*!
    \internal
*/
void QGraphicsViewPrivate::updateLastCenterPoint()
{
    Q_Q(QGraphicsView);
    lastCenterPoint = q->mapToScene(q->mapFromScene(q->mapToScene(renderWidget->rect().center())));
}

/*!
    \internal
*/
void QGraphicsViewPrivate::replayLastMouseEvent()
{
    if (!useLastMouseEvent)
        return;

    QMouseEvent *mouseEvent = new QMouseEvent(QEvent::MouseMove, lastMouseEvent.pos(), lastMouseEvent.globalPos(),
                                              lastMouseEvent.button(), lastMouseEvent.buttons(), lastMouseEvent.modifiers());
    QApplication::postEvent(renderWidget, mouseEvent);
}

/*!
    \internal
*/
void QGraphicsViewPrivate::storeMouseEvent(QMouseEvent *event)
{
    useLastMouseEvent = true;
    lastMouseEvent = QMouseEvent(QEvent::MouseMove, event->pos(), event->globalPos(),
                                 event->button(), event->buttons(), event->modifiers());
}

/*!
    Constructs a QGraphicsView and sets the visualized scene to \a
    scene. \a parent is passed to QWidget's constructor.
*/
QGraphicsView::QGraphicsView(QGraphicsScene *scene, QWidget *parent)
    : QAbstractScrollArea(*new QGraphicsViewPrivate, parent)
{
    Q_D(QGraphicsView);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    viewport()->setLayout(layout);
    d->setupRenderWidget(0);

    setScene(scene);
}

/*!
    Destructs the QGraphicsView object.
*/
QGraphicsView::~QGraphicsView()
{
}

/*!
    \property QGraphicsView::renderHints
    \brief the default render hints for the view

    These hints are
    used to initialize QPainter before each visible item is drawn. QPainter
    uses render hints to toggle rendering features such as antialiasing and
    smooth pixmap transformation.

    The default render hint is QPainter::TextAntialiasing.

    Example:

    \code
        QGraphicsScene scene;
        scene.addRect(QRectF(-10, -10, 20, 20));

        QGraphicsView view(&scene);
        view.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        view.show();
    \endcode
*/
QPainter::RenderHints QGraphicsView::renderHints() const
{
    Q_D(const QGraphicsView);
    return d->renderHints;
}
void QGraphicsView::setRenderHints(QPainter::RenderHints hints)
{
    Q_D(QGraphicsView);
    d->renderHints = hints;
}

/*!
    If \a enabled is true, the render hint \a hint is enabled; otherwise it
    is disabled.

    \sa renderHints
*/
void QGraphicsView::setRenderHint(QPainter::RenderHint hint, bool enabled)
{
    Q_D(QGraphicsView);
    if (enabled)
        d->renderHints |= hint;
    else
        d->renderHints &= ~hint;
    d->renderWidget->update();
}

/*!
    \property QGraphicsView::alignment
    \brief the alignment of the scene in the view when the whole
    scene is visible.

    If the whole scene is visible in the view, (i.e., there are no visible
    scrollbars,) the view's alignment will decide where the scene will be
    rendered in the view. For example, if the alignment is Qt::AlignCenter,
    which is default, the scene will be centered in the view, and if the
    alignment is (Qt::AlignLeft | Qt::AlignTop), the scene will be rendered in
    the top-left corner of the view.
*/
Qt::Alignment QGraphicsView::alignment() const
{
    Q_D(const QGraphicsView);
    return d->alignment;
}
void QGraphicsView::setAlignment(Qt::Alignment alignment)
{
    Q_D(QGraphicsView);
    if (d->alignment != alignment) {
        d->alignment = alignment;
        d->recalculateContentSize();
    }
}

/*!
    \internal
*/
QGraphicsView::SelectionMode QGraphicsView::selectionMode() const
{
    Q_D(const QGraphicsView);
    return d->selectionMode;
}

/*!
    \internal
*/
void QGraphicsView::setSelectionMode(SelectionMode mode)
{
    Q_D(QGraphicsView);
    d->selectionMode = mode;
}

/*!
    \property QGraphicsView::sceneInteractionAllowed
    \brief whether the view allowed scene interaction.

    If enabled, this view is set to allow scene interaction. Otherwise, this
    view will not allow interaction, and any mouse or key events are ignored
    (i.e., it will act as a read-only view).
*/
bool QGraphicsView::isSceneInteractionAllowed() const
{
    Q_D(const QGraphicsView);
    return d->sceneInteractionAllowed;
}
void QGraphicsView::setSceneInteractionAllowed(bool allowed)
{
    Q_D(QGraphicsView);
    d->sceneInteractionAllowed = allowed;
}

/*!
    Returns a pointer to the scene that is currently visualized in the
    view. If no scene is currently visualized, 0 is returned.

    \sa setScene()
*/
QGraphicsScene *QGraphicsView::scene() const
{
    Q_D(const QGraphicsView);
    return d->scene;
}

/*!
    Sets the current scene to \a scene. If \a scene is already being
    viewed, this function does nothing.

    When a scene is set on a view, the QGraphicsScene::changed() signal
    is automatically connected to this view's update() slot, and the
    view's scrollbars are adjusted to fit the size of the scene.
*/
void QGraphicsView::setScene(QGraphicsScene *scene)
{
    Q_D(QGraphicsView);
    if (d->scene == scene)
        return;

    if (d->scene) {
        disconnect(d->scene, SIGNAL(changed(const QList<QRectF> &)),
                   this, SLOT(update(const QList<QRectF> &)));
    }

    if ((d->scene = scene)) {
        connect(d->scene, SIGNAL(changed(const QList<QRectF> &)),
                this, SLOT(update(const QList<QRectF> &)));
        d->recalculateContentSize();
        d->lastCenterPoint = d->matrix.map(d->sceneRect.center());
    }
}


/*!
    \property QGraphicsView::sceneRect
    \brief the area of the scene visualized by this view.

    If \a rect is invalid, the view will guess the scene rect by calling
    QGraphicsScene::itemsBoundingRect().

    The maximum size of \a rect is limited by the range of
    QAbstractScrollArea's scrollbars, which operate in integer coordinates.
*/
QRectF QGraphicsView::sceneRect() const
{
    Q_D(const QGraphicsView);
    return d->sceneRect;
}
void QGraphicsView::setSceneRect(const QRectF &rect)
{
    Q_D(QGraphicsView);
    d->sceneRect = rect;
    d->recalculateContentSize();
    d->lastCenterPoint = d->matrix.map(rect.center());
}

/*!
    Returns the view's render widget, which is always != 0. The render widget
    is the widget that QGraphicsView draws onto. By default, QGraphicsView
    assigns a regular QWidget, which uses the platform's default paint engine,
    but custom render widget can also be set by calling setRenderWidget().

    \sa setRenderWidget()
*/
QWidget *QGraphicsView::renderWidget() const
{
    Q_D(const QGraphicsView);
    return d->renderWidget;
}

/*!
    Replaces the view's current render widget with \a widget, and deletes the
    old one. The render widget is the widget that QGraphicsView draws onto. By
    default, and if \a widget is 0, QGraphicsView internally assigns a regular
    QWidget to be the render widget, using the platform's default paint
    engine, but you can set any QWidget subclass to be the render widget. The
    widget is automatically resized to fit the size of the view.

    Call this function to enable OpenGL on the view. For example:

    \code
        QGraphicsScene scene;
        scene.addRect(QRectF(-10, -10, 20, 20));

        QGraphicsView view(&scene);
        view.setRenderWidget(new QGLWidget);
        view.show();
    \endcode

    QGraphicsView takes ownership of \a widget, so you do not need to delete
    it.

    \sa renderWidget()
*/
void QGraphicsView::setRenderWidget(QWidget *widget)
{
    Q_D(QGraphicsView);
    d->setupRenderWidget(widget);
}
/*
void QGraphicsView::renderToDevice(QPaintDevice *device, const QRect &rect,
                                   PaintOptions options)
{
    QRect printRect = !rect.isNull() ? rect : d->renderWidget->rect();

    QPainter painter(device);

    if (options & StretchContents) {
        qDebug() << (device->width() / qreal(printRect.width()))
                 << (device->height() / qreal(printRect.height()));
        
        painter.scale(device->width() / qreal(printRect.width()),
                      device->height() / qreal(printRect.height()));
    }

    qDebug() << "Rect:" << printRect;
    qDebug() << "Mapped rect:" << painter.deviceMatrix().mapRect(printRect);

    d->paintEvent(&painter, QRegion(printRect), options);
}
*/
/*!
    \property QGraphicsView::backgroundBrush
    \brief the background brush of the view.

    Set this property to changes the view's background to a different color,
    gradient or texture. The default background brush is Qt::NoBrush. The
    background is drawn before (behind) the items.

    Example:

    \code
        QGraphicsView view;
        view.show();

        // a blue background
        view.setBackgroundBrush(Qt::blue);

        // a gradient background
        QRadialGradient gradient(0, 0, 10);
        gradient.setSpread(QGradient::RepeatSpread);
        view.setBackgroundBrush(gradient);
    \endcode

    The background and foreground are drawn using scene coordinates. Any
    transformations applied on the view also apply to the background
    brush. For example, if your gradient is centered on the (0, 0) point as in
    the example above, the center point of the radial gradient will move when
    you scroll the view.

    QGraphicsView calls paintBackground() to paint its background. For more
    detailed control over how the background is drawn, you can reimplement
    paintBackground() in a subclass of QGraphicsView.
*/
QBrush QGraphicsView::backgroundBrush() const
{
    Q_D(const QGraphicsView);
    return d->backgroundBrush;
}
void QGraphicsView::setBackgroundBrush(const QBrush &brush)
{
    Q_D(QGraphicsView);
    d->backgroundBrush = brush;
    d->renderWidget->update();
}

/*!
    \property QGraphicsView::foregroundBrush
    \brief the foreground brush of the view.

    Change this property to set the view's foreground to a different color,
    gradient or texture. If the style of \a brush is Qt::NoBrush (e.g., if
    you passed QBrush()), the foreground is not drawn.  The foreground is
    drawn after (on top of) the items. The default foreground brush is
    Qt::NoBrush.

    Example:

    \code
        QGraphicsView view;
        view.show();

        // a white semi-transparent foreground
        view.setForegroundBrush(QColor(255, 255, 255, 127));

        // a grid foreground
        view.setForegroundBrush(QBrush(Qt::lightGray, Qt::CrossPattern));
    \endcode

    The foreground and background are drawn using scene coordinates. Any
    transformations applied on the view also apply to the foreground
    brush. For example, if your foreground brush is a gradient, centered on
    the (0, 0) point as in the example above, the center point of the radial
    gradient will move when you scroll the view.

    QGraphicsView calls paintForeground() to paint its foreground. For more
    detailed control over how the foreground is drawn, you can reimplement
    paintForeground() in a subclass of QGraphicsView.
*/
QBrush QGraphicsView::foregroundBrush() const
{
    Q_D(const QGraphicsView);
    return d->foregroundBrush;
}
void QGraphicsView::setForegroundBrush(const QBrush &brush)
{
    Q_D(QGraphicsView);
    d->foregroundBrush = brush;
    d->renderWidget->update();
}

/*!
    Returns the current transformation matrix for the view. If no current
    transformation is set, the identity matrix is returned.

    \sa setMatrix(), rotate(), scale(), shear(), translate()
*/
QMatrix QGraphicsView::matrix() const
{
    Q_D(const QGraphicsView);
    return d->matrix;
}

/*!
    Sets the view's current transformation matrix to \a matrix.

    If \a combine is true, then \a matrix is combined with the current matrix;
    otherwise, \a matrix \e replaces the current matrix. \a combine is false
    by default.

    The transformation matrix tranforms the scene into view coordinates. Using
    the default transformation, provided by the identity matrix, one pixel in
    the view represents one unit in the scene (e.g., a 10x10 rectangular item
    is drawn using 10x10 pixels in the view). If a 2x2 scaling matrix is
    applied, the scene will be drawn in 1:2 (e.g., a 10x10 rectangular item is
    then drawn using 20x20 pixels in the view).

    Example:

    \code
        QGraphicsScene scene;
        scene.addText("GraphicsView rotated clockwise");
    
        QGraphicsView view(&scene);
        view.rotate(90); // the text is rendered with a 90 degree clockwise rotation
        view.show();
    \endcode    
    
    To simplify interation with items using a transformed view, QGraphicsView
    provides mapTo... and mapFrom... functions that can translate between
    scene and view coordinates. For example, you can call mapToScene() to map
    a view coordiate to a floating point scene coordinate, or mapFromScene()
    to map from floating point scene coordinates to view coordinates.

    \sa matrix(), rotate(), scale(), shear(), translate()
*/
void QGraphicsView::setMatrix(const QMatrix &matrix, bool combine)
{
    Q_D(QGraphicsView);
    QMatrix oldMatrix = d->matrix;
    if (!combine)
        d->matrix = matrix;
    else
        d->matrix = matrix * d->matrix;
    if (oldMatrix == d->matrix)
        return;

    if (d->scene) {
        d->recalculateContentSize();
        centerOn(d->lastCenterPoint);
    } else {
        d->updateLastCenterPoint();
    }

    if (d->sceneInteractionAllowed)
        d->replayLastMouseEvent();
}

/*!
    Resets the view transformation matrix to the identity matrix.
*/
void QGraphicsView::resetMatrix()
{
    setMatrix(QMatrix());
}

/*!
    Rotates the current view transformation \a angle degrees clockwise.

    \sa setMatrix(), matrix(), scale(), shear(), translate()
*/
void QGraphicsView::rotate(qreal angle)
{
    Q_D(QGraphicsView);
    QMatrix matrix = d->matrix;
    matrix.rotate(angle);
    setMatrix(matrix);
}

/*!
    Scales the current view transformation by (\a sx, \a sy).

    \sa setMatrix(), matrix(), rotate(), shear(), translate()
*/
void QGraphicsView::scale(qreal sx, qreal sy)
{
    Q_D(QGraphicsView);
    QMatrix matrix = d->matrix;
    matrix.scale(sx, sy);
    setMatrix(matrix);
}

/*!
    Shears the current view transformation by (\a sh, \a sv).

    \sa setMatrix(), matrix(), rotate(), scale(), translate()
*/
void QGraphicsView::shear(qreal sh, qreal sv)
{
    Q_D(QGraphicsView);
    QMatrix matrix = d->matrix;
    matrix.shear(sh, sv);
    setMatrix(matrix);
}

/*!
    Translates the current view transformation by (\a dx, \a dy).

    \sa setMatrix(), matrix(), rotate(), shear()
*/
void QGraphicsView::translate(qreal dx, qreal dy)
{
    Q_D(QGraphicsView);
    QMatrix matrix = d->matrix;
    matrix.translate(dx, dy);
    setMatrix(matrix);
}

/*!
    Scrolls the contents of the viewport to ensure that the scene
    coordinate \a pos, is centered in the view.

    Because \a pos is a floating point coordinate, and the scroll bars operate
    on integer coordinates, the centering is only an approximation.

    \sa ensureVisible()
*/
void QGraphicsView::centerOn(const QPointF &pos)
{
    Q_D(QGraphicsView);
    qreal width = d->renderWidget->width();
    qreal height = d->renderWidget->height();
    QPointF viewPoint = d->matrix.map(pos);
    QPointF oldCenterPoint = pos;
    if (!d->leftIndent)
        horizontalScrollBar()->setValue(int(viewPoint.x() - width / 2.0));
    if (!d->topIndent)
        verticalScrollBar()->setValue(int(viewPoint.y() - height / 2.0));
    d->lastCenterPoint = oldCenterPoint;
}

/*!
    \fn QGraphicsView::centerOn(qreal x, qreal y)
    \overload

    This function is provided for convenience. It's equivalent to calling
    centerOn(QPointF(\a x, \a y)).
*/

/*!
    \overload

    Scrolls the contents of the viewport to ensure that \a item
    is centered in the view.

    \sa ensureVisible()
*/
void QGraphicsView::centerOn(const QGraphicsItem *item)
{
    centerOn(item->sceneBoundingRect().center());
}

/*!
    Scrolls the contents of the viewport so that the scene coordinate \a pos
    is visible, with margins specified in pixels by \a xmargin and \a
    ymargin. If the specified point cannot be reached, the contents are
    scrolled to the nearest valid position. The default value for both margins
    is 50 pixels.

    \sa centerOn()
*/
void QGraphicsView::ensureVisible(const QPointF &pos, int xmargin, int ymargin)
{
    Q_D(QGraphicsView);
    Q_UNUSED(xmargin);
    Q_UNUSED(ymargin);
    qreal width = d->renderWidget->width();
    qreal height = d->renderWidget->height();
    QPointF viewPoint = d->matrix.map(pos);
    QPointF oldCenterPoint = pos;

    qreal left = d->leftIndent ? d->leftIndent : horizontalScrollBar()->value();
    qreal right = left + width;
    qreal top = d->topIndent ? d->topIndent : verticalScrollBar()->value();
    qreal bottom = top + height;
    
    if (viewPoint.x() <= left + xmargin) {
        // need to scroll from the left
        if (!d->leftIndent)
            horizontalScrollBar()->setValue(int(viewPoint.x() - xmargin - 0.5));
    }
    if (viewPoint.x() >= right - xmargin) {
        // need to scroll from the right
        if (!d->leftIndent)
            horizontalScrollBar()->setValue(int(viewPoint.x() - width + xmargin + 0.5));
    }
    if (viewPoint.y() <= top + ymargin) {
        // need to scroll from the top
        if (!d->topIndent)
            verticalScrollBar()->setValue(int(viewPoint.y() - ymargin - 0.5));
    }
    if (viewPoint.y() >= bottom - ymargin) {
        // need to scroll from the bottom
        if (!d->topIndent)
            verticalScrollBar()->setValue(int(viewPoint.y() - height + ymargin + 0.5));
    }

    d->lastCenterPoint = oldCenterPoint;
}

/*!
    \fn QGraphicsView::ensureVisible(qreal x, qreal y, int xmargin, int ymargin)
    \overload

    This function is provided for convenience. It's equivalent to calling
    ensureVisible(QPointF(\a x, \a y), \a xmargin, \a ymargin).
*/

/*!
    \overload

    Scrolls the contents of the viewport so that the center of item \a item is
    visible, with margins specified in pixels by \a xmargin and \a ymargin. If
    the specified point cannot be reached, the contents are scrolled to the
    nearest valid position. The default value for both margins is 50 pixels.

    \sa centerOn()
*/
void QGraphicsView::ensureVisible(const QGraphicsItem *item, int xmargin, int ymargin)
{
    ensureVisible(item->sceneBoundingRect().center(), xmargin, ymargin);
}

/*!
    Returns a list of all the items in the associated scene.

    \sa QGraphicsScene::items()
*/
QList<QGraphicsItem *> QGraphicsView::items() const
{
    Q_D(const QGraphicsView);
    if (!d->scene)
        return QList<QGraphicsItem *>();
    return d->scene->items();
}

/*!
    Returns a list of all the items at the position \a pos in the view. The
    items are listed in descending Z order (i.e., the first item in the list
    is the top-most item, and the last item is the bottom-most item). \a pos
    is in viewport coordinates.

    This function is most commonly called from within mouse event handlers in
    a subclass in QGraphicsView. \a pos is in untransformed viewport
    coordinates, just like QMouseEvent::pos().

    \code
        void CustomView::mousePressEvent(QMouseEvent *event)
        {
            qDebug() << "There are" << items(event->pos()).size()
                     << "items at position" << mapToScene(event->pos());
        }
    \endcode

    \sa QGraphicsScene::items(), QGraphicsItem::zValue()
*/
QList<QGraphicsItem *> QGraphicsView::items(const QPoint &pos) const
{
    Q_D(const QGraphicsView);
    if (!d->scene)
        return QList<QGraphicsItem *>();
    return d->scene->items(mapToScene(pos));
}

/*!
    \fn QGraphicsView::items(int x, int y) const

    This function is provided for convenience. It's equivalent to calling
    items(QPoint(\a x, \a y)).
*/

/*!
    \overload

    Returns a list of all the items that are either contained by or intersect
    with \a rect. \a rect is in viewport coordinates.

    \sa itemAt(), items(), mapToScene()
*/
QList<QGraphicsItem *> QGraphicsView::items(const QRect &rect) const
{
    Q_D(const QGraphicsView);
    if (!d->scene)
        return QList<QGraphicsItem *>();
    return d->scene->items(mapToScene(rect));
}

/*!
    \overload

    Returns a list of all the items that are either contained by or intersect
    with \a polygon. \a polygon is in viewport coordinates.

    \sa itemAt(), items(), mapToScene()
*/
QList<QGraphicsItem *> QGraphicsView::items(const QPolygon &polygon) const
{
    Q_D(const QGraphicsView);
    if (!d->scene)
        return QList<QGraphicsItem *>();
    return d->scene->items(mapToScene(polygon));
}

/*!
    \overload

    Returns a list of all the items that are either contained by or intersect
    with \a path. \a path is in viewport coordinates.

    \sa itemAt(), items(), mapToScene()
*/
QList<QGraphicsItem *> QGraphicsView::items(const QPainterPath &path) const
{
    Q_D(const QGraphicsView);
    if (!d->scene)
        return QList<QGraphicsItem *>();
    return d->scene->items(mapToScene(path));
}

/*!
    Returns the item at position \a pos, which is in viewport coordinates.
    If there are several items at this position, this function returns
    the topmost item.

    Example:

    \code
        void CustomView::mousePressEvent(QMouseEvent *event)
        {
            if (QGraphicsItem *item = itemAt(event->pos())) {
                qDebug() << "You clicked on item" << item;
            } else {
                qDebug() << "You didn't click on an item.";
            }
        }
    \endcode

    \sa items()
*/
QGraphicsItem *QGraphicsView::itemAt(const QPoint &pos) const
{
    Q_D(const QGraphicsView);
    return d->scene ? d->scene->itemAt(mapToScene(pos)) : 0;
}

/*!
    \overload
    \fn QGraphicsItem *QGraphicsView::itemAt(int x, int y) const

    This function is provided for convenience. It's equivalent to
    calling itemAt(QPoint(\a x, \a y)).
*/

/*!
    Returns the viewport coordinate \a point mapped to scene coordinates.

    \sa mapFromScene()
*/
QPointF QGraphicsView::mapToScene(const QPoint &point) const
{
    Q_D(const QGraphicsView);
    QPointF p = point;
    p.rx() += horizontalScrollBar()->value() - d->leftIndent;
    p.ry() += verticalScrollBar()->value() - d->topIndent;
    return d->matrix.inverted().map(p);
}

/*!
    \fn QGraphicsView::mapToScene(int x, int y) const

    This function is provided for convenience. It's equivalent to calling
    mapToScene(QPoint(\a x, \a y)).
*/

/*!
    Returns the viewport rectangle \a rect mapped to a scene coordinate
    polygon.

    \sa mapFromScene()
*/
QPolygonF QGraphicsView::mapToScene(const QRect &rect) const
{
    if (!rect.isValid())
        return QPolygonF();
    return QPolygonF(QVector<QPointF>()
                     << mapToScene(rect.topLeft())
                     << mapToScene(rect.topRight())
                     << mapToScene(rect.bottomRight())
                     << mapToScene(rect.bottomLeft()));
}

/*!
    \fn QGraphicsView::mapToScene(int x, int y, int w, int h) const

    This function is provided for convenience. It's equivalent to calling
    mapToScene(QRect(\a x, \a y, \a w, \a h)).
*/

/*!
    Returns the viewport polygon \a polygon mapped to a scene coordinate
    polygon.

    \sa mapFromScene()
*/
QPolygonF QGraphicsView::mapToScene(const QPolygon &polygon) const
{
    QPolygonF poly;
    foreach (QPoint point, polygon)
        poly << mapToScene(point);
    return poly;
}

/*!
    Returns the viewport painter path \a path mapped to a scene coordinate
    painter path.

    \sa mapFromScene()
*/
QPainterPath QGraphicsView::mapToScene(const QPainterPath &path) const
{
    Q_D(const QGraphicsView);
    QMatrix moveMatrix;
    moveMatrix.translate(horizontalScrollBar()->value() - d->leftIndent,
                         verticalScrollBar()->value() - d->topIndent);
    return (moveMatrix * d->matrix.inverted()).map(path);
}

/*!
    Returns the scene coordinate \a point to viewport coordinates.

    \sa mapToScene()
*/
QPoint QGraphicsView::mapFromScene(const QPointF &point) const
{
    Q_D(const QGraphicsView);
    QPointF p = d->matrix.map(point);
    p.rx() -= horizontalScrollBar()->value() - d->leftIndent;
    p.ry() -= verticalScrollBar()->value() - d->topIndent;
    return p.toPoint();
}

/*!
    \fn QGraphicsView::mapFromScene(qreal x, qreal y) const

    This function is provided for convenience. It's equivalent to
    calling mapFromScene(QPointF(\a x, \a y)).
*/

/*!
    Returns the scene rectangle \a rect to a viewport coordinate
    polygon.

    \sa mapToScene()
*/
QPolygon QGraphicsView::mapFromScene(const QRectF &rect) const
{
    if (!rect.isValid())
        return QPolygon();
    return QPolygon(QVector<QPoint>()
                    << mapFromScene(rect.topLeft())
                    << mapFromScene(rect.topRight())
                    << mapFromScene(rect.bottomRight())
                    << mapFromScene(rect.bottomLeft()));
}

/*!
    \fn QGraphicsView::mapFromScene(qreal x, qreal y, qreal w, qreal h) const

    This function is provided for convenience. It's equivalent to
    calling mapFromScene(QRectF(\a x, \a y, \a w, \a h)).
*/

/*!
    Returns the scene coordinate polygon \a polygon to a viewport coordinate
    polygon.

    \sa mapToScene()
*/
QPolygon QGraphicsView::mapFromScene(const QPolygonF &polygon) const
{
    QPolygon poly;
    foreach (QPointF point, polygon)
        poly << mapFromScene(point);
    return poly;
}

/*!
    Returns the scene coordinate painter path \a path to a viewport coordinate
    painter path.

    \sa mapToScene()
*/
QPainterPath QGraphicsView::mapFromScene(const QPainterPath &path) const
{
    Q_D(const QGraphicsView);
    QMatrix moveMatrix;
    moveMatrix.translate(-horizontalScrollBar()->value() + d->leftIndent,
                         -verticalScrollBar()->value() + d->topIndent);
    return (d->matrix * moveMatrix).map(path);
}

/*!
    Schedules an update of the scene rectangles \a rects.

    \sa QGraphicsScene::changed()
*/
void QGraphicsView::update(const QList<QRectF> &rects)
{
    Q_D(QGraphicsView);
    QRect renderWidgetRect = d->renderWidget->rect();

    QRect sumRect;
    foreach (QRectF rect, rects) {
        // Find the item's bounding rect and map it to view coordiates.
        // Adjust with 2 pixels for antialiasing.
        QRect mappedRect = mapFromScene(rect).boundingRect().adjusted(-2, -2, 2, 2);
        if (renderWidgetRect.contains(mappedRect) || renderWidgetRect.intersects(mappedRect)) {
            sumRect |= mappedRect;
            if (!d->accelerateScrolling)
                break;
        }
    }

    if (!sumRect.isNull())
        d->renderWidget->update(!d->accelerateScrolling ? renderWidgetRect : sumRect);
}

/*!
    \reimp
*/
bool QGraphicsView::eventFilter(QObject *receiver, QEvent *event)
{
    Q_D(QGraphicsView);
    if (receiver != d->renderWidget) {
        if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
            return false;
    }
    if (!d->renderWidget || (!d->scene && event->type() != QEvent::Paint))
        return false;

    switch (event->type()) {
    case QEvent::Paint: {
        QPainter painter(d->renderWidget);
        d->paintEvent(&painter, static_cast<QPaintEvent *>(event)->region()/*, 0 */);
        break;
    }
    case QEvent::ContextMenu:
        d->contextMenuEvent(static_cast<QContextMenuEvent *>(event));
        break;
    case QEvent::KeyPress:
        d->keyPressEvent(static_cast<QKeyEvent *>(event));
        if (event->isAccepted())
            return true;
        break;
    case QEvent::KeyRelease:
        d->keyReleaseEvent(static_cast<QKeyEvent *>(event));
        if (event->isAccepted())
            return true;
        break;
    case QEvent::MouseButtonPress:
        d->mousePressEvent(static_cast<QMouseEvent *>(event));
        break;
    case QEvent::MouseButtonDblClick:
        d->mouseDoubleClickEvent(static_cast<QMouseEvent *>(event));
        break;
    case QEvent::MouseButtonRelease:
        d->mouseReleaseEvent(static_cast<QMouseEvent *>(event));
        break;
    case QEvent::MouseMove:
        d->useLastMouseEvent = false;
        d->mouseMoveEvent(static_cast<QMouseEvent *>(event));
        break;
    case QEvent::Resize:
        d->recalculateContentSize();
        centerOn(QPointF(d->lastCenterPoint));
        break;
    case QEvent::Leave:
        d->useLastMouseEvent = false;
        break;
    case QEvent::FocusIn:
        d->focusInEvent(static_cast<QFocusEvent *>(event));
        break;
    case QEvent::FocusOut:
        d->focusOutEvent(static_cast<QFocusEvent *>(event));
        break;
    default:
        break;
    }

    return false;
}

/*!
    \reimp
*/
bool QGraphicsView::event(QEvent *event)
{
    return QAbstractScrollArea::event(event);
}

/*!
    \reimp
*/
void QGraphicsView::scrollContentsBy(int dx, int dy)
{
    Q_D(QGraphicsView);
    if (d->accelerateScrolling)
        d->renderWidget->scroll(dx, dy);
    else
        d->renderWidget->update();
    d->updateLastCenterPoint();
}

/*!
    Paints the background of the view using \a painter, before any items are
    painted. Reimplement this function to provide a custom background for the
    view.  The default implementation fills the background with the brush from
    backgroundBrush(). If you simply want to set a background color, gradient
    or texture, you can call setBackgroundBrush() instead of reimplementing
    this function.

    \a painter is pre-transformed with the view matrix; any painting is done in
    \e scene coordinates. \a rect is the exposed rectangle, also in scene
    coordinates.

    \sa paintForeground(), paintItems()
*/
void QGraphicsView::paintBackground(QPainter *painter, const QRectF &rect)
{
    Q_D(QGraphicsView);
    if (backgroundBrush().style() != Qt::NoBrush) {
        painter->save();
        painter->setBrushOrigin(0, 0);
        painter->fillRect(rect, backgroundBrush());
        painter->restore();
    } else if (d->renderWidget->inherits("QGLWidget")) {
        painter->fillRect(rect, d->renderWidget->palette().brush(d->renderWidget->backgroundRole()));
    }
}

/*!
    Paints the foreground of the view using \a painter, after the background
    and all items are painted. Reimplement this function to provide a custom
    foreground for the view.  The default implementation fills the background
    with the brush from foregroundBrush(). If you simply want to set a
    foreground color, gradient or texture, you can call setForegroundBrush()
    instead of reimplementing this function.

    \a painter is pre-transformed with the view matrix; any painting is done in
    \e scene coordinates. \a rect is the exposed rectangle, also in scene
    coordinates.

    \sa paintBackground(), paintItems()
*/
void QGraphicsView::paintForeground(QPainter *painter, const QRectF &rect)
{
    if (foregroundBrush().style() != Qt::NoBrush) {
        painter->save();
        painter->setBrushOrigin(0, 0);
        painter->fillRect(rect, foregroundBrush());
        painter->restore();
    }
}

/*!
    Paints the items in \a items using \a painter, after the background has
    been drawn, and before the foreground has been drawn. Reimplement this
    function to provide custom painting of all items. The default
    implementation prepares the painter matrix, and calls
    QGraphicsItem::paint() on all items.

    \a painter is pre-transformed with the view matrix; any painting is done
    in \e scene coordinates. Before drawing each item, the painter must be
    transformed using QGraphicsItem::sceneMatrix().
    
    By reimplementing this function, you gain complete control over how each
    item is drawn, and in some cases this can increase drawing performance
    significantly.

    Example:

    \code
        void CustomView::paintItems(QPainter *painter,
                                    const QList<QGraphicsItem *> items)
        {
            foreach (QGraphicsItem *item, items) {
                // Create the styleoption object
                QStyleOptionGraphicsItem option = styleOptionForItem(item);

                // Draw the item
                painter->save();
                painter->setMatrix(item->sceneMatrix(), true);
                item->paint(painter, &option, renderWidget());
                painter->restore();
            }
        }
    \endcode

    \sa styleOptionForItem(), paintBackground(), paintForeground()
*/
void QGraphicsView::paintItems(QPainter *painter, const QList<QGraphicsItem *> items)
{
    Q_D(QGraphicsView);
    QMatrix painterMatrix = painter->deviceMatrix();

    foreach (QGraphicsItem *item, items) {
        // Create the styleoption object
        QStyleOptionGraphicsItem option = styleOptionForItem(item);

        QMatrix itemMatrix = item->sceneMatrix();
        QMatrix transformationMatrix = itemMatrix * painterMatrix;

        // Calculate a simple level-of-detail metric.
        QRectF mappedRect = transformationMatrix.mapRect(QRectF(0, 0, 1, 1));
        qreal dx = transformationMatrix.mapRect(QRectF(0, 0, 1, 1)).size().width();
        qreal dy = transformationMatrix.mapRect(QRectF(0, 0, 1, 1)).size().height();
        option.levelOfDetail = qMin(dx, dy);

        // Also pass the device matrix, so the item can do more advanced
        // level-of-detail calculations.
        option.matrix = transformationMatrix;

        // Draw the item
        painter->save();
        painter->setMatrix(itemMatrix, true);
        item->paint(painter, &option, d->renderWidget);
        painter->restore();
    }
}

/*!
    Returns a style option object for the item \a item. This is a convenience
    function, provided for those who reimplement paintItems().

    \sa paintItems()
*/
QStyleOptionGraphicsItem QGraphicsView::styleOptionForItem(QGraphicsItem *item) const
{
    Q_D(const QGraphicsView);
    QStyleOptionGraphicsItem option;
    option.state = QStyle::State_None;
    option.rect = item->boundingRect().toRect();
    if (item->isSelected())
        option.state |= QStyle::State_Selected;
    if (item->isEnabled())
        option.state |= QStyle::State_Enabled;
    if (item->hasFocus())
        option.state |= QStyle::State_HasFocus;
    if (d->scene->d_func()->hoverItems.contains(item))
        option.state |= QStyle::State_MouseOver;
    if (item == d->scene->mouseGrabberItem())
        option.state |= QStyle::State_Sunken;
    return option;
}
