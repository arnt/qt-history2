/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qt_xdnd.cpp#6 $
**
** XDND implementation for Qt.  See http://www.cco.caltech.edu/~jafl/xdnd/
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


#include <X11/X.h> // for Atom
#include <X11/Xlib.h> // for XEvent
#include <X11/Xatom.h> // for XA_STRING and friends


RCSTAG("$Id: //depot/qt/main/src/kernel/qt_xdnd.cpp#6 $");

// this stuff is copied from qapp_x11.cpp

extern void qt_x11_intern_atom( const char *, Atom * );
extern Window qt_x11_findClientWindow( Window, Atom, bool );

// and all this stuff is copied -into- qapp_x11.cpp

void qt_xdnd_setup();
void qt_handle_xdnd_enter( QWidget *, const XEvent * );
void qt_handle_xdnd_position( QWidget *, const XEvent * );
void qt_handle_xdnd_status( QWidget *, const XEvent * );
void qt_handle_xdnd_leave( QWidget *, const XEvent * );
void qt_handle_xdnd_drop( QWidget *, const XEvent * );

// this one's copied into qwid_x11.cpp
void qt_xdnd_add_type( const char * );

// clean up the stuff used.
static void qt_xdnd_cleanup();

// client messages
Atom qt_xdnd_enter;
Atom qt_xdnd_position;
Atom qt_xdnd_status;
Atom qt_xdnd_leave;
Atom qt_xdnd_drop;

// end of copied stuff

// XDND selection
static Atom qt_xdnd_selection;
// other selection
static Atom qt_selection_property;

// property for XDND drop sites
Atom qt_xdnd_aware;

// real variables:
// xid of current drag source
static Atom qt_xdnd_dragsource_xid = 0;
// the types in this drop.  100 is no good, but at least it's big.
static Atom qt_xdnd_types[100];
// preferred type, as atom
static Atom qt_xdnd_preferred_type = 0;

static QIntDict<QString> * qt_xdnd_drag_types = 0;
static QDict<Atom> qt_xdnd_atom_numbers( 17 );

// rectangle in which the answer will be the same
static QRect qt_xdnd_sameanswer;
// widget we sent position to last.
Window qt_xdnd_current_target;


