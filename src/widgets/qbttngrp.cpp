/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbttngrp.cpp#1 $
**
** Implementation of QButtonGroup class
**
** Author  : Eirik Eng
** Created : 950130
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qbttngrp.h"
#include "qbutton.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qbttngrp.cpp#1 $";
#endif

struct QButtonItem
{
    QButton *b;
    int     id;
};

#include "qlist.h"
typedef declare(QListM,QButtonItem)         QButtonList;
typedef declare(QListIteratorM,QButtonItem) QButtonListIt;

QButtonGroup::QButtonGroup( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    initMetaObject();
    buttons = new QButtonList;
    buttons->setAutoDelete( TRUE );
}

void QButtonGroup::init()
{
    buttons = new QButtonList;
    buttons->setAutoDelete( TRUE );
}


QButtonGroup::~QButtonGroup()
{
    delete buttons;
}

void QButtonGroup::insert( QButton *b, int id, int index )
{
    if ( index > (int)buttons->count() ) {
#if defined(CHECK_RANGE)
	warning( "QButtonGroup::insertItem: Index %d out of range", index );
#endif
	return;
    }
    if ( index < 0 )				// append
	index = buttons->count();    
    register QButtonItem *bi = new QButtonItem;
    CHECK_PTR( bi );
    bi->id = id == -1 ? index : id;
    bi->b  = b;
    buttons->insert( index, bi );
    connect( b, SIGNAL(clicked()), SLOT(buttonClicked()) );
}

void QButtonGroup::buttonClicked()
{
    int id = -1;
    register QButton *b;
    register QButtonItem *bi = buttons->first();
    while ( bi ) {
        b = bi->b;
        if ( b == sender() ) {
	    if ( !b->isOn() )
	        b->switchOn();
            id = bi->id;
	} else {
	    if ( b->isOn() )
	        b->switchOff();
	}
	bi = buttons->next();
    }
    if ( id != -1 )
	emit selected( id );
}

