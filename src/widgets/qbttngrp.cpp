/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbttngrp.cpp#5 $
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
static char ident[] = "$Id: //depot/qt/main/src/widgets/qbttngrp.cpp#5 $";
#endif


/*!
\class QButtonGroup qbttngrp.h
\brief The QButtonGroup organizes QButton widgets in a group for managing
buttons that belong together.

The QButtonGroup class contains an internal list of QButton widgets, and
the contained buttons do not have to be child widgets of the button group.

There are two basic ways of using a button group:
<ol>
<li> A button group can be a normal parent widget for a set of buttons.
Because QButtonGroup inherits QGroupBox, it can display a frame and
a title.
<li> A button group can be an invisible widget that only maintains a
list of buttons. The buttons will then have other parent widgets.
</ol>

The QButtonGroup is especially useful for radio buttons (QRadioButton).
When one radiobutton in the group is switched on, all other radio
buttons will be automatically switched off.

All buttons that are inserted into the button group have an (optional)
identifier. The button group connects the QButton::clicked() signal to
buttonClicked() slot.  When a button is clicked, the button group receives
the signal and emits an clicked() signal, containing the button identifier.
*/


struct QButtonItem
{
    QButton *button;
    int	     id;
};

typedef declare(QListM,QButtonItem);


/*!
Constructs a button group with a parent widget and a widget name.
*/

QButtonGroup::QButtonGroup( QWidget *parent, const char *name )
    : QGroupBox( parent, name )
{
    initMetaObject();
    buttons = new QButtonList;
    CHECK_PTR( buttons );
    buttons->setAutoDelete( FALSE );
}

/*!
Destroys the button group and all its child widgets.
*/

QButtonGroup::~QButtonGroup()
{
    for ( register QButtonItem *bi=buttons->first(); bi; bi=buttons->next() )
	bi->button->group = 0;
    delete buttons;
}


/*!
Inserts a button with the identifier \e id into the button group.

If \e id is -1, then the button will get an identifier (number of
buttons in the group).  Inserting several buttons with \e id = -1
will assign the identifers 0, 1, 2, etc.
*/

void QButtonGroup::insert( QButton *button, int id )
{
    register QButtonItem *bi = new QButtonItem;
    CHECK_PTR( bi );
    bi->id = id == -1 ? buttons->count() : id;
    bi->button = button;
    button->group  = this;
    buttons->append( bi );
    connect( button, SIGNAL(clicked()), SLOT(buttonClicked()) );
}

/*!
Removes a button from the button group.
*/

void QButtonGroup::remove( QButton *button )
{
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() ) {
	if ( i->button == button ) {
	    buttons->remove();
	    button->group = 0;
	    button->disconnect( this );
	    break;
	}
    }
}


void QButtonGroup::buttonClicked()
{
    int id = -1;
    QObject *sobj = sender();			// object that sent the signal
    bool switch_off = sobj->inherits("QRadioButton");
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() ) {
	if ( sobj == i->button )		// button was clicked
	    id = i->id;
	else if ( switch_off )			// switch all other off
	    i->button->switchOff();
    }
    if ( id != -1 )
	emit clicked( id );
}
