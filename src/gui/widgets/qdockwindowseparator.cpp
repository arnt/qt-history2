#include "qdockwindowseparator_p.h"

#include "qdockwindow.h"
#include "qdockwindowlayout_p.h"

#include <qapplication.h>
#include <qevent.h>
#include <qpainter.h>
#include <qstyle.h>



QDockWindowSeparator::QDockWindowSeparator(QDockWindowLayout *l, QWidget *parent)
    : QWidget(parent), layout(l), state(0)
{
    setCursor(layout->orientation == Horizontal ? SplitHCursor : SplitVCursor);
}

/*!
    Returns a rectangle centered around \a point.  The parent dock
    orientation is used to calculate a rectangle that is appropriate
    for use while the user is moving the separator.
 */
QRect QDockWindowSeparator::calcRect(const QPoint &point)
{
    int pos = pick(layout->orientation, point);
    int ext = pick(layout->orientation, size());
    int sz = pick_perp(layout->orientation, size());
    return (layout->orientation == Horizontal ?
	    QRect(pos - (ext/2), 0, ext, sz) :
	    QRect(0, pos - (ext/2), sz, ext));
}

void QDockWindowSeparator::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != LeftButton) return;

    Q_ASSERT(!state);
    state = new DragState;

    state->origin = state->last = parentWidget()->mapFromGlobal(event->globalPos());

    // clear focus... it will be restored when the mouse button is released
    state->prevFocus = qApp->focusWidget();
    if (state->prevFocus) state->prevFocus->clearFocus();

    layout->saveLayoutInfo();
}

void QDockWindowSeparator::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != LeftButton) return;

    layout->relayout();
    layout->discardLayoutInfo();

    // restore focus
    if (state->prevFocus) state->prevFocus->setFocus();
    state->prevFocus = 0;

    delete state;
    state = 0;
}

void QDockWindowSeparator::mouseMoveEvent(QMouseEvent *event)
{
    Q_ASSERT(state != 0);

    QPoint p = parentWidget()->mapFromGlobal(event->globalPos());
    int delta = pick(layout->orientation, p - state->origin);

    // constrain the mouse move event
    layout->resetLayoutInfo();
    p = state->origin + layout->constrain(this, delta);
    if (p == state->last) return;
    state->last = p;

    layout->relayout();
}

void QDockWindowSeparator::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    uint flags = QStyle::Style_Default;
    if (isEnabled())
	flags |= QStyle::Style_Enabled;
    if (layout->orientation != Horizontal)
	flags |= QStyle::Style_Horizontal;

    style().drawPrimitive(QStyle::PE_DockWindowResizeHandle, &p, rect(), palette(), flags);
}
