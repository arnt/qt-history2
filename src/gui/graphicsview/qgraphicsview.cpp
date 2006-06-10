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

//#define QGRAPHICSVIEW_DEBUG

// Constants
static const int GraphicsViewRegionRectThreshold = 20;

/*!
    \class QGraphicsView
    \brief The QGraphicsView class provides a widget for displaying the
    contents of a QGraphicsScene.
    \since 4.2
    \ingroup multimedia

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
    certain area is visible, (but not necessarily centered,) you can call
    ensureVisible() instead.

    QGraphicsView can be used to visualize a whole scene, or only parts of it.
    The visualized area is by default detected automatically when the view is
    displayed for the first time (by calling
    QGraphicsScene::itemsBoundingRect()). To set the visualized area rectangle
    yourself, you can call setSceneRect(). This will adjust the scrollbars'
    ranges appropriately.

    QGraphicsView visualizes the scene by calling render(). By default, the
    items are drawn onto the viewport by using a regular QPainter, and using
    default render hints. To change the default render hints that
    QGraphicsView passes to QPainter when painting items, you can call
    setRenderHints().

    By default, QGraphicsView provides a regular QWidget for the viewport
    widget. You can access this widget by calling viewport(), or you can
    replace it by calling setViewport(). To render using OpenGL, simply call
    setViewport(new QGLWidget). QGraphicsView takes ownership of the viewport
    widget.

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
    interaction is enabled by default, and you can toggle it by calling
    setInteractive().

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
    \enum QGraphicsView::CacheModeFlag

    This enum describes the flags that you can set for a QGraphicsView's cache
    mode.

    \value CacheNone All painting is done directly onto the viewport.

    \value CacheBackground The background is cached. This affects both custom
    backgrounds, and backgrounds based on the backgroundBrush property. When
    this flag is enabled, QGraphicsView will allocate one pixmap with the full
    size of the viewport.

    \sa cacheMode
*/

/*!
    \enum QGraphicsView::DragMode

    This enum describes the default action for the view when pressing and
    dragging the mouse over the viewport.

    \value NoDrag Nothing happens; the mouse event is ignored.

    \value ScrollHandDrag The cursor changes into a pointing hand, and
    dragging the mouse around will scroll the scrolbars.

    \value RubberBandDrag A rubber band will appear. Dragging the mouse will
    set the rubber band geometry, and all items covered by the rubber band are
    selected.

    \sa dragMode, QGraphicsScene::setSelectionArea()
*/

#include "qgraphicsview.h"

#ifndef QT_NO_GRAPHICSVIEW

#include "qgraphicsitem.h"
#include "qgraphicsscene.h"
#include "qgraphicsscene_p.h"
#include "qgraphicssceneevent.h"

#include <QtCore/qdatetime.h>
#include <QtCore/qdebug.h>
#include <QtGui/qapplication.h>
#include <QtGui/qdesktopwidget.h>
#include <QtGui/qevent.h>
#include <QtGui/qlayout.h>
#include <QtGui/qmatrix.h>
#include <QtGui/qpainter.h>
#include <QtGui/qrubberband.h>
#include <QtGui/qscrollbar.h>
#include <QtGui/qstyleoption.h>
#include <private/qabstractscrollarea_p.h>

class QGraphicsViewPrivate : public QAbstractScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsView)
public:
    QGraphicsViewPrivate();

    void recalculateContentSize();

    QPainter::RenderHints renderHints;

    QGraphicsView::DragMode dragMode;
    bool sceneInteractionAllowed;
    QRectF sceneRect;
    bool hasSceneRect;
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
    Qt::Alignment alignment;

    QGraphicsScene *scene;
    QRubberBand *rubberBand;
    bool rubberBanding;
    bool handScrolling;

    QGraphicsView::CacheMode cacheMode;

    QBrush backgroundBrush;
    QBrush foregroundBrush;
    QPixmap backgroundPixmap;
    bool mustResizeBackgroundPixmap;
    QRegion backgroundPixmapExposed;

#ifndef QT_NO_CURSOR
    QCursor viewCursor;
    bool hasViewCursor;
#endif

    QGraphicsSceneDragDropEvent *lastDragDropEvent;
    void storeDragDropEvent(const QGraphicsSceneDragDropEvent *event);
    void populateSceneDragDropEvent(QGraphicsSceneDragDropEvent *dest,
                                    QDropEvent *source);
};

/*!
    \internal
*/
QGraphicsViewPrivate::QGraphicsViewPrivate()
    : renderHints(QPainter::TextAntialiasing | QPainter::Antialiasing),
      dragMode(QGraphicsView::NoDrag),
      sceneInteractionAllowed(true), hasSceneRect(false), accelerateScrolling(true),
      leftIndent(0), topIndent(0),
      lastMouseEvent(QEvent::None, QPoint(), Qt::NoButton, 0, 0),
      useLastMouseEvent(false), alignment(Qt::AlignCenter),
      scene(0), rubberBand(0), rubberBanding(false),
      handScrolling(false), cacheMode(0), mustResizeBackgroundPixmap(true),
#ifndef QT_NO_CURSOR
      hasViewCursor(false),
#endif
      lastDragDropEvent(0)
{
}

/*!
    \internal
*/
void QGraphicsViewPrivate::recalculateContentSize()
{
    Q_Q(QGraphicsView);

    int width = q->viewport()->width();
    int height = q->viewport()->height();
    int margin = 4;
    QRectF viewRect = matrix.mapRect(q->sceneRect());

    // Setting the ranges of these scroll bars can/will cause the values to
    // change, and scrollContentsBy() will be called correspondingly. This
    // will reset the last center point.
    QPointF savedLastCenterPoint = lastCenterPoint;

    // Remember the former indent settings
    qreal oldLeftIndent = leftIndent;
    qreal oldTopIndent = topIndent;

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
    } else {
        q->verticalScrollBar()->setRange(int(viewRect.top() - margin),
                                         int(viewRect.bottom() - height + margin));
        q->verticalScrollBar()->setPageStep(height);
        q->verticalScrollBar()->setSingleStep(height / 20);
        topIndent = 0;
    }

    // Restorethe center point from before the ranges changed.
    lastCenterPoint = savedLastCenterPoint;

    // Issue a full update if the indents change
    if (oldLeftIndent != leftIndent || oldTopIndent != topIndent)
        q->viewport()->update();

    if (cacheMode & QGraphicsView::CacheBackground) {
        // Invalidate the background pixmap
        mustResizeBackgroundPixmap = true;
    }
}

/*!
    \internal
*/
void QGraphicsViewPrivate::updateLastCenterPoint()
{
    Q_Q(QGraphicsView);
    lastCenterPoint = q->mapToScene(q->mapFromScene(q->mapToScene(q->viewport()->rect().center())));
}

