/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qaccel.cpp#35 $
**
** Implementation of QAccel class
**
** Created : 950419
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define QAccelList QListM_QAccelItem
#include "qaccel.h"
#include "qapp.h"
#include "qwidget.h"
#include "qlist.h"
#include "qsignal.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qaccel.cpp#35 $");


/*!
  \class QAccel qaccel.h
  \brief The QAccel class handles keyboard accelerator keys.

  \ingroup uiclasses

  A QAccel contains a list of accelerator items. Each accelerator item
  consists of an identifier and a keyboard code combined with modifiers
  (\c SHIFT, \c CTRL, \c ALT or \c ASCII_ACCEL).

  For example, <code>CTRL + Key_P</code> could be a shortcut for printing
  a document. The key codes are listed in qkeycode.h.

  When pressed, an accelerator key sends out the signal activated() with a
  number that identifies this particular accelerator item.  Accelerator
  items can also be individually connected, so that two different keys
  will activate two different slots (see connectItem()).

  A QAccel object handles key events to its parent widget and all children
  of this parent widget.

  Example:
  \code
     QAccel *a = new QAccel( myWindow );	// create accels for myWindow
     a->connectItem( a->insertItem(Key_P+CTRL), // adds Ctrl+P accelerator
		     myWindow,			// connected to myWindow's
		     SLOT(printDoc()) );	// printDoc() slot
  \endcode
*/


struct QAccelItem {				// internal accelerator item
    QAccelItem( int k, int i ) { key=k; id=i; enabled=TRUE; signal=0; }
   ~QAccelItem()	       { delete signal; }
    int		id;
    int		key;
    bool	enabled;
    QSignal    *signal;
};

typedef Q_DECLARE(QListM,QAccelItem) QAccelList; // internal accelerator list


static QAccelItem *find_id( QAccelList *list, int id )
{
    register QAccelItem *item = list->first();
    while ( item && item->id != id )
	item = list->next();
    return item;
}

static QAccelItem *find_key( QAccelList *list, int key, int ascii )
{
    register QAccelItem *item = list->first();
    while ( item ) {
	int k = item->key;
	if ( (k & ASCII_ACCEL) != 0 && (k & 0xff) == ascii ) {
	    break;
	} else {
	    if ( k == key )
		break;
	}
	item = list->next();
    }
    return item;
}


/*!
  Creates a QAccel object with a parent widget and a name.
*/

QAccel::QAccel( QWidget *parent, const char *name )
    : QObject( parent, name )
{
    aitems = new QAccelList;
    CHECK_PTR( aitems );
    aitems->setAutoDelete( TRUE );
    enabled = TRUE;
    if ( parent && parent->isWidgetType() ) {	// install event filter
	tlw = parent->topLevelWidget();
	tlw->installEventFilter( this );
	connect( tlw, SIGNAL(destroyed()), SLOT(tlwDestroyed()) );
    } else {
	tlw = 0;
#if defined(CHECK_NULL)
	warning( "QAccel: An accelerator must have a parent widget" );
#endif
    }
}

/*!
  Destroys the accelerator object.
*/

QAccel::~QAccel()
{
    if ( tlw )
	tlw->removeEventFilter( this );
    delete aitems;
}


/*!  Make sure that the accelerator is watching the correct event
  filter.  Used by QWidget::recreate().
*/

void QAccel::fixupEventFilter()
{
    QWidget * ntlw = 0;

    if ( parent() && parent()->isWidgetType() )
	ntlw = ((QWidget*)parent())->topLevelWidget();

    if ( tlw != ntlw ) {
	if ( tlw ) {
	    tlw->removeEventFilter( this );
	    disconnect( tlw, SIGNAL(destroyed()), this, SLOT(tlwDestroyed()) );
	}
	tlw = ntlw;
	if ( tlw ) {
	    tlw->installEventFilter( this );
	    connect( tlw, SIGNAL(destroyed()), this, SLOT(tlwDestroyed()) );
	}
    }
}


/*!
  \fn void QAccel::activated( int id )
  This signal is emitted when an accelerator key is pressed. \e id is
  a number that identifies this particular accelerator item.
*/

/*!
  \fn bool QAccel::isEnabled() const
  Returns TRUE if the accelerator is enabled, or FALSE if it is disabled.
  \sa setEnabled(), isItemEnabled()
*/

