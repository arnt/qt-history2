/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbuttongroup.cpp#18 $
**
** Implementation of QButtonGroup class
**
** Author  : Eirik Eng
** Created : 950130
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define	 QButtonList QListM_QButtonItem
#include "qbttngrp.h"
#include "qbutton.h"
#include "qlist.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qbuttongroup.cpp#18 $")


/*----------------------------------------------------------------------------
  \class QButtonGroup qbttngrp.h
  \brief The QButtonGroup widget organizes QButton widgets in a group.

  \ingroup realwidgets

  A button group widget makes it easier to deal with groups of buttons.  A
  button in a button group can be associated with a unique identifer. The
  button group emits a clicked() signal with this identifier when the
  button is clicked.

  A button group which contains radio buttons (QRadioButton) will switch
  off all radio buttons except the one that was clicked.

  There are two standard ways of using a button group:
  <ol>
  <li>A button group can be a normal parent widget for a set of buttons.
  Because QButtonGroup inherits QGroupBox, it can display a frame and
  a title. The buttons are assigned identifiers 0, 1, 2 etc. in the order
  they are inserted.
  <li>A button group can be an invisible widget and the contained buttons
  have some other parent widget.
  A button must be manually inserted using the insert() function with an
  identifer.
  </ol>
 ----------------------------------------------------------------------------*/


struct QButtonItem
{
    QButton *button;
    int	     id;
};

declare(QListM,QButtonItem);


/*----------------------------------------------------------------------------
  Constructs a button group with no title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
 ----------------------------------------------------------------------------*/

QButtonGroup::QButtonGroup( QWidget *parent, const char *name )
    : QGroupBox( parent, name )
{
    init();
}

/*----------------------------------------------------------------------------
  Constructs a button group with a title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
 ----------------------------------------------------------------------------*/

QButtonGroup::QButtonGroup( const char *title, QWidget *parent,
			    const char *name )
    : QGroupBox( title, parent, name )
{
    init();
}

/*----------------------------------------------------------------------------
  \internal
  Initializes the button group.
 ----------------------------------------------------------------------------*/

void QButtonGroup::init()
{
    initMetaObject();
    buttons = new QButtonList;
    CHECK_PTR( buttons );
    buttons->setAutoDelete( TRUE );
}

/*----------------------------------------------------------------------------
  Destroys the button group and its child widgets.
 ----------------------------------------------------------------------------*/

QButtonGroup::~QButtonGroup()
{
    for ( register QButtonItem *bi=buttons->first(); bi; bi=buttons->next() )
	bi->button->group = 0;
    delete buttons;
}


/*----------------------------------------------------------------------------
  Inserts a button with the identifier \e id into the button group.
  Returns the button identifier.

  The button is assigned the identifier \e id or an automatically
  generated identifier.  It works as follows: If \e id >= 0, this
  identifier is assigned.  If \e id == -1 (default), the identifier is
  equal to the number of buttons in the group.  If \e id is any other
  negative integer, for instance -2, a unique identifier (negative
  integer \< -1) is generated.

  Inserting several buttons with \e id = -1 assigns the identifers 0,
  1, 2, etc.

  \sa remove()
 ----------------------------------------------------------------------------*/

int QButtonGroup::insert( QButton *button, int id )
{
    static int seq_no = -2;
    register QButtonItem *bi = new QButtonItem;
    CHECK_PTR( bi );
    if ( id < -1 )
	bi->id = seq_no--;
    else if ( id == -1 )
	bi->id = buttons->count();
    else
	bi->id = id;
    bi->button = button;
    button->group  = this;
    buttons->append( bi );
    connect( button, SIGNAL(pressed()) , SLOT(buttonPressed()) );
    connect( button, SIGNAL(released()), SLOT(buttonReleased()) );
    connect( button, SIGNAL(clicked()) , SLOT(buttonClicked()) );
    return bi->id;
}

/*----------------------------------------------------------------------------
  Removes a button from the button group.
  \sa insert()
 ----------------------------------------------------------------------------*/

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


/*----------------------------------------------------------------------------
  \fn void QButtonGroup::pressed( int id )
  This signal is emitted when a button in the group is
  \link QButton::pressed() pressed\endlink.
  The \e id argument is the button's identifier.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QButtonGroup::released( int id )
  This signal is emitted when a button in the group is
  \link QButton::released() released\endlink.
  The \e id argument is the button's identifier.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QButtonGroup::clicked( int id )
  This signal is emitted when a button in the group is
  \link QButton::clicked() clicked\endlink.
  The \e id argument is the button's identifier.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::pressed() signal.
 ----------------------------------------------------------------------------*/

void QButtonGroup::buttonPressed()
{
    int id = -1;
    QObject *sobj = sender();			// object that sent the signal
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() )
	if ( sobj == i->button ) {		// button was clicked
	    id = i->id;
            break;
	}
    if ( id != -1 )
	emit pressed( id );
}

/*----------------------------------------------------------------------------
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::released() signal.
 ----------------------------------------------------------------------------*/

void QButtonGroup::buttonReleased()
{
    int id = -1;
    QObject *sobj = sender();			// object that sent the signal
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() )
	if ( sobj == i->button ) {		// button was clicked
	    id = i->id;
            break;
	}
    if ( id != -1 )
	emit released( id );
}

/*----------------------------------------------------------------------------
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::clicked() signal.
 ----------------------------------------------------------------------------*/

void QButtonGroup::buttonClicked()
{
    int id = -1;
    QObject *sobj = sender();			// object that sent the signal
    bool switch_off = sobj->inherits("QRadioButton");
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() ) {
	if ( sobj == i->button )		// button was clicked
	    id = i->id;
	else if ( switch_off && i->button->inherits("QRadioButton") )
	    i->button->switchOff();		// turn other radio buttons off
    }
    if ( id != -1 )
	emit clicked( id );
}