/*!
    \internal
*/
void QGraphicsViewPrivate::replayLastMouseEvent()
{
    Q_Q(QGraphicsView);
    if (!useLastMouseEvent)
        return;

    QMouseEvent *mouseEvent = new QMouseEvent(QEvent::MouseMove, lastMouseEvent.pos(), lastMouseEvent.globalPos(),
                                              lastMouseEvent.button(), lastMouseEvent.buttons(), lastMouseEvent.modifiers());
    QApplication::postEvent(q->viewport(), mouseEvent);
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
    \internal
*/
void QGraphicsViewPrivate::storeDragDropEvent(const QGraphicsSceneDragDropEvent *event)
{
    delete lastDragDropEvent;
    lastDragDropEvent = new QGraphicsSceneDragDropEvent(event->type());
    lastDragDropEvent->setScenePos(event->scenePos());
    lastDragDropEvent->setScreenPos(event->screenPos());
    lastDragDropEvent->setButtons(event->buttons());
    lastDragDropEvent->setModifiers(event->modifiers());
    lastDragDropEvent->setPossibleActions(event->possibleActions());
    lastDragDropEvent->setProposedAction(event->proposedAction());
    lastDragDropEvent->setDropAction(event->dropAction());
    lastDragDropEvent->setMimeData(event->mimeData());
    lastDragDropEvent->setWidget(event->widget());
    lastDragDropEvent->setSource(event->source());
}

/*!
    \internal
*/
void QGraphicsViewPrivate::populateSceneDragDropEvent(QGraphicsSceneDragDropEvent *dest,
                                                      QDropEvent *source)
{
#ifndef QT_NO_DRAGANDDROP
    Q_Q(QGraphicsView);
    dest->setScenePos(q->mapToScene(source->pos()));
    dest->setScreenPos(q->mapToGlobal(source->pos()));
    dest->setButtons(source->mouseButtons());
    dest->setModifiers(source->keyboardModifiers());
    dest->setPossibleActions(source->possibleActions());
    dest->setProposedAction(source->proposedAction());
    dest->setDropAction(source->dropAction());
    dest->setMimeData(source->mimeData());
    dest->setWidget(q->viewport());
    dest->setSource(source->source());
#else
    Q_UNUSED(dest)
    Q_UNUSED(source)
#endif
}

/*!
    Constructs a QGraphicsView and sets the visualized scene to \a
    scene. \a parent is passed to QWidget's constructor.
*/
QGraphicsView::QGraphicsView(QGraphicsScene *scene, QWidget *parent)
    : QAbstractScrollArea(*new QGraphicsViewPrivate, parent)
{
    setScene(scene);
    setViewport(0);
    setAcceptDrops(true);
    setBackgroundRole(QPalette::Base);
    setAttribute(Qt::WA_InputMethodEnabled);
}

/*!
    Destructs the QGraphicsView object.
*/
QGraphicsView::~QGraphicsView()
{
    Q_D(QGraphicsView);
    delete d->lastDragDropEvent;
}

/*!
    \reimp
*/
QSize QGraphicsView::sizeHint() const
{
    Q_D(const QGraphicsView);
    if (d->scene) {
        return d->matrix.inverted().mapRect(sceneRect())
            .size()
            .boundedTo((3 * QApplication::desktop()->size()) / 4)
            .toSize();
    }
    return QAbstractScrollArea::sizeHint();
}

/*!
    \property QGraphicsView::renderHints
    \brief the default render hints for the view

    These hints are
    used to initialize QPainter before each visible item is drawn. QPainter
    uses render hints to toggle rendering features such as antialiasing and
    smooth pixmap transformation.

    The render hints QPainter::TextAntialiasing and QPainter::Antialiasing are
    enabled by default.

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
    viewport()->update();
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
    \property QGraphicsView::dragMode
    \brief the behavior for dragging the mouse over the scene while
    the left mouse button is pressed.

    This property defines what should happen when the user clicks on the scene
    background and drags the mouse (e.g., scrolling the viewport contents
    using a pointing hand cursor, or selecting multiple items with a rubber
    band). The default value, NoDrag, does nothing.

    This behavior only affects mouse clicks that are not handled by any item.
    You can define a custom behavior by creating a subclass of QGraphicsView
    and reimplementing mouseMoveEvent().
*/
QGraphicsView::DragMode QGraphicsView::dragMode() const
{
    Q_D(const QGraphicsView);
    return d->dragMode;
}
void QGraphicsView::setDragMode(DragMode mode)
{
    Q_D(QGraphicsView);
    d->dragMode = mode;
#ifndef QT_NO_CURSOR
    if (d->dragMode == ScrollHandDrag) {
        d->hasViewCursor = true;
        d->viewCursor = QCursor(Qt::OpenHandCursor);
        viewport()->setCursor(d->viewCursor);
    } else {
        unsetCursor();
    }
#endif
}

/*!
    \property QGraphicsView::cacheMode
    \brief which parts of the view are cached

    QGraphicsView can cache pre-rendered content in a QPixmap, which is then
    drawn onto the viewport. The purpose of such cacheing is to speed up the
    total rendering time for areas that are slow to render.  Texture, gradient
    and alpha blended backgrounds, for example, can be notibly slow to render;
    especially with a transformed view. The CacheBackground flag enables
    cacheing of the view's background. For example:

    \code
        QGraphicsView view;
        view.setBackgroundBrush(":/images/backgroundtile.png");
        view.setCacheMode(QGraphicsView::CacheBackground);
    \endcode

    The cache is invalidated every time the view is transformed. However, when
    scrolling, only partial invalidation is required.

    By default, nothing is cached.

    \sa QPixmapCache
*/
QGraphicsView::CacheMode QGraphicsView::cacheMode() const
{
    Q_D(const QGraphicsView);
    return d->cacheMode;
}
void QGraphicsView::setCacheMode(CacheMode mode)
{
    Q_D(QGraphicsView);
    CacheMode oldCacheMode = d->cacheMode;
    d->cacheMode = mode;
    if (d->cacheMode & CacheBackground) {
        if (!(oldCacheMode & CacheBackground)) {
            // Background cacheing is enabled.
            d->mustResizeBackgroundPixmap = true;
        }
    } else {
        if (oldCacheMode & CacheBackground) {
            // Background cacheing is disabled.
            // Cleanup, free some resources.
            d->mustResizeBackgroundPixmap = false;
            d->backgroundPixmap = QPixmap();
            d->backgroundPixmapExposed = QRegion();
        }
    }
}

/*!
    \property QGraphicsView::interactive
    \brief whether the view allowed scene interaction.

    If enabled, this view is set to allow scene interaction. Otherwise, this
    view will not allow interaction, and any mouse or key events are ignored
    (i.e., it will act as a read-only view).
*/
bool QGraphicsView::isInteractive() const
{
    Q_D(const QGraphicsView);
    return d->sceneInteractionAllowed;
}
void QGraphicsView::setInteractive(bool allowed)
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
    is automatically connected to this view's updateScene() slot, and the
    view's scrollbars are adjusted to fit the size of the scene.
*/
void QGraphicsView::setScene(QGraphicsScene *scene)
{
    Q_D(QGraphicsView);
    if (d->scene == scene)
        return;

    if (d->scene) {
        disconnect(d->scene, SIGNAL(changed(const QList<QRectF> &)),
                   this, SLOT(updateScene(const QList<QRectF> &)));
        disconnect(d->scene, SIGNAL(sceneRectChanged(const QRectF &)),
                   this, SLOT(updateSceneRect(const QRectF &)));
        d->scene->d_func()->views.removeAll(this);
    }

    if ((d->scene = scene)) {
        connect(d->scene, SIGNAL(changed(const QList<QRectF> &)),
                this, SLOT(updateScene(const QList<QRectF> &)));
        connect(d->scene, SIGNAL(sceneRectChanged(const QRectF &)),
                this, SLOT(updateSceneRect(const QRectF &)));
        d->scene->d_func()->views << this;
        d->recalculateContentSize();
        d->lastCenterPoint = sceneRect().center();
    }
}

/*!
    \property QGraphicsView::sceneRect
    \brief the area of the scene visualized by this view.

    The scene rect defines the extent of the scene, and in the view's case,
    this means the area of the scene that you can navigate using the scroll
    bars.

    If unset, this property has the same value as QGraphicsScene::sceneRect().

    Note: The maximum size of the view's scene rect is limited by the range of
    QAbstractScrollArea's scrollbars, which operate in integer coordinates.

    \sa QGraphicsScene::sceneRect
*/
QRectF QGraphicsView::sceneRect() const
{
    Q_D(const QGraphicsView);
    if (d->hasSceneRect)
        return d->sceneRect;
    if (d->scene)
        return d->scene->sceneRect();
    return QRectF();
}
void QGraphicsView::setSceneRect(const QRectF &rect)
{
    Q_D(QGraphicsView);
    d->hasSceneRect = !rect.isNull();
    d->sceneRect = rect;
    d->recalculateContentSize();
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

    // Any matrix operation requires a full update.
    viewport()->update();
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
    qreal width = viewport()->width();
    qreal height = viewport()->height();
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
    Scrolls the contents of the viewport so that the scene rectangle \a rect
    is visible, with margins specified in pixels by \a xmargin and \a
    ymargin. If the specified rect cannot be reached, the contents are
    scrolled to the nearest valid position. The default value for both margins
    is 50 pixels.

    \sa centerOn()
*/
void QGraphicsView::ensureVisible(const QRectF &rect, int xmargin, int ymargin)
{
    Q_D(QGraphicsView);
    Q_UNUSED(xmargin);
    Q_UNUSED(ymargin);
    qreal width = viewport()->width();
    qreal height = viewport()->height();
    QRectF viewRect = d->matrix.mapRect(rect);

    qreal left = d->leftIndent ? d->leftIndent : horizontalScrollBar()->value();
    qreal right = left + width;
    qreal top = d->topIndent ? d->topIndent : verticalScrollBar()->value();
    qreal bottom = top + height;

    if (viewRect.left() <= left + xmargin) {
        // need to scroll from the left
        if (!d->leftIndent)
            horizontalScrollBar()->setValue(int(viewRect.left() - xmargin - 0.5));
    }
    if (viewRect.right() >= right - xmargin) {
        // need to scroll from the right
        if (!d->leftIndent)
            horizontalScrollBar()->setValue(int(viewRect.right() - width + xmargin + 0.5));
    }
    if (viewRect.top() <= top + ymargin) {
        // need to scroll from the top
        if (!d->topIndent)
            verticalScrollBar()->setValue(int(viewRect.top() - ymargin - 0.5));
    }
    if (viewRect.bottom() >= bottom - ymargin) {
        // need to scroll from the bottom
        if (!d->topIndent)
            verticalScrollBar()->setValue(int(viewRect.bottom() - height + ymargin + 0.5));
    }
}

/*!
    \fn QGraphicsView::ensureVisible(qreal x, qreal y, qreal w, qreal h,
    int xmargin, int ymargin)
    \overload

    This function is provided for convenience. It's equivalent to calling
    ensureVisible(QRectF(\a x, \a y, \a w, \a h), \a xmargin, \a ymargin).
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
    ensureVisible(item->sceneBoundingRect(), xmargin, ymargin);
}

/*!
    Scales the view matrix and scrolls the scroll bars to ensures that the
    scene rectangle \a rect fits inside the view.

    This function keeps the view's rotation, translation, or shear. The view
    is scaled according to \a aspectRatioMode. \a rect will be centered in the
    view if it does not fit tightly.

    \sa setMatrix(), ensureVisible(), centerOn()
*/
void QGraphicsView::fitInView(const QRectF &rect, Qt::AspectRatioMode aspectRatioMode)
{
    Q_D(QGraphicsView);
    if (!d->scene || rect.isNull())
        return;

    // Reset the view scale to 1:1.
    QRectF unity = d->matrix.mapRect(QRectF(0, 0, 1, 1));
    scale(1 / unity.width(), 1 / unity.height());

    // Find the ideal x / y scaling ratio to fit \a rect in the view.
    int margin = 2;
    QRectF viewRect = viewport()->rect().adjusted(margin, margin, -margin, -margin);
    QRectF sceneRect = d->matrix.mapRect(rect);
    qreal xratio = viewRect.width() / sceneRect.width();
    qreal yratio = viewRect.height() / sceneRect.height();

    // Respect the aspect ratio mode.
    switch (aspectRatioMode) {
    case Qt::KeepAspectRatio:
        xratio = yratio = qMin(xratio, yratio);
        break;
    case Qt::KeepAspectRatioByExpanding:
        xratio = yratio = qMax(xratio, yratio);
        break;
    case Qt::IgnoreAspectRatio:
        break;
    }

    // Scale and center on the center of \a rect.
    scale(xratio, yratio);
    centerOn(rect.center());
}

/*!
    \fn void QGraphicsView::fitInView(qreal x, qreal y, qreal w, qreal h,
    Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio)

    \overload

    This convenience function is equivalent to calling
    fitInView(QRectF(\a x, \a y, \a w, \a h), \a aspectRatioMode).

    \sa ensureVisible(), centerOn()
*/

/*!
    \overload

    Ensures that \a item fits tightly inside the view, scaling the view
    according to \a aspectRatioMode.

    \sa ensureVisible(), centerOn()
*/
void QGraphicsView::fitInView(const QGraphicsItem *item, Qt::AspectRatioMode aspectRatioMode)
{
    fitInView(item->sceneMatrix().map(item->shape()).boundingRect(), aspectRatioMode);
}

/*!
    Renders the \a source rect, which is in view coordinates, from the scene
    into \a target, which is in paint device coordinates, using \a
    painter. This function is useful for capturing the contents of the view
    onto a paint device, such as a QImage (e.g., to take a screenshot), or for
    printing to QPrinter. For example:

    \code
        QGraphicsScene scene;
        scene.addItem(...
        ...

        QGraphicsView view(&scene);
        view.show();
        ...

        QPrinter printer(QPrinter::HighResolution);
        printer.setPageSize(QPrinter::A4);
        QPainter painter(&printer);

        // print, fitting the viewport contents into a full page
        view.render(&painter);

        // print the upper half of the viewport into the lower.
        // half of the page.
        QRect viewport = view.viewport()->rect();
        view.render(&painter,
                    QRectF(0, printer.height() / 2,
                           printer.width(), printer.height() / 2),
                    viewport.adjusted(0, 0, 0, -viewport.height() / 2));

    \endcode

    If \a source is a null rect, this function will use viewport()->rect() to
    determine what to draw. If \a target is a null rect, the full dimensions
    of \a painter's paint device (e.g., for a QPrinter, the page size) will be
    used.

    The source rect contents will be transformed according to \a
    aspectRatioMode to fit into the target rect. By default, the aspect ratio
    is ignored, and \a source is scaled to fit tightly in \a target.

    \sa QGraphicsScene::render()
*/
void QGraphicsView::render(QPainter *painter, const QRectF &target, const QRect &source,
                           Qt::AspectRatioMode aspectRatioMode)
{
    Q_D(QGraphicsView);
    if (!d->scene)
        return;

    // Default source rect = viewport rect
    QRect sourceRect = source;
    if (source.isNull())
        sourceRect = viewport()->rect();

    // Default target rect = device rect
    QRectF targetRect = target;
    if (target.isNull())
        targetRect.setRect(0, 0, painter->device()->width(), painter->device()->height());

    // Find the ideal x / y scaling ratio to fit \a source into \a target.
    qreal xratio = targetRect.width() / sourceRect.width();
    qreal yratio = targetRect.height() / sourceRect.height();

    // Scale according to the aspect ratio mode.
    switch (aspectRatioMode) {
    case Qt::KeepAspectRatio:
        xratio = yratio = qMin(xratio, yratio);
        break;
    case Qt::KeepAspectRatioByExpanding:
        xratio = yratio = qMax(xratio, yratio);
        break;
    case Qt::IgnoreAspectRatio:
        break;
    }

    // Find all items to draw, and reverse the list (we want to draw
    // in reverse order).
    QList<QGraphicsItem *> itemList = d->scene->items(mapToScene(sourceRect));
    if (!itemList.isEmpty()) {
        QGraphicsItem **a = &itemList.first();
        QGraphicsItem **b = &itemList.last();
        QGraphicsItem *tmp = 0;
        while (a < b) {
            tmp = *a;
            *a = *b;
            *b = tmp;
            ++a; --b;
        }
    }

    // Setup painter
    QMatrix moveMatrix;
    moveMatrix.translate(-horizontalScrollBar()->value() + d->leftIndent,
                         -verticalScrollBar()->value() + d->topIndent);
    QMatrix painterMatrix = d->matrix * moveMatrix;

    // Generate the style options
    QList<QStyleOptionGraphicsItem> styleOptions;
    for (int i = 0; i < itemList.size(); ++i) {
        QGraphicsItem *item = itemList.at(i);

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

        // Calculate a simple level-of-detail metric.
        QMatrix neo = item->sceneMatrix() * painterMatrix;
        QRectF mappedRect = neo.mapRect(QRectF(0, 0, 1, 1));
        qreal dx = neo.mapRect(QRectF(0, 0, 1, 1)).size().width();
        qreal dy = neo.mapRect(QRectF(0, 0, 1, 1)).size().height();
        option.levelOfDetail = qMin(dx, dy);
        option.matrix = neo;

        option.exposedRect = item->boundingRect();
        option.exposedRect &= neo.inverted().mapRect(targetRect);

        styleOptions << option;
    }

    painter->save();

    // Transform the painter.
    painter->setClipRect(targetRect);
    painter->setMatrix(painterMatrix
                       * QMatrix().translate(targetRect.left(), targetRect.top())
                       * QMatrix().scale(xratio, yratio)
                       * QMatrix().translate(-sourceRect.left(), -sourceRect.top()));
    QPainterPath path;
    path.addPolygon(mapToScene(sourceRect));
    painter->setClipPath(path);

    // Render the scene.
    d->scene->drawBackground(painter, sourceRect);
    d->scene->drawItems(painter, itemList, styleOptions);
    d->scene->drawForeground(painter, sourceRect);

    painter->restore();
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
    \reimp
*/
QVariant QGraphicsView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    Q_D(const QGraphicsView);
    QVariant value = d->scene->inputMethodQuery(query);
    if (value.type() == QVariant::RectF)
        value = mapFromScene(value.toRectF()).boundingRect();
    else if (value.type() == QVariant::PointF)
        value = mapFromScene(value.toPointF());
    else if (value.type() == QVariant::Rect)
        value = mapFromScene(value.toRect()).boundingRect();
    else if (value.type() == QVariant::Point)
        value = mapFromScene(value.toPoint());
    return value;
}

/*!
    \property QGraphicsView::backgroundBrush
    \brief the background brush of the scene.

    This property sets the background brush for the scene in this view. It is
    used to override the scene's own background, and defines the behavior of
    drawBackground(). To provide custom background drawing for this view, you
    can reimplement drawBackground() instead.

    \sa QGraphicsScene::backgroundBrush, foregroundBrush
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
    update();
}

/*!
    \property QGraphicsView::foregroundBrush
    \brief the foreground brush of the scene.

    This property sets the foreground brush for the scene in this view. It is
    used to override the scene's own foreground, and defines the behavior of
    drawForeground(). To provide custom background drawing for this view, you
    can reimplement foreBackground() instead.

    \sa QGraphicsScene::foregroundBrush, backgroundBrush
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
    viewport()->update();
}

/*!
    Schedules an update of the scene rectangles \a rects.

    \sa QGraphicsScene::changed()
*/
void QGraphicsView::updateScene(const QList<QRectF> &rects)
{
    Q_D(QGraphicsView);
    QRect viewportRect = viewport()->rect();

    QRegion updateRegion;
    QRect sumRect;
    foreach (QRectF rect, rects) {
        // Find the item's bounding rect and map it to view coordiates.
        // Adjust with 2 pixels for antialiasing.
        QRect mappedRect = mapFromScene(rect).boundingRect().adjusted(-2, -2, 2, 2);
        if (viewportRect.contains(mappedRect) || viewportRect.intersects(mappedRect)) {
            if (rects.size() < GraphicsViewRegionRectThreshold)
                updateRegion += mappedRect;
            sumRect |= mappedRect;
            if (!d->accelerateScrolling)
                break;
        }
    }

    if (!updateRegion.isEmpty())
        viewport()->update(!d->accelerateScrolling ? QRegion(viewportRect) : updateRegion);
    else if (!sumRect.isNull())
        viewport()->update(!d->accelerateScrolling ? viewportRect : sumRect);
}

/*!
    Notifies QGraphicsView that the scene's scene rect has changed.  \a rect
    is the new scene rect.

    \sa sceneRect, QGraphicsScene::sceneRectChanged()
*/
void QGraphicsView::updateSceneRect(const QRectF &rect)
{
    Q_D(QGraphicsView);
    d->sceneRect = rect;
    d->recalculateContentSize();
}

/*!
    This slot is called by QAbstractScrollArea after setViewport() has been
    called. Reimplement this function in a subclass of QGraphicsView to
    initialize the new viewport \a widget before it is used.

    \sa setViewport()
*/
void QGraphicsView::setupViewport(QWidget *widget)
{
    Q_D(QGraphicsView);

    if (!widget) {
        qWarning("QGraphicsView::setupViewport: cannot initialize null widget");
        return;
    }

    d->accelerateScrolling = !widget->inherits("QGLWidget");

    widget->setFocusPolicy(Qt::StrongFocus);

    // autoFillBackground enables scroll acceleration.
    widget->setAutoFillBackground(true);

    if (d->scene) {
        d->recalculateContentSize();
        centerOn(d->lastCenterPoint);
    }

    widget->setMouseTracking(true);
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
bool QGraphicsView::viewportEvent(QEvent *event)
{
    Q_D(QGraphicsView);

    switch (event->type()) {
    case QEvent::Leave:
#ifndef QT_NO_CURSOR
        if (d->hasViewCursor && !d->scene->mouseGrabberItem()) {
            d->hasViewCursor = false;
            viewport()->setCursor(d->viewCursor);
        }
#endif
        break;
#ifndef QT_NO_TOOLTIP
    case QEvent::ToolTip: {
        QHelpEvent *toolTip = static_cast<QHelpEvent *>(event);
        QGraphicsSceneHelpEvent helpEvent(QEvent::GraphicsSceneHelp);
        helpEvent.setScreenPos(toolTip->globalPos());
        helpEvent.setScenePos(mapToScene(toolTip->pos()));
        QApplication::sendEvent(d->scene, &helpEvent);
        break;
    }
#endif
    default:
        break;
    }

    return QAbstractScrollArea::viewportEvent(event);
}

/*!
    \reimp
*/
void QGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
    Q_D(QGraphicsView);
    if (!d->scene || !d->sceneInteractionAllowed)
        return;

    d->mousePressViewPoint = event->pos();
    d->mousePressScenePoint = mapToScene(d->mousePressViewPoint);
    d->mousePressScreenPoint = event->globalPos();
    d->lastMouseMoveScenePoint = d->mousePressScenePoint;

    QGraphicsSceneContextMenuEvent contextEvent(QEvent::GraphicsSceneContextMenu);
    contextEvent.setWidget(viewport());
    contextEvent.setScenePos(d->mousePressScenePoint);
    contextEvent.setScreenPos(d->mousePressScreenPoint);
    contextEvent.setModifiers(event->modifiers());
    contextEvent.setReason(static_cast<QGraphicsSceneContextMenuEvent::Reason>(event->reason()));
    QApplication::sendEvent(d->scene, &contextEvent);
}

/*!
    \reimp
*/
void QGraphicsView::dropEvent(QDropEvent *event)
{
#ifndef QT_NO_DRAGANDDROP
    Q_D(QGraphicsView);
    if (!d->scene || !d->sceneInteractionAllowed)
        return;

    // Generate a scene event.
    QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDrop);
    d->populateSceneDragDropEvent(&sceneEvent, event);

    // Send it to the scene.
    QApplication::sendEvent(d->scene, &sceneEvent);

    // Accept the originating event if the scene accepted the scene event.
    if (sceneEvent.isAccepted()) {
        event->setAccepted(true);
        event->setDropAction(sceneEvent.dropAction());
    }
#else
    Q_UNUSED(event)
#endif
}

/*!
    \reimp
*/
void QGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
#ifndef QT_NO_DRAGANDDROP
    Q_D(QGraphicsView);
    if (!d->scene || !d->sceneInteractionAllowed)
        return;

    // Generate a scene event.
    QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDragEnter);
    d->populateSceneDragDropEvent(&sceneEvent, event);

    // Store it for later use.
    d->storeDragDropEvent(&sceneEvent);

    // Send it to the scene.
    QApplication::sendEvent(d->scene, &sceneEvent);

    // Accept the originating event if the scene accepted the scene event.
    if (sceneEvent.isAccepted()) {
        event->setAccepted(true);
        event->setDropAction(sceneEvent.dropAction());
    }
