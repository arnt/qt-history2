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

#include "qdockwidgetlayout_p.h"

#include "qmainwindow.h"
#include "qmainwindowlayout_p.h"

#ifndef QT_NO_MAINWINDOW

#include <qapplication.h>
#include <qevent.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>


QDockSeparator::QDockSeparator(QDockWidgetLayout *d, QWidget *parent)
    : QWidget(parent), hover(0), state(0)
{ setDock(d); }

void QDockSeparator::setDock(QDockWidgetLayout *d)
{
    Q_ASSERT(d != 0);
    dock = d;
    orientation = dock->orientation;
    setCursor((orientation == Qt::Horizontal) ? Qt::SplitVCursor : Qt::SplitHCursor);
}

bool QDockSeparator::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::HoverEnter:
        hover = true;
        update();
        break;
    case QEvent::HoverLeave:
        hover = false;
        update();
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

void QDockSeparator::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    Q_ASSERT(!state);
    state = new DragState;

    // we map from global coordinates to avoid nasty effects when
    // event compression kicks in
    state->origin = parentWidget()->mapFromGlobal(event->globalPos());

    // clear focus... it will be restored when the mouse button is released
    state->prevFocus = qApp->focusWidget();
    if (state->prevFocus)
        state->prevFocus->clearFocus();

    qobject_cast<QMainWindowLayout*>(parentWidget()->layout())->saveLayoutInfo();
}

void QDockSeparator::mouseMoveEvent(QMouseEvent *event)
{
    Q_ASSERT(state != 0);

    // we map from global coordinates to avoid nasty effects when
    // event compression kicks in
    QMainWindow *mw = qobject_cast<QMainWindow *>(parentWidget());
    QPoint p = mw->mapFromGlobal(event->globalPos());
    int delta = pick_perp(orientation, p - state->origin);

    // constrain the mouse move event
    if (qobject_cast<QMainWindowLayout *>(mw->layout())->constrain(dock, delta) != 0)
	qobject_cast<QMainWindowLayout *>(mw->layout())->relayout();
}

void QDockSeparator::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    QMainWindowLayout *l = qobject_cast<QMainWindowLayout*>(parentWidget()->layout());
    Q_ASSERT(l != 0);
    l->relayout();
    l->discardLayoutInfo();

    Q_ASSERT(state != 0);

    // restore focus
    if (state->prevFocus)
        state->prevFocus->setFocus();

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
    if (hover)
	opt.state |= QStyle::State_MouseOver;
    opt.rect = rect();
    opt.palette = palette();
    style()->drawPrimitive(QStyle::PE_IndicatorDockWidgetResizeHandle, &opt, &p, this);
}

#endif // QT_NO_MAINWINDOW
