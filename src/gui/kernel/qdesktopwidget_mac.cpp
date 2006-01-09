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

#include "qapplication.h"
#include "qdesktopwidget.h"
#include <private/qt_mac_p.h>
#include "qwidget_p.h"

/*****************************************************************************
  Externals
 *****************************************************************************/
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp

/*****************************************************************************
  QDesktopWidget member functions
 *****************************************************************************/
class QDesktopWidgetPrivate : public QWidgetPrivate
{
public:
    QDesktopWidgetPrivate();

    int appScreen;
    int screenCount;

    QVector<GDHandle> devs;
    QVector<QRect> rects;
    static void readScreenInformation(QVector<GDHandle> &devices, QVector<QRect> &screenRects, int &screenCount);
};

QDesktopWidgetPrivate::QDesktopWidgetPrivate()
{
    appScreen = screenCount = 0;
    readScreenInformation(devs, rects, screenCount);

}

void QDesktopWidgetPrivate::readScreenInformation(QVector<GDHandle> &devices, QVector<QRect> &screenRects, int &screenCount)
{
    screenCount = 0;
    for (GDHandle g = GetMainDevice(); g; g = GetNextDevice(g))
        ++screenCount;
    devices.resize(screenCount);
    screenRects.resize(screenCount);
    int i = 0;
    for (GDHandle g = GetMainDevice(); i < screenCount && g; g = GetNextDevice(g), ++i) {
        devices[i] = g;
        Rect r = (*g)->gdRect;
        screenRects[i] = QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
    }
}

QDesktopWidget::QDesktopWidget()
: QWidget(*new QDesktopWidgetPrivate, 0, Qt::Desktop)
{
    setObjectName(QLatin1String("desktop"));
    setAttribute(Qt::WA_WState_Visible);
}

QDesktopWidget::~QDesktopWidget()
{
}

bool QDesktopWidget::isVirtualDesktop() const
{
    return true;
}

int QDesktopWidget::primaryScreen() const
{
    return d_func()->appScreen;
}

int QDesktopWidget::numScreens() const
{
    return d_func()->screenCount;
}

QWidget *QDesktopWidget::screen(int)
{
    return this;
}

const QRect QDesktopWidget::availableGeometry(int screen) const
{
    Q_D(const QDesktopWidget);
    if(screen < 0 || screen >= d->screenCount)
        screen = d->appScreen;
    Rect r;
    GetAvailableWindowPositioningBounds(d->devs[screen], &r);
    return QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

const QRect QDesktopWidget::screenGeometry(int screen) const
{
    Q_D(const QDesktopWidget);
    if(screen < 0 || screen >= d->screenCount)
        screen = d->appScreen;
    return d->rects[screen];
}

int QDesktopWidget::screenNumber(const QWidget *widget) const
{
    Q_D(const QDesktopWidget);
    if(!widget)
        return d->appScreen;
    QRect frame = widget->frameGeometry();
    if(!widget->isWindow())
        frame.moveTopLeft(widget->mapToGlobal(QPoint(0,0)));
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
    Q_D(const QDesktopWidget);
    int closestScreen = -1;
    int shortestDistance = INT_MAX;
    for (int i = 0; i < d->screenCount; ++i) {
        int thisDistance = d->pointToRect(point, d->rects.at(i));
        if (thisDistance < shortestDistance) {
            shortestDistance = thisDistance;
            closestScreen = i;
        }
    }
    return closestScreen;
}

void QDesktopWidget::resizeEvent(QResizeEvent *)
{
    Q_D(QDesktopWidget);
    int oldScreenCount = d->screenCount;
    QVector<QRect> oldRects = d->rects;

    QVector<GDHandle> newDevs;
    QVector<QRect> newRects;
    int newScreenCount;
    QDesktopWidgetPrivate::readScreenInformation(newDevs, newRects, newScreenCount);
    for (int i = 0; i < newScreenCount; ++i) {
        if (i > oldScreenCount || newRects.at(i) != oldRects.at(i))
            emit resized(i);
    }
    d->screenCount = newScreenCount;
    d->devs = newDevs;
    d->rects = newRects;
}
