/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbttngrp.cpp#2 $
**
** Implementation of QButtonGroup class
**
** Author  : Eirik Eng
** Created : 950130
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#define  QButtonList QListM_QButtonItem
#include "qbttngrp.h"
#include "qbutton.h"
#include "qlist.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qbttngrp.cpp#2 $";
#endif


struct QButtonItem
{
    QButton *button;
    int	     id;
};

typedef declare(QListM,QButtonItem);


QButtonGroup::QButtonGroup( QWidget *parent, const char *name )
	: QGroupBox( parent, name )
{
    initMetaObject();
    buttons = new QButtonList;
    CHECK_PTR( buttons );
    buttons->setAutoDelete( TRUE );
}

QButtonGroup::~QButtonGroup()
{
    delete buttons;
}


void QButtonGroup::insert( QButton *button, int id )
{
    register QButtonItem *i = new QButtonItem;
    CHECK_PTR( i );
    i->id = id == -1 ? buttons->count() : id;
    i->button = button;
    buttons->append( i );
    connect( button, SIGNAL(clicked()), SLOT(buttonClicked()) );
}

void QButtonGroup::remove( QButton *button )
{
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() ) {
	if ( i->button == button ) {
	    buttons->remove();
	    break;
	}
    }
}


void QButtonGroup::buttonClicked()
{
    int id = -1;
    QObject *sobj = sender();			// object that sent the signal
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() ) {
	if ( sobj == i->button )
	    id = i->button->id();
	else
	    i->button->switchOff();
    }
    if ( id != -1 )
	emit selected( id );
}
