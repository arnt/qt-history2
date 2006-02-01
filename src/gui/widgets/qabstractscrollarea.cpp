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

#include "qabstractscrollarea.h"

#ifndef QT_NO_SCROLLAREA

#include "qscrollbar.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qevent.h"

#include "qabstractscrollarea_p.h"
#include <qwidget.h>
#include <qdebug.h>

#ifdef Q_WS_MAC
#include <qmacstyle_mac.h>
#endif

/*!
    \class QAbstractScrollArea qabstractscrollarea.h

    \brief The QAbstractScrollArea widget provides a scrolling area with
    on-demand scroll bars.

    \ingroup abstractwidgets

    QAbstractScrollArea is a low-level abstraction of a scrolling area. It gives
    you full control of the scroll bars, at the cost of simplicity. In
    most cases, using a QScrollArea is preferable.

    QAbstractScrollArea's central child widget is the scrolling area itself,
    called viewport(). The viewport widget uses all available
    space. Next to the viewport is a vertical scroll bar (accessible
    with verticalScrollBar()), and below a horizontal scroll bar
    (accessible with horizontalScrollBar()). Each scroll bar can be
    either visible or hidden, depending on the scroll bar's policy
    (see \l verticalScrollBarPolicy and \l horizontalScrollBarPolicy).
    When a scroll bar is hidden, the viewport expands in order to
    cover all available space. When a scroll bar becomes visible
    again, the viewport shrinks in order to make room for the scroll
    bar.

    With a scroll bar policy of Qt::ScrollBarAsNeeded (the default),
    QAbstractScrollArea shows scroll bars when those provide a non-zero
    scrolling range, and hides them otherwise. You control the range
    of each scroll bar with QAbstractSlider::setRange().

    In order to track scroll bar movements, reimplement the virtual
    function scrollContentsBy(). In order to fine-tune scrolling
    behavior, connect to a scroll bar's
    QAbstractSlider::actionTriggered() signal and adjust the \l
    QAbstractSlider::sliderPosition as you wish.

    It is possible to reserve a margin area around the viewport, see
    setViewportMargins(). The feature is mostly used to place a
    QHeaderView widget above or beside the scrolling area.

    For convience, QAbstractScrollArea makes all viewport events available in
    the virtual viewportEvent() handler.  QWidget's specialised
    handlers are remapped to viewport events in the cases where this
    makes sense. The remapped specialised handlers are: paintEvent(),
    mousePressEvent(), mouseReleaseEvent(), mouseDoubleClickEvent(),
    mouseMoveEvent(), wheelEvent(), dragEnterEvent(), dragMoveEvent(),
    dragLeaveEvent(), dropEvent(), contextMenuEvent().  and
    resizeEvent().

*/

inline  bool QAbstractScrollAreaPrivate::viewportEvent(QEvent *e)
{ Q_Q(QAbstractScrollArea); return q->viewportEvent(e); }

class QAbstractScrollAreaViewport : public QWidget
{
    Q_OBJECT
public:
    QAbstractScrollAreaViewport(QWidget *parent):QWidget(parent){ setObjectName(QLatin1String("qt_scrollarea_viewport")); }
    bool event(QEvent *e);
    friend class QAbstractScrollArea;
};
bool QAbstractScrollAreaViewport::event(QEvent *e) {
    if (QAbstractScrollArea* viewport = qobject_cast<QAbstractScrollArea*>(parentWidget()))
        return ((QAbstractScrollAreaPrivate*)((QAbstractScrollAreaViewport*)viewport)->d_ptr)->viewportEvent(e);
    return QWidget::event(e);
}

QAbstractScrollAreaPrivate::QAbstractScrollAreaPrivate()
    :hbar(0), vbar(0), vbarpolicy(Qt::ScrollBarAsNeeded), hbarpolicy(Qt::ScrollBarAsNeeded),
     viewport(0), cornerWidget(0), corner(Qt::BottomRightCorner), left(0), top(0), right(0), bottom(0),
     xoffset(0), yoffset(0)
{
}