#else
    Q_UNUSED(event)
#endif
}

/*!
    \reimp
*/
void QGraphicsView::dragLeaveEvent(QDragLeaveEvent *event)
{
#ifndef QT_NO_DRAGANDDROP
    Q_D(QGraphicsView);
    if (!d->scene || !d->sceneInteractionAllowed)
        return;
    if (!d->lastDragDropEvent) {
        qWarning("QGraphicsView::dragLeaveEvent: drag leave received before drag enter");
        return;
    }

    // Generate a scene event.
    QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDragLeave);
    sceneEvent.setScenePos(d->lastDragDropEvent->scenePos());
    sceneEvent.setScreenPos(d->lastDragDropEvent->screenPos());
    sceneEvent.setButtons(d->lastDragDropEvent->buttons());
    sceneEvent.setModifiers(d->lastDragDropEvent->modifiers());
    sceneEvent.setPossibleActions(d->lastDragDropEvent->possibleActions());
    sceneEvent.setProposedAction(d->lastDragDropEvent->proposedAction());
    sceneEvent.setDropAction(d->lastDragDropEvent->dropAction());
    sceneEvent.setMimeData(d->lastDragDropEvent->mimeData());
    sceneEvent.setWidget(d->lastDragDropEvent->widget());
    sceneEvent.setSource(d->lastDragDropEvent->source());
    delete d->lastDragDropEvent;
    d->lastDragDropEvent = 0;

    // Send it to the scene.
    QApplication::sendEvent(d->scene, &sceneEvent);

    // Accept the originating event if the scene accepted the scene event.
    if (sceneEvent.isAccepted())
        event->setAccepted(true);
