/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard_x11.cpp#40 $
**
** Implementation of QClipboard class for X11
**
** Created : 960430
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qclipboard.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qdatetime.h"
#include "qdragobject.h"
#include "qbuffer.h"
#include "qt_x11.h"


/*****************************************************************************
  Internal QClipboard functions for X11.
 *****************************************************************************/

extern Time qt_x_clipboardtime;			// def. in qapplication_x11.cpp
extern Atom qt_selection_property;
extern Atom* qt_xdnd_str_to_atom( const char *mimeType );
extern const char* qt_xdnd_atom_to_str( Atom );


static QWidget * owner = 0;

static void cleanup() {
    // ### when qapp stops deleting no-parent widgets, we must delete owner
    owner = 0;
}

static
void setupOwner()
{
    if ( owner )
	return;
    owner = new QWidget( 0, "internal clibpoard owner" );
    qAddPostRoutine( cleanup );
}


class QClipboardData
{
public:
    QClipboardData();
   ~QClipboardData();

    void		setSource(QMimeSource* s)
	{ delete src; src = s; }
    QMimeSource*	source()
	{ return src; }

    void		clear();

private:
    // NEW
    QMimeSource* src;
};

QClipboardData::QClipboardData()
{
    src = 0;
}

QClipboardData::~QClipboardData()
{
    delete src;
}

void QClipboardData::clear()
{
    delete src;
    src = 0;
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
    setText( QString::null );
}


bool qt_xclb_wait_for_event( Display *dpy, Window win, int type, XEvent *event,
			     int timeout )
{
    QTime started = QTime::currentTime();
    QTime now = started;
    do {
	if ( XCheckTypedWindowEvent(dpy,win,type,event) )
	    return TRUE;
	now = QTime::currentTime();
	if ( started > now )			// crossed midnight
	    started = now;
	XSync( dpy, FALSE );			// toss a ball while we wait
    } while ( started.msecsTo(now) < timeout );
    return FALSE;
}


static inline int maxSelectionIncr( Display *dpy )
{
    return XMaxRequestSize(dpy) > 65536 ?
	4*65536 : XMaxRequestSize(dpy)*4 - 100;
}


// uglehack: externed into qt_xdnd.cpp.  qt is really not designed for
// single-platform, multi-purpose blocks of code...
bool qt_xclb_read_property( Display *dpy, Window win, Atom property,
			   bool deleteProperty,
			   QByteArray *buffer, int *size, Atom *type,
			   int *format, bool nullterm )
{
    int	   maxsize = maxSelectionIncr(dpy);
    ulong  bytes_left;
    ulong  length;
    uchar *data;
    Atom   dummy_type;
    int    dummy_format;
    int    r;

    if ( !type )				// allow null args
	type = &dummy_type;
    if ( !format )
	format = &dummy_format;

    // Don't read anything, just get the size of the property data
    r = XGetWindowProperty( dpy, win, property, 0, 0, FALSE,
			    AnyPropertyType, type, format,
			    &length, &bytes_left, &data );
    if ( r != Success ) {
	buffer->resize( 0 );
	return FALSE;
    }
    XFree( (char*)data );

    int  offset = 0;
    bool ok = buffer->resize( (int)bytes_left+ (nullterm?1:0) );

    if ( ok ) {					// could allocate buffer
	while ( bytes_left ) {			// more to read...
	    r = XGetWindowProperty( dpy, win, property, offset/4, maxsize/4,
				    FALSE, AnyPropertyType, type, format,
				    &length, &bytes_left, &data );
	    if ( r != Success )
		break;
	    length *= *format/8;		// length in bytes
	    // Here we check if we get a buffer overflow and tries to
	    // recover -- this shouldn't normally happen, but it doesn't
	    // hurt to be defensive
	    if ( offset+length > buffer->size() ) {
		length = buffer->size() - offset;
		bytes_left = 0;			// escape loop
	    }
	    memcpy( buffer->data()+offset, data, (unsigned int)length );
	    offset += (unsigned int)length;
	    XFree( (char*)data );
	}
	if (nullterm)
	    buffer->at(offset) = '\0';		// zero-terminate (for text)
    }
    if ( size )
	*size = offset;				// correct size, not 0-term.
    XFlush( dpy );
    if ( deleteProperty ) {
	XDeleteProperty( dpy, win, property );
	XFlush( dpy );
    }
    return ok;
}


