/****************************************************************************
** $Id: //depot/qt/main/examples/forever/forever.h#1 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
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
    void	timerEvent( QTimerEvent * );
private slots:
    void	updateCaption();
private:
    int		rectangles;
    QColor	colors[numColors];
};


#endif
