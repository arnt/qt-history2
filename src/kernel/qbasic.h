/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbasic.h#1 $
**
**   Geometry Management
**
**  Author:   Paul Olav Tvete
**  Created:  960406
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBASIC_H
#define QBASIC_H

#include <qintdict.h>
#include <qwidget.h>

class QChain;

class QBasicManager : public QObject
{
    Q_OBJECT
public:
    QBasicManager( QWidget *parent, const char *name=0 );
    void setBorder( int b ) { border = b; } 

    enum Direction { LeftToRight, RightToLeft, Down, Up };
    const int unlimited = 16383;

    QChain *newSerChain( Direction );
    QChain *newParChain( Direction );

    bool add( QChain *destination, QChain *source, int stretch = 0 );
    bool addWidget( QChain *, QWidget *, int stretch = 0 );
    bool addSpacing( QChain *, int minSize, int stretch = 0, int maxSize = unlimited );

    bool doIt();

    QChain *xChain() {  return xC; }
    QChain *yChain() {  return yC; }

protected:
    bool  eventFilter( QObject *, QEvent * );

private:

    enum chainType { Parallel, Serial }; 

    int border;

    void      resizeHandle( QWidget *, const QSize & );
    void      resizeAll();

    QChain *xC;
    QChain *yC;
    QWidget *main;

    
};



#endif