// this is externed into qt_xdnd.cpp too.
QByteArray qt_xclb_read_incremental_property( Display *dpy, Window win,
					      Atom property, int nbytes,
					      bool nullterm )
{
    XEvent event;

    QByteArray buf;
    QByteArray tmp_buf;
    bool alloc_error = FALSE;
    int  length;
    int  offset = 0;

    XWindowAttributes wa;
    XGetWindowAttributes( dpy, win, &wa );
    // Change the event mask for the window, it will be restored before
    // this function ends
    XSelectInput( dpy, win, PropertyChangeMask);

    if ( nbytes > 0 ) {
	// Reserve buffer + zero-terminator (for text data)
	// We want to complete the INCR transfer even if we cannot
	// allocate more memory
	alloc_error = !buf.resize(nbytes+1);
    }

    while ( TRUE ) {
	if ( !qt_xclb_wait_for_event(dpy,win,PropertyNotify,
				     (XEvent*)&event,5000) )
	    break;
	XFlush( dpy );
	if ( event.xproperty.atom != property ||
	     event.xproperty.state != PropertyNewValue )
	    continue;
	if ( qt_xclb_read_property(dpy, win, property, TRUE, &tmp_buf,
					&length,0, 0, nullterm) ) {
	    if ( length == 0 ) {		// no more data, we're done
		buf.at( offset ) = '\0';
		buf.resize( offset+1 );
		break;
	    } else if ( !alloc_error ) {
		if ( offset+length > (int)buf.size() ) {
		    if ( !buf.resize(offset+length+65535) ) {
			alloc_error = TRUE;
			length = buf.size() - offset;
		    }
		}
		memcpy( buf.data()+offset, tmp_buf.data(), length );
		tmp_buf.resize( 0 );
		offset += length;
	    }
	} else {
	    break;
	}
    }
    // Restore the event mask
    XSelectInput( dpy, win, wa.your_event_mask & ~PropertyChangeMask );
    return buf;
}


/*!
  Returns a pointer to the clipboard data, where \e format is the clipboard
  format.

  We recommend that you use text() or pixmap() instead.
*/

