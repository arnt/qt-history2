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

#ifndef QDECORATIONWINDOWS_QWS_H
#define QDECORATIONWINDOWS_QWS_H

#include "QtGui/qdecorationdefault_qws.h"

QT_MODULE(Gui)

#if !defined(QT_NO_QWS_DECORATION_WINDOWS) || defined(QT_PLUGIN)

class QDecorationWindows : public QDecorationDefault
{
public:
    QDecorationWindows();
    virtual ~QDecorationWindows();

    QRegion region(const QWidget *widget, const QRect &rect, int decorationRegion = All);
    bool paint(QPainter *painter, const QWidget *widget, int decorationRegion = All,
               DecorationState state = Normal);

protected:
    void paintButton(QPainter *painter, const QWidget *widget, int buttonRegion,
                     DecorationState state, const QPalette &pal);
    const char **xpmForRegion(int reg);
};

#endif // QT_NO_QWS_DECORATION_WINDOWS

#endif // QDECORATIONWINDOWS_QWS_H
