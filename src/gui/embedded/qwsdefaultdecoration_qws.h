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

#ifndef QWSDEFAULTDECORATION_QWS_H
#define QWSDEFAULTDECORATION_QWS_H

#ifndef QT_H
#include "qwsmanager_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_MANAGER

#define CORNER_GRAB        16
#define BORDER_WIDTH        4
#define BOTTOM_BORDER_WIDTH        2*BORDER_WIDTH


class QWSDefaultDecoration : public QWSDecoration
{
public:
    QWSDefaultDecoration();
    virtual ~QWSDefaultDecoration();

    virtual QRegion region(const QWidget *, const QRect &rect, Region);
    virtual void paint(QPainter *, const QWidget *);
    virtual void paintButton(QPainter *, const QWidget *, Region, int state);

protected:
    virtual QPixmap pixmapFor(const QWidget *, Region, bool, int&, int&);

    /* Added these virtual functions to enable other styles to be added more easily */
    virtual int getTitleWidth(const QWidget *);
    virtual int getTitleHeight(const QWidget *);

#ifndef QT_NO_IMAGEIO_XPM
    virtual const char **menuPixmap();
    virtual const char **closePixmap();
    virtual const char **minimizePixmap();
    virtual const char **maximizePixmap();
    virtual const char **normalizePixmap();
#endif

private:

    static QPixmap * staticMenuPixmap;
    static QPixmap * staticClosePixmap;
    static QPixmap * staticMinimizePixmap;
    static QPixmap * staticMaximizePixmap;
    static QPixmap * staticNormalizePixmap;

};

#endif // QT_NO_QWS_MANAGER

#endif // QWSDEFAULTDECORATION_QWS_H
