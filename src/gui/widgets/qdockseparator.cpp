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

#include "qdockseparator_p.h"

#include "qdockwindowlayout_p.h"

#include "qmainwindow.h"
#include "qmainwindowlayout_p.h"

#include <qapplication.h>
#include <qevent.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>


QDockSeparator::QDockSeparator(QDockWindowLayout *d, QWidget *parent)
    : QWidget(parent), state(0)
{
    setDock(d);
}

void QDockSeparator::setDock(QDockWindowLayout *d)
{
    Q_ASSERT(d != 0);
    dock = d;
    orientation = dock->orientation;
    setCursor((orientation == Qt::Horizontal) ? Qt::SplitVCursor : Qt::SplitHCursor);
}

void QDockSeparator::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    Q_ASSERT(!state);
    state = new DragState;

    // we map from global coordinates to avoid nasty effects when
    // event compression kicks in
    state->origin = parentWidget()->mapFromGlobal(event->globalPos());

    // clear focus... it will be restored when the mouse button is released
    state->prevFocus = qApp->focusWidget();
    if (state->prevFocus) state->prevFocus->clearFocus();

    qt_cast<QMainWindowLayout*>(parentWidget()->layout())->saveLayoutInfo();
}

void QDockSeparator::mouseMoveEvent(QMouseEvent *event)
{
    Q_ASSERT(state != 0);

    // we map from global coordinates to avoid nasty effects when
    // event compression kicks in
    QMainWindow *mw = qt_cast<QMainWindow *>(parentWidget());
    QPoint p = mw->mapFromGlobal(event->globalPos());
    int delta = pick_perp(orientation, p - state->origin);

    // constrain the mouse move event
    if (qt_cast<QMainWindowLayout *>(mw->layout())->constrain(dock, delta) != 0)
	qt_cast<QMainWindowLayout *>(mw->layout())->relayout();
}

void QDockSeparator::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    QMainWindowLayout *l = qt_cast<QMainWindowLayout*>(parentWidget()->layout());
    Q_ASSERT(l != 0);
    l->relayout();
    l->discardLayoutInfo();

    // restore focus
    if (state->prevFocus) state->prevFocus->setFocus();

    delete state;
    state = 0;
}

void QDockSeparator::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt(0);
    opt.state = QStyle::State_None;
    if (isEnabled())
	opt.state |= QStyle::State_Enabled;
    if (orientation == Qt::Horizontal)
	opt.state |= QStyle::State_Horizontal;
    opt.rect = rect();
    opt.palette = palette();

    style()->drawPrimitive(QStyle::PE_IndicatorDockWindowResizeHandle, &opt, &p, this);
}