void qt_xdnd_setup() {
    // set up protocol atoms
    qt_x11_intern_atom( "XdndEnter", &qt_xdnd_enter );
    qt_x11_intern_atom( "XdndPosition", &qt_xdnd_position );
    qt_x11_intern_atom( "XdndStatus", &qt_xdnd_status );
    qt_x11_intern_atom( "XdndLeave", &qt_xdnd_leave );
    qt_x11_intern_atom( "XdndDrop", &qt_xdnd_drop );

    qt_x11_intern_atom( "XdndSelection", &qt_xdnd_selection );

    qt_x11_intern_atom( "XdndAware", &qt_xdnd_aware );

    qt_x11_intern_atom( "QT_SELECTION", &qt_selection_property );

    // common MIME types
    qt_xdnd_add_type( "text/plain" );
    qt_xdnd_add_type( "image/gif" );
    qt_xdnd_add_type( "image/xpm" );
    qt_xdnd_add_type( "image/xbm" );

    qAddPostRoutine( qt_xdnd_cleanup );
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

    if ( version != 0 )
	return;

    qt_xdnd_dragsource_xid = l[0];

    // XSelectInput() for DestroyNotify on qt_dnd_dragsource_xid
    // so we'll know if it goes away

    // get the first types
    int i;
    int j = 0;
    for( i=2; i < 5; i++ ) {
	if ( qt_xdnd_drag_types->find( l[i] ) ) {
	    qt_xdnd_types[j++] = l[i];
	    debug( "l[%d] = %s", i,
		   qt_xdnd_drag_types->find( l[i] )->data() );
	} else {
	    debug( "l[%d] = unknown (%ld)", i, l[i] );
	}
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

    debug( "xdnd pos" );

    if ( l[0] != qt_xdnd_dragsource_xid ) {
	debug( "xdnd drag position from unexpected source (%08lx not %08lx)",
	       l[0], qt_xdnd_dragsource_xid );
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

    int i = 0;
    while( i < 100 /* ### */ && qt_xdnd_types[i] ) {
	QString * s = qt_xdnd_drag_types->find( qt_xdnd_types[i] );
	if ( s ) {
	    QDragMoveEvent me( QPoint( 0,0 ), *s );
	    QApplication::sendEvent( w, &me );
	    if ( me.isAccepted() ) {
		qt_xdnd_preferred_type = qt_xdnd_types[i];
		response.data.l[1] = 1; // yess!!!!
		break;
	    }
	}
	i++;
    }

    XSendEvent( w->x11Display(), qt_xdnd_dragsource_xid, FALSE, NoEventMask,
		(XEvent*)&response );
}

void qt_handle_xdnd_status( QWidget * w, const XEvent * xe )
{
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    QDragResponseEvent e( l[1] & 1 );
    QApplication::sendEvent( w, &e );
}


void qt_handle_xdnd_leave( QWidget *w, const XEvent * xe )
{
    const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    debug( "xdnd leave" );

    if ( l[0] != qt_xdnd_dragsource_xid ) {
	debug( "xdnd drag leave from unexpected source (%08lx not %08lx",
	       l[0], qt_xdnd_dragsource_xid );
	return;
    }

    QDragLeaveEvent e;
    QApplication::sendEvent( w, &e );

    qt_xdnd_dragsource_xid = 0;
    qt_xdnd_preferred_type = 0;
    qt_xdnd_types[0] = 0;
}


void qt_handle_xdnd_drop( QWidget *w, const XEvent * xe )
{
   const unsigned long *l = (const unsigned long *)xe->xclient.data.l;

    debug( "xdnd drop" );

    if ( l[0] != qt_xdnd_dragsource_xid ) {
	debug( "xdnd drag leave from unexpected source (%08lx not %08lx",
	       l[0], qt_xdnd_dragsource_xid );
	return;
    }

    // ### ugle follows.  beep beep beep.  ###

    if ( XGetSelectionOwner(w->x11Display(), qt_xdnd_selection ) == None )
	return;

    XConvertSelection( w->x11Display(), qt_xdnd_selection,
		       qt_xdnd_preferred_type, qt_selection_property,
		       w->winId(), CurrentTime );

    XFlush( w->x11Display() );

    XEvent xevent;

    QTime started = QTime::currentTime();
    while ( TRUE ) {
	if ( XCheckTypedWindowEvent(w->x11Display(), w->winId(),
				    SelectionNotify, &xevent) )
	    break;
	QTime now = QTime::currentTime();
	if ( started > now )			// crossed midnight
	    started = now;
	if ( started.msecsTo(now) > 5000 ) {
	    return;
	}
    }

    Atom prop = xevent.xselection.property;
    Window win = xevent.xselection.requestor;

    if ( !prop || !win )
	return;

    static QByteArray buf( 256 );
    Atom actual_type;
    ulong nitems, bytes_after;
    int actual_format;
    int nread = 0;
    uchar *back;

    do {
	int r = XGetWindowProperty( w->x11Display(), win, prop,
				    nread/4, 1024, TRUE,
				    AnyPropertyType, &actual_type,
				    &actual_format, &nitems,
				    &bytes_after, &back );
	if ( r != Success || actual_type != qt_xdnd_preferred_type )
	    break;
	while ( nread + nitems >= buf.size() )
	    buf.resize( buf.size()*2 );
	memcpy( buf.data()+nread, back, nitems );
	nread += nitems;
	XFree( (char *)back );
    } while ( bytes_after > 0 );

    buf[nread] = 0;

    QString * format = qt_xdnd_drag_types->find( qt_xdnd_preferred_type );
    if ( format ) {
	QDropEvent de( QPoint( 0,0 ), *format, buf );
	QApplication::sendEvent( w, &de );
    }
}

/*
    Atom qt_xdnd_version = (Atom)1;
    extern Atom qt_xdnd_aware;
    XChangeProperty ( dpy, id, qt_xdnd_aware, XA_ATOM, 32, PropModeReplace,
		      (unsigned char *)&qt_xdnd_version, 1 );
*/



void qt_xdnd_add_type( const char * mimeType )
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



void qt_xdnd_cleanup()
{
    delete qt_xdnd_drag_types;
    qt_xdnd_drag_types = 0;
}


void qt_xdnd_send_move( Window target, QDragObject * object, const QPoint &p )
{
    if ( qt_xdnd_sameanswer.contains( p ) )
	return;

    if ( !object->source() )
	return;

    int x = p.x(), y=p.y(), lx, ly;
    Window child = target;
    while( child ) {
	if ( !XTranslateCoordinates(qt_xdisplay(), target, target,
				    x, y,
				    &lx, &ly, &child) ) {
	    // somehow got to a different screen?  ignore for now
	    child = 0;
	}
	if ( child ) {
	    target = child;
	    x = lx;
	    y = ly;
	}
    }

    if ( target != qt_xdnd_current_target ) {
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

	XSendEvent( qt_xdisplay(), target, FALSE, NoEventMask,
		    (XEvent*)&enter );
	qt_xdnd_current_target = target;
	// provisionally set the rectangle to 5x5 pixels...
	qt_xdnd_sameanswer = QRect( p.x() - 2, p.y() -2 , 5, 5 );
    }

    XClientMessageEvent move;
    move.type = ClientMessage;
    move.window = target;
    move.format = 32;
    move.message_type = qt_xdnd_position;
    move.window = object->source()->winId();
    move.data.l[1] = 0; // flags
    move.data.l[2] = x << 16 + y;
    move.data.l[3] = 0;
    move.data.l[4] = 0;

    XSendEvent( qt_xdisplay(), target, FALSE, NoEventMask, (XEvent*)&move );

}