/*!
  Enables the accelerator if \e enable is TRUE, or disables it if
  \e enable is FALSE.

  Individual keys can also be enabled or disabled.

  \sa isEnabled(), setItemEnabled()
*/

void QAccel::setEnabled( bool enable )
{
    enabled = enable;
}


/*!
  Returns the number of accelerator items.
*/

uint QAccel::count() const
{
    return aitems->count();
}


/*!
  Inserts an accelerator item and returns the item's identifier.

  \arg \e key is a key code plus a combination of SHIFT, CTRL and ALT.
  \arg \e id is the accelerator item id.

  If \e id is negative, then the item will be assigned a unique
  identifier.

  \code
    QAccel *a = new QAccel( myWindow );		// create accels for myWindow
    a->insertItem( Key_P + CTRL, 200 );		// Ctrl+P to print document
    a->insertItem( Key_X + ALT , 201 );		// Alt+X  to quit
    a->insertItem( ASCII_ACCEL + 'q', 202 );	// ASCII 'q' to quit
    a->insertItem( Key_D );			// gets id 2
    a->insertItem( Key_P + CTRL + SHIFT );	// gets id 3
  \endcode
*/

int QAccel::insertItem( int key, int id )
{
    if ( id == -1 )
	id = aitems->count();
    aitems->insert( 0, new QAccelItem(key,id) );
    return id;
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
  Removes all accelerator items.
*/

void QAccel::clear()
{
    aitems->clear();
}


/*!
  Returns the key code of the accelerator item with the identifier \e id,
  or zero if the id cannot be found.
*/

int QAccel::key( int id )
{
    QAccelItem *item = find_id(aitems, id);
    return item ? item->key : 0;
}

/*!
  Returns the identifier of the accelerator item with the key code \e key, or
  -1 if the item cannot be found.
*/

int QAccel::findKey( int key ) const
{
    QAccelItem *item = find_key(aitems, key, key & 0xff );
    return item ? item->id : -1;
}


/*!
  Returns TRUE if the accelerator item with the identifier \e id is enabled.
  Returns FALSE if the item is disabled or cannot be found.
  \sa setItemEnabled(), isEnabled()
*/

bool QAccel::isItemEnabled( int id ) const
{
    QAccelItem *item = find_id(aitems, id);
    return item ? item->enabled : FALSE;
}

/*!
  Enables or disables an accelerator item.
  \arg \e id is the item identifier.
  \arg \e enable specifies whether the item should be enabled or disabled.

  \sa isItemEnabled(), isEnabled()
*/

void QAccel::setItemEnabled( int id, bool enable )
{
    QAccelItem *item = find_id(aitems, id);
    if ( item )
	item->enabled = enable;
}


/*!
  Connects an accelerator item to a slot/signal in another object.

  \arg \e id is the accelerator item id.
  \arg \e receiver is the object to receive a signal.
  \arg \e member is a slot or signal function in the receiver.

  \code
    a->connectItem( 201, mainView, SLOT(quit()) );
  \endcode

  \sa disconnectItem()
*/

bool QAccel::connectItem( int id, const QObject *receiver, const char *member )
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
  Disconnects an accelerator item from a function in another
  object.
  \sa connectItem()
*/

bool QAccel::disconnectItem( int id, const QObject *receiver,
			     const char *member )
{
    QAccelItem *item = find_id(aitems, id);
    if ( item && item->signal )
	return item->signal->disconnect( receiver, member );
    return FALSE;
}


/*!
  Processes accelerator events intended for the top level widget.
*/

bool QAccel::eventFilter( QObject *, QEvent *e )
{
    if ( enabled && e->type() == Event_Accel &&
	 parent() && parent()->isWidgetType() &&
	 ((QWidget *)parent())->isVisibleToTLW() ) {
	QKeyEvent *k = (QKeyEvent *)e;
	int key = k->key();
	if ( k->state() & ShiftButton )
	    key |= SHIFT;
	if ( k->state() & ControlButton )
	    key |= CTRL;
	if ( k->state() & AltButton )
	    key |= ALT;
	QAccelItem *item = find_key(aitems,key,k->ascii());
	if ( item && item->enabled ) {
	    if ( item->signal )
		item->signal->activate();
	    else
		emit activated( item->id );
	    k->accept();
	    return TRUE;
	}
    }
    return FALSE;
}


/*!
  \internal
  This slot is called when the top level widget that owns the accelerator
  is destroyed.
*/

void QAccel::tlwDestroyed()
{
    tlw = 0;
}