void *QClipboard::data( const char *format ) const
{
    // TODO: use new system in some trivial way (eg. "qt/void" mime type)
    return (void*)format;
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
    // #### Needs to deal with Unicode

    if ( e->type() != QEvent::Clipboard )
	return QObject::event( e );

    XEvent *xevent = (XEvent *)(((QCustomEvent *)e)->data());
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

	case SelectionRequest:
	  {		// someone wants our data
	    XSelectionRequestEvent *req = &xevent->xselectionrequest;
	    XEvent evt;
	    evt.xselection.type = SelectionNotify;
	    evt.xselection.display	= req->display;
	    evt.xselection.requestor	= req->requestor;
	    evt.xselection.selection	= req->selection;
	    evt.xselection.target	= req->target;
	    evt.xselection.property	= None;
	    evt.xselection.time = req->time;
	    // ### Should we check that we own the clipboard?
	    
	    const char* fmt;
	    QByteArray data;
	    static Atom xa_targets = *qt_xdnd_str_to_atom( "TARGETS" );
	    static Atom xa_multiple = *qt_xdnd_str_to_atom( "MULTIPLE" );
	    struct AtomPair { Atom target; Atom property; } *multi = 0;
	    int nmulti = 0;
	    int imulti = -1;
	    if ( req->target == xa_multiple ) {
		if ( qt_xclb_read_property( dpy,
			   req->requestor, req->property,
			   FALSE, &data, 0, 0, 0, 0 ) )
		{
		    nmulti = data.size()/sizeof(*multi);
		    multi = new AtomPair[nmulti];
		    memcpy(multi,data.data(),data.size());
		}
		imulti = 0;
	    }

	    while ( imulti < nmulti ) {
		Window target;
		Atom property;

		if ( multi ) {
		    target = multi[imulti].target;
		    property = multi[imulti].property;
		    imulti++;
		} else {
		    target = req->target;
		    property = req->property;
		}

		if ( target == xa_targets ) {
		    int n = 0;
		    while (d->source()->format(n))
			n++;
		    data = QByteArray(n*sizeof(Atom));
		    Atom* target = (Atom*)data.data();
		    n = 0;
		    while ((fmt=d->source()->format(n))) {
			target[n++] = *qt_xdnd_str_to_atom(fmt);
		    }
		    XChangeProperty ( dpy, req->requestor, property,
				      xa_targets, 32,
				      PropModeReplace,
				      (uchar *)data.data(),
				      data.size() );
		    evt.xselection.property = property;
		} else {
		    bool already_done = FALSE;
		    if ( target == XA_STRING ) {
			fmt = "text/plain";
		    } else if ( target == XA_PIXMAP ) {
			fmt = "image/ppm";
			data = d->source()->encodedData(fmt);
			QPixmap pm;
			pm.loadFromData(data);
			Pixmap ph = pm.handle();
			XChangeProperty ( dpy, req->requestor, property,
					  target, 32,
					  PropModeReplace,
					  (uchar *)&ph,
					  sizeof(Pixmap));
			evt.xselection.property = property;
			already_done = TRUE;
		    } else {
			fmt = qt_xdnd_atom_to_str(target);
			if ( fmt && !d->source()->provides(fmt) )
			    fmt = 0; // Not a MIME type we can produce
		    }
		    if ( fmt ) {
			if ( !already_done ) {
			    data = d->source()->encodedData(fmt);
			    XChangeProperty ( dpy, req->requestor, property,
					      target, 8,
					      PropModeReplace,
					      (uchar *)data.data(),
					      data.size() );
			    evt.xselection.property = property;
			}
		    }
		}
		XSendEvent( dpy, req->requestor, False, 0, &evt );
		if ( !nmulti )
		    break;
	    }
	  }
	  break;
    }

    return TRUE;
}

/*!
  Returns the clipboard text, or null if the clipboard does not contains
  any text.
  \sa setText()
*/

QString QClipboard::text() const
{
    QString r;
    QTextDrag::decode(data(),r);
    return r;
}

/*!
  Copies \e text into the clipboard.
  \sa text()
*/

void QClipboard::setText( const QString &text )
{
    setData(new QTextDrag(text));
}


/*!
  Returns the clipboard pixmap, or null if the clipboard does not contains
  any pixmap.
  \sa setText()
*/

QPixmap QClipboard::pixmap() const
{
    QPixmap r;
    QImageDrag::decode(data(),r);
    return r;
}

/*!
  Copies \e pixmap into the clipboard.
  \sa pixmap()
*/

void QClipboard::setPixmap( const QPixmap &pixmap )
{
    setData(new QImageDrag(pixmap.convertToImage()));
}


class QClipboardWatcher : public QMimeSource {
public:
    QClipboardWatcher()
    {
	setupOwner();
    }

    bool empty() const
    {
	Display *dpy   = owner->x11Display();
	return XGetSelectionOwner(dpy,XA_PRIMARY) == None;
    }

    const char* format( int n ) const
    {
	if ( empty() ) return 0;

	// TODO: record these once
	static Atom xa_targets = *qt_xdnd_str_to_atom( "TARGETS" );
	QByteArray targets = getDataInFormat(xa_targets);
	if ( targets.size()/sizeof(Atom) > (uint)n ) {
	    Atom* target = (Atom*)targets.data();
	    const char* fmt = qt_xdnd_atom_to_str(target[n]);
	    return fmt;
	} else {
	    if ( n == 0 )
		return "text/plain";
	}
	return 0;
    }