#else
    Q_UNUSED(event)
#endif
}

/*!
    \reimp
*/
void QGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
#ifndef QT_NO_DRAGANDDROP
    Q_D(QGraphicsView);
    if (!d->scene || !d->sceneInteractionAllowed)
        return;

    // Generate a scene event.
    QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDragMove);
    d->populateSceneDragDropEvent(&sceneEvent, event);

    // Store it for later use.
    d->storeDragDropEvent(&sceneEvent);

    // Send it to the scene.
    QApplication::sendEvent(d->scene, &sceneEvent);

    // Ignore the originating event if the scene ignored the scene event.
    if (sceneEvent.isAccepted()) {
        event->setAccepted(true);
        event->setDropAction(sceneEvent.dropAction());
    }
#else
    Q_UNUSED(event)
#endif
}

/*!
    \reimp
*/
void QGraphicsView::focusInEvent(QFocusEvent *event)
{
    Q_D(QGraphicsView);
    QApplication::sendEvent(d->scene, event);
}

/*!
    \reimp
*/
void QGraphicsView::focusOutEvent(QFocusEvent *event)
{
    Q_D(QGraphicsView);
    QApplication::sendEvent(d->scene, event);
}

/*!
    \reimp
*/
void QGraphicsView::keyPressEvent(QKeyEvent *event)
{
    Q_D(QGraphicsView);
    if (!d->scene || !d->sceneInteractionAllowed) {
        QAbstractScrollArea::keyPressEvent(event);
        return;
    }
    QApplication::sendEvent(d->scene, event);
    if (!event->isAccepted())
        QAbstractScrollArea::keyPressEvent(event);
}

