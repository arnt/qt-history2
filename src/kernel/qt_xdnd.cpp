/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qt_xdnd.cpp#2 $
**
** XDND implementation for Qt.  See http://www.cco.caltech.edu/~jafl/xdnd/
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qwidget.h"
#include "qintdict.h"
#include "qdatetm.h"

#include <X11/X.h> // for Atom
#include <X11/Xlib.h> // for XEvent


RCSTAG("$Id: //depot/qt/main/src/kernel/qt_xdnd.cpp#2 $");

// this stuff is copied from qapp_x11.cpp

extern void qt_x11_intern_atom( const char *, Atom * );

// and all this stuff is copied -into- qapp_x11.cpp

void qt_xdnd_setup();
void qt_handle_xdnd_enter( QWidget *, const XEvent * );
void qt_handle_xdnd_position( QWidget *, const XEvent * );
void qt_handle_xdnd_status( QWidget *, const XEvent * );
void qt_handle_xdnd_leave( QWidget *, const XEvent * );
void qt_handle_xdnd_drop( QWidget *, const XEvent * );

// client messages
Atom qt_xdnd_enter;
Atom qt_xdnd_position;
Atom qt_xdnd_status;
Atom qt_xdnd_leave;
Atom qt_xdnd_drop;

// end of copied stuff

// XDND selection
static Atom qt_xdnd_selection;

// property for XDND drop sites
static Atom qt_xdnd_aware;

// real variables:
// xid of current drag source
static Atom qt_xdnd_dragsource_xid = 0;
//
static Atom qt_xdnd_types[100];

static QIntDict<QString> qt_xdnd_drag_types( 17 );



void qt_xdnd_setup() {
    // set up protocol atoms
    qt_x11_intern_atom( "XdndEnter", &qt_xdnd_enter );
    qt_x11_intern_atom( "XdndPosition", &qt_xdnd_position );
    qt_x11_intern_atom( "XdndStatus", &qt_xdnd_status );
    qt_x11_intern_atom( "XdndLeave", &qt_xdnd_leave );
    qt_x11_intern_atom( "XdndDrop", &qt_xdnd_drop );

    qt_x11_intern_atom( "XdndSelection", &qt_xdnd_selection );

    qt_x11_intern_atom( "XdndAware", &qt_xdnd_aware );
}


void qt_handle_xdnd_enter( QWidget *, const XEvent * xe ) {
    const long *l = xe->xclient.data.l;

    debug( "xdnd enter" );

    int version = (int)(((unsigned long)(l[1])) >> 24);

    if ( version != 0 )
	return;

    qt_xdnd_dragsource_xid = l[0];

    // XSelectInput() for DestroyNotify on qt_dnd_dragsource_xid
    // so we'll know if it goes away

    // get the first types
    int i;
    for( i=2; i < 5; i++ ) {
	qt_xdnd_types[i-2] = l[i];
	debug( "l[%d] = %ld", i, l[i] );
    }

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
	if ( qt_xdnd_types[i] == 42424242
	    /* qt_xdnd_drag_types[l[i]] */ ) {
	    QString promp("text/plain");
	    QDragMoveEvent me( QPoint( 0,0 ), promp );
			       //*(qt_xdnd_drag_types[l[i]]) );
	    QApplication::sendEvent( w, &me );
	    if ( me.isAccepted() ) {
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

    // ### ugle hack follows.  beep beep beep.  ###

    if ( XGetSelectionOwner(w->x11Display(), qt_xdnd_selection ) == None )
	return;

    extern Atom qt_selection_property; // from qapp_x11.cpp
    XConvertSelection( w->x11Display(), qt_xdnd_selection,
		       42424242, qt_selection_property,
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
	if ( r != Success  || actual_type != 42424242 ) {
	    char *n = XGetAtomName( w->x11Display(), actual_type );
	    XFree( n );  // ### tissbæsjpromp?
	}
	if ( r != Success || actual_type != 42424242 )
	    break;
	while ( nread + nitems >= buf.size() )
	    buf.resize( buf.size()*2 );
	memcpy( buf.data()+nread, back, nitems );
	nread += nitems;
	XFree( (char *)back );
    } while ( bytes_after > 0 );

    buf[nread] = 0;

    QString format( "text/plain" );

    QDropEvent de( QPoint( 0,0 ), format, buf );
    QApplication::sendEvent( w, &de );
}

/*
    Atom qt_xdnd_version = 0;
    extern Atom qt_xdnd_aware;
    XChangeProperty ( dpy, id, qt_xdnd_aware, XA_ATOM, 32, PropModeReplace,
		      (unsigned char *)&qt_xdnd_version, 1 );
*/

