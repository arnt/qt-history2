/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard_x11.cpp#17 $
**
** Implementation of QClipboard class for X11
**
** Created : 960430
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qclipbrd.h"
#include "qapp.h"
#include "qpixmap.h"
#include "qdatetm.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qclipboard_x11.cpp#17 $");


/*****************************************************************************
  Internal QClipboard functions for X11.
 *****************************************************************************/

extern Time qt_x_clipboardtime;			// def. in qapp_x11.cpp

static QWidget *clipboardOwner()
{
    static QWidget *fakeOwner = 0;
    if ( fakeOwner )				// fakeOwner already created
	return fakeOwner;
    if ( qApp->mainWidget() )			// there's a main widget
	fakeOwner = qApp->mainWidget();
    else					// otherwise create fake widget
	fakeOwner = new QWidget( 0, "internalClipboardOwner" );
    return fakeOwner;
}

enum ClipboardFormat { CFNothing, CFText, CFPixmap };

static ClipboardFormat getFormat( const char *format )
{
    if ( strcmp(format,"TEXT") == 0 )
	 return CFText;
    else if ( strcmp(format,"PIXMAP") == 0 )
	return CFPixmap;
    return CFNothing;
}

class QClipboardData
{
public:
    QClipboardData();
   ~QClipboardData();

    ClipboardFormat	format() const;

    void	       *data( const char *format ) const;
    void		setData( const char *format, void * );

    char	       *text() const;
    void		setText( const char * );
    QPixmap	       *pixmap() const;
    void		setPixmap( QPixmap * );

    void		clear();

private:
    ClipboardFormat	f;
    QString		t;
    QPixmap	       *p;

};

QClipboardData::QClipboardData()
{
    f = CFNothing;
    p = 0;
}

QClipboardData::~QClipboardData()
{
    delete p;
}

inline ClipboardFormat QClipboardData::format() const
{
    return f;
}

inline char *QClipboardData::text() const
{
    return t.data();
}

inline void QClipboardData::setText( const char *text )
{
    t = text;
    f = CFText;
}

inline QPixmap *QClipboardData::pixmap() const
{
    return p;
}

inline void QClipboardData::setPixmap( QPixmap *pixmap )
{
    if ( p )
	delete p;
    p = new QPixmap( *pixmap );
    f = CFPixmap;
}

void *QClipboardData::data( const char *format ) const
{
    switch ( getFormat(format) ) {
	case CFText:
	    return text();
	case CFPixmap:
	    return pixmap();
	default:
	    return 0;
    }
}

void QClipboardData::setData( const char *format, void *data )
{
    switch ( getFormat(format) ) {
	case CFText:
	    setText( (const char *)data );
	    break;
	case CFPixmap:
	    setPixmap( (QPixmap *)data );
	    break;
	default:
	    break;
    }
}

void QClipboardData::clear()
{
    t.resize( 0 );
    delete p;
    p = 0;
    f = CFNothing;
}


static QClipboardData *internalCbData;

static void cleanupClipboardData()
{
    delete internalCbData;
}

static QClipboardData *clipboardData()
{
    if ( internalCbData == 0 ) {
	internalCbData = new QClipboardData;
	CHECK_PTR( internalCbData );
	qAddPostRoutine( cleanupClipboardData );
    }
    return internalCbData;
}


/*****************************************************************************
  QClipboard member functions for X11.
 *****************************************************************************/

/*!
  Clears the clipboard contents.
*/

void QClipboard::clear()
{
    setData( "TEXT", 0 );
}


/*!
  Returns a pointer to the clipboard data, where \e format is the clipboard
  format.

  We recommend that you use text() or pixmap() instead.
*/

