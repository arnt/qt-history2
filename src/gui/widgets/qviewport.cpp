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

#include "qviewport.h"
#include "qscrollbar.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qevent.h"

#include "qviewport_p.h"
#include <qwidget.h>
#define d d_func()
#define q q_func()



/*!
    \class QViewport qviewport.h

    \brief The QViewport widget provides a scrolling area with
    on-demand scroll bars.

    \ingroup abstractwidgets

    QViewport is a low-level abstraction of a scrolling area. It gives
    you full control of the scroll bars, at the cost of simplicity. In
    most cases, using a QWidgetView is preferable.

    QViewport's central child widget is the scrolling area itself,
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
    QViewport shows scroll bars when those provide a non-zero
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

    For convience, QViewport makes all viewport events available in
    the virtual viewportEvent()-handler.  QWidget's specialised
    handlers are remapped to viewport events in the cases where this
    makes sense. The remapped specialised handlers are: paintEvent(),
    mousePressEvent(), mouseReleaseEvent(), mouseDoubleClickEvent(),
    mouseMoveEvent(), wheelEvent(), dragEnterEvent(), dragMoveEvent(),
    dragLeaveEvent(), dropEvent(), contextMenuEvent().  and
    resizeEvent().

*/


/*!
    \enum Qt::ScrollBarPolicy

    This enum type describes the various modes of QViewport's scroll
    bars.

    \value ScrollBarAsNeeded QViewport shows a scroll bar when the
    content is too large to fit and not otherwise. This is the
    default.

    \value ScrollBarAlwaysOff QViewport never shows a scroll bar.

    \value ScrollBarAlwaysOn  QViewport always shows a scroll bar.

    (The modes for the horizontal and vertical scroll bars are
    independent.)
*/

inline  bool QViewportPrivate::viewportEvent(QEvent *e) { return q->viewportEvent(e); }


class QViewportHelper : public QWidget
{
public:
    QViewportHelper(QWidget *parent):QWidget(parent){}
    bool event(QEvent *e);
    friend class QViewport;
};
bool QViewportHelper::event(QEvent *e) {
    if (QViewport* viewport = qt_cast<QViewport*>(parentWidget()))
        return ((QViewportPrivate*)((QViewportHelper*)viewport)->d_ptr)->viewportEvent(e);
    return QWidget::event(e);
}

QViewportPrivate::QViewportPrivate()
    :hbar(0), vbar(0), vbarpolicy(Qt::ScrollBarAsNeeded), hbarpolicy(Qt::ScrollBarAsNeeded),
     viewport(0), left(0), top(0), right(0), bottom(0),
     xoffset(0), yoffset(0)
{
}


void QViewportPrivate::init()
{
    q->setFocusPolicy(Qt::WheelFocus);
    q->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hbar = new QScrollBar(Qt::Horizontal,  q);
    QObject::connect(hbar, SIGNAL(valueChanged(int)), q, SLOT(hslide(int)));
    QObject::connect(hbar, SIGNAL(rangeChanged(int,int)), q, SLOT(showOrHideScrollBars()), Qt::QueuedConnection);
    vbar = new QScrollBar(Qt::Vertical, q);
    QObject::connect(vbar, SIGNAL(valueChanged(int)), q, SLOT(vslide(int)));
    QObject::connect(hbar, SIGNAL(rangeChanged(int,int)), q, SLOT(showOrHideScrollBars()), Qt::QueuedConnection);
    viewport = new QViewportHelper(q);
    viewport->setBackgroundRole(QPalette::Base);
    viewport->setFocusProxy(q);
    QEvent userEvent(QEvent::User);
    QApplication::sendEvent(viewport, &userEvent);
}

