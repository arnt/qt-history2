/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgmanager.h#13 $
**
** Definition of QGManager class (workhorse for QLayout classes)
**
** Created : 960406
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGMANAGER_H
#define QGMANAGER_H

#ifndef QT_H
#include "qintdict.h"
#include "qwidget.h"
#endif // QT_H


class QChain;
struct QGManagerData;

class QGManager : public QObject
{
    Q_OBJECT
public:
    QGManager( QWidget *parent, const char *name=0 );
    ~QGManager();

    void setBorder( int b ) { border = b; }

    enum Direction { LeftToRight, RightToLeft, Down, Up };
    enum { unlimited = QCOORD_MAX };

    QChain *newSerChain( Direction );
    //    QChain *newSerChain( Direction, int );
    QChain *newParChain( Direction );

    bool add( QChain *destination, QChain *source, int stretch = 0 );
    bool addWidget( QChain *, QWidget *, int stretch = 0 );
    bool addSpacing( QChain *, int minSize, int stretch = 0, int maxSize = unlimited );

    bool addBranch( QChain *destination, QChain *branch, int fromIndex,
		    int toIndex );
    void setStretch( QChain*, int );
    bool activate();

    void freeze( int w = 0, int h = 0 );
    void unFreeze();

    QChain *xChain() {	return xC; }
    QChain *yChain() {	return yC; }

    void  setMenuBar( QWidget *w ) { menuBar = w; }

    QWidget *mainWidget() { return main; }

    void remove( QWidget *w );

    void setName( QChain *, const char * );
        
protected:
    bool  eventFilter( QObject *, QEvent * );

private:
    int border;

    void      resizeHandle( QWidget *, const QSize & );
    void      resizeAll();

    QChain *xC;
    QChain *yC;
    QWidget *main;
    QWidget *menuBar;
    int	    menuBarHeight;
    QGManagerData *extraData;
    bool frozen;

private:	// Disabled copy constructor and operator=
    QGManager( const QGManager & );
    QGManager &operator=( const QGManager & );
};


#endif // QGMANAGER_H