/*!
    \reimp
*/
void QGraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(QGraphicsView);
    if (!d->scene || !d->sceneInteractionAllowed)
        return;
    QApplication::sendEvent(d->scene, event);
    if (!event->isAccepted())
        QAbstractScrollArea::keyReleaseEvent(event);
}

/*!
    \reimp
*/
void QGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(QGraphicsView);
    if (!d->scene || !d->sceneInteractionAllowed)
        return;

    d->storeMouseEvent(event);
    d->mousePressViewPoint = event->pos();
    d->mousePressScenePoint = mapToScene(d->mousePressViewPoint);
    d->mousePressScreenPoint = event->globalPos();
    d->lastMouseMoveScenePoint = d->mousePressScenePoint;
    d->mousePressButton = event->button();

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseDoubleClick);
    mouseEvent.setWidget(viewport());
    mouseEvent.setButtonDownScenePos(d->mousePressButton, d->mousePressScenePoint);
    mouseEvent.setButtonDownScreenPos(d->mousePressButton, d->mousePressScreenPoint);
    mouseEvent.setScenePos(mapToScene(d->mousePressViewPoint));
    mouseEvent.setScreenPos(d->mousePressScreenPoint);
    mouseEvent.setLastScenePos(d->lastMouseMoveScenePoint);
    mouseEvent.setLastScreenPos(mapFromScene(d->lastMouseMoveScenePoint));
    mouseEvent.setButtons(event->buttons());
    mouseEvent.setButtons(event->buttons());

    mouseEvent.setButton(event->button());
    mouseEvent.setModifiers(event->modifiers());
    QApplication::sendEvent(d->scene, &mouseEvent);
}

