/****************************************************************************
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

#ifndef QWSKDEDECORATION_QWS_H
#define QWSKDEDECORATION_QWS_H

#ifndef QT_H
#include "qwsdefaultdecoration_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_KDE_WM_STYLE


class QWSKDEDecoration : public QWSDefaultDecoration
{
public:
    QWSKDEDecoration();
    virtual ~QWSKDEDecoration();

    virtual QRegion region(const QWidget *, const QRect &rect, Region);
    virtual void paint(QPainter *, const QWidget *);
    virtual void paintButton(QPainter *, const QWidget *, Region, int state);
protected:
/*
    virtual int getTitleWidth(const QWidget *);
    virtual int getTitleHeight(const QWidget *);
    virtual const char **menuPixmap();
    virtual const char **closePixmap();
    virtual const char **minimizePixmap();
    virtual const char **maximizePixmap();
    virtual const char **normalizePixmap();
*/
};

#endif // QT_NO_QWS_KDE_WM_STYLE

#endif // QWSKDEDECORATION_QWS_H
