/****************************************************************************
** $Id: //depot/qt/main/examples/sheet/pie.h#1 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef PIE_H
#define PIE_H

#include <qwidget.h>

class PieView : public QWidget
{
//    Q_OBJECT
public:
    PieView( int *, QString * = 0 );
    void restart( int *, QString * = 0 );
protected:
    void   paintEvent( QPaintEvent * );
    void   drawPies(  QPainter * );
private:
    int *nums;
    QString *strs;
};

#endif