/*!
    \reimp
*/
void QGraphicsView::mousePressEvent(QMouseEvent *event)
{
    Q_D(QGraphicsView);
    if (!d->sceneInteractionAllowed)
        return;

    d->mousePressViewPoint = event->pos();
    d->mousePressScenePoint = mapToScene(d->mousePressViewPoint);
    d->mousePressScreenPoint = event->globalPos();
    d->lastMouseMoveScenePoint = d->mousePressScenePoint;
    d->mousePressButton = event->button();

    d->storeMouseEvent(event);

    if (d->dragMode == QGraphicsView::RubberBandDrag) {
        if (!d->scene) {
            d->rubberBanding = true;
        } else if ((d->rubberBanding = !d->scene->itemAt(d->mousePressScenePoint))) {
            d->scene->clearSelection();
        }
    } else if (d->dragMode == QGraphicsView::ScrollHandDrag
               && event->button() == Qt::LeftButton) {
        if (!d->scene) {
            d->handScrolling = true;
#ifndef QT_NO_CURSOR
            if (!d->hasViewCursor) {
                d->hasViewCursor = true;
                d->viewCursor = viewport()->cursor();
            }
            viewport()->setCursor(Qt::ClosedHandCursor);
#endif
            return;
        }
        if ((d->handScrolling = !d->scene->itemAt(d->mousePressScenePoint))) {
#ifndef QT_NO_CURSOR
            if (!d->hasViewCursor) {
                d->hasViewCursor = true;
                d->viewCursor = viewport()->cursor();
            }
            viewport()->setCursor(Qt::ClosedHandCursor);
#endif
            return;
        }
#ifndef QT_NO_CURSOR
        if (d->hasViewCursor) {
            d->hasViewCursor = false;
            viewport()->setCursor(d->viewCursor);
        }
#endif
    }

    if (!d->scene)
        return;

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
    mouseEvent.setWidget(viewport());
    mouseEvent.setButtonDownScenePos(d->mousePressButton, d->mousePressScenePoint);
    mouseEvent.setButtonDownScreenPos(d->mousePressButton, d->mousePressScreenPoint);
    mouseEvent.setScenePos(d->mousePressScenePoint);
    mouseEvent.setScreenPos(d->mousePressScreenPoint);
    mouseEvent.setLastScenePos(d->lastMouseMoveScenePoint);
    mouseEvent.setLastScreenPos(mapFromScene(d->lastMouseMoveScenePoint));
    mouseEvent.setButtons(event->buttons());
    mouseEvent.setButton(event->button());
    mouseEvent.setModifiers(event->modifiers());
    QApplication::sendEvent(d->scene, &mouseEvent);
}

/*!
    \reimp
*/
void QGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QGraphicsView);
    if (!d->sceneInteractionAllowed)
        return;

    if (d->dragMode == QGraphicsView::RubberBandDrag) {
        if (d->rubberBanding) {
            // Invoke El Rubber
            if (!d->rubberBand) {
                if ((d->mousePressViewPoint - event->pos()).manhattanLength()
                    < QApplication::startDragDistance()) {
                    return;
                }

                d->rubberBand = new QRubberBand(QRubberBand::Rectangle, viewport());
            }
            QRect selectionRect = QRect(d->mousePressViewPoint, event->pos()).normalized();
            d->rubberBand->setGeometry(selectionRect);
            QPainterPath selectionArea;
            selectionArea.addPolygon(mapToScene(selectionRect));
            if (d->scene)
                d->scene->setSelectionArea(selectionArea);
            if (!d->rubberBand->isVisible())
                d->rubberBand->show();
            return;
        }
    } else if (d->dragMode == QGraphicsView::ScrollHandDrag) {
        if (d->handScrolling) {
            QScrollBar *hBar = horizontalScrollBar();
            QScrollBar *vBar = verticalScrollBar();
            QPoint delta = event->pos() - d->lastMouseEvent.pos();
            hBar->setValue(hBar->value() - delta.x());
            vBar->setValue(vBar->value() - delta.y());
        }
    }

    d->storeMouseEvent(event);

    if (d->handScrolling)
        return;

    if (!d->scene)
        return;

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
    mouseEvent.setWidget(viewport());
    mouseEvent.setButtonDownScenePos(d->mousePressButton, d->mousePressScenePoint);
    mouseEvent.setButtonDownScreenPos(d->mousePressButton, d->mousePressScreenPoint);
    mouseEvent.setScenePos(mapToScene(event->pos()));
    mouseEvent.setScreenPos(event->globalPos());
    mouseEvent.setLastScenePos(d->lastMouseMoveScenePoint);
    mouseEvent.setLastScreenPos(mapFromScene(d->lastMouseMoveScenePoint));
    mouseEvent.setButtons(event->buttons());
    mouseEvent.setButton(event->button());
    mouseEvent.setModifiers(event->modifiers());
    d->lastMouseMoveScenePoint = mouseEvent.scenePos();
    QApplication::sendEvent(d->scene, &mouseEvent);

#ifndef QT_NO_CURSOR
    if (QGraphicsItem *item = d->scene->itemAt(mapToScene(event->pos()))) {
        if (!d->hasViewCursor) {
            d->hasViewCursor = true;
            d->viewCursor = viewport()->cursor();
        }
        viewport()->setCursor(item->cursor());
    } else {
        if (d->hasViewCursor) {
            d->hasViewCursor = false;
            viewport()->setCursor(d->viewCursor);
        }
    }
