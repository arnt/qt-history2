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

#ifndef QDECORATIONKDE2_QWS_H
#define QDECORATIONKDE2_QWS_H

#include "qdecorationdefault_qws.h"

#if !defined(QT_NO_QWS_DECORATION_KDE2) || defined(QT_PLUGIN)

class QDecorationKDE2 : public QDecorationDefault
{
public:
    QDecorationKDE2();
    virtual ~QDecorationKDE2();

    QRegion region(const QWidget *widget, const QRect &rect, int decorationRegion = All);
    bool paint(QPainter *painter, const QWidget *widget, int decorationRegion = All,
               DecorationState state = Normal);
    int titleBarHeight(const QWidget *widget);
};

#endif // QT_NO_QWS_DECORATION_KDE2

#endif // QDECORATIONKDE2_QWS_H
