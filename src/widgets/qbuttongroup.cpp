/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbuttongroup.cpp#7 $
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
static char ident[] = "$Id: //depot/qt/main/src/widgets/qbuttongroup.cpp#7 $";
#endif


/*!
\class QButtonGroup qbttngrp.h
\brief The QButtonGroup widget organizes QButton widgets in a group.

A button group widget makes it easier to deal with groups of buttons.  A
button in a button group can be associated with a unique identifer. The
button group emits a clicked() signal with this identifier when the button
is clicked.

A button group which contains radio buttons (QRadioButton) will switch
off all radio buttons except the one that was clicked.

There are two standard ways of using a button group:
<ol>
<li> A button group can be a normal parent widget for a set of buttons.
Because QButtonGroup inherits QGroupBox, it can display a frame and
a title. The buttons get identifiers 0, 1, 2 etc. in the order they are
inserted.
<li> A button group can be an invisible widget and the contained buttons
have some other parent widget.
A button must be manually inserted using the insert() function with an
identifer.
</ol>
*/


struct QButtonItem
{
    QButton *button;
    int	     id;
};

typedef declare(QListM,QButtonItem);


/*!
Constructs a button group with no title.

The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QButtonGroup::QButtonGroup( QWidget *parent, const char *name )
    : QGroupBox( parent, name )
{
    init();
}

/*!
Constructs a button group with a title.

The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QButtonGroup::QButtonGroup( const char *title, QWidget *parent,
			    const char *name )
    : QGroupBox( title, parent, name )
{
    init();
}

void QButtonGroup::init()
{
    initMetaObject();
    buttons = new QButtonList;
    CHECK_PTR( buttons );
    buttons->setAutoDelete( FALSE );
}

/*!
Destroys the button group and its child widgets.
*/

QButtonGroup::~QButtonGroup()
{
    for ( register QButtonItem *bi=buttons->first(); bi; bi=buttons->next() )
	bi->button->group = 0;
    delete buttons;
}


/*!
Inserts a button with the identifier \e id into the button group.
Returns the identifier.

If \e id is -1, then the button will get a unique identifier.
*/

int QButtonGroup::insert( QButton *button, int id )
{
    static int auto_id_count = 0;
    register QButtonItem *bi = new QButtonItem;
    CHECK_PTR( bi );
    bi->id = id == -1 ? --auto_id_count : id;
    bi->button = button;
    button->group  = this;
    buttons->append( bi );
    connect( button, SIGNAL(clicked()), SLOT(buttonClicked()) );
    return bi->id;
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


/*!
\internal
This slot is activated when one of the buttons in the group emits the
QButton::clicked() signal.
*/

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