void QAbstractScrollAreaPrivate::init()
{
    Q_Q(QAbstractScrollArea);
    hbar = new QScrollBar(Qt::Horizontal, q);
    hbar->setRange(0,0);
    hbar->setVisible(false);
    QObject::connect(hbar, SIGNAL(valueChanged(int)), q, SLOT(hslide(int)));
    QObject::connect(hbar, SIGNAL(rangeChanged(int,int)), q, SLOT(showOrHideScrollBars()), Qt::QueuedConnection);
    vbar = new QScrollBar(Qt::Vertical, q);
    vbar->setRange(0,0);
    vbar->setVisible(false);
    QObject::connect(vbar, SIGNAL(valueChanged(int)), q, SLOT(vslide(int)));
    QObject::connect(vbar, SIGNAL(rangeChanged(int,int)), q, SLOT(showOrHideScrollBars()), Qt::QueuedConnection);
    viewport = new QAbstractScrollAreaViewport(q);
    viewport->setBackgroundRole(QPalette::Base);
    viewport->setAutoFillBackground(true);
    viewport->setFocusProxy(q);
    q->setFocusPolicy(Qt::WheelFocus);
    q->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layoutChildren();
}

void QAbstractScrollAreaPrivate::layoutChildren()
{
    Q_Q(QAbstractScrollArea);
    bool needh = (hbarpolicy == Qt::ScrollBarAlwaysOn
                  || (hbarpolicy == Qt::ScrollBarAsNeeded && hbar->minimum() < hbar->maximum()));

    bool needv = (vbarpolicy == Qt::ScrollBarAlwaysOn
                  || (vbarpolicy == Qt::ScrollBarAsNeeded && vbar->minimum() < vbar->maximum()));

    int hsbExt = hbar->sizeHint().height();
    int vsbExt = vbar->sizeHint().width();

    QRect vr = q->rect();
    QStyleOption opt(0);
    opt.init(q);

    int extra = 0;
    bool frameContentsOnly = q->style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents, &opt, q);
    if (frameContentsOnly)
        extra = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2;

    bool hasCornerWidget = (cornerWidget != 0);

// If the scrollbars are at the very right and bottom of the window we
// move their positions to be alligned with the size grip.
#ifdef Q_OS_MAC
    // Use small scrollbars for tool windows.
    const QMacStyle::WidgetSizePolicy hpolicy = QMacStyle::widgetSizePolicy(hbar);
    const QMacStyle::WidgetSizePolicy vpolicy = QMacStyle::widgetSizePolicy(vbar);
    if (q->window()->windowType() == Qt::Tool) {
        if (hpolicy != QMacStyle::SizeSmall)
            QMacStyle::setWidgetSizePolicy(hbar, QMacStyle::SizeSmall);
        if (vpolicy != QMacStyle::SizeSmall)
            QMacStyle::setWidgetSizePolicy(vbar, QMacStyle::SizeSmall);
    } else {
        if (hpolicy != QMacStyle::SizeDefault)
            QMacStyle::setWidgetSizePolicy(hbar, QMacStyle::SizeDefault);
        if (vpolicy != QMacStyle::SizeDefault)
            QMacStyle::setWidgetSizePolicy(vbar, QMacStyle::SizeDefault);
    }

    // Get the size of the size-grip from the style.
    const int sizeGripSize = q->style()->pixelMetric(QStyle::PM_SizeGripSize, &opt, q);

    // Get coordiantes for the bottom-right corner of the scroll area and its window.
    const QPoint scrollAreaBottomRight = q->mapTo(q->window(), vr.bottomRight());
    const QPoint windowBottomRight = q->window()->rect().bottomRight();
    const QPoint offset = windowBottomRight - scrollAreaBottomRight;

    if (offset.manhattanLength() < sizeGripSize)
        hasCornerWidget = true;
