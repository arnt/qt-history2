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

#ifndef QDECORATIONDEFAULT_QWS_H
#define QDECORATIONDEFAULT_QWS_H

#include "qwsmanager_qws.h"

#ifndef QT_NO_QWS_MANAGER
#if !defined(QT_NO_QWS_DECORATION_DEFAULT) || defined(QT_PLUGIN)


#define CORNER_GRAB 16
#define BORDER_WIDTH  4
#define BOTTOM_BORDER_WIDTH BORDER_WIDTH


class QDecorationDefault : public QDecoration
{
public:
    QDecorationDefault();
    virtual ~QDecorationDefault();

    virtual QRegion region(const QWidget *widget, const QRect &rect, int decorationRegion = All);
    virtual bool paint(QPainter *painter, const QWidget *widget, int decorationRegion = All,
                       DecorationState state = Normal);

protected:
    virtual QPixmap pixmapFor(const QWidget *widget, int decorationRegion, bool on, int &xoff,
                              int &yoff);

    /* Added these virtual functions to enable other styles to be added more easily */
    virtual int getTitleWidth(const QWidget *widget);
    virtual int getTitleHeight(const QWidget *widget);

#ifndef QT_NO_IMAGEIO_XPM
    virtual const char **helpPixmap();
    virtual const char **menuPixmap();
    virtual const char **closePixmap();
    virtual const char **minimizePixmap();
    virtual const char **maximizePixmap();
    virtual const char **normalizePixmap();
#endif

private:

    static QPixmap * staticHelpPixmap;
    static QPixmap * staticMenuPixmap;
    static QPixmap * staticClosePixmap;
    static QPixmap * staticMinimizePixmap;
    static QPixmap * staticMaximizePixmap;
    static QPixmap * staticNormalizePixmap;

};
#endif // QT_NO_QWS_DECORATION_DEFAULT
#endif // QT_NO_QWS_MANAGER

#endif // QDECORATIONDEFAULT_QWS_H
