/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdragobject.cpp#1 $
**
** C++ file skeleton
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qdragobject.h"
#include "qapp.h"
#include "qcursor.h"
#include "qpoint.h"
#include "qkeycode.h"
#include "qwidget.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qdragobject.cpp#1 $");


// both a struct for storing stuff in and a wrapper to avoid polluting
// the name space

struct QDragData {
    QDragData(): autoDelete( TRUE ) {}

    QByteArray enc;
    bool autoDelete;

    // the one-of-a-kind drag manager
    class Manager: public QObject {
    public:
	Manager();
	~Manager();

	bool eventFilter( QObject *, QEvent * );

	void startDrag( QDragObject *, QByteArray encodedData );

	void cancel();
	void move( const QPoint & );
	void drop( const QPoint & );

    private:
	QDragObject * object;
	QByteArray enc;
	QByteArray fmt;
	QWidget * dragSource;
	QWidget * dropWidget;
	bool beingCancelled;
	bool restoreCursor;

	friend QDragObject;
    };
};


// and here it is
static QDragData::Manager * manager = 0;

static void clean_up_drag_manager() {
    delete manager;
    manager = 0;
}



QDragData::Manager::Manager()
    : QObject( 0, "global drag manager" )
{
    object = 0;
    dragSource = 0;
    dropWidget = 0;
    if ( !manager ) {
	manager = this;
	qAddPostRoutine( clean_up_drag_manager );
    }
    beingCancelled = FALSE;
    restoreCursor = FALSE;
}


QDragData::Manager::~Manager()
{
    if ( restoreCursor )
	QApplication::restoreOverrideCursor();
    manager = 0;
}



void QDragData::Manager::startDrag( QDragObject * o, QByteArray encodedData )
{
    if ( object == o ) {
	debug( "meaningless" );
	return;
    }

    if ( object ) {
	cancel();
	dragSource->removeEventFilter( this );
	beingCancelled = FALSE;
    }

    object = o;
    enc = encodedData;
    enc.detach(); // ### remove once QByteArray becomes implictly shared
    dragSource = (QWidget *)(object->parent());
    dragSource->installEventFilter( this );
}


void QDragData::Manager::cancel()
{
    debug( "c " );
    if ( object ) {
	beingCancelled = TRUE;
	if ( object->autoDelete() )
	    delete object;
	object = 0;
    }
    if ( dropWidget ) {
	// send cancel event
	dropWidget = 0;
    }
    if ( restoreCursor ) {
	QApplication::restoreOverrideCursor();
	restoreCursor = FALSE;
    }
}


void QDragData::Manager::move( const QPoint & globalPos )
{
    bool a = TRUE;
    QWidget * dest = QApplication::widgetAt( globalPos, TRUE );
    if ( dropWidget && dropWidget != dest ) {
	QDragLeaveEvent m;
	QApplication::sendEvent( dropWidget, &m );
    }
    if ( dest ) {
	QDragMoveEvent m( dest->mapFromGlobal( globalPos ), fmt );
	QApplication::sendEvent( dest, &m );
	a = m.isAccepted();
    } else {
	a = FALSE;
    }
    dropWidget = a ? dest : 0;
    QApplication::setOverrideCursor( a ? arrowCursor : crossCursor,
				     restoreCursor );
    restoreCursor = TRUE;
}


void QDragData::Manager::drop( const QPoint & globalPos )
{
    debug( "d " );
    if ( dropWidget ) {
	QDropEvent m( dropWidget->mapFromGlobal( globalPos ), fmt, enc );
	QApplication::sendEvent( dropWidget, &m );
	// was it accepted?  who cares!
    }

    if ( restoreCursor )
	QApplication::restoreOverrideCursor();
    dropWidget = 0;
    restoreCursor = FALSE;

    if ( object ) {
	if ( object->autoDelete() )
	    delete object;
	object = 0;
    }

    if ( dragSource ) {
	dragSource->removeEventFilter( this );
	dragSource = 0;
    }
}


bool QDragData::Manager::eventFilter( QObject * o, QEvent * e)
{
    if ( o != dragSource ) {
	debug( "unexpected event for object %p - %s/%s",
	       o, o->name(), o->className() );
	o->removeEventFilter( this );
    }

    ASSERT( object != 0 );

    if ( beingCancelled ) {
	if ( e->type() == Event_KeyRelease &&
	     ((QKeyEvent*)e)->key() == Key_Escape ) {
	    dragSource->removeEventFilter( this );
	    if ( object && object->autoDelete() )
		delete object;
	    object = 0;
	    dragSource = 0;
	    beingCancelled = FALSE;
	    return TRUE; // block the key release
	}
	return FALSE;
    }

    if ( e->type() == Event_MouseMove ) {
	move( dragSource->mapToGlobal( ((QMouseEvent *)e)->pos() ) );
	return TRUE;
    } else if ( e->type() == Event_MouseButtonRelease ) {
	drop( dragSource->mapToGlobal( ((QMouseEvent *)e)->pos() ) );
	return TRUE;
    } else if ( e->type() == Event_KeyPress &&
		((QKeyEvent*)e)->key() == Key_Escape ) {
	cancel();
	return TRUE;
    }
	
    return FALSE;
}




/*!  Creates a drag object which is a child of \a dragSource and
  named \a name.

  Note that the drag object will be deleted when \a dragSource is.
*/

QDragObject::QDragObject( QWidget * dragSource, const char * name )
    : QObject( dragSource, name )
{
    d = new QDragData();
}


/*!  Deletes the drag object and frees up the storage used.
*/

QDragObject::~QDragObject()
{
    d->autoDelete = FALSE; // so cancel() won't delete this object
    if ( manager && manager->object == this )
	manager->cancel();
    delete d;
}


/*!  Sets the encoded data of this drag object to \a encodedData.  The
  encoded data is what's delivered to the drop sites, and must be in a
  strictly defined and portable format.

  Every subclass must call this function, normally in a higher-level
  function such as QTextDragObject::setText().

  The drag object can't be dropped (by the user) until this function
  has been called.
*/

void QDragObject::setEncodedData( QByteArray & encodedData )
{
    d->enc = encodedData;
    d->enc.detach();
    if ( !manager )
	new QDragData::Manager();
    manager->startDrag( this, encodedData );
}


/*!  Sets this object to be deleted automatically when Qt no longer
  needs it if \a enable is TRUE, and to not be deleted by Qt if \a
  enable is FALSE.

  The default is TRUE. */

void QDragObject::setAutoDelete( bool enable )
{
    d->autoDelete = enable;
}


/*!  Returns TRUE if the object will be deleted when the drag
  operation finishes, or FALSE if it will not. */

bool QDragObject::autoDelete() const
{
    return d->autoDelete;
}


/*!  Creates a text drag object and sets it to \a text.  \a parent
  must be the drag source, \a name is the object name.
*/

QTextDragObject::QTextDragObject( const char * text,
				  QWidget * parent, const char * name )
    : QDragObject( parent, name )
{
    setText( text );
}


/*!  Creates a default text drag object.  \a parent must be the drag
  source, \a name is the object name.
*/

QTextDragObject::QTextDragObject( QWidget * parent, const char * name )
    : QDragObject( parent, name )
{
    // nothing
}


/*!  Destroys the text drag object and frees all allocated resources.
*/

QTextDragObject::~QTextDragObject()
{
    // nothing
}


/*!

*/

void QTextDragObject::setText( const char * text )
{
    QString tmp( text );
    setEncodedData( tmp );
}