#endif
    if (frameContentsOnly) {
        QRect frameRect = vr;
        if (needh) {
            frameRect.setBottom(frameRect.bottom() - hsbExt - extra);
            hbar->setGeometry(QStyle::visualRect(opt.direction, opt.rect,
                        QRect(0, frameRect.bottom() + 1 + extra,
                            frameRect.width() + 1 - ((hasCornerWidget && !needv)?vsbExt:0) -(needv?(vsbExt):0), hsbExt)));
        }
        if (needv) {
            frameRect.setRight(frameRect.right() - vsbExt - extra);
            vbar->setGeometry(QStyle::visualRect(opt.direction, opt.rect,
                        QRect(frameRect.right() + 1 + extra, (corner != Qt::BottomRightCorner ? vsbExt : 0),
                            vsbExt, frameRect.height() + 1 - ((hasCornerWidget && !needh)?(hsbExt): - extra))));
        }
        q->setFrameRect(QStyle::visualRect(opt.direction, opt.rect, frameRect));
        vr = q->contentsRect();
    } else {
        q->setFrameRect(vr);
        vr = q->contentsRect();
        if (needh) {
            vr.setBottom(vr.bottom() - hsbExt);
            hbar->setGeometry(QStyle::visualRect(opt.direction, opt.rect,
                        QRect(vr.left(), vr.bottom() + 1,
                              vr.width() - (needv?vsbExt:0) - ((hasCornerWidget && !needv)?vsbExt:0), hsbExt)));
        }
        if (needv) {
            vr.setRight(vr.right() - vsbExt);
            vbar->setGeometry(QStyle::visualRect(opt.direction, opt.rect,
                        QRect(vr.right() + 1, vr.top() + (corner != Qt::BottomRightCorner ? vsbExt :0),
                              vsbExt, vr.height() - ((hasCornerWidget && !needh)?hsbExt:0))));
        }
        vr = QStyle::visualRect(opt.direction, opt.rect, vr);
    }

    if (cornerWidget){
        QRect r = QStyle::visualRect(opt.direction, opt.rect,
                  QRect(q->width()-vsbExt, corner != Qt::BottomRightCorner ? 0:q->height()-hsbExt, vsbExt, hsbExt));
        cornerWidget->setGeometry(r);
    }

    hbar->setVisible(needh);
    vbar->setVisible(needv);
    vr.adjust(left, top, -right, -bottom);
    viewport->setGeometry(vr); // resize the viewport last
}

/*!
    \internal

    Creates a new QAbstractScrollAreaPrivate, \a dd with the given \a parent.
*/
QAbstractScrollArea::QAbstractScrollArea(QAbstractScrollAreaPrivate &dd, QWidget *parent)
    :QFrame(dd, parent)
{
    Q_D(QAbstractScrollArea);
    d->init();
}

/*!
    Constructs a viewport.

    The \a parent arguments is sent to the QWidget constructor.
*/
QAbstractScrollArea::QAbstractScrollArea(QWidget *parent)
    :QFrame(*new QAbstractScrollAreaPrivate, parent)
{
    Q_D(QAbstractScrollArea);
    d->init();
}


/*!
  Destroys the viewport.
 */
QAbstractScrollArea::~QAbstractScrollArea()
{
}

/*! Returns the viewport widget.
 */
QWidget *QAbstractScrollArea::viewport() const
{
    Q_D(const QAbstractScrollArea);
    return d->viewport;
}


/*!
Returns the size of the viewport as if the scroll bars had no valid
scrolling range.
*/
// ### still thinking about the name
QSize QAbstractScrollArea::maximumViewportSize() const
{
    Q_D(const QAbstractScrollArea);
    int hsbExt = d->hbar->sizeHint().height();
    int vsbExt = d->vbar->sizeHint().width();

    int f = 2 * d->frameWidth;
    QSize max = size() - QSize(f + d->left + d->right, f + d->top + d->bottom);
    if (d->vbarpolicy == Qt::ScrollBarAlwaysOn)
        max.rwidth() -= vsbExt;
    if (d->hbarpolicy == Qt::ScrollBarAlwaysOn)
        max.rheight() -= hsbExt;
    return max;
}

/*!
    \property QAbstractScrollArea::verticalScrollBarPolicy
    \brief the policy for the vertical scroll bar

    The default policy is Qt::ScrollBarAsNeeded.

    \sa horizontalScrollBarPolicy
*/

Qt::ScrollBarPolicy QAbstractScrollArea::verticalScrollBarPolicy() const
{
    Q_D(const QAbstractScrollArea);
    return d->vbarpolicy;
}

void QAbstractScrollArea::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    Q_D(QAbstractScrollArea);
    d->vbarpolicy = policy;
    if (isVisible())
        d->layoutChildren();
}


/*!
  Returns the vertical scroll bar.

  \sa verticalScrollBarPolicy, horizontalScrollBar()
 */
