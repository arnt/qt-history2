/****************************************************************************
**
** Definition of something or other.
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

#ifndef FOREVER_H
#define FOREVER_H

#include <qwidget.h>


const int numColors = 120;


class Forever : public QWidget
{
    Q_OBJECT
public:
    Forever( QWidget *parent=0, const char *name=0 );
protected:
    void	paintEvent( QPaintEvent * );
private slots:
    void	updateCaption();
private:
    int		rectangles;
    QColor	colors[numColors];
};


#endif