void QViewportPrivate::layoutChildren()
{
    bool needh = (hbarpolicy == Qt::ScrollBarAlwaysOn
                  || (hbarpolicy == Qt::ScrollBarAsNeeded && hbar->minimum() < hbar->maximum()));

    bool needv = (vbarpolicy == Qt::ScrollBarAlwaysOn
                  || (vbarpolicy == Qt::ScrollBarAsNeeded && vbar->minimum() < vbar->maximum()));

    int hsbExt = hbar->sizeHint().height();
    int vsbExt = vbar->sizeHint().width();

    QRect vr = q->rect();
    QStyleOption opt(0);
    opt.init(q);
    if (q->style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents, &opt, q)) {
        QRect fr = vr;
        if (needh) {
            fr.setBottom(fr.bottom() - hsbExt);
            hbar->setGeometry(QStyle::visualRect(opt.direction, opt.rect, QRect(0, fr.bottom() + 1, fr.width() - (needv?vsbExt:0), hsbExt)));
        }
        if (needv) {
            fr.setRight(fr.right() - vsbExt);
            vbar->setGeometry(QStyle::visualRect(opt.direction, opt.rect, QRect(fr.right() + 1, 0, vsbExt, fr.height())));
        }
        q->setFrameRect(QStyle::visualRect(opt.direction, opt.rect, fr));
        vr = q->contentsRect();
    } else {
        q->setFrameRect(vr);
        vr = q->contentsRect();
        if (needh) {
            vr.setBottom(vr.bottom() - hsbExt);
            hbar->setGeometry(QStyle::visualRect(opt.direction, opt.rect,  QRect(vr.left(), vr.bottom() + 1, vr.width() - (needv?vsbExt:0), hsbExt)));
        }
        if (needv) {
            vr.setRight(vr.right() - vsbExt);
            vbar->setGeometry(QStyle::visualRect(opt.direction, opt.rect, QRect(vr.right() + 1, vr.top(), vsbExt, vr.height())));
        }
        vr = QStyle::visualRect(opt.direction, opt.rect, vr);
    }
    hbar->setShown(needh);
    vbar->setShown(needv);
    vr.addCoords(left, top, -right, -bottom);
    viewport->setGeometry(vr); // resize the viewport last
}

/*!
    \internal

    Creates a new QViewportPrivate, \a dd with the given \a parent.
*/
QViewport::QViewport(QViewportPrivate &dd, QWidget *parent)
    :QFrame(dd, parent)
{
    d->init();
}

/*!
    Constructs a viewport.

    The \a parent arguments is sent to the QWidget constructor.
*/
QViewport::QViewport(QWidget *parent)
    :QFrame(*new QViewportPrivate, parent)
{
    d->init();
}


/*!
  Destroys the viewport.
 */
QViewport::~QViewport()
{
}

/*! Returns the viewport's viewport
 */
QWidget *QViewport::viewport() const
{
    return d->viewport;
}


/*!
Returns the size of the viewport as if the scroll bars had no valid
scrolling range.
*/
// ### still thinking about the name
QSize QViewport::maximumViewportSize() const
{
    int hsbExt = d->hbar->sizeHint().height();
    int vsbExt = d->vbar->sizeHint().width();

    int f = 2 * d->frameWidth;
    QSize max = size() - QSize(f,f);
    if (d->vbarpolicy == Qt::ScrollBarAlwaysOn)
        max.rwidth() -= vsbExt;
    if (d->hbarpolicy == Qt::ScrollBarAlwaysOn)
        max.rheight() -= hsbExt;
    return max;
}

/*!
    \property QViewport::verticalScrollBarPolicy
    \brief the policy for the vertical scroll bar

    The default policy is \c Qt::ScrollBarAsNeeded.

    \sa horizontalScrollBarPolicy
*/

Qt::ScrollBarPolicy QViewport::verticalScrollBarPolicy() const
{
    return d->vbarpolicy;
}

void QViewport::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    d->vbarpolicy = policy;
    if (isVisible())
        d->layoutChildren();
}


/*!
  Returns the vertical scroll bar.

  \sa verticalScrollBarPolicy, horizontalScrollBar()
 */
QScrollBar *QViewport::verticalScrollBar() const
{
    return d->vbar;
}

/*!
    \property QViewport::horizontalScrollBarPolicy
    \brief the policy for the horizontal scroll bar

    The default policy is \c Qt::ScrollBarAsNeeded.

    \sa verticalScrollBarPolicy
*/

Qt::ScrollBarPolicy QViewport::horizontalScrollBarPolicy() const
{
    return d->hbarpolicy;
}

void QViewport::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    d->hbarpolicy = policy;
    if (isVisible())
        d->layoutChildren();
}

/*!
  Returns the horizontal scroll bar.

  \sa horizontalScrollBarPolicy, verticalScrollBar()
 */
QScrollBar *QViewport::horizontalScrollBar() const
{
    return d->hbar;
}

/*!
    Sets the margins around the scrolling area to \a left, \a top, \a
    right and \a bottom. This is useful for applications such as
    spreadsheets with "locked" rows and columns. The marginal space is
    is left blank; put widgets in the unused area.

    By default all margins are zero.

*/
void QViewport::setViewportMargins(int left, int top, int right, int bottom)
{
    d->left = left;
    d->top = top;
    d->right = right;
    d->bottom = bottom;
    d->layoutChildren();
}