QScrollBar *QAbstractScrollArea::verticalScrollBar() const
{
    Q_D(const QAbstractScrollArea);
    return d->vbar;
}

/*!
    \property QAbstractScrollArea::horizontalScrollBarPolicy
    \brief the policy for the horizontal scroll bar

    The default policy is Qt::ScrollBarAsNeeded.

    \sa verticalScrollBarPolicy
*/

Qt::ScrollBarPolicy QAbstractScrollArea::horizontalScrollBarPolicy() const
{
    Q_D(const QAbstractScrollArea);
    return d->hbarpolicy;
}

void QAbstractScrollArea::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    Q_D(QAbstractScrollArea);
    d->hbarpolicy = policy;
    if (isVisible())
        d->layoutChildren();
}

/*!
  Returns the horizontal scroll bar.

  \sa horizontalScrollBarPolicy, verticalScrollBar()
 */
QScrollBar *QAbstractScrollArea::horizontalScrollBar() const
{
    Q_D(const QAbstractScrollArea);
    return d->hbar;
}

/*!
    Returns the widget in the corner between the two scroll bars.

    By default, no corner widget is present.
*/
QWidget *QAbstractScrollArea::cornerWidget() const
{
    Q_D(const QAbstractScrollArea);
    return d->cornerWidget;
}

/*!
    Sets the widget in the \a corner between the two scroll bars.

    You will probably also want to set at least one of the scroll bar
    modes to \c AlwaysOn.

    Passing 0 shows no widget in the corner.

    Any previous \a corner widget is hidden.

    You may call setCornerWidget() with the same widget at different
    times.

    All widgets set here will be deleted by the QScrollView when it is
    destroyed unless you separately reparent the widget after setting
    some other corner widget (or 0).

    Any \e newly set widget should have no current parent.

    By default, no corner widget is present.

    \sa setVScrollBarMode(), setHScrollBarMode()
*/
void QAbstractScrollArea::setCornerWidget(QWidget *widget, Qt::Corner corner)
{
    Q_D(QAbstractScrollArea);
    QWidget* oldWidget = d->cornerWidget;
    if (oldWidget != widget) {
        if (oldWidget)
            oldWidget->hide();
        d->cornerWidget = widget;
        d->corner = corner;

        if (widget && widget->parentWidget() != this) {
            widget->setParent( this );
        }

        d->layoutChildren();
        if (widget)
            widget->show();
    } else {
        d->corner = corner;
        d->layoutChildren();
    }
}

/*!
    Sets the margins around the scrolling area to \a left, \a top, \a
    right and \a bottom. This is useful for applications such as
    spreadsheets with "locked" rows and columns. The marginal space is
    is left blank; put widgets in the unused area.

    By default all margins are zero.

*/
void QAbstractScrollArea::setViewportMargins(int left, int top, int right, int bottom)
{
    Q_D(QAbstractScrollArea);
    d->left = left;
    d->top = top;
    d->right = right;
    d->bottom = bottom;
    d->layoutChildren();
}

/*!
    This is the main event handler for the QAbstractScrollArea widget (\e not
    the scrolling area viewport()). The event is passed in \a e.
*/
bool QAbstractScrollArea::event(QEvent *e)
{
    Q_D(QAbstractScrollArea);
    switch (e->type()) {
    case QEvent::AcceptDropsChange:
        d->viewport->setAcceptDrops(acceptDrops());
        break;
    case QEvent::MouseTrackingChange:
        d->viewport->setMouseTracking(hasMouseTracking());
        break;
    case QEvent::Resize:
            d->layoutChildren();
            break;
    case QEvent::Paint:
        QFrame::paintEvent((QPaintEvent*)e);
        break;
    case QEvent::ContextMenu:
        if (static_cast<QContextMenuEvent *>(e)->reason() == QContextMenuEvent::Keyboard)
           return QFrame::event(e);
        e->ignore();
        break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::Wheel:
#ifndef QT_NO_DRAGANDDROP
    case QEvent::Drop:
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::DragLeave:
#endif
        return false;
    case QEvent::StyleChange:
        d->layoutChildren();
        // fall through
    default:
        return QFrame::event(e);
    }
    return true;
}

