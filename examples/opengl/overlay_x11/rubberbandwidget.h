/****************************************************************************
**
** Definition of a widget that draws a rubberband. Designed to be used .
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef RUBBERBANDWIDGET_H
#define RUBBERBANDWIDGET_H

#include <qwidget.h>


class RubberbandWidget : public QWidget
{
public:
    RubberbandWidget( QColor transparentColor, QWidget *parent=0, 
		      const char *name=0, WFlags f=0 );

protected:
    void mousePressEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );

    QColor c;
    QPoint p1;
    QPoint p2;
    QPoint p3;
    bool on;
};

#endif
