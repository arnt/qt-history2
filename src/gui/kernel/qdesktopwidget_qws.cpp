/****************************************************************************
**
** Implementation of QDesktopWidget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesktopwidget.h"
#include "qvector.h"

class QDesktopWidgetPrivate
{
public:
    QDesktopWidgetPrivate();

    int appScreen;
    int screenCount;

    QVector<QRect> rects;
};

QDesktopWidgetPrivate::QDesktopWidgetPrivate()
{
    appScreen = 0;
    screenCount = 1;

    rects.resize(screenCount);
    //### Get the rects for the different screens and put them into rects
}

QDesktopWidget::QDesktopWidget()
: QWidget(0, "desktop", WType_Desktop)
{
    d = new QDesktopWidgetPrivate;
}

QDesktopWidget::~QDesktopWidget()
{
    delete d;
}

bool QDesktopWidget::isVirtualDesktop() const
{
    return true;
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
    return screenGeometry(screen);
}

const QRect& QDesktopWidget::screenGeometry(int) const
{
    // use max window rect?
#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
    static QRect r;
    r = frameGeometry();
#else
    static QRect r = frameGeometry();
#endif
    return r;
}

int QDesktopWidget::screenNumber(const QWidget *) const
{
    return d->appScreen;
}

int QDesktopWidget::screenNumber(const QPoint &) const
{
    return d->appScreen;
}

void QDesktopWidget::resizeEvent(QResizeEvent *)
{
    delete d;
    d = new QDesktopWidgetPrivate;
}