/*!  The main event handler for the scrolling area (the viewport()
  widget). It handles event \a e.

  You can reimplement this function in a subclass, but we recommend
  using one of the specialized event handlers instead.

  Specialised handlers for viewport events are: paintEvent(),
  mousePressEvent(), mouseReleaseEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), wheelEvent(), dragEnterEvent(), dragMoveEvent(),
  dragLeaveEvent(), dropEvent(), contextMenuEvent(), and
  resizeEvent().

 */
bool QAbstractScrollArea::viewportEvent(QEvent *e)
{
    Q_D(QAbstractScrollArea);
    switch (e->type()) {
    case QEvent::Resize:
    case QEvent::Paint:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::ContextMenu:
#ifndef QT_NO_DRAGANDDROP
    case QEvent::Drop:
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::DragLeave:
#endif
        return QFrame::event(e);
    case QEvent::LayoutRequest:
        return event(e);
#ifndef QT_NO_WHEELEVENT
    case QEvent::Wheel:
        if (!QFrame::event(e) || !e->isAccepted()) {
            if (static_cast<QWheelEvent*>(e)->orientation() == Qt::Horizontal)
                return QApplication::sendEvent(d->hbar, e);
            return QApplication::sendEvent(d->vbar, e);
        }
        return true;
#endif
    default:
        break;
    }
    return static_cast<QAbstractScrollAreaViewport*>(d->viewport)->QWidget::event(e);
}

/*!
    \fn void QAbstractScrollArea::resizeEvent(QResizeEvent *event)

    This event handler can be reimplemented in a subclass to receive
    resize events (passed in \a event), for the viewport() widget.

    When resizeEvent() is called, the viewport already has its new
    geometry: Its new size is accessible through the
    QResizeEvent::size() function, and the old size through
    QResizeEvent::oldSize().

    \sa QWidget::resizeEvent()
 */
void QAbstractScrollArea::resizeEvent(QResizeEvent *)
{
}

