/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdnd_x11.cpp#31 $
**
** XDND implementation for Qt.  See http://www.cco.caltech.edu/~jafl/xdnd2/
**
** Created : 980320
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

// ### you are now entering an uglehack zone
//
// this code is bad bad bad.  we need to rethink and clean up in 2.0,
// when we've had some real experience with use of all this.

#include "qapplication.h"
#include "qwidget.h"
#include "qintdict.h"
#include "qdatetime.h"
#include "qdict.h"
#include "qdragobject.h"
#include "qobjectlist.h"
#include "qkeycode.h"
#include "qbitmap.h"


#include <X11/X.h> // for Atom
#include <X11/Xlib.h> // for XEvent
#include <X11/Xatom.h> // for XA_STRING and friends

// this stuff is copied from qapp_x11.cpp

extern void qt_x11_intern_atom( const char *, Atom * );

extern Window qt_x11_findClientWindow( Window, Atom, bool );
extern Atom qt_wm_state;
extern Time qt_x_clipboardtime;

// this stuff is copied from qclb_x11.cpp

extern bool qt_xclb_wait_for_event( Display *dpy, Window win, int type,
				    XEvent *event, int timeout );
extern bool qt_xclb_read_property( Display *dpy, Window win, Atom property,
				   bool deleteProperty,
				   QByteArray *buffer, int *size, Atom *type,
				   int *format );
extern QByteArray qt_xclb_read_incremental_property( Display *dpy, Window win,
						     Atom property,
						     int nbytes );
// and all this stuff is copied -into- qapp_x11.cpp

void qt_xdnd_setup();
void qt_handle_xdnd_enter( QWidget *, const XEvent * );
void qt_handle_xdnd_position( QWidget *, const XEvent * );
void qt_handle_xdnd_status( QWidget *, const XEvent * );
void qt_handle_xdnd_leave( QWidget *, const XEvent * );
void qt_handle_xdnd_drop( QWidget *, const XEvent * );
void qt_handle_xdnd_finished( QWidget *, const XEvent * );
void qt_xdnd_handle_selection_request( const XSelectionRequestEvent * );
void qt_xdnd_handle_destroy_notify( const XDestroyWindowEvent * );
// client messages
Atom qt_xdnd_enter;
Atom qt_xdnd_position;
Atom qt_xdnd_status;
Atom qt_xdnd_leave;
Atom qt_xdnd_drop;
Atom qt_xdnd_finished;

// end of copied stuff

// clean up the stuff used.
static void qt_xdnd_cleanup();

// XDND selection
Atom qt_xdnd_selection;
// other selection
static Atom qt_selection_property;
// INCR
static Atom qt_incr_atom;

// property for XDND drop sites
Atom qt_xdnd_aware;

// real variables:
// xid of current drag source
static Atom qt_xdnd_dragsource_xid = 0;

// the types in this drop.  100 is no good, but at least it's big.
static Atom qt_xdnd_types[100];

static QIntDict<QString> * qt_xdnd_drag_types = 0;
static QDict<Atom> * qt_xdnd_atom_numbers = 0;

// rectangle in which the answer will be the same
static QRect qt_xdnd_source_sameanswer;
static QRect qt_xdnd_target_sameanswer;
// top-level window we sent position to last.
static Window qt_xdnd_current_target;
// widget we forwarded position to last, and local position
static QWidget * qt_xdnd_current_widget;
static QPoint qt_xdnd_current_position;
// time of this drop, as type Atom to save on casts
static Atom qt_xdnd_source_current_time;
static Atom qt_xdnd_target_current_time;

// dict of payload data, sorted by type atom
QIntDict<QByteArray> * qt_xdnd_target_data = 0;

// first drag object, or 0
QDragObject * qt_xdnd_source_object = 0;

void qt_xdnd_add_type( const char * mimeType )
{
    if ( !mimeType || !*mimeType )
	return;
    if ( !qt_xdnd_atom_numbers )
	qt_xdnd_atom_numbers = new QDict<Atom>( 17 );
    if ( qt_xdnd_atom_numbers->find( mimeType ) )
	return;

    Atom * tmp = new Atom;
    *tmp = 0;
    qt_xdnd_atom_numbers->insert( mimeType, tmp );
    qt_x11_intern_atom( mimeType, tmp );
    if ( qt_xdnd_drag_types ) {
	QString * s = new QString( mimeType );
	qt_xdnd_drag_types->insert( (long)(*tmp), s );
    }
    qt_xdnd_atom_numbers->setAutoDelete( TRUE );
}


