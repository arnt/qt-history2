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

#ifndef QDECORATIONHYDRO_QWS_H
#define QDECORATIONHYDRO_QWS_H

#include "QtGui/qdecorationdefault_qws.h"

#if !defined(QT_NO_QWS_DECORATION_HYDRO) || defined(QT_PLUGIN)

class QDecorationHydro : public QDecorationDefault
{
public:
    QDecorationHydro();
    virtual ~QDecorationHydro();

    virtual QRegion region(const QWidget *, const QRect &rect, DecorItem);
    virtual void paintItem(QPainter *, const QWidget *, DecorItem item = All,
                           DecoreState state = Normal);
protected:
//    virtual int getTitleWidth(const QWidget *);
    virtual int getTitleHeight(const QWidget *);
    virtual const char **menuPixmap();
    virtual const char **closePixmap();
    virtual const char **minimizePixmap();
    virtual const char **maximizePixmap();
    virtual const char **normalizePixmap();
};

#endif // QT_NO_QWS_DECORATION_HYDRO

#endif // QDECORATIONHYDRO_QWS_H