#endif
}

/*!
    \reimp
*/
void QGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QGraphicsView);
    if (!d->sceneInteractionAllowed)
        return;

    if (d->dragMode == QGraphicsView::RubberBandDrag) {
        if (d->rubberBanding) {
            // Withdraw El Rubber
            delete d->rubberBand;
            d->rubberBand = 0;
            d->rubberBanding = false;
            return;
        }
    } else if (d->dragMode == QGraphicsView::ScrollHandDrag) {
#ifndef QT_NO_CURSOR
        if (d->hasViewCursor) {
            d->hasViewCursor = false;
            viewport()->setCursor(d->viewCursor);
        }
#endif
        d->handScrolling = false;
    }

    d->storeMouseEvent(event);

    if (!d->scene)
        return;

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
    mouseEvent.setWidget(viewport());
    mouseEvent.setButtonDownScenePos(d->mousePressButton, d->mousePressScenePoint);
    mouseEvent.setButtonDownScreenPos(d->mousePressButton, d->mousePressScreenPoint);
    mouseEvent.setScenePos(mapToScene(event->pos()));
    mouseEvent.setScreenPos(event->globalPos());
    mouseEvent.setLastScenePos(d->lastMouseMoveScenePoint);
    mouseEvent.setLastScreenPos(mapFromScene(d->lastMouseMoveScenePoint));
    mouseEvent.setButtons(event->buttons());
    mouseEvent.setButton(event->button());
    mouseEvent.setModifiers(event->modifiers());
    QApplication::sendEvent(d->scene, &mouseEvent);
}

/*!
    \reimp
*/
void QGraphicsView::wheelEvent(QWheelEvent *event)
{
    Q_D(QGraphicsView);
    if (!d->scene || !d->sceneInteractionAllowed) {
        QAbstractScrollArea::wheelEvent(event);
        return;
    }

    QGraphicsSceneWheelEvent wheelEvent(QEvent::GraphicsSceneWheel);
    wheelEvent.setWidget(viewport());
    wheelEvent.setScenePos(mapToScene(event->pos()));
    wheelEvent.setScreenPos(event->globalPos());
    wheelEvent.setButtons(event->buttons());
    wheelEvent.setModifiers(event->modifiers());
    wheelEvent.setDelta(event->delta());
    wheelEvent.setAccepted(event->isAccepted());
    QApplication::sendEvent(d->scene, &wheelEvent);
    event->setAccepted(wheelEvent.isAccepted());
    if (!event->isAccepted())
        QAbstractScrollArea::wheelEvent(event);
}

/*!
    \reimp
*/
void QGraphicsView::paintEvent(QPaintEvent *event)
{
    Q_D(QGraphicsView);
    if (!d->scene) {
        QAbstractScrollArea::paintEvent(event);
        return;
    }

    // Determine the exposed region
    QRegion exposedRegion = event->region();
    if (!d->accelerateScrolling || !d->scene)
        exposedRegion = viewport()->rect();

    // Set up the painter
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing,
                          d->renderHints & QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,
                          d->renderHints & QPainter::SmoothPixmapTransform);
    QMatrix moveMatrix;
    moveMatrix.translate(-horizontalScrollBar()->value() + d->leftIndent,
                         -verticalScrollBar()->value() + d->topIndent);
    QMatrix painterMatrix = d->matrix * moveMatrix;
    painter.setMatrix(painterMatrix);

#ifdef QGRAPHICSVIEW_DEBUG
    QTime stopWatch;
    stopWatch.start();
    qDebug() << "QGraphicsView::paintEvent(" << event->region() << ")";
#endif

    // Transform the exposed viewport rects to scene polygons
    QList<QRectF> exposedRects;
    foreach (QRect rect, exposedRegion.rects())
        exposedRects << mapToScene(rect.adjusted(-1, -1, 1, 1)).boundingRect();

    // Find all exposed items
    QList<QGraphicsItem *> itemList;
    QSet<QGraphicsItem *> tmp;
    foreach (QRectF rect, exposedRects) {
        foreach (QGraphicsItem *item, d->scene->items(rect)) {
            if (!tmp.contains(item)) {
                tmp << item;
                itemList << item;
            }
        }
    }
    tmp.clear();
    QGraphicsScenePrivate::sortItems(&itemList);

    // Reverse the item; we want to draw them in reverse order
    if (!itemList.isEmpty()) {
        QGraphicsItem **a = &itemList.first();
        QGraphicsItem **b = &itemList.last();
        QGraphicsItem *tmp = 0;
        while (a < b) {
            tmp = *a;
            *a = *b;
            *b = tmp;
            ++a; --b;
        }
    }

#ifdef QGRAPHICSVIEW_DEBUG
    int exposedTime = stopWatch.elapsed();
#endif

    if (d->cacheMode & CacheBackground) {
        if (d->mustResizeBackgroundPixmap) {
            // Recreate the background pixmap, and flag the whole background as
            // exposed.
            d->backgroundPixmap = QPixmap(viewport()->size());
            QPainter p(&d->backgroundPixmap);
            p.fillRect(0, 0, d->backgroundPixmap.width(), d->backgroundPixmap.height(),
                       viewport()->palette().brush(viewport()->backgroundRole()));
            d->backgroundPixmapExposed = QRegion(event->rect());
            d->mustResizeBackgroundPixmap = false;
        }

        // Redraw exposed areas
        QPainter backgroundPainter(&d->backgroundPixmap);
        backgroundPainter.setMatrix(painterMatrix);
        foreach (QRect rect, d->backgroundPixmapExposed.rects()) {
            backgroundPainter.save();
            QRectF exposedSceneRect = mapToScene(rect.adjusted(-1, -1, 1, 1)).boundingRect();
            backgroundPainter.setClipRect(exposedSceneRect.adjusted(-1, -1, 1, 1));
            drawBackground(&backgroundPainter, exposedSceneRect);
            backgroundPainter.restore();
        }
        d->backgroundPixmapExposed = QRegion();

        // Blit the background from the background pixmap
        painter.setMatrixEnabled(false);
        foreach (QRect rect, event->region().rects())
            painter.drawPixmap(rect, d->backgroundPixmap, rect);
        painter.setMatrixEnabled(true);
    } else {
        // Draw the background directly
        foreach (QRectF rect, exposedRects) {
            painter.save();
            painter.setClipRect(rect.adjusted(-1, -1, 1, 1));
            drawBackground(&painter, rect);
            painter.restore();
        }
    }

#ifdef QGRAPHICSVIEW_DEBUG
    int backgroundTime = stopWatch.elapsed() - exposedTime;