void qt_xdnd_setup() {
    // set up protocol atoms
    qt_x11_intern_atom( "XdndEnter", &qt_xdnd_enter );
    qt_x11_intern_atom( "XdndPosition", &qt_xdnd_position );
    qt_x11_intern_atom( "XdndStatus", &qt_xdnd_status );
    qt_x11_intern_atom( "XdndLeave", &qt_xdnd_leave );
    qt_x11_intern_atom( "XdndDrop", &qt_xdnd_drop );
    qt_x11_intern_atom( "XdndFinished", &qt_xdnd_finished );

    qt_x11_intern_atom( "XdndSelection", &qt_xdnd_selection );

    qt_x11_intern_atom( "XdndAware", &qt_xdnd_aware );

    qt_x11_intern_atom( "QT_SELECTION", &qt_selection_property );
    qt_x11_intern_atom( "INCR", &qt_incr_atom );

    // added for speed, since it's faster to do this at startup
    qt_xdnd_add_type( "text/plain" );
    qt_xdnd_add_type( "image/gif" );
    qt_xdnd_add_type( "image/xpm" );
    qt_xdnd_add_type( "image/xbm" );

    qAddPostRoutine( qt_xdnd_cleanup );
}


void qt_xdnd_cleanup()
{
    delete qt_xdnd_drag_types;
    qt_xdnd_drag_types = 0;
    delete qt_xdnd_atom_numbers;
    qt_xdnd_atom_numbers = 0;
    delete qt_xdnd_target_data;
    qt_xdnd_target_data = 0;
}


static QWidget * find_child( QWidget * tlw, QPoint & p )
{
    QWidget * w = tlw;

    p = w->mapFromGlobal( p );
    bool done = FALSE;
    while ( !done ) {
	done = TRUE;
	if ( w->children() ) {
	    QObjectListIt it( *w->children() );
	    it.toLast();
	    QObject * o;
	    while( (o=it.current()) ) {
		--it;
		if ( o->isWidgetType() &&
		     ((QWidget*)o)->isVisible() &&
		     ((QWidget*)o)->geometry().contains( p ) ) {
		    w = (QWidget *)o;
		    done = FALSE;
		    p = w->mapFromParent( p );
		    break;
		}
	    }
	}
    }
    return w;
}


void qt_handle_xdnd_enter( QWidget *, const XEvent * xe ) {
    if ( !qt_xdnd_atom_numbers )
	return; // haven't been set up for dnd

    const long *l = xe->xclient.data.l;
    // first, build the atom dict, if possible
    if ( !qt_xdnd_drag_types ) {
	qt_xdnd_drag_types = new QIntDict<QString>( 17 );
	qt_xdnd_drag_types->setAutoDelete( TRUE );
	QDictIterator<Atom> it( *qt_xdnd_atom_numbers );
	Atom * a;
	while( (a=it.current()) != 0 ) {
	    QString * s = new QString( it.currentKey() );
	    s->detach();
	    ++it;
	    qt_xdnd_drag_types->insert( (long)(*a), s );
	}
	// ### unfinished!  treat XA_STRING as text/plain.  remove ASAP
	QString * s;
	s = new QString( "text/plain" );
	qt_xdnd_drag_types->insert( (long)XA_STRING, s );
    }

    int version = (int)(((unsigned long)(l[1])) >> 24);

    if ( version > 2 )
	return;

    qt_xdnd_dragsource_xid = l[0];
    // ### can crash if the xid is wild
    if ( qt_xdnd_dragsource_xid && !QWidget::find( qt_xdnd_dragsource_xid ) )
	XSelectInput( qt_xdisplay(), qt_xdnd_dragsource_xid,
		      SubstructureNotifyMask );

    // get the first types
    int i;
    int j = 0;
    for( i=2; i < 5; i++ ) {
	if ( qt_xdnd_drag_types->find( l[i] ) )
	    qt_xdnd_types[j++] = l[i];
    }
    qt_xdnd_types[j] = 0;

    if ( l[1] & 1 ) {
	// should retrieve that property
	//debug( "more types from %08lx", qt_xdnd_dragsource_xid );
    }
}