/*!
    This is the main event handler for the QViewport widget (\e not
    the scrolling area viewport()). The event is passed in \a e.
*/
bool QViewport::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseTrackingChange:
        d->viewport->setMouseTracking(hasMouseTracking());
        break;
    case QEvent::Resize:
            d->layoutChildren();
            break;
    case QEvent::Paint:
        QFrame::paintEvent((QPaintEvent*)e);
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
    case QEvent::ContextMenu:
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
bool QViewport::viewportEvent(QEvent *e)
{
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
    case QEvent::Wheel:
        if (!QFrame::event(e) || !e->isAccepted()) {
            if (static_cast<QWheelEvent*>(e)->orientation() == Qt::Horizontal)
                return QApplication::sendEvent(d->hbar, e);
            return QApplication::sendEvent(d->vbar, e);
        }
    default:
        break;
    }
    return static_cast<QViewportHelper*>(d->viewport)->QWidget::event(e);
}

/*!
    \fn void QViewport::resizeEvent(QResizeEvent *event)

    This event handler can be reimplemented in a subclass to receive
    resize events (passed in \a event), for the viewport() widget.
    When resizeEvent() is called, the viewport already has its new
    geometry. The old size is accessible through
    QResizeEvent::oldSize().

    \sa QWidget::resizeEvent()
 */
void QViewport::resizeEvent(QResizeEvent *)
{
}

/*!
    \fn void QViewport::paintEvent(QPaintEvent *event)

    This event handler can be reimplemented in a subclass to receive
    paint events (passed in \a event), for the viewport() widget.

    Note: If you open a painter, make sure to open it on the
    viewport().

    \sa QWidget::paintEvent()
*/
void QViewport::paintEvent(QPaintEvent*)
{
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse press events for the viewport() widget. The event is passed
    in \a e.

    \sa QWidget::mousePressEvent()
*/
void QViewport::mousePressEvent(QMouseEvent *e)
{
    e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse release events for the viewport() widget. The event is
    passed in \a e.

    \sa QWidget::mouseReleaseEvent()
*/
void QViewport::mouseReleaseEvent(QMouseEvent *e)
{
    e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse double click events for the viewport() widget. The event is
    passed in \a e.

    \sa QWidget::mouseDoubleClickEvent()
*/
void QViewport::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse move events for the viewport() widget. The event is passed
    in \a e.

    \sa QWidget::mouseMoveEvent()
*/
void QViewport::mouseMoveEvent(QMouseEvent *e)
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
void QViewport::wheelEvent(QWheelEvent *e)
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
void QViewport::contextMenuEvent(QContextMenuEvent *e)
{
    QFrame::contextMenuEvent(e);
}

/*!
    This function is called with key event \a e when key presses
    occur. It handles PageUp, PageDown, Up, Down, Left, and Right, and
    ignores all other key presses.
*/
void QViewport::keyPressEvent(QKeyEvent * e)
{
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
    \fn void QViewport::dragEnterEvent(QDragEnterEvent *event)

    This event handler can be reimplemented in a subclass to receive
    drag enter events (passed in \a event), for the viewport() widget.

    \sa QWidget::dragEnterEvent()
*/
void QViewport::dragEnterEvent(QDragEnterEvent *)
{
}

/*!
    \fn void QViewport::dragMoveEvent(QDragMoveEvent *event)

    This event handler can be reimplemented in a subclass to receive
    drag move events (passed in \a event), for the viewport() widget.

    \sa QWidget::dragMoveEvent()
*/
void QViewport::dragMoveEvent(QDragMoveEvent *)
{
}

/*!
    \fn void QViewport::dragLeaveEvent(QDragLeaveEvent *event)

    This event handler can be reimplemented in a subclass to receive
    drag leave events (passed in \a event), for the viewport() widget.

    \sa QWidget::dragLeaveEvent()
*/
void QViewport::dragLeaveEvent(QDragLeaveEvent *)
{
}

/*!
    \fn void QViewport::dropEvent(QDropEvent *event)

    This event handler can be reimplemented in a subclass to receive
    drop events (passed in \a event), for the viewport() widget.

    \sa QWidget::dropEvent()
*/
void QViewport::dropEvent(QDropEvent *)
{
}


#endif

/*!
    Scrolls the viewport's contents by \a dx, \a dy.
*/
void QViewport::scrollContentsBy(int, int)
{
    viewport()->update();
}

void QViewportPrivate::hslide(int x)
{
    int dx = xoffset - x;
    xoffset = x;
    q->scrollContentsBy(dx, 0);
}

void QViewportPrivate::vslide(int y)
{
    int dy = yoffset - y;
    yoffset = y;
    q->scrollContentsBy(0, dy);
}

void QViewportPrivate::showOrHideScrollBars()
{
    layoutChildren();
}

/*!
    \reimp

*/
QSize QViewport::minimumSizeHint() const
{
    int h = qMax(10, fontMetrics().height());
    int f = 2 * d->frameWidth;
    return QSize((6 * h) + f, (4 * h) + f);
}

/*!
    \reimp
*/
QSize QViewport::sizeHint() const
{
    return QSize(256, 192);
}

#include "moc_qviewport.cpp"
