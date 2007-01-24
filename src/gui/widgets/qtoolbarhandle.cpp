/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "private/qtoolbarhandle_p.h"

#ifndef QT_NO_TOOLBAR

#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbar.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qevent.h>

#include <private/qmainwindowlayout_p.h>
#include <private/qwidget_p.h>

void QToolBarHandle::initStyleOption(QStyleOption *option) const
{
    option->initFrom(this);
    if (orientation() == Qt::Horizontal)
	option->state |= QStyle::State_Horizontal;
}

QToolBarHandle::QToolBarHandle(QToolBar *parent)
    : QWidget(parent), state(0)
{
    setOrientation(parent->orientation());
#ifndef QT_NO_CURSOR
    setCursor(Qt::SizeAllCursor);
#endif
}

void QToolBarHandle::setOrientation(Qt::Orientation orientation)
{
    orient = orientation;

    if (orientation == Qt::Horizontal) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    } else {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    }

    update();
}

Qt::Orientation QToolBarHandle::orientation() const
{ return orient; }

QSize QToolBarHandle::sizeHint() const
{
    QStyleOption opt;
    initStyleOption(&opt);
    const int extent = style()->pixelMetric(QStyle::PM_ToolBarHandleExtent, &opt, this);
    return QSize(extent, extent);
}

void QToolBarHandle::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt;
    initStyleOption(&opt);
    style()->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt, &p, this);
}

void QToolBarHandle::setWindowState(bool floating, bool unplug, const QRect &rect)
{
    bool visible = parentWidget()->isVisible();

    parentWidget()->hide();

    Qt::WindowFlags flags = floating ? Qt::Tool : Qt::Widget;

    flags |= Qt::FramelessWindowHint;
    if (unplug)
        flags |= Qt::X11BypassWindowManagerHint;

    parentWidget()->setWindowFlags(flags);

    if (!rect.isNull())
        parentWidget()->setGeometry(rect);

    if (visible)
        parentWidget()->show();
}

void QToolBarHandle::initDrag(const QPoint &pos)
{
    Q_ASSERT(state == 0);

    state = new DragState;
    state->pressPos = mapToParent(pos);
    state->dragging = false;
    state->widgetItem = 0;
}

void QToolBarHandle::startDrag()
{
    Q_ASSERT(state != 0);
    if (state->dragging)
        return;

    QMainWindow *win = qobject_cast<QMainWindow*>(parentWidget()->parentWidget());
    Q_ASSERT(win != 0);
    QMainWindowLayout *layout = qobject_cast<QMainWindowLayout*>(win->layout());
    Q_ASSERT(layout != 0);

    state->widgetItem = layout->unplug(parentWidget());
    Q_ASSERT(state->widgetItem != 0);
    state->dragging = true;
}

void QToolBarHandle::endDrag()
{
    Q_ASSERT(state != 0);

    if (state->dragging) {
        QMainWindow *win = qobject_cast<QMainWindow*>(parentWidget()->parentWidget());
        Q_ASSERT(win != 0);
        QMainWindowLayout *layout = qobject_cast<QMainWindowLayout*>(win->layout());
        Q_ASSERT(layout != 0);

        if (!layout->plug(state->widgetItem))
            layout->revert(state->widgetItem);
    }
    delete state;
    state = 0;
}

void QToolBarHandle::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    initDrag(event->pos());
}

void QToolBarHandle::mouseReleaseEvent(QMouseEvent*)
{
    releaseMouse();
    endDrag();
}

#ifdef Q_OS_WIN
// a hack to get at grabMouseWhileInWindow() in QWidgetPrivate
class QWidgetPrivateHack : public QWidgetPrivate
{
public:
    void grabMouseWhileInWindow()
        { QWidgetPrivate::grabMouseWhileInWindow(); }
};
#endif

void QToolBarHandle::mouseMoveEvent(QMouseEvent *event)
{
    if (!state)
        return;

    QMainWindow *win = qobject_cast<QMainWindow*>(parentWidget()->parentWidget());
    Q_ASSERT(win != 0);
    QMainWindowLayout *layout = qobject_cast<QMainWindowLayout*>(win->layout());
    Q_ASSERT(layout != 0);

    if (!state->dragging
        && layout->pluggingWidget == 0
        && (event->pos() - state->pressPos).manhattanLength() > QApplication::startDragDistance()) {
            startDrag();
#ifdef Q_OS_WIN
            static_cast<QWidgetPrivateHack*>(d_ptr)->grabMouseWhileInWindow();
#else
            grabMouse();
#endif
    }

    if (state->dragging) {
        QPoint pos = event->globalPos() - state->pressPos;
        parentWidget()->move(pos);
        layout->hover(state->widgetItem, event->globalPos());
    }
}

void QToolBarHandle::unplug(const QRect &_r)
{
    QRect r = _r;
    r.moveTopLeft(parentWidget()->mapToGlobal(QPoint(0, 0)));
    setWindowState(true, true, r);
}

void QToolBarHandle::plug(const QRect &r)
{
    setWindowState(false, false, r);
}

QToolBarWidgetItem::QToolBarWidgetItem(QToolBar *toolBar)
    : QWidgetItem(toolBar)
{
}

QSize QToolBarWidgetItem::sizeHint() const
{
    QWidget *w = const_cast<QToolBarWidgetItem*>(this)->widget();
    QToolBar *tb = qobject_cast<QToolBar*>(w);
    QBoxLayout *layout = qobject_cast<QBoxLayout*>(w->layout());
    Qt::Orientation o = tb->orientation();
    int spacing = layout->spacing();
    int margin = layout->margin();

    int a = 0, b = 0;
    int i = 0;
    while (QLayoutItem *item = layout->itemAt(i)) {
        QSize hint = item->widget()->sizeHint();

        if (i != 0)
            a += spacing;

        a += pick(o, hint);
        b = qMax(b, perp(o, hint));
        ++i;
    }

    if (i == 1) // only the toolbar grip is present
        return QSize(0, 0);

    QSize result;

    rpick(o, result) = a + 2*margin;
    rperp(o, result) = b + 2*margin;

    return result;
}

QSize QToolBarWidgetItem::minimumSize() const
{
    QWidget *w = const_cast<QToolBarWidgetItem*>(this)->widget();
    QToolBar *tb = qobject_cast<QToolBar*>(w);
    QBoxLayout *layout = qobject_cast<QBoxLayout*>(w->layout());
    QStyle *style = tb->parentWidget()->style();
    Qt::Orientation o = tb->orientation();
    int spacing = layout->spacing();
    int margin = layout->margin();

    int a = 0, b = 0;
    int i = 0;
    while (QLayoutItem *item = layout->itemAt(i)) {
        QSize hint = item->widget()->sizeHint();

        if (i != 0)
            a += spacing;

        if (i < 2)
            a += pick(o, hint);
        b = qMax(b, perp(o, hint));

        ++i;
    }

    if (i == 1) // only the toolbar grip is present
        return QSize(0, 0);

    QSize result;
    rpick(o, result) = a + 2*margin + spacing
                        + style->pixelMetric(QStyle::PM_ToolBarExtensionExtent);
    rperp(o, result) = b + 2*margin;

    return result;
}

#endif // QT_NO_TOOLBAR