void qt_handle_xdnd_position( QWidget *w, const XEvent * xe )
{
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    QPoint p( (l[2] & 0xffff0000) >> 16, l[2] & 0x0000ffff );
    QWidget * c = find_child( w, p );

    if ( l[0] != qt_xdnd_dragsource_xid ) {
	//	debug( "xdnd drag position from unexpected source (%08lx not %08lx)",
	//       l[0], qt_xdnd_dragsource_xid );
	return;
    }

    XClientMessageEvent response;
    response.type = ClientMessage;
    response.window = qt_xdnd_dragsource_xid;
    response.format = 32;
    response.message_type = qt_xdnd_status;
    response.data.l[0] = w->winId();
    response.data.l[1] = 0; // flags
    response.data.l[2] = 0; // x, y
    response.data.l[3] = 0; // w, h
    response.data.l[4] = 0; // just null

    QRect answerRect( c->mapToGlobal( p ), QSize( 1,1 ) );

    if ( qt_xdnd_current_widget != c ) {
	if ( qt_xdnd_current_widget ) {
	    QDragLeaveEvent e;
	    QApplication::sendEvent( qt_xdnd_current_widget, &e );
	}
	QDragEnterEvent de( p );
	de.ignore( c->rect() );
	QApplication::sendEvent( c, &de );
	if ( de.isAccepted() )
	    response.data.l[1] = 1; // yess!!!!
	answerRect = QRect( c->mapToGlobal( de.answerRect().topLeft() ),
			    de.answerRect().size() );
    }

    qt_xdnd_current_widget = c;
    qt_xdnd_current_position = p;
    qt_xdnd_target_current_time = l[3]; // will be 0 for xdnd1

    QDragMoveEvent me( p );
    QApplication::sendEvent( c, &me );
    if ( me.isAccepted() )
	response.data.l[1] = 1; // yess!!!!
    answerRect= QRect( c->mapToGlobal( me.answerRect().topLeft() ),
		       me.answerRect().size() );

    if ( answerRect.width() < 0 )
	answerRect.setWidth( 0 );
    if ( answerRect.height() < 0 )
	answerRect.setHeight( 0 );
    if ( answerRect.left() < 0 )
	answerRect.setLeft( 0 );
    if ( answerRect.right() > 4096 )
	answerRect.setRight( 4096 );
    if ( answerRect.top() < 0 )
	answerRect.setTop( 0 );
    if ( answerRect.bottom() > 4096 )
	answerRect.setBottom( 4096 );

    response.data.l[2] = (answerRect.x() << 16) + answerRect.y();
    response.data.l[3] = (answerRect.width() << 16) + answerRect.height();

    QWidget * source = QWidget::find( qt_xdnd_dragsource_xid );
    if ( source )
	qt_handle_xdnd_status( source, (const XEvent *)&response );
    else
	XSendEvent( w->x11Display(), qt_xdnd_dragsource_xid, FALSE,
		    NoEventMask, (XEvent*)&response );
}


void qt_handle_xdnd_status( QWidget * w, const XEvent * xe )
{

    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;
    QDragResponseEvent e( (int)(l[1] & 1) );
    QApplication::sendEvent( w, &e );

    QPoint p( (l[2] & 0xffff0000) >> 16, l[2] & 0x0000ffff );
    QSize s( (l[3] & 0xffff0000) >> 16, l[3] & 0x0000ffff );
    if ( s.width() < 4096 && s.height() < 4097 &&
	 s.width() > 0 && s.height() > 0 ) {
	qt_xdnd_source_sameanswer = QRect( p, s );
    }
    else
	qt_xdnd_source_sameanswer = QRect( p, QSize( 1, 1 ) );
}


