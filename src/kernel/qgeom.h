/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgeom.h#3 $
**
**  Studies in Geometry Management
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

class QBox;

//public inheritance because we need enum Direction...
class QGeomManager : public QBasicManager
{
public:    
    QGeomManager( QWidget *parent, Direction, int border=0, 
		  int autoBorder = -1, const char *name=0 );
    QBox *box() { return topBox; }
    int defaultBorder() const { return defBorder; }
private:
    QBox * topBox;
    int defBorder;
    void initBox( Direction, int, int );
};


class QBox 
{
public:

    QBox( QGeomManager*, QGeomManager::Direction d );
    QBox( QGeomManager*, QChain *, QChain *, QGeomManager::Direction d ); 

    enum alignment { alignCenter, alignTop, alignLeft=alignTop,
		 alignBottom, alignRight=alignBottom, alignFixed };
    
    void addBox( QBox *, int stretch = 0, alignment a = alignFixed );
    void addWidget( QWidget *, int stretch = 0, alignment a = alignFixed );
    void addSpacing( int size, int stretch = 0 ); 

    QGeomManager::Direction direction() const { return dir; }

    void addMinStrut( int );
    void addMaxStrut( int );

private:
    void addB( QBox *, int stretch );
    QGeomManager::Direction dir;
    QGeomManager * gm;
    QChain * parChain;
    QChain * serChain;
    bool    pristine;
    int     defBorder() { return gm->defaultBorder(); }
};





#endif
