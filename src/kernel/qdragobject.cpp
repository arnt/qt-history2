/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdragobject.cpp#9 $
**
** Implementation of Drag and Drop support
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
#include "qobjcoll.h"


#if defined(_WS_X11_)
#include <X11/Xlib.h>

extern void qt_xdnd_send_move( Window, QDragObject *, const QPoint & );
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qdragobject.cpp#9 $");


// both a struct for storing stuff in and a wrapper to avoid polluting
// the name space

struct QDragData {
    QDragData(): autoDelete( TRUE ) {}

    QString fmt;
    QByteArray enc;
    bool autoDelete;

    // the one-of-a-kind drag manager
    class Manager: public QObject {
    public:
	Manager();
	~Manager();

	bool eventFilter( QObject *, QEvent * );

	void startDrag( QDragObject * );

	void cancel();
	void move( const QPoint & );
	void drop( const QPoint & );

    private:
	QByteArray enc;
	QDragObject * object;

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



void QDragData::Manager::startDrag( QDragObject * o )
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

    int lx, ly;
    QWidget * w;
#if defined( _WS_X11_ )
    Window target;
    if ( !XTranslateCoordinates(qt_xdisplay(), qt_xrootwin(), qt_xrootwin(),
				globalPos.x(), globalPos.y(),
				&lx, &ly, &target) ) {
	// somehow got to a different screen?  ignore for now
	return;
    }
    w = QWidget::find( target );
#elif defined(_WS_WIN_)
    return;
#endif


    if ( w ) {
	// this process, so short-circuit

	QPoint local = w->mapFromGlobal( globalPos );
	QWidget * dest = 0;
	while ( w->children() && !dest ) {
	    bool doneWithThisOne = FALSE;
	    QWidget * c;
	    QObjectListIt it( *w->children() );
	    it.toLast();
	    while( !doneWithThisOne && (c=((QWidget*)(it.current()))) != 0 ) {
		--it;
		if ( c->isWidgetType() &&
		     c->geometry().contains( local ) ) {
		    doneWithThisOne = TRUE;
		    w = c;
		    local = w->mapFromParent( local );
		}
	    }
	    if ( !doneWithThisOne )
		dest = w;
	}
	if ( !dest )
	    dest = w;
	
	if ( dropWidget && dropWidget != dest ) {
	    QDragLeaveEvent m;
	    QApplication::sendEvent( dropWidget, &m );
	}
	QDragMoveEvent m( local, QString( object->format() ) );
	QApplication::sendEvent( dest, &m );
	a = m.isAccepted();
	dropWidget = a ? dest : 0;
	QApplication::setOverrideCursor( a ? arrowCursor : crossCursor,
					 restoreCursor );
	restoreCursor = TRUE;
    } else {
	// another process, so do the IPC thing
#if defined( _WS_X11_ )
	qt_xdnd_send_move( target, object, globalPos );
#elif defined(_WS_WIN_)
	QApplication::setOverrideCursor( crossCursor,
					 restoreCursor );
	restoreCursor = TRUE;
#endif
    }
}


void QDragData::Manager::drop( const QPoint & globalPos )
{
    debug( "d " );
    if ( dropWidget ) {
	if ( enc.isEmpty() )
	    enc = object->encodedData();
	QDropEvent m( dropWidget->mapFromGlobal( globalPos ),
		      QString( object->format() ), enc );
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

    if ( !enc.isEmpty() ) {
	enc.detach();
	enc.resize( 0 );
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
	       o, o->name( "unnamed" ), o->className() );
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
    } else if ( e->type() == Event_DragResponse ) {
	if ( ((QDragResponseEvent *)e)->dragAccepted() ) {
	    QApplication::setOverrideCursor( crossCursor, restoreCursor );
	    restoreCursor = TRUE;
	} else {
	    QApplication::setOverrideCursor( arrowCursor, restoreCursor );
	    restoreCursor = TRUE;
	}
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


/*!  Deletes the drag object and frees up the storage used. */

QDragObject::~QDragObject()
{
    d->autoDelete = FALSE; // so cancel() won't delete this object
    if ( manager && manager->object == this )
	manager->cancel();
    delete d;
}


/*!  Starts a drag operation using the contents of this object. */

void QDragObject::startDrag()
{
    manager->startDrag( this );
}


/*!  Sets the encoded data of this drag object to \a encodedData.  The
  encoded data is what's delivered to the drop sites, and must be in a
  strictly defined and portable format.

  Every subclass must call this function, normally in a higher-level
  function such as QTextDragObject::setText(), or in a
  reimplementation of encodedData() in case the class wants lazy
  evaluation of the data.

  The drag object can't be dropped (by the user) until this function
  has been called.
*/

void QDragObject::setEncodedData( QByteArray & encodedData )
{
    d->enc = encodedData;
    d->enc.detach();
    if ( !manager )
	(void)new QDragData::Manager();
}


/*!  Returns the encoded payload of this object.  The drag manager
  calls this when the recipient needs to see the content of the drag;
  this generally doesn't happen until the actual drop.

  The default returns whatever was set using setEncodedData().
*/

const QByteArray QDragObject::encodedData() const
{
    return d->enc;
}


/*!  Sets this object to be deleted automatically when Qt no longer
  needs it if \a enable is TRUE, and to not be deleted by Qt if \a
  enable is FALSE.

  The default is TRUE. */

void QDragObject::setAutoDelete( bool enable )
{
    d->autoDelete = enable;
}


/*!  Returns TRUE if the object should be deleted when the drag
  operation finishes, or FALSE if it should not. */

bool QDragObject::autoDelete() const
{
    return d->autoDelete;
}


/*!

*/

void QDragObject::setFormat( const char * mimeType )
{
    d->fmt = mimeType;
    d->fmt.detach();
}


/*!

*/

const char * QDragObject::format() const
{
    return d->fmt;
}


/*!

*/

void QDragObject::encode()
{
    // nothing
}



/*! \class QTextDragObject qdragobject.h

  \brief The QTextDragObject provides a drag and drop object for
  tranferring plain text.

  \ingroup kernel

  Plain text is defined as single- or multi-line US-ASCII or an
  unspecified 8-bit character set.

  Qt provides no built-in mechanism for delivering only single-line
  or only US-ASCII text.
*/


/*!  Creates a text drag object and sets it to \a text.  \a parent
  must be the drag source, \a name is the object name. */

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


/*!  Returns a pointer to the drag source where this object originated.
*/

QWidget * QDragObject::source()
{
    if ( parent()->isWidgetType() )
	return (QWidget *)parent();
    else
	return 0;
}
