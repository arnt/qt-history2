/****************************************************************************
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

#ifndef QWSBEOSDECORATION_QWS_H
#define QWSBEOSDECORATION_QWS_H

#ifndef QT_H
#include "qwsdefaultdecoration_qws.h"
#endif // QT_H


#ifndef QT_NO_QWS_BEOS_WM_STYLE


class QWSBeOSDecoration : public QWSDefaultDecoration
{
public:
    QWSBeOSDecoration();
    virtual ~QWSBeOSDecoration();

    virtual QRegion region(const QWidget *, const QRect &rect, Region);
    virtual void paint(QPainter *, const QWidget *);
    virtual void paintButton(QPainter *, const QWidget *, Region, int state);
protected:
    virtual int getTitleWidth(const QWidget *);
    virtual int getTitleHeight(const QWidget *);
    virtual const char **menuPixmap();
//    virtual const char **normalizePixmap();
};

#endif // QT_NO_QWS_BEOS_WM_STYLE

#endif // QWSBEOSDECORATION_QWS_H