void *QClipboard::data( const char *format ) const
{
    ClipboardFormat f = getFormat( format );
    switch ( f ) {
	case CFText:
	    break;				// text is ok
	case CFPixmap:
#if defined(CHECK_RANGE)
	    warning( "QClipboard::data: PIXMAP format not supported" );
#endif
	    return 0;
	default:
#if defined(CHECK_RANGE)
	    warning( "QClipboard::data: Unknown format: %s", format );
#endif
	    return 0;
    }

    QClipboardData *d = clipboardData();
    QWidget *owner = clipboardOwner();
    Window   win   = owner->winId();
    Display *dpy   = owner->x11Display();

    if ( d->format() != CFNothing ) {		// we own the clipboard
	ASSERT( XGetSelectionOwner(dpy,XA_PRIMARY) == win );
	return d->data(format);
    }

    if ( XGetSelectionOwner(dpy,XA_PRIMARY) == None )
	return 0;

    extern Atom qt_selection_id; // from qapp_x11.cpp
    XConvertSelection( dpy, XA_PRIMARY, XA_STRING, qt_selection_id, win,
		       CurrentTime );

    XFlush( dpy );

    XEvent xevent;

    QTime started = QTime::currentTime();
    while ( TRUE ) {
	if ( XCheckTypedWindowEvent(dpy,win,SelectionNotify,&xevent) )
	    break;
	QTime now = QTime::currentTime();
	if ( started > now )			// crossed midnight
	    started = now;
	if ( started.msecsTo(now) > 5000 ) {
	    return 0;
	}
        XSync(dpy,FALSE); // Toss a ball while we wait.
    }

    Atom prop = xevent.xselection.property;
    win	 = xevent.xselection.requestor;

    static QByteArray buf( 256 );
    Atom	actual_type;
    ulong	nitems, bytes_after;
    int		actual_format;
    int		nread = 0;
    uchar      *back;

    do {
	int r = XGetWindowProperty( dpy, win, prop,
				    nread/4, 1024, TRUE,
				    AnyPropertyType, &actual_type,
				    &actual_format, &nitems,
				    &bytes_after, &back );
	if ( r != Success  || actual_type != XA_STRING ) {
	    char *n = XGetAtomName( dpy, actual_type );
	    XFree( n );
	}
	if ( r != Success || actual_type != XA_STRING )
	    break;
	while ( nread + nitems >= buf.size() )
	    buf.resize( buf.size()*2 );
	memcpy( buf.data()+nread, back, nitems );
	nread += nitems;
	XFree( (char *)back );
    } while ( bytes_after > 0 );

    buf[nread] = 0;

    return buf.data();
}


/*!
  Copies text into the clipboard, where \e format is the clipboard format
  string and \e data is the data to be copied.

  We recommend that you use setText() or setPixmap() instead.
*/

void QClipboard::setData( const char *format, void *data )
{
    ClipboardFormat f = getFormat( format );
    switch ( f ) {
	case CFText:
	    break;
	case CFPixmap:
#if defined(CHECK_RANGE)
	    warning( "QClipboard::setData: PIXMAP format not supported" );
#endif
	    return;
	default:
#if defined(CHECK_RANGE)
	    warning( "QClipboard::setData: Unknown format: %s", format );
#endif
	    return;
    }

    QClipboardData *d = clipboardData();
    QWidget *owner = clipboardOwner();
    Window   win   = owner->winId();
    Display *dpy   = owner->x11Display();

    if ( d->format() != CFNothing ) {		// we own the clipboard
#if defined(DEBUG)
	ASSERT( XGetSelectionOwner(dpy,XA_PRIMARY) == win );
#endif
	d->setData( format, data );
	emit dataChanged();
	return;
    }

    d->clear();
    d->setData( format, data );

    XSetSelectionOwner( dpy, XA_PRIMARY, win, qt_x_clipboardtime );
    if ( XGetSelectionOwner(dpy,XA_PRIMARY) != win ) {
#if defined(DEBUG)
	warning( "QClipboard::setData: Cannot set X11 selection owner" );
#endif
	return;
    }
}


/*!
  \internal
  Internal cleanup for Windows.
*/

void QClipboard::ownerDestroyed()
{
}


/*!
  \internal
  Internal optimization for Windows.
*/

void QClipboard::connectNotify( const char * )
{
}


/*!
  Handles clipboard events (very platform-specific).
*/

bool QClipboard::event( QEvent *e )
{
    if ( e->type() != Event_Clipboard )
	return QObject::event( e );

    XEvent *xevent = (XEvent *)Q_CUSTOM_EVENT(e)->data();
    Display *dpy = qt_xdisplay();
    QClipboardData *d = clipboardData();

    switch ( xevent->type ) {

	case SelectionClear:			// new selection owner
	    clipboardData()->clear();
	    emit dataChanged();
	    break;

	case SelectionNotify:
	    clipboardData()->clear();
	    break;

	case SelectionRequest: {		// someone wants our data
	    XSelectionRequestEvent *req = &xevent->xselectionrequest;
	    XEvent evt;
	    evt.xselection.type = SelectionNotify;
	    evt.xselection.display	= req->display;
	    evt.xselection.requestor	= req->requestor;
	    evt.xselection.selection	= req->selection;
	    evt.xselection.target	= req->target;
	    evt.xselection.property	= None;
	    evt.xselection.time = req->time;
	    if ( req->target == XA_STRING ) {
		XChangeProperty ( dpy, req->requestor, req->property,
				  XA_STRING, 8,
				  PropModeReplace,
				  (uchar *)d->text(), strlen(d->text()) );
		evt.xselection.property = req->property;
	    }
	    XSendEvent( dpy, req->requestor, False, 0, &evt );
	    }
	    break;
    }

    return TRUE;
}
