/****************************************************************************
**
** Implementation of QDesktopWidget class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesktopwidget.h"
#include <qapplication.h>
#include "qt_mac.h"

class QDesktopWidgetPrivate
{
public:
    QDesktopWidgetPrivate();

    int appScreen;
    int screenCount;

    QMemArray<GDHandle> devs;
    QMemArray<QRect> avail_rects;
    QMemArray<QRect> rects;
};

QDesktopWidgetPrivate::QDesktopWidgetPrivate()
{
    appScreen = screenCount = 0;
    for(GDHandle g = GetMainDevice(); g; g = GetNextDevice(g)) 
	screenCount++;
    devs.resize(screenCount);
    rects.resize(screenCount);
    avail_rects.resize(screenCount);
    int i = 0;
    for(GDHandle g = GetMainDevice(); i < screenCount && g; g = GetNextDevice(g), i++) {
	devs[i] = g;
	Rect r = (*g)->gdRect;
	rects[i] = QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
    }
}

QDesktopWidget::QDesktopWidget()
: QWidget(0, "desktop", WType_Desktop)
{
    d = new QDesktopWidgetPrivate;
    setWState(WState_Visible);
}

QDesktopWidget::~QDesktopWidget()
{
    delete d;
}

bool QDesktopWidget::isVirtualDesktop() const
{
    return TRUE;
}

int QDesktopWidget::primaryScreen() const
{
    return d->appScreen;
}

int QDesktopWidget::numScreens() const
{
    return d->screenCount;
}

QWidget *QDesktopWidget::screen(int)
{
    return this;
}

const QRect& QDesktopWidget::availableGeometry(int screen) const
{
    if(screen < 0 || screen >= d->screenCount)
	screen = d->appScreen;
    Rect r;
    GetAvailableWindowPositioningBounds(d->devs[screen], &r);
    return d->avail_rects[screen] = QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

const QRect& QDesktopWidget::screenGeometry(int screen) const
{
    if(screen < 0 || screen >= d->screenCount)
	screen = d->appScreen;
    return d->rects[screen];
}

int QDesktopWidget::screenNumber(QWidget *widget) const
{
    if(!widget)
	return d->appScreen;
    QRect frame = widget->frameGeometry();
    if(!widget->isTopLevel())
	frame.moveTopLeft(widget->mapToGlobal(frame.topLeft()));
    int maxSize = -1, maxScreen = -1;
    for(int i = 0; i < d->screenCount; ++i) {
	QRect sect = d->rects[i].intersect(frame);
	int size = sect.width() * sect.height();
	if(size > maxSize && sect.width() > 0 && sect.height() > 0) {
	    maxSize = size;
	    maxScreen = i;
	}
    }
    return maxScreen;
}

int QDesktopWidget::screenNumber(const QPoint &point) const
{
    for(int i = 0; i < d->screenCount; ++i) {
	if(d->rects[i].contains(point))
	    return i;
    }
    return -1;
}

void QDesktopWidget::resizeEvent(QResizeEvent *)
{
    QDesktopWidgetPrivate *old_d = d;
    d = new QDesktopWidgetPrivate;
    for(int i = 0; i < d->screenCount; i++) {
	if(i > old_d->screenCount || d->rects[i] != old_d->rects[i]) 
	    emit resized(i);
    }
    delete old_d;
}
