/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qgrid.h#5 $
**
** Definition of grid layout widget
**
** Created : 980220
**
** Copyright (C) 1996-1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGRID_H
#define QGRID_H

#include "qwidget.h"

class QGridLayout;

class QGrid : public QWidget
{
    Q_OBJECT
public:
    enum Direction { Horizontal, Vertical };
    QGrid( int n, QWidget *parent=0, const char *name=0, WFlags f=0 );
    QGrid( int n, Direction, QWidget *parent=0, const char *name=0, WFlags f=0 );
    bool event( QEvent * );

    void skip();
protected:
    virtual void childEvent( QChildEvent * );
private:
    QGridLayout *lay;
    int row;
    int col;
    int nRows, nCols;
    Direction dir;
};

#endif //QGRID_H
