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

#include "qtoolbarhandle_p.h"

#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbar.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qevent.h>

#include <private/qmainwindowlayout_p.h>

static QStyleOption getStyleOption(const QToolBarHandle *tbh)
{
    QStyleOption opt;
    opt.init(tbh);
    if (tbh->orientation() == Qt::Horizontal)
	opt.state |= QStyle::State_Horizontal;
    return opt;
}

QToolBarHandle::QToolBarHandle(QToolBar *parent)
    : QWidget(parent), orient(parent->orientation()), state(0)
{
    setCursor(Qt::SizeAllCursor);
}

void QToolBarHandle::setOrientation(Qt::Orientation orientation)
{
    orient = orientation;

    if (orientation == Qt::Horizontal) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    } else {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    }

    // if we're dragging - swap the offset coords around as well
    if (state) {
	QPoint p = state->offset;
	state->offset = QPoint(p.y(), p.x());
    }

    update();
}

Qt::Orientation QToolBarHandle::orientation() const
{ return orient; }

QSize QToolBarHandle::sizeHint() const
{
    QStyleOption opt = getStyleOption(this);
    const int extent = style()->pixelMetric(QStyle::PM_ToolBarHandleExtent, &opt, this);
    return QSize(extent, extent);
}

void QToolBarHandle::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt = getStyleOption(this);
    style()->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt, &p, this);
}

void QToolBarHandle::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    Q_ASSERT(parentWidget());
    Q_ASSERT(!state);
    state = new DragState;

    state->offset = QStyle::visualPos(QApplication::layoutDirection(),
				      parentWidget()->rect(),
				      mapTo(parentWidget(), event->pos()));
}

void QToolBarHandle::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    delete state;
    state = 0;
}

void QToolBarHandle::mouseMoveEvent(QMouseEvent *event)
{
    if (!state)
        return;

    // see if there is a main window under us, and ask it to place the
    // tool bar accordingly
    QWidget *widget = QApplication::widgetAt(event->globalPos());
    if (!widget)
        return;

    while (widget && (!qt_cast<QMainWindow *>(widget)))
        widget = widget->parentWidget();

    if (!widget)
        return;

    QMainWindowLayout *layout = qt_cast<QMainWindowLayout *>(widget->layout());
    Q_ASSERT_X(layout != 0, "QMainWindow", "must have a layout");
    QPoint p = QStyle::visualPos(QApplication::layoutDirection(), parentWidget()->rect(),
				 parentWidget()->mapFromGlobal(event->globalPos())) 
	       - state->offset;

    // ### the offset is measured from the widget origin
    if (orient == Qt::Vertical)
        p.setX(state->offset.x() + p.x());
    else
        p.setY(state->offset.y() + p.y());

    // re-position toolbar
    layout->dropToolBar(qt_cast<QToolBar *>(parentWidget()), event->globalPos(), p);
}