/*!
    \fn void QAbstractScrollArea::paintEvent(QPaintEvent *event)

    This event handler can be reimplemented in a subclass to receive
    paint events (passed in \a event), for the viewport() widget.

    Note: If you open a painter, make sure to open it on the
    viewport().

    \sa QWidget::paintEvent()
*/
void QAbstractScrollArea::paintEvent(QPaintEvent*)
{
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse press events for the viewport() widget. The event is passed
    in \a e.

    \sa QWidget::mousePressEvent()
*/
void QAbstractScrollArea::mousePressEvent(QMouseEvent *e)
{
    e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse release events for the viewport() widget. The event is
    passed in \a e.

    \sa QWidget::mouseReleaseEvent()
*/
void QAbstractScrollArea::mouseReleaseEvent(QMouseEvent *e)
{
    e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse double click events for the viewport() widget. The event is
    passed in \a e.

    \sa QWidget::mouseDoubleClickEvent()
*/
void QAbstractScrollArea::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse move events for the viewport() widget. The event is passed
    in \a e.

    \sa QWidget::mouseMoveEvent()
*/
void QAbstractScrollArea::mouseMoveEvent(QMouseEvent *e)
{
    e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    wheel events for the viewport() widget. The event is passed in \a
    e.

    \sa QWidget::wheelEvent()
*/
#ifndef QT_NO_WHEELEVENT
void QAbstractScrollArea::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}
#endif

/*!
    This event handler can be reimplemented in a subclass to receive
    context menu events for the viewport() widget. The event is passed
    in \a e.

    \sa QWidget::contextMenuEvent()
*/
void QAbstractScrollArea::contextMenuEvent(QContextMenuEvent *e)
{
    e->ignore();
}

/*!
    This function is called with key event \a e when key presses
    occur. It handles PageUp, PageDown, Up, Down, Left, and Right, and
    ignores all other key presses.
*/
void QAbstractScrollArea::keyPressEvent(QKeyEvent * e)
{
    Q_D(QAbstractScrollArea);
    switch (e->key()) {
    case Qt::Key_PageUp:
        d->vbar->triggerAction(QScrollBar::SliderPageStepSub);
        break;
    case Qt::Key_PageDown:
        d->vbar->triggerAction(QScrollBar::SliderPageStepAdd);
        break;
    case Qt::Key_Up:
        d->vbar->triggerAction(QScrollBar::SliderSingleStepSub);
        break;
    case Qt::Key_Down:
        d->vbar->triggerAction(QScrollBar::SliderSingleStepAdd);
        break;
    case Qt::Key_Left:
        d->hbar->triggerAction(QScrollBar::SliderSingleStepSub);
        break;
    case Qt::Key_Right:
        d->hbar->triggerAction(QScrollBar::SliderSingleStepAdd);
        break;
    default:
        e->ignore();
        return;
    }
    e->accept();
}


#ifndef QT_NO_DRAGANDDROP
/*!
    \fn void QAbstractScrollArea::dragEnterEvent(QDragEnterEvent *event)

    This event handler can be reimplemented in a subclass to receive
    drag enter events (passed in \a event), for the viewport() widget.

    \sa QWidget::dragEnterEvent()
*/
void QAbstractScrollArea::dragEnterEvent(QDragEnterEvent *)
{
}

/*!
    \fn void QAbstractScrollArea::dragMoveEvent(QDragMoveEvent *event)

    This event handler can be reimplemented in a subclass to receive
    drag move events (passed in \a event), for the viewport() widget.

    \sa QWidget::dragMoveEvent()
*/
void QAbstractScrollArea::dragMoveEvent(QDragMoveEvent *)
{
}

/*!
    \fn void QAbstractScrollArea::dragLeaveEvent(QDragLeaveEvent *event)

    This event handler can be reimplemented in a subclass to receive
    drag leave events (passed in \a event), for the viewport() widget.

    \sa QWidget::dragLeaveEvent()
*/
void QAbstractScrollArea::dragLeaveEvent(QDragLeaveEvent *)
{
}

/*!
    \fn void QAbstractScrollArea::dropEvent(QDropEvent *event)

    This event handler can be reimplemented in a subclass to receive
    drop events (passed in \a event), for the viewport() widget.

    \sa QWidget::dropEvent()
*/
void QAbstractScrollArea::dropEvent(QDropEvent *)
{
}


#endif

/*!
    This virtual handler is called when the scroll bars are moved by
    \a dx, \a dy, and consequently the viewport's contents should be
    scrolled accordingly.

    The default implementation simply calls update() on the entire
    viewport(), subclasses can reimplement this handler for
    optimization purposes, or - like QScrollArea - to move a contents
    widget. The paramters \a dx and \a dy are there for convenience,
    so that the class knows how much should be scrolled (useful
    e.g. when doing pixel-shifts). You may just as well ignore these
    values and scroll directly to the position the scroll bars
    indicate.

    Calling this function in order to scroll programmatically is an
    error, use the scroll bars instead (e.g. by calling
    QScrollBar::setValue() directly).
*/
void QAbstractScrollArea::scrollContentsBy(int, int)
{
    viewport()->update();
}

void QAbstractScrollAreaPrivate::hslide(int x)
{
    Q_Q(QAbstractScrollArea);
    int dx = xoffset - x;
    xoffset = x;
    q->scrollContentsBy(dx, 0);
}

void QAbstractScrollAreaPrivate::vslide(int y)
{
    Q_Q(QAbstractScrollArea);
    int dy = yoffset - y;
    yoffset = y;
    q->scrollContentsBy(0, dy);
}

void QAbstractScrollAreaPrivate::showOrHideScrollBars()
{
    layoutChildren();
}

/*!
    \reimp

*/
QSize QAbstractScrollArea::minimumSizeHint() const
{
    Q_D(const QAbstractScrollArea);
    int hsbExt = d->hbar->sizeHint().height();
    int vsbExt = d->vbar->sizeHint().width();
    int f = 2 * d->frameWidth;
    return QSize(3*vsbExt + f, 3*hsbExt + f);
}

/*!
    \reimp
*/
QSize QAbstractScrollArea::sizeHint() const
{
    return QSize(256, 192);
#if 0
    Q_D(const QAbstractScrollArea);
    int h = qMax(10, fontMetrics().height());
    int f = 2 * d->frameWidth;
    return QSize((6 * h) + f, (4 * h) + f);
#endif
}

#include "moc_qabstractscrollarea.cpp"
#include "qabstractscrollarea.moc"
#endif // QT_NO_SCROLLAREA