void qt_handle_xdnd_leave( QWidget *w, const XEvent * xe )
{
    if ( !qt_xdnd_current_widget ||
	 w->topLevelWidget() != qt_xdnd_current_widget->topLevelWidget() ) {
	qt_xdnd_dragsource_xid = 0;
	return; // sanity
    }

    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    //debug( "xdnd leave" );

    if ( l[0] != qt_xdnd_dragsource_xid ) {
	//debug( "xdnd drag leave from unexpected source (%08lx not %08lx",
	//       l[0], qt_xdnd_dragsource_xid );
	qt_xdnd_current_widget = 0;
	return;
    }

    QDragLeaveEvent e;
    QApplication::sendEvent( qt_xdnd_current_widget, &e );

    if ( qt_xdnd_dragsource_xid && !QWidget::find( qt_xdnd_dragsource_xid ) )
	XSelectInput( w->x11Display(), qt_xdnd_dragsource_xid, 0 );
    qt_xdnd_dragsource_xid = 0;
    qt_xdnd_types[0] = 0;
    qt_xdnd_current_widget = 0;
}


static void qt_xdnd_send_leave()
{
    if ( !qt_xdnd_current_target )
	return;

    XClientMessageEvent leave;
    leave.type = ClientMessage;
    leave.window = qt_xdnd_current_target;
    leave.format = 32;
    leave.message_type = qt_xdnd_leave;
    leave.data.l[0] = qt_xdnd_dragsource_xid;
    leave.data.l[1] = 0; // flags
    leave.data.l[2] = 0; // x, y
    leave.data.l[3] = 0; // w, h
    leave.data.l[4] = 0; // just null

    QWidget * w = QWidget::find( qt_xdnd_current_target );
    if ( w ) {
	qt_handle_xdnd_leave( w, (const XEvent *)&leave );
    } else {
	XSendEvent( w->x11Display(), qt_xdnd_current_target, FALSE,
		    NoEventMask, (XEvent*)&leave );
	XSelectInput( w->x11Display(), qt_xdnd_current_target, 0 );
    }
    qt_xdnd_current_target = 0;
}


void qt_handle_xdnd_drop( QWidget *, const XEvent * xe )
{
    if ( !qt_xdnd_current_widget ) {
	qt_xdnd_dragsource_xid = 0;
	return; // sanity
    }

    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    //debug( "xdnd drop" );

    if ( l[0] != qt_xdnd_dragsource_xid ) {
	//debug( "xdnd drop from unexpected source (%08lx not %08lx",
	//       l[0], qt_xdnd_dragsource_xid );
	return;
    }
    QDropEvent de( qt_xdnd_current_position );
    QApplication::sendEvent( qt_xdnd_current_widget, &de );
    QDragLeaveEvent e;
    QApplication::sendEvent( qt_xdnd_current_widget, &e );
    qt_xdnd_dragsource_xid = 0;
    qt_xdnd_current_widget = 0;
}


void qt_handle_xdnd_finished( QWidget *, const XEvent * xe )
{
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    if ( l[0] && l[0] == qt_xdnd_current_target ) {
	//
	qt_xdnd_current_target = 0;
    }
}

static QCursor *noDropCursor = 0;

#define noDropCursorWidth 18
#define noDropCursorHeight 18
static unsigned char noDropCutBits[] = {
 0xc0,0x0f,0xfc,0xf0,0x3f,0xfc,0x78,0x78,0xfc,0x1c,0xe0,0xfc,0x3e,0xc0,0xfd,
 0x76,0x80,0xfd,0xe7,0x80,0xff,0xc3,0x01,0xff,0x83,0x03,0xff,0x03,0x07,0xff,
 0x03,0x0e,0xff,0x07,0x9c,0xff,0x06,0xb8,0xfd,0x0e,0xf0,0xfd,0x1c,0xe0,0xfc,
 0x78,0x70,0xfc,0xf0,0x3f,0xfc,0xc0,0x0f,0xfc};



