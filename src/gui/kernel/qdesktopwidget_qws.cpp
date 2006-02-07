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

#include "qdesktopwidget.h"
#include "qvector.h"
#include "qwidget_p.h"

class QDesktopWidgetPrivate : public QWidgetPrivate
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
: QWidget(*new QDesktopWidgetPrivate, 0, Qt::Desktop)
{
    setObjectName(QLatin1String("desktop"));
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

const QRect QDesktopWidget::availableGeometry(int) const
{
    extern QRect qt_maxWindowRect;
    QRect r = qt_maxWindowRect;
    return r;
}

const QRect QDesktopWidget::screenGeometry(int) const
{
    QRect r = frameGeometry();
    return r;
}

int QDesktopWidget::screenNumber(const QWidget *) const
{
    return d_func()->appScreen;
}

int QDesktopWidget::screenNumber(const QPoint &) const
{
    return d_func()->appScreen;
}

void QDesktopWidget::resizeEvent(QResizeEvent *)
{
}
