/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgeom.h#12 $
**
**  Geometry Management
**
**  Created:  960416
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QGEOM_H
#define QGEOM_H

#include "qbasic.h"

class QBoxLayout : public QObject
{
public:
    QBoxLayout(	 QWidget *parent, QBasicManager::Direction, int border=0,
		 int autoBorder = -1, const char *name=0 );
    int defaultBorder() const { return defBorder; }

    bool doIt() { return bm->doIt(); }
    void freeze( int w, int h );
    void freeze() { freeze( 0, 0 ); }

    enum alignment { alignCenter, alignTop, alignLeft,
		 alignBottom, alignRight /*, alignBoth */};

    QBoxLayout *addNewBox( QBasicManager::Direction, int stretch = 0
			      /*alignment a = alignBoth*/ );
    void addWidget( QWidget *, int stretch = 0, alignment a = alignCenter );
    void addSpacing( int size );
    void addStretch( int stretch = 0 );

    QBasicManager::Direction direction() const { return dir; }

    void addStrut( int );
    //void addMaxStrut( int );

private:
    QBoxLayout(	 QBoxLayout *parent, QBasicManager::Direction,
		 const char *name=0 );
    void addB( QBoxLayout *, int stretch );

    QBasicManager * bm;
    int defBorder;
    QBasicManager::Direction dir;
    QChain * parChain;
    QChain * serChain;
    bool    pristine;
    bool    topLevel;


private:	// Disabled copy constructor and operator=
    QBoxLayout( const QBoxLayout & ) {}
    QBoxLayout &operator=( const QBoxLayout & ) { return *this; }

};

#endif
