/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgeom.h#6 $
**
**  Geometry Management
**
**  Author:   Paul Olav Tvete
**  Created:  960416
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGEOM_H
#define QGEOM_H

#include "qbasic.h"

class QBoxLayout
{
public:
    QBoxLayout(  QWidget *parent, QBasicManager::Direction, int border=0, 
		 int autoBorder = -1, const char *name=0 );
    int     defaultBorder() { return defBorder; }

    bool doIt() { return bm->doIt(); }
public:

    enum alignment { alignCenter, alignTop, alignLeft,
		 alignBottom, alignRight /*, alignBoth */};
    
    QBoxLayout *insertNewBox( QBasicManager::Direction, int stretch = 0
			      /*alignment a = alignBoth*/ );
    void insert( QWidget *, int stretch = 0, alignment a = alignCenter );
    void addSpacing( int size, int stretch = 0 ); 

    QBasicManager::Direction direction() const { return dir; }

    void addStrut( int );
    //void addMaxStrut( int );

private:
    QBoxLayout(  QBoxLayout *parent, QBasicManager::Direction );
    void addB( QBoxLayout *, int stretch );
    //   QBox( QGeomManager*, QChain *, QChain *, QGeomManager::Direction d );

    QBasicManager * bm;
    int defBorder;
    QBasicManager::Direction dir;
    QChain * parChain;
    QChain * serChain;
    bool    pristine;

private:	// Disabled copy constructor and operator=
    QBoxLayout( const QBoxLayout & ) {}
    QBoxLayout &operator=( const QBoxLayout & ) { return *this; }

};

#endif
