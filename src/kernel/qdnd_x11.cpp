/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdnd_x11.cpp#7 $
**
** XDND implementation for Qt.  See http://www.cco.caltech.edu/~jafl/xdnd2/
**
** Created : 980320
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qwidget.h"
#include "qintdict.h"
#include "qdatetm.h"
#include "qdict.h"
#include "qdragobject.h"
#include "qobjcoll.h"


#include <X11/X.h> // for Atom
#include <X11/Xlib.h> // for XEvent
#include <X11/Xatom.h> // for XA_STRING and friends


RCSTAG("$Id: //depot/qt/main/src/kernel/qdnd_x11.cpp#7 $");

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
static QDict<Atom> qt_xdnd_atom_numbers( 17 );

// rectangle in which the answer will be the same
static QRect qt_xdnd_source_sameanswer;
static QRect qt_xdnd_target_sameanswer;
// top-level window we sent position to last.
Window qt_xdnd_current_target;
// widget we forwarded position to last, and local position
QWidget * qt_xdnd_current_widget;
QPoint qt_xdnd_current_position;

// dict of stuff that is to be deleted at the target's request
QIntDict<QDragObject> * qt_xdnd_stored_drag_objects = 0;

// dict of payload data, sorted by type atom
QIntDict<QByteArray> qt_xdnd_target_data;

// first drag object, or 0
QDragObject * qt_xdnd_source_object = 0;

static void qt_xdnd_add_type( const char * mimeType )
{
    if ( !mimeType || !*mimeType )
	return;
    if ( qt_xdnd_atom_numbers.find( mimeType ) )
	return;

    Atom * tmp = new Atom;
    *tmp = 0;
    qt_xdnd_atom_numbers.insert( mimeType, tmp );
    qt_x11_intern_atom( mimeType, tmp );
    if ( qt_xdnd_drag_types ) {
	QString * s = new QString( mimeType );
	qt_xdnd_drag_types->insert( (long)(*tmp), s );
    }
    qt_xdnd_atom_numbers.setAutoDelete( TRUE );
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
    delete qt_xdnd_stored_drag_objects;
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
    const long *l = xe->xclient.data.l;

    debug( "xdnd enter" );

    // first, build the atom dict, if possible
    if ( !qt_xdnd_drag_types ) {
	qt_xdnd_drag_types = new QIntDict<QString>( 17 );
	qt_xdnd_drag_types->setAutoDelete( TRUE );
	QDictIterator<Atom> it( qt_xdnd_atom_numbers );
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

    // XSelectInput() for DestroyNotify on qt_dnd_dragsource_xid
    // so we'll know if it goes away

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
	debug( "more types than expected from %08lx", qt_xdnd_dragsource_xid );
    }
}


void qt_handle_xdnd_position( QWidget *w, const XEvent * xe )
{
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    QPoint p( (l[2] & 0xffff0000) >> 16, l[2] & 0x0000ffff );
    QWidget * c = find_child( w, p );

    if ( l[0] != qt_xdnd_dragsource_xid ) {
	debug( "xdnd drag position from unexpected source (%08lx not %08lx)",
	       l[0], qt_xdnd_dragsource_xid );
	return;
    }

    qt_xdnd_current_widget = c;
    qt_xdnd_current_position = p;

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

    QDragMoveEvent me( p );
    QApplication::sendEvent( w, &me );
    if ( me.isAccepted() )
	response.data.l[1] = 1; // yess!!!!

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
}


void qt_handle_xdnd_leave( QWidget *w, const XEvent * xe )
{
    if ( !qt_xdnd_current_widget ||
	 w->topLevelWidget() != qt_xdnd_current_widget->topLevelWidget() )
	return; // sanity

    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    debug( "xdnd leave" );

    if ( l[0] != qt_xdnd_dragsource_xid ) {
	debug( "xdnd drag leave from unexpected source (%08lx not %08lx",
	       l[0], qt_xdnd_dragsource_xid );
	qt_xdnd_current_widget = 0;
	return;
    }

    QDragLeaveEvent e;
    QApplication::sendEvent( qt_xdnd_current_widget, &e );

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
    if ( w )
	qt_handle_xdnd_status( w, (const XEvent *)&leave );
    else
	XSendEvent( w->x11Display(), qt_xdnd_current_target, FALSE,
		    NoEventMask, (XEvent*)&leave );
}


void qt_handle_xdnd_drop( QWidget *, const XEvent * xe )
{
    if ( !qt_xdnd_current_widget )
	return; // sanity

    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    debug( "xdnd drop" );

    if ( l[0] != qt_xdnd_dragsource_xid ) {
	debug( "xdnd drop from unexpected source (%08lx not %08lx",
	       l[0], qt_xdnd_dragsource_xid );
	return;
    }
    QDropEvent de( qt_xdnd_current_position );
    QApplication::sendEvent( qt_xdnd_current_widget, &de );

		
}


