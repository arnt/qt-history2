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

#include "qdesktopwidget.h"
#include "qscreen_qws.h"
#include "private/qapplication_p.h"

QDesktopWidget::QDesktopWidget()
    : QWidget(0, Qt::Desktop)
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
    return 0;
}

int QDesktopWidget::numScreens() const
{
    QScreen *screen = QScreen::instance();
    if (!screen)
        return 0;

    const QList<QScreen*> subScreens = screen->subScreens();
    return qMax(subScreens.size(), 1);
}

QWidget *QDesktopWidget::screen(int)
{
    return this;
}

const QRect QDesktopWidget::availableGeometry(int screenNo) const
{
    const QScreen *screen = QScreen::instance();
    if (screenNo == -1)
        screenNo = 0;
    if (!screen || screenNo < 0)
        return QRect();

    const QList<QScreen*> subScreens = screen->subScreens();
    if (!subScreens.isEmpty()) {
        if (screenNo >= subScreens.size())
            return QRect();
        screen = subScreens.at(screenNo);
    }

    QApplicationPrivate *ap = QApplicationPrivate::instance();
    const QRect r = ap->maxWindowRect(screen);
    if (!r.isEmpty())
        return r;

    return screen->region().boundingRect();
}

const QRect QDesktopWidget::screenGeometry(int screenNo) const
{
    const QScreen *screen = QScreen::instance();
    if (screenNo == -1)
        screenNo = 0;
    if (!screen || screenNo < 0)
        return QRect();

    const QList<QScreen*> subScreens = screen->subScreens();
    if (subScreens.size() == 0 && screenNo == 0)
        return screen->region().boundingRect();

    if (screenNo >= subScreens.size())
        return QRect();

    return subScreens.at(screenNo)->region().boundingRect();
}

int QDesktopWidget::screenNumber(const QWidget *w) const
{
    if (!w)
        return 0;

    QRect frame = w->frameGeometry();
    if (!w->isWindow())
        frame.moveTopLeft(w->mapToGlobal(QPoint(0, 0)));
    const QPoint midpoint = (frame.topLeft() + frame.bottomRight()) / 2;
    return screenNumber(midpoint);
}

int QDesktopWidget::screenNumber(const QPoint &p) const
{
    const QScreen *screen = QScreen::instance();
    if (!screen || !screen->region().contains(p))
        return -1;

    const QList<QScreen*> subScreens = screen->subScreens();
    if (subScreens.size() == 0)
        return 0;

    for (int i = 0; i < subScreens.size(); ++i)
        if (subScreens.at(i)->region().contains(p))
            return i;

    return -1;
}

void QDesktopWidget::resizeEvent(QResizeEvent *)
{
}