    QByteArray encodedData( const char* fmt ) const
    {
	if ( empty() ) return 0;

	Atom fmtatom = 0;

	if ( 0==qstrcmp(fmt,"text/plain") ) {
	    fmtatom = XA_STRING;
	} else if ( 0==qstrcmp(fmt,"image/ppm") ) {
	    fmtatom = XA_PIXMAP;
	    QByteArray pmd = getDataInFormat(fmtatom);
	    if ( pmd.size() == sizeof(Pixmap) ) {
		Pixmap xpm = *((Pixmap*)pmd.data());
		Display *dpy   = owner->x11Display();
		Window r;
		int x,y;
		uint w,h,bw,d;
		XGetGeometry(dpy,xpm, &r,&x,&y,&w,&h,&bw,&d);
		QImageIO iio;
		GC gc = XCreateGC( dpy, xpm, 0, 0 );
		if ( d == 1 ) {
		    QBitmap qbm(w,h);
		    XCopyArea(dpy,xpm,qbm.handle(),gc,0,0,w,h,0,0);
		    iio.setFormat("PBMRAW");
		    iio.setImage(qbm.convertToImage());
		} else {
		    QPixmap qpm(w,h);
		    XCopyArea(dpy,xpm,qpm.handle(),gc,0,0,w,h,0,0);
		    iio.setFormat("PPMRAW");
		    iio.setImage(qpm.convertToImage());
		}
		XFreeGC(dpy,gc);
		QBuffer buf;
		buf.open(IO_WriteOnly);
		iio.setIODevice(&buf);
		iio.write();
		return buf.buffer();
	    } else {
		fmtatom = *qt_xdnd_str_to_atom(fmt);
	    }
	} else {
	    // TODO: X11 pixmap standard (XA_PIXMAP?)

	    fmtatom = *qt_xdnd_str_to_atom(fmt);
	}
	return getDataInFormat(fmtatom);
    }

    QByteArray getDataInFormat(Atom fmtatom) const
    {
	QByteArray buf;

	Window   win   = owner->winId();
	Display *dpy   = owner->x11Display();

	XConvertSelection( dpy, XA_PRIMARY, fmtatom,
			   qt_selection_property, win, CurrentTime );
	XFlush( dpy );

	XEvent xevent;
	if ( !qt_xclb_wait_for_event(dpy,win,SelectionNotify,&xevent,5000) )
	    return buf;

	Atom   type;

	if ( qt_xclb_read_property(dpy,win,qt_selection_property,TRUE,
				   &buf,0,&type,0,FALSE) ) {
	    if ( type == XInternAtom(dpy,"INCR",FALSE) ) {
		int nbytes = buf.size() >= 4 ? *((int*)buf.data()) : 0;
		buf = qt_xclb_read_incremental_property( dpy, win,
							  qt_selection_property,
							  nbytes, FALSE );
	    }
	}

	return buf;
    }
};


/*!
  Returns a reference to a QMimeSource representation
  of the current clipboard data.
*/
QMimeSource* QClipboard::data() const
{
    QClipboardData *d = clipboardData();

    if ( !d->source() )
	d->setSource(new QClipboardWatcher());

    return d->source();
}

/*!
  Sets the clipboard data.  Ownership of the data is transferred
  to the clipboard - the only way to remove this data is to set
  something else, or to call clear().  The QDragObject subclasses
  are reasonable things to put on the clipboard (but do not try
  to \link QDragObject::drag() drag\endlink the same object).
  Do not put QDragMoveEvent or QDropEvent subclasses on the clipboard,
  as they do not belong to the event handler which receives them.

  The setText() and setPixmap() functions are shorthand ways
  of setting the data.
*/
void QClipboard::setData( QMimeSource* src )
{
    QClipboardData *d = clipboardData();
    setupOwner();
    Window   win   = owner->winId();
    Display *dpy   = owner->x11Display();

    d->setSource( src );
    emit dataChanged();

    XSetSelectionOwner( dpy, XA_PRIMARY, win, qt_x_clipboardtime );
    if ( XGetSelectionOwner(dpy,XA_PRIMARY) != win ) {
#if defined(DEBUG)
	warning( "QClipboard::setData: Cannot set X11 selection owner" );
#endif
	return;
    }
}
