/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbasic.h#15 $
**
**   Geometry Management
**
**  Created:  960406
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QBASIC_H
#define QBASIC_H

#include "qintdict.h"
#include "qwidget.h"

class QChain;
struct QBasicManagerData;

class QBasicManager : public QObject
{
    Q_OBJECT
public:
    QBasicManager( QWidget *parent, const char *name=0 );
    ~QBasicManager();

    void setBorder( int b ) { border = b; }

    enum Direction { LeftToRight, RightToLeft, Down, Up };
    enum { unlimited = QCOORD_MAX };

    QChain *newSerChain( Direction );
    QChain *newParChain( Direction );

    bool add( QChain *destination, QChain *source, int stretch = 0 );
    bool addWidget( QChain *, QWidget *, int stretch = 0 );
    bool addSpacing( QChain *, int minSize, int stretch = 0, int maxSize = unlimited );

    bool addBranch( QChain *destination, QChain *branch, int fromIndex,
		    int toIndex );
    void setStretch( QChain*, int );
    bool activate();

    void freeze( int w = 0, int h = 0 );

    QChain *xChain() {	return xC; }
    QChain *yChain() {	return yC; }

    void  setMenuBar( QWidget *w ) { menuBar = w; }

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
    QWidget *menuBar;
    QBasicManagerData *extraData;
    bool frozen;

private:	// Disabled copy constructor and operator=
    QBasicManager( const QBasicManager & ) {}
    QBasicManager &operator=( const QBasicManager & ) { return *this; }
};


#endif