#endif

    // Generate the style options
    QList<QStyleOptionGraphicsItem> styleOptions;
    for (int i = 0; i < itemList.size(); ++i) {
        QGraphicsItem *item = itemList.at(i);

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

        // Calculate a simple level-of-detail metric.
        QMatrix itemSceneMatrix = item->sceneMatrix();
        QMatrix neo = itemSceneMatrix * painter.matrix();
        QRectF mappedRect = neo.mapRect(QRectF(0, 0, 1, 1));
        qreal dx = neo.mapRect(QRectF(0, 0, 1, 1)).size().width();
        qreal dy = neo.mapRect(QRectF(0, 0, 1, 1)).size().height();
        option.levelOfDetail = qMin(dx, dy);
        option.matrix = neo;

        // Determine the item's exposed area
        QMatrix reverseMap = itemSceneMatrix.inverted();
        foreach (QRectF rect, exposedRects)
            option.exposedRect |= reverseMap.mapRect(rect);
        option.exposedRect &= item->boundingRect();

        styleOptions << option;
    }

    // Items
    drawItems(&painter, itemList, styleOptions);

#ifdef QGRAPHICSVIEW_DEBUG
    int itemsTime = stopWatch.elapsed() - exposedTime - backgroundTime;
#endif

    // Foreground
    foreach (QRectF rect, exposedRects) {
        painter.save();
        painter.setClipRect(rect.adjusted(-1, -1, 1, 1));
        drawForeground(&painter, rect);
        painter.restore();
    }

#ifdef QGRAPHICSVIEW_DEBUG
    int foregroundTime = stopWatch.elapsed() - exposedTime - backgroundTime - itemsTime;
#endif

    painter.end();

#ifdef QGRAPHICSVIEW_DEBUG
    qDebug() << "\tItem discovery....... " << exposedTime << "msecs (" << itemList.size() << "items,"
             << (exposedTime > 0 ? (itemList.size() * 1000.0 / exposedTime) : -1) << "/ sec )";
    qDebug() << "\tDrawing background... " << backgroundTime << "msecs (" << exposedRects.size() << "segments )";
    qDebug() << "\tDrawing items........ " << itemsTime << "msecs ("
             << (itemsTime > 0 ? (itemList.size() * 1000.0 / itemsTime) : -1) << "/ sec )";
    qDebug() << "\tDrawing foreground... " << foregroundTime << "msecs (" << exposedRects.size() << "segments )";
    qDebug() << "\tTotal rendering time: " << stopWatch.elapsed() << "msecs ("
             << (stopWatch.elapsed() > 0 ? (1000.0 / stopWatch.elapsed()) : -1.0) << "fps )";
#endif
}

/*!
    \reimp
*/
void QGraphicsView::resizeEvent(QResizeEvent *event)
{
    Q_D(QGraphicsView);
    // Save the last center point - the resize may scroll the view, which
    // changes the center point.
    QPointF oldLastCenterPoint = d->lastCenterPoint;

    QAbstractScrollArea::resizeEvent(event);
    d->recalculateContentSize();

    // Restore the center point again.
    d->lastCenterPoint = oldLastCenterPoint;
    centerOn(d->lastCenterPoint);

    if (d->cacheMode & CacheBackground) {
        // Invalidate the background pixmap
        d->mustResizeBackgroundPixmap = true;
    }
}

/*!
    \reimp
*/
void QGraphicsView::scrollContentsBy(int dx, int dy)
{
    Q_D(QGraphicsView);
    if (d->accelerateScrolling)
        viewport()->scroll(dx, dy);
    else
        viewport()->update();
    d->updateLastCenterPoint();

    if (d->cacheMode & CacheBackground) {
        // Invalidate the background pixmap
        d->backgroundPixmapExposed.translate(dx, 0);
        if (dx > 0) {
            d->backgroundPixmapExposed += QRect(0, 0, dx, viewport()->height());
        } else if (dx < 0) {
            d->backgroundPixmapExposed += QRect(viewport()->width() + dx, 0,
                                                -dx, viewport()->height());
        }
        d->backgroundPixmapExposed.translate(0, dy);
        if (dy > 0) {
            d->backgroundPixmapExposed += QRect(0, 0, viewport()->width(), dy);
        } else if (dy < 0) {
            d->backgroundPixmapExposed += QRect(0, viewport()->height() + dy - 1,
                                                viewport()->width(), -dy + 1);
        }

        // Scroll the background pixmap
        if (!d->backgroundPixmap.isNull()) {
#ifdef Q_OS_WIN
            QPixmap tmp = d->backgroundPixmap;
            QPainter painter(&d->backgroundPixmap);
            painter.drawPixmap(dx, dy, tmp);
#else
            QPainter painter(&d->backgroundPixmap);
            painter.drawPixmap(dx, dy, d->backgroundPixmap);
#endif
        }
    }
}

/*!
    \reimp
*/
void QGraphicsView::showEvent(QShowEvent *event)
{
    Q_D(QGraphicsView);
    d->recalculateContentSize();
    centerOn(d->lastCenterPoint);
    QAbstractScrollArea::showEvent(event);
}

/*!
    \reimp
*/
void QGraphicsView::inputMethodEvent(QInputMethodEvent *event)
{
    Q_D(QGraphicsView);
    QApplication::sendEvent(d->scene, event);
}

/*!
    Draws the background of the scene using \a painter, before any items and
    the foreground are drawn. Reimplement this function to provide a custom
    background for this view.

    If all you want is to define a color, texture or gradient for the
    background, you can call setBackgroundBrush() instead.

    All painting is done in \e scene coordinates. \a rect is the exposed
    rectangle.

    The default implementation fills \a rect using the view's backgroundBrush.
    If no such brush is defined (the default), the scene's drawBackground()
    function is called instead.

    \sa drawForeground(), QGraphicsScene::drawForeground()
*/
void QGraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_D(QGraphicsView);
    if (d->scene && d->backgroundBrush.style() == Qt::NoBrush) {
        d->scene->drawBackground(painter, rect);
        return;
    }

    painter->fillRect(rect, d->backgroundBrush);
}

/*!
    Draws the foreground of the scene using \a painter, after the background
    and all items are drawn. Reimplement this function to provide a custom
    foreground for this view.

    If all you want is to define a color, texture or gradient for the
    foreground, you can call setForegroundBrush() instead.

    All painting is done in \e scene coordinates. \a rect is the exposed
    rectangle.

    The default implementation fills \a rect using the view's foregroundBrush.
    If no such brush is defined (the default), the scene's drawForeground()
    function is called instead.

    \sa drawBackground(), QGraphicsScene::drawBackground()
*/
void QGraphicsView::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_D(QGraphicsView);
    if (d->scene && d->foregroundBrush.style() == Qt::NoBrush) {
        d->scene->drawForeground(painter, rect);
        return;
    }

    painter->fillRect(rect, d->foregroundBrush);
}

/*!
    Draws the items \a items in the scene using \a painter, after the
    background and before the foreground are drawn. Reimplement this function
    to provide custom item drawing for this view. \a options is a list of
    styleoptions; one for each item.

    The default implementation calls the scene's drawItems() function.

    \sa drawForeground(), drawBackground(), QGraphicsScene::drawItems()
*/
void QGraphicsView::drawItems(QPainter *painter, const QList<QGraphicsItem *> &items,
                              const QList<QStyleOptionGraphicsItem> &options)
{
    Q_D(QGraphicsView);
    if (d->scene)
        d->scene->drawItems(painter, items, options);
}

#endif // QT_NO_GRAPHICSVIEW
