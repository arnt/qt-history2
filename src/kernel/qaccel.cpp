/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qaccel.cpp#1 $
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
#include "qwidget.h"
#include "qlist.h"
#include "qsignal.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qaccel.cpp#1 $";
#endif


/*!
\class QAccel qaccel.h

The QAccel class contains a number of accelerator items, which are
activated when the user presses certain keys.  An accelerator must
be associated with a widget in order to work.  A widget can have at
most one accelerator object, however, several accelerators can be
merged using the insertItems() function.
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
    QAccelItem *item = list->first();
    while ( item && item->id != id )
	item = list->next();
    return item;
}

static QAccelItem *find_key( QAccelList *list, long key )
{
    QAccelItem *item = list->first();
    while ( item && item->key != key )
	item = list->next();
    return item;
}


class FriendlyWidget : public QWidget		// cheat to set WHasAccel flag
{
public:
    void setAccelFlag() { setFlag( WHasAccel ); }
};


/*!
Creates a QAccel object with a parent object and a name.
*/
QAccel::QAccel( QWidget *parent, const char *name ) : QObject( parent, name )
{
    aitems = new QAccelList;
    CHECK_PTR( aitems );
    aitems->setAutoDelete( TRUE );
    enabled = TRUE;
    if ( parent ) {				// set accelerator flag
	FriendlyWidget *w = (FriendlyWidget *)parent;
	w->setAccelFlag();
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
    delete aitems;
}


/*!
Enables all accelerator keys.
*/
void QAccel::enable()
{
    enabled = TRUE;
}

/*!
Disables all accelerator keys.  Individual keys cannot be enabled in this mode.
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
a->insertItem( Key_O | CTRL, 200 );	\/ Ctrl+O to open document
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
Inserts all accelerator item from the other accelerator \e a.
This functions lets you merge two accelerators.
Individual connections for accelerator items in \e a will not be copied.
*/
void QAccel::insertItems( const QAccel *a )
{
    QAccelList *list = a->aitems;
    QAccelItem *item = list->last();
    while ( item ) {
	aitems->insert( new QAccelItem(item->key,item->id) );
	item = list->prev();
    }
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
Returns TRUE if this accelerator contains an item with the key code \e key.
*/
bool QAccel::containsItem( long key ) const
{
    return find_key(aitems,key) != 0;
}

/*!
Returns TRUE if this accelerator contains one or more items that also can be
found in another accelerator \e a.
*/
bool QAccel::containsItems( const QAccel *a ) const
{
    QAccelList *list = a->aitems;
    QAccelItem *item = list->first();
    while ( item ) {
	if ( find_key(aitems,item->key) != 0 )
	    return TRUE;
	item = list->next();
    }
    return FALSE;
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
