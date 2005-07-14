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

#ifndef QDECORATIONSTYLED_QWS_H
#define QDECORATIONSTYLED_QWS_H

#include "QtGui/qdecorationdefault_qws.h"

QT_MODULE(Gui)

#if !defined(QT_NO_QWS_DECORATION_STYLED) || defined(QT_PLUGIN)

class QDecorationStyled : public QDecorationDefault
{
public:
    QDecorationStyled();
    virtual ~QDecorationStyled();

    QRegion region(const QWidget *widget, const QRect &rect, int decorationRegion = All);
    bool paint(QPainter *painter, const QWidget *widget, int decorationRegion = All,
               DecorationState state = Normal);
    int titleBarHeight(const QWidget *widget);
};

#endif // QT_NO_QWS_DECORATION_STYLED

#endif // QDECORATIONSTYLED_QWS_H