bool QDragManager::eventFilter( QObject * o, QEvent * e)
{
    if ( o != dragSource ) {
	//debug( "unexpected event for object %p - %s/%s",
	//       o, o->name( "unnamed" ), o->className() );
	o->removeEventFilter( this );
	return FALSE;
    }

    ASSERT( object != 0 );

    if ( beingCancelled ) {
	if ( e->type() == Event_KeyRelease &&
	     ((QKeyEvent*)e)->key() == Key_Escape ) {
	    dragSource->removeEventFilter( this );
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
	if ( willDrop )
	    drop();
	else
	    cancel();
	dragSource->removeEventFilter( this );
	object = 0;
	dragSource = 0;
	beingCancelled = FALSE;
	return TRUE;
    } else if ( e->type() == Event_KeyPress &&
		((QKeyEvent*)e)->key() == Key_Escape ) {
	cancel();
	dragSource->removeEventFilter( this );
	object = 0;
	dragSource = 0;
	beingCancelled = FALSE;
	return TRUE;
    } else if ( e->type() == Event_DragResponse ) {
	if ( ((QDragResponseEvent *)e)->dragAccepted() ) {
	    QApplication::setOverrideCursor( arrowCursor, restoreCursor );
	    restoreCursor = TRUE;
	    willDrop = TRUE;
	} else {
	    if ( !noDropCursor ) {
		QBitmap b( noDropCursorWidth, noDropCursorHeight, noDropCutBits, TRUE );
		noDropCursor = new QCursor( b, b );
	    }
	    QApplication::setOverrideCursor( *noDropCursor, restoreCursor );
	    restoreCursor = TRUE;
	    willDrop = FALSE;
	}
	return TRUE;
    }

    return FALSE;
}


void QDragManager::cancel()
{
    if ( object ) {
	beingCancelled = TRUE;
	if ( object->autoDelete() )
	    delete object;
	object = 0;
    }

    if ( qt_xdnd_current_target ) {
	qt_xdnd_send_leave();
    }

    if ( restoreCursor ) {
	QApplication::restoreOverrideCursor();
	restoreCursor = FALSE;
    }

    qt_xdnd_source_object = 0;
}


void QDragManager::move( const QPoint & globalPos )
{
    if ( qt_xdnd_source_sameanswer.contains( globalPos ) && 
	 qt_xdnd_source_sameanswer.isValid() && 
	 !qt_xdnd_source_sameanswer.isEmpty() ) // ### probably unnecessary
	return;

    Window target = 0;
    int lx = 0, ly = 0;
    if ( !XTranslateCoordinates( qt_xdisplay(), qt_xrootwin(), qt_xrootwin(),
				 globalPos.x(), globalPos.y(),
				 &lx, &ly, &target) ) {
	// somehow got to a different screen?  ignore for now
	return;
    }

    if ( target != 0 )
	target = qt_x11_findClientWindow( target, qt_wm_state, TRUE );

    if ( target == 0 )
	target = qt_xrootwin();


    QWidget * w = QWidget::find( (WId)target );

    if ( target != qt_xdnd_current_target ) {
	if ( qt_xdnd_current_target )
	    qt_xdnd_send_leave();

	Atom * type[3]={0,0,0};
	const char* fmt;
	int nfmt=0;
	for (nfmt=0; nfmt<3 && (fmt=object->format(nfmt)); nfmt++) {
	    type[nfmt] = qt_xdnd_atom_numbers->find( fmt );
	}
	XClientMessageEvent enter;
	enter.type = ClientMessage;
	enter.window = target;
	enter.format = 32;
	enter.message_type = qt_xdnd_enter;
	enter.data.l[0] = object->source()->winId();
	enter.data.l[1] = 1 << 24; // flags
	enter.data.l[2] = type[0] ? *type[0] : 0; // ###
	enter.data.l[3] = type[1] ? *type[1] : 0;
	enter.data.l[4] = type[2] ? *type[2] : 0;

	qt_xdnd_current_target = target;
	// provisionally set the rectangle to 5x5 pixels...
	qt_xdnd_source_sameanswer = QRect( globalPos.x() - 2,
					   globalPos.y() -2 , 5, 5 );

	if ( w ) {
	    qt_handle_xdnd_enter( w, (const XEvent *)&enter );
	} else {
	    XSendEvent( qt_xdisplay(), target, FALSE, NoEventMask,
			(XEvent*)&enter );
	    XSelectInput( qt_xdisplay(), target, SubstructureNotifyMask );
	}

    }

    XClientMessageEvent move;
    move.type = ClientMessage;
    move.window = target;
    move.format = 32;
    move.message_type = qt_xdnd_position;
    move.window = target;
    move.data.l[0] = object->source()->winId();
    move.data.l[1] = 0; // flags
    move.data.l[2] = (globalPos.x() << 16) + globalPos.y();
    move.data.l[3] = qt_x_clipboardtime;
    move.data.l[4] = 0;

    if ( w )
	qt_handle_xdnd_position( w, (const XEvent *)&move );
    else
	XSendEvent( qt_xdisplay(), target, FALSE, NoEventMask,
		    (XEvent*)&move );
}


void QDragManager::drop()
{
    if ( !qt_xdnd_current_target )
	return;

    XClientMessageEvent drop;
    drop.type = ClientMessage;
    drop.window = qt_xdnd_current_target;
    drop.format = 32;
    drop.message_type = qt_xdnd_drop;
    drop.data.l[0] = object->source()->winId();
    drop.data.l[1] = 1 << 24; // flags
    drop.data.l[2] = 0; // ###
    drop.data.l[3] = 0;
    drop.data.l[4] = 0;

    QWidget * w = QWidget::find( qt_xdnd_current_target );
    if ( w )
	qt_handle_xdnd_drop( w, (const XEvent *)&drop );
    else
	XSendEvent( qt_xdisplay(), qt_xdnd_current_target, FALSE, NoEventMask,
		    (XEvent*)&drop );
    qt_xdnd_current_target = 0;
    if ( restoreCursor ) {
	QApplication::restoreOverrideCursor();
	restoreCursor = FALSE;
    }
}



void qt_xdnd_handle_destroy_notify( const XDestroyWindowEvent * e )
{
    if ( e->window == qt_xdnd_current_target ) {
	qt_xdnd_current_target = 0;
    }
    if ( e->window == (Window)qt_xdnd_dragsource_xid ) {
	qt_xdnd_dragsource_xid = 0;
	if ( qt_xdnd_current_widget ) {
	    QDragLeaveEvent e;
	    QApplication::sendEvent( qt_xdnd_current_widget, &e );
	    qt_xdnd_current_widget = 0;
	}
    }
}


/*!  Returns a string describing one of the available data types for
  this drag.  Common examples are "text/plain" and "image/gif".  If \a
  n is less than zero or greater than the number of available data
  types, format() returns 0.

  This function is provided mainly for debugging.  Most drop targets
  are probably better off using provides().

  \sa data() provides()
*/

const char * QDragMoveEvent::format( int n )
{
    int i = 0;
    while( i<n && qt_xdnd_types[i] )
	i++;
    if ( i < n )
	return 0;

    QString * name = qt_xdnd_drag_types->find( (long)(qt_xdnd_types[i]) );
    if ( !name )
	return 0;

    return *name;
}

/*!  Returns TRUE if this drag object provides format \a mimeType or
  FALSE if it does not.

  \sa data()
*/

bool QDragMoveEvent::provides( const char * mimeType )
{
    int n=0;
    const char * p;
    do {
	p = format( n );
	if ( !p )
	    return FALSE;
	n++;
    } while( qstricmp( mimeType, p ) );
    return TRUE;
}

void qt_xdnd_handle_selection_request( const XSelectionRequestEvent * req )
{
    if ( !req || !qt_xdnd_drag_types )
	return;
    XEvent evt;
    evt.xselection.type = SelectionNotify;
    evt.xselection.display = req->display;
    evt.xselection.requestor = req->requestor;
    evt.xselection.selection = req->selection;
    evt.xselection.target = req->target;
    evt.xselection.property = None;
    evt.xselection.time = req->time;
    QString * format = qt_xdnd_drag_types->find( req->target );
    if ( format && qt_xdnd_source_object &&
	 qt_xdnd_source_object->provides( *format ) ) {
	QByteArray a = qt_xdnd_source_object->encodedData(*format);
	XChangeProperty ( qt_xdisplay(), req->requestor, req->property,
			  req->target, 8, PropModeReplace,
			  (unsigned char *)a.data(), a.size() );
	evt.xselection.property = req->property;
    }
    // ### this can die if req->requestor crashes at the wrong
    // ### moment
    XSendEvent( qt_xdisplay(), req->requestor, False, 0, &evt );
}

/*
	XChangeProperty ( qt_xdisplay(), req->requestor, req->property,
			  XA_STRING, 8,
			  PropModeReplace,
			  (uchar *)d->text(), strlen(d->text()) );
	evt.xselection.property = req->property;
*/

static QByteArray qt_xdnd_obtain_data( const char * format )
{
    QByteArray result;

    debug( "want <%s>", format );

    if ( qt_xdnd_dragsource_xid && qt_xdnd_source_object &&
	 QWidget::find( qt_xdnd_dragsource_xid ) ) {
	QDragObject * o = qt_xdnd_source_object;
	if ( o->provides( format ) )
	    result = o->encodedData(format);
	return result;
    }

    Atom * a = qt_xdnd_atom_numbers->find( format );
    if ( !a || !*a )
	return result;

    if ( !qt_xdnd_target_data )
	qt_xdnd_target_data = new QIntDict<QByteArray>( 17 );

    if ( qt_xdnd_target_data->find( (int)*a ) ) {
	result = *(qt_xdnd_target_data->find( (int)*a ));
    } else {
	if ( XGetSelectionOwner( qt_xdnd_current_widget->x11Display(),
				 qt_xdnd_selection ) == None )
	    return result; // should never happen?

	XConvertSelection( qt_xdnd_current_widget->x11Display(),
			   qt_xdnd_selection, *a,
			   qt_xdnd_selection,
			   qt_xdnd_current_widget->winId(), CurrentTime );
	XFlush( qt_xdnd_current_widget->x11Display() );

	XEvent xevent;
	if ( !qt_xclb_wait_for_event( qt_xdnd_current_widget->x11Display(),
				      qt_xdnd_current_widget->winId(),
				      SelectionNotify, &xevent, 5000) )
	    return result;

	Atom type;

	if ( qt_xclb_read_property( qt_xdnd_current_widget->x11Display(),
				    qt_xdnd_current_widget->winId(),
				    qt_xdnd_selection, TRUE,
				    &result, 0, &type, 0 ) ) {
	    if ( type == None ) {
		return result;
	    } else if ( type == qt_incr_atom ) {
		int nbytes = result.size() >= 4 ? *((int*)result.data()) : 0;
		result = qt_xclb_read_incremental_property( qt_xdnd_current_widget->x11Display(),
							    qt_xdnd_current_widget->winId(),
							    qt_xdnd_selection,
							    nbytes );
	    } else if ( type != *a ) {
		//debug( "Qt clipboard: unknown atom %ld", type);
	    }
	}
	qt_xdnd_target_data->insert( (int)a, new QByteArray( result ) );
    }
	
    return result;
}

/*!  Returns a byte array containing the payload data of this drag, in
  \a format.

  data() normally needs to get the data from the drag source, which is
  potentially very slow, so it's advisable to call this function only
  if you're sure that you will need the data in \a format.

  \sa format()
*/

QByteArray QDragMoveEvent::data( const char * format )
{
    return qt_xdnd_obtain_data( format );
}


/*!  Returns a byte array containing the payload data of this drag, in
  \a format.

  data() normally needs to get the data from the drag source, which is
  potentially very slow, so it's advisable to call this function only
  if you're sure that you will need the data in \a format.

  \sa QDragMoveEvent::data() QDragMoveEvent::format()
*/

QByteArray QDropEvent::data( const char * format )
{
    return qt_xdnd_obtain_data( format );
}


void QDragManager::startDrag( QDragObject * o )
{
    if ( object == o )
	return;

    if ( object ) {
	cancel();
	dragSource->removeEventFilter( this );
	beingCancelled = FALSE;
    }

    qt_xdnd_source_object = o;

    willDrop = FALSE;

    object = o;
    dragSource = (QWidget *)(object->parent());
    dragSource->installEventFilter( this );
    qt_xdnd_source_current_time = qt_x_clipboardtime;
    XSetSelectionOwner( qt_xdisplay(), qt_xdnd_selection,
			dragSource->topLevelWidget()->winId(),
			qt_xdnd_source_current_time );
}
