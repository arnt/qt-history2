/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qaccel.cpp#3 $
**
** Implementation of QAccel class
**
** Author  : Haavard Nord
** Created : 950419
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define QAccelList QListM_QAccelItem
#include "qaccel.h"
#include "qapp.h"
#include "qlist.h"
#include "qsignal.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qaccel.cpp#3 $";
#endif


/*!
\class QAccel qaccel.h

The QAccel class contains a number of accelerator items. An accelerator item
consists of a keyboard code combined with modifiers (SHIFT, CTRL and ALT),
for example <code>CTRL + Key_P</code> could be a shortcut for printing a
document.

When pressed, an accelerator key sends out the signal activated() with a
number that identifies this particular accelerator item.  Accelerator items
can also be individually connected, so that two different keys will
activate two different slots (see connectItem()).

A QAccel object works for its parent widget and all children of this parent
widget.
*/

struct QAccelItem {				// accelerator item
    QAccelItem( int k, int i ) { key=k; id=i; enabled=TRUE; signal=0; }
   ~QAccelItem() 	       { delete signal; }
    int		id;
    int		key;
    bool	enabled;
    QSignal    *signal;
};

typedef declare(QListM,QAccelItem) QAccelList;	// list of accelerators


static QAccelItem *find_id( QAccelList *list, int id )
{
    register QAccelItem *item = list->first();
    while ( item && item->id != id )
	item = list->next();
    return item;
}

static QAccelItem *find_key( QAccelList *list, long key )
{
    register QAccelItem *item = list->first();
    while ( item && item->key != key )
	item = list->next();
    return item;
}


/*!
Creates a QAccel object with a parent widget and a name.
*/

QAccel::QAccel( QWidget *parent, const char *name ) : QObject( 0, name )
{
    hiPriority = TRUE;				// accel has high priority
    aitems = new QAccelList;
    CHECK_PTR( aitems );
    aitems->setAutoDelete( TRUE );
    enabled = TRUE;
    if ( parent ) {
	parent->insertChild( this );		// insert as hi priority obj
	QEvent e( Event_AccelInserted );
	QApplication::sendEvent( parent, &e );	// notify parent about accel
    }
#if defined(CHECK_NULL)
    else
	warning( "QAccel: An accelerator should have a widget parent" );
#endif
}

/*!
Destroys the accelerator object.
*/

QAccel::~QAccel()
{
    emit destroyed();
    delete aitems;
}


/*!
Enables the accelerator.  The accelerator is initially enabled.
Individual keys can be enabled/disabled with the setItemEnabled(), enableItem()
and disableItem() functions.
*/

void QAccel::enable()
{
    enabled = TRUE;
}

/*!
Disables the accelerator.  Individual keys cannot be enabled in this mode.
*/

void QAccel::disable()
{
    enabled = FALSE;
}

/*!
\fn bool QAccel::isDisabled() const
Returns TRUE is the accelerator is disabled.
*/


/*!
Inserts an accelerator item.

\arg \e key is a key code plus a combination of SHIFT, CTRL and ALT.
\arg \e id is the accelerator item id.

If \e id is negative, then the item will
be assigned an identifer which is the number of accelerator items already
defined.
\code{{
QAccel *a = new QAccel( mainView );	\/ mainView is a top level widget
a->insertItem( Key_P | CTRL, 200 );	\/ Ctrl+P to print document
a->insertItem( Key_X | ALT , 201 );	\/ Alt+X  to quit
a->insertItem( Key_D );			\/ gets id 2
a->insertItem( Key_P | CTRL | SHIFT );	\/ gets id 3
}}
*/

void QAccel::insertItem( long key, int id )
{
    aitems->insert( new QAccelItem(key,id) );
}

/*!
Removes the accelerator item with the identifier \e id.
*/

void QAccel::removeItem( int id )
{
    if ( find_id(aitems, id) )
	aitems->remove();
}


/*!
Returns the key code of the accelerator item with the identifier \e id,
or zero if the id cannot be found.
*/

long QAccel::key( int id )
{
    QAccelItem *item = find_id(aitems, id);
    return item ? item->key : 0;
}

/*!
Returns the identifier of the accelerator item with the key code \e key, or
-1 if the item cannot be found.
*/

int QAccel::findKey( long key ) const
{
    QAccelItem *item = find_key(aitems, key);
    return item ? item->id : -1;
}


/*!
Returns TRUE if the accelerator items with the identifier \e id is disabled.
Returns FALSE if the item is enabled or cannot be found.
*/

bool QAccel::isItemDisabled( int id ) const
{
    QAccelItem *item = find_id(aitems, id);
    return item ? !item->enabled : FALSE;
}

/*!
\fn bool QAccel::isItemEnabled( int id ) const
Returns TRUE if the accelerator items with the identifier \e id is enabled.
Returns FALSE if the item is disabled or cannot be found.
*/

/*!
Enables or disables an accelerator item.
\arg \e id is the item identifier.
\arg \e enable specifies wether the item should be enabled or disabled.
*/

void QAccel::setItemEnabled( int id, bool enable )
{
    QAccelItem *item = find_id(aitems, id);
    if ( item )
	item->enabled = enable;
}

/*!
\fn void QAccel::enableItem( int id )
Enables the accelerator items with the identifier \e id.
*/

/*!
\fn void QAccel::disableItem( int id )
Disables the accelerator items with the identifier \e id.
*/


/*!
Connects an accelerator item to a function another object.

\arg \e id is the accelerator item id.
\arg \e receiver is the object to receive a signal.
\arg \e member is a slot or signal function in the receiver.

\code{{
a->connectItem( 201, mainView, SLOT(quit()) );
}}

*/

bool QAccel::connectItem( int id, const QObject *receiver,
			  const char *member )
{
    QAccelItem *item = find_id(aitems, id);
    if ( item ) {
	if ( !item->signal ) {
	    item->signal = new QSignal;
	    CHECK_PTR( item->signal );
	}
	return item->signal->connect( receiver, member );
    }
    return FALSE;
}

/*!
Disconnects an accelerator item from a function in another object.
*/

bool QAccel::disconnectItem( int id, const QObject *receiver,
			     const char *member )
{
    QAccelItem *item = find_id(aitems, id);
    if ( item && item->signal )
	return item->signal->disconnect( receiver, member );
    return FALSE;    
}


bool QAccel::event( QEvent *e )
{
    if ( enabled && e->type() == Event_KeyPress ) {
	QKeyEvent *k = (QKeyEvent *)e;
	long key = k->key();
	if ( k->state() & ShiftButton )
	    key |= SHIFT;
	if ( k->state() & ControlButton )
	    key |= CTRL;
	if ( k->state() & AltButton )
	    key |= ALT;
	QAccelItem *item = find_key(aitems,key);
	if ( item && item->enabled ) {
	    if ( item->signal )
		item->signal->activate();
	    else
		emit activated( item->id );
	    k->ignore();
	    return TRUE;
	}
    }
    return FALSE;
}