void qt_handle_xdnd_finished( QWidget *, const XEvent * xe )
{
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    if ( l[0] )
	qt_xdnd_stored_drag_objects->remove( l[0] );
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
    if ( qt_xdnd_source_sameanswer.contains( globalPos ) )
	return;

    Window target = 0;
    int lx = 0, ly = 0;
    if ( !XTranslateCoordinates( qt_xdisplay(), qt_xrootwin(), qt_xrootwin(),
				 globalPos.x(), globalPos.y(),
				 &lx, &ly, &target) ) {
	// somehow got to a different screen?  ignore for now
	return;
    }
    target = qt_x11_findClientWindow( target, qt_wm_state, TRUE );

    QWidget * w = QWidget::find( (WId)target );

    if ( target != qt_xdnd_current_target ) {
	if ( qt_xdnd_current_target )
	    qt_xdnd_send_leave();

	Atom * primaryType = qt_xdnd_atom_numbers.find( object->format() );
	XClientMessageEvent enter;
	enter.type = ClientMessage;
	enter.window = target;
	enter.format = 32;
	enter.message_type = qt_xdnd_enter;
	enter.data.l[0] = object->source()->winId();
	enter.data.l[1] = 1 << 24; // flags
	enter.data.l[2] = primaryType ? *primaryType : 0; // ###
	enter.data.l[3] = 0;
	enter.data.l[4] = 0;

	qt_xdnd_current_target = target;
	// provisionally set the rectangle to 5x5 pixels...
	qt_xdnd_source_sameanswer = QRect( globalPos.x() - 2,
					   globalPos.y() -2 , 5, 5 );

	if ( w )
	    qt_handle_xdnd_enter( w, (const XEvent *)&enter );
	else
	    XSendEvent( qt_xdisplay(), target, FALSE, NoEventMask,
			(XEvent*)&enter );
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
    move.data.l[3] = 0;
    move.data.l[4] = 0;

    if ( w ) {
	qt_handle_xdnd_position( w, (const XEvent *)&move );
    } else {
	XSendEvent( qt_xdisplay(), target, FALSE, NoEventMask,
		    (XEvent*)&move );
    }
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
}



void QDragManager::registerDropType( QWidget *, const char * mimeType )
{
    qt_xdnd_add_type( mimeType );
}


/*!  Returns a string describing one of the available data types for
  this drag.  Common examples are "text/plain" and "image/gif".

  If \a n is less than zero or greater than the number of available
  data types, format() returns 0.

  \sa data()
*/

const char * QDragMoveEvent::format( int  )
{
    return 0;
}


void qt_xdnd_handle_selection_request( const XSelectionRequestEvent * req )
{
    if ( !req || !qt_xdnd_drag_types )
	return;
    QString * format = qt_xdnd_drag_types->find( req->target );
    debug ( "hsr format %08lx string <%s> %p", req->target, format->data(),
	    qt_xdnd_source_object );
    if ( format && qt_xdnd_source_object ) {
	XEvent evt;
	evt.xselection.type = SelectionNotify;
	evt.xselection.display = req->display;
	evt.xselection.requestor = req->requestor;
	evt.xselection.selection = req->selection;
	evt.xselection.target = req->target;
	evt.xselection.property = None;
	evt.xselection.time = req->time;
	QDragObject * o = qt_xdnd_source_object;
	while( o && *format != o->format() )
	    o = o->alternative();
	if ( o ) {
	    XChangeProperty ( qt_xdisplay(), req->requestor, req->property,
			      req->target, 8,
			      PropModeReplace,
			      (unsigned char *)(o->encodedData().data()),
			      o->encodedData().size() );
	}
	XSendEvent( qt_xdisplay(), req->requestor, False, 0, &evt );
    }
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
    debug ( "xdnd od %s", format );
    QByteArray result;

    if ( qt_xdnd_dragsource_xid && qt_xdnd_source_object &&
	 QWidget::find( qt_xdnd_dragsource_xid ) ) {
	QDragObject * o = qt_xdnd_source_object;
	while( o && result.isNull() ) {
	    if ( !qstrcmp( o->format(), format ) )
		result = o->encodedData();
	    o = o->alternative();
	}
	return result;
    }

    Atom * a = qt_xdnd_atom_numbers.find( format );
    if ( !a || !*a )
	return result;

    if ( qt_xdnd_target_data.find( (int)*a ) ) {
	result = *qt_xdnd_target_data.find( (int)*a );
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
	    if ( type == qt_incr_atom ) {
		int nbytes = result.size() >= 4 ? *((int*)result.data()) : 0;
		result = qt_xclb_read_incremental_property( qt_xdnd_current_widget->x11Display(),
							    qt_xdnd_current_widget->winId(),
							    qt_xdnd_selection,
							    nbytes );
	    } else if ( type != *a ) {
		debug( "Qt clipboard: unknown atom %ld", type);
	    }
	}
	qt_xdnd_target_data.insert( (int)a, new QByteArray( result ) );
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

const QByteArray QDragMoveEvent::data( const char * format )
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
    if ( object == o ) {
	debug( "meaningless" );
	return;
    }

    if ( object ) {
	cancel();
	dragSource->removeEventFilter( this );
	beingCancelled = FALSE;
    }

    qt_xdnd_source_object = o;

    object = o;
    dragSource = (QWidget *)(object->parent());
    dragSource->installEventFilter( this );
    XSetSelectionOwner( qt_xdisplay(), qt_xdnd_selection,
			dragSource->topLevelWidget()->winId(),
			qt_x_clipboardtime );
}


