/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qgrid.h#11 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QGRID_H
#define QGRID_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class QGridLayout;

class Q_EXPORT QGrid : public QWidget
{
    Q_OBJECT
public:
    enum Direction { Horizontal, Vertical };
    QGrid( int n, QWidget *parent=0, const char *name=0, WFlags f=0 );
    QGrid( int n, Direction, QWidget *parent=0, const char *name=0, WFlags f=0 );

protected:
    virtual void childEvent( QChildEvent * );
private:
    void skip(); // child events are deferred, so this cannot be public
    QGridLayout *lay;
    int row;
    int col;
    int nRows, nCols;
    Direction dir;
};

#endif //QGRID_H
