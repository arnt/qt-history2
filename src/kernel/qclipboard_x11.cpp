/****************************************************************************
** $Id$
**
** Implementation of QClipboard class for X11
**
** Created : 960430
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

// #define QCLIPBOARD_DEBUG
// #define QCLIPBOARD_DEBUG_VERBOSE

#include "qplatformdefs.h"
#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

#include "qapplication.h"
#include "qbitmap.h"
#include "qdatetime.h"
#include "qdragobject.h"
#include "qbuffer.h"
#include "qvaluelist.h"
#include "qt_x11.h"
#include "qapplication_p.h"


// REVISED: arnt

/*****************************************************************************
  Internal QClipboard functions for X11.
 *****************************************************************************/

extern Time qt_x_time;			// def. in qapplication_x11.cpp
extern Time qt_x_incr;			// def. in qapplication_x11.cpp
extern Atom qt_xa_clipboard;
extern Atom qt_selection_property;
extern Atom qt_clipboard_sentinel;
extern Atom qt_selection_sentinel;
extern Atom* qt_xdnd_str_to_atom( const char *mimeType );
extern const char* qt_xdnd_atom_to_str( Atom );


static QWidget * owner = 0;
static bool inSelectionMode_obsolete = FALSE; // ### remove 4.0
static bool timer_event_clear = FALSE;
static int timer_id = 0;

class QClipboardWatcher; // forward decl
static QClipboardWatcher *selection_watcher = 0;
static QClipboardWatcher *clipboard_watcher = 0;

static void cleanup()
{
    delete owner;
    owner = 0;
}

static
void setupOwner()
{
    if ( owner )
	return;
    owner = new QWidget( 0, "internal clipboard owner" );
    qAddPostRoutine( cleanup );
}

class QClipboardWatcher : public QMimeSource {
public:
    QClipboardWatcher( QClipboard::Mode mode );
    ~QClipboardWatcher();
    bool empty() const;
    const char* format( int n ) const;
    QByteArray encodedData( const char* fmt ) const;
    QByteArray getDataInFormat(Atom fmtatom) const;

    Atom atom;
    QValueList<const char *> formatList;
};



class QClipboardData
{
public:
    QClipboardData();
    ~QClipboardData();

    void setSource(QMimeSource* s)
    {
	QMimeSource *s2 = src;
	src = s;

	delete s2;
    }

    QMimeSource *source() const { return src; }

    void addTransferredPixmap(QPixmap pm)
    {
	/* TODO: queue them */
	transferred[tindex] = pm;
	tindex=(tindex+1)%2;
    }
    void clearTransfers()
    {
	transferred[0] = QPixmap();
	transferred[1] = QPixmap();
    }

    void clear();

    // private:
    QMimeSource *src;

    QPixmap transferred[2];
    int tindex;
};

QClipboardData::QClipboardData()
{
    src = 0;
    tindex=0;
}

QClipboardData::~QClipboardData()
{
    QMimeSource *s2 = src;
    src = 0;
    delete s2;
}

void QClipboardData::clear()
{
    QMimeSource *s2 = src;
    src = 0;
    delete s2;
}


static QClipboardData *internalCbData = 0;
static QClipboardData *internalSelData = 0;

static void cleanupClipboardData()
{
    delete internalCbData;
    internalCbData = 0;
}

static QClipboardData *clipboardData()
{
    if ( internalCbData == 0 ) {
	internalCbData = new QClipboardData;
	Q_CHECK_PTR( internalCbData );
	qAddPostRoutine( cleanupClipboardData );
    }
    return internalCbData;
}

static void cleanupSelectionData()
{
    delete internalSelData;
    internalSelData = 0;
}

static QClipboardData *selectionData()
{
    if (internalSelData == 0) {
	internalSelData = new QClipboardData;
	Q_CHECK_PTR(internalSelData);
	qAddPostRoutine(cleanupSelectionData);
    }
    return internalSelData;
}


/*****************************************************************************
  QClipboard member functions for X11.
 *****************************************************************************/


void QClipboard::clear( Mode mode )
{
    setText( QString::null, mode );
}


/*!
    Returns TRUE if the clipboard supports mouse selection; otherwise
    returns FALSE.
*/
bool QClipboard::supportsSelection() const
{
    return TRUE;
}


/*!
    Returns TRUE if this clipboard object owns the mouse selection
    data; otherwise returns FALSE.
*/
bool QClipboard::ownsSelection() const
{
    if (owner &&
	XGetSelectionOwner(owner->x11Display(), XA_PRIMARY) == owner->winId())
	return TRUE;

    return FALSE;
}

/*!
    Returns TRUE if this clipboard object owns the clipboard data;
    otherwise returns FALSE.
*/
bool QClipboard::ownsClipboard() const
{
    if (owner &&
	XGetSelectionOwner(owner->x11Display(), qt_xa_clipboard) == owner->winId())
	return TRUE;

    return FALSE;
}


/*! \obsolete

    Use the QClipboard::data(), QClipboard::setData() and related functions
    which take a QClipboard::Mode argument.

    Sets the clipboard selection mode. If \a enable is TRUE, then
    subsequent calls to QClipboard::setData() and other functions
    which put data into the clipboard will put the data into the mouse
    selection, otherwise the data will be put into the clipboard.

    \sa supportsSelection(), selectionModeEnabled()
*/
void QClipboard::setSelectionMode(bool enable)
{
    inSelectionMode_obsolete = enable;
}


/*! \obsolete

    Use the QClipboard::data(), QClipboard::setData() and related functions
    which take a QClipboard::Mode argument.

    Returns the selection mode.

    \sa setSelectionMode(), supportsSelection()
*/
bool QClipboard::selectionModeEnabled() const
{
    return inSelectionMode_obsolete;
}


/*! \internal
    Internal function to clear mouse selection and clipboard contents.
*/
void QClipboard::clobber()
{
    if (internalCbData) {
	qRemovePostRoutine(cleanupClipboardData);

	internalCbData->src = 0;
	delete internalCbData;
	internalCbData = 0;

	Window win = XGetSelectionOwner(owner->x11Display(), XA_PRIMARY);

	if (win == owner->winId())
	    XSetSelectionOwner(owner->x11Display(), XA_PRIMARY, None, qt_x_time);
    }

    if (internalSelData) {
	qRemovePostRoutine(cleanupSelectionData);

	internalSelData->src = 0;
	delete internalSelData;
	internalSelData = 0;

	Window win = XGetSelectionOwner(owner->x11Display(), qt_xa_clipboard);

	if (win == owner->winId())
	    XSetSelectionOwner(owner->x11Display(), qt_xa_clipboard, None, qt_x_time);
    }
}


bool qt_xclb_wait_for_event( Display *dpy, Window win, int type, XEvent *event,
			     int timeout )
{
    QTime started = QTime::currentTime();
    QTime now = started;
    bool flushed = FALSE;

    do {
	if ( XCheckTypedWindowEvent(dpy,win,type,event) )
	    return TRUE;

	now = QTime::currentTime();
	if ( started > now )			// crossed midnight
	    started = now;

	if(!flushed) {
	    XFlush( dpy );
	    flushed = TRUE;
	}

	// sleep 50ms, so we don't use up CPU cycles all the time.
	struct timeval usleep_tv;
	usleep_tv.tv_sec = 0;
	usleep_tv.tv_usec = 50000;
	select(0, 0, 0, 0, &usleep_tv);
    } while ( started.msecsTo(now) < timeout );

    return FALSE;
}


static inline int maxSelectionIncr( Display *dpy )
{
    return XMaxRequestSize(dpy) > 65536 ?
	4*65536 : XMaxRequestSize(dpy)*4 - 100;
}


// uglyhack: externed into qt_xdnd.cpp. qt is really not designed for
// single-platform, multi-purpose blocks of code...
bool qt_xclb_read_property( Display *dpy, Window win, Atom property,
			   bool deleteProperty,
			   QByteArray *buffer, int *size, Atom *type,
			   int *format, bool nullterm )
{
    int	   maxsize = maxSelectionIncr(dpy);
    ulong  bytes_left; // bytes_after
    ulong  length;     // nitems
    uchar *data;
    Atom   dummy_type;
    int    dummy_format;
    int    r;

    if ( !type )				// allow null args
	type = &dummy_type;
    if ( !format )
	format = &dummy_format;

    // Don't read anything, just get the size of the property data
    r = XGetWindowProperty( dpy, win, property, 0, 0, False,
			    AnyPropertyType, type, format,
			    &length, &bytes_left, &data );
    if ( r != Success ) {
	buffer->resize( 0 );
	return FALSE;
    }
    XFree( (char*)data );

    int  offset = 0, buffer_offset = 0, format_inc = 1, proplen = bytes_left;

#if defined (QCLIPBOARD_DEBUG)
    qDebug("qt_xclb_read_property: initial property length: %d", proplen);
#endif

    switch (*format) {
    case 8:
    default:
	format_inc = sizeof(char) / 1;
	break;

    case 16:
	format_inc = sizeof(short) / 2;
	proplen *= sizeof(short) / 2;
	break;

    case 32:
	format_inc = sizeof(long) / 4;
	proplen *= sizeof(long) / 4;
	break;
    }

    bool ok = buffer->resize( proplen + (nullterm ? 1 : 0) );

#if defined(QCLIPBOARD_DEBUG)
    qDebug("qt_xclb_read_property: buffer resized to %d", buffer->size());
#endif

    if ( ok ) {
	// could allocate buffer

	while ( bytes_left ) {
	    // more to read...

	    r = XGetWindowProperty( dpy, win, property, offset, maxsize/4,
				    False, AnyPropertyType, type, format,
				    &length, &bytes_left, &data );
	    if ( r != Success )
		break;

	    offset += length / (32 / *format);
	    length *= format_inc * (*format) / 8;

	    // Here we check if we get a buffer overflow and tries to
	    // recover -- this shouldn't normally happen, but it doesn't
	    // hurt to be defensive
	    if (buffer_offset + length > buffer->size()) {
		length = buffer->size() - buffer_offset;

		// escape loop
		bytes_left = 0;
	    }

	    memcpy(buffer->data() + buffer_offset, data, length);
	    buffer_offset += length;

	    XFree( (char*)data );
	}

 	static Atom xa_compound_text = *qt_xdnd_str_to_atom( "COMPOUND_TEXT" );
 	if ( *format == 8 && *type == xa_compound_text ) {
	    // convert COMPOUND_TEXT to a multibyte string
 	    XTextProperty textprop;
 	    textprop.encoding = *type;
 	    textprop.format = *format;
 	    textprop.nitems = length;
 	    textprop.value = (unsigned char *) buffer->data();

 	    char **list_ret = 0;
 	    int count;
	    if ( XmbTextPropertyToTextList( dpy, &textprop, &list_ret,
					    &count ) == Success &&
		 count && list_ret ) {
		offset = strlen( list_ret[0] );
		buffer->resize( offset + ( nullterm ? 1 : 0 ) );
		memcpy( buffer->data(), list_ret[0], offset );
	    }
 	    if (list_ret) XFreeStringList(list_ret);
 	}

	// zero-terminate (for text)
       	if (nullterm)
	    buffer->at(buffer_offset) = '\0';
    }

    // correct size, not 0-term.
    if ( size )
	*size = buffer_offset;

#if defined(QCLIPBOARD_DEBUG)
    qDebug("qt_xclb_read_property: buffer size %d, buffer offset %d, offset %d",
	   buffer->size(), buffer_offset, offset);
#endif

    if ( deleteProperty )
	XDeleteProperty( dpy, win, property );

    XFlush( dpy );

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

    for (;;) {
	if ( !qt_xclb_wait_for_event(dpy,win,PropertyNotify,
				     (XEvent*)&event,1000) )
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


/*! \internal
    Internal cleanup for Windows.
*/

void QClipboard::ownerDestroyed()
{
    owner = 0;
}


/*! \internal
    Internal optimization for Windows.
*/

void QClipboard::connectNotify( const char * )
{
}


/*! \reimp
 */
bool QClipboard::event( QEvent *e )
{
    if (e->type() == QEvent::Timer) {
	QTimerEvent *te = (QTimerEvent *) e;

	if (te->timerId() == timer_id) {
	    killTimer(timer_id);
	    timer_id = 0;

	    timer_event_clear = TRUE;
	    if ( selection_watcher ) // clear selection
		selectionData()->clear();
	    if ( clipboard_watcher ) // clear clipboard
		clipboardData()->clear();
	    timer_event_clear = FALSE;

	    return TRUE;
	} else
	    return QObject::event( e );
    } else if ( e->type() != QEvent::Clipboard )
	return QObject::event( e );

    XEvent *xevent = (XEvent *)(((QCustomEvent *)e)->data());
    Display *dpy = qt_xdisplay();

    switch ( xevent->type ) {

    case SelectionClear:
	// new selection owner
	if (xevent->xselectionclear.selection == XA_PRIMARY) {

#if defined(QCLIPBOARD_DEBUG)
	    qDebug("qclipboard_x11.cpp: new selection owner 0x%lx",
		   XGetSelectionOwner(dpy, XA_PRIMARY));
#endif
	    selectionData()->clear();
	    emit selectionChanged();
	} else {

#if defined (QCLIPBOARD_DEBUG)
	    qDebug("QClipboard: new clipboard owner 0x%lx",
		   XGetSelectionOwner(dpy, qt_xa_clipboard));
#endif

	    clipboardData()->clear();
	    emit dataChanged();
	}

	break;

    case SelectionNotify:
	// some has delivered data to us
	if (xevent->xselection.selection == XA_PRIMARY)
	    selectionData()->clear();
	else
	    clipboardData()->clear();

	break;

    case SelectionRequest:
	{
	    // someone wants our data
	    XSelectionRequestEvent *req = &xevent->xselectionrequest;

	    if (req->requestor == owner->winId()) break;

	    XEvent evt;
	    evt.xselection.type = SelectionNotify;
	    evt.xselection.display	= req->display;
	    evt.xselection.requestor	= req->requestor;
	    evt.xselection.selection	= req->selection;
	    evt.xselection.target	= req->target;
	    evt.xselection.property	= None;
	    evt.xselection.time = req->time;

#if defined(QCLIPBOARD_DEBUG)
	    qDebug("qclipboard_x11.cpp: selection requested\n"
		   "                    from 0x%lx\n"
		   "                    to 0x%lx (%s) 0x%lx (%s)",
		   req->requestor,
		   req->selection,
		   XGetAtomName(req->display, req->selection),
		   req->target,
		   XGetAtomName(req->display, req->target));
#endif

	    QClipboardData *d;
	    Mode m;

	    if ( req->selection == XA_PRIMARY ) {
		m = Selection;
		d = selectionData();
	    } else if ( req->selection == qt_xa_clipboard ) {
		m = Clipboard;
		d = clipboardData();
	    }
#ifdef QT_CHECK_RANGE
	    else {
		qWarning( "QClipboard::event: unknown selection request '%lx'",
			  req->selection );
		break;
	    }
#endif // QT_CHECK_RANGE

	    if ( !d->source() )
		d->setSource( new QClipboardWatcher( m ) );

	    const char* fmt;
	    QByteArray data;
	    static Atom xa_targets = *qt_xdnd_str_to_atom( "TARGETS" );
	    static Atom xa_multiple = *qt_xdnd_str_to_atom( "MULTIPLE" );
	    static Atom xa_utf8_string = *qt_xdnd_str_to_atom( "UTF8_STRING" );
	    static Atom xa_text = *qt_xdnd_str_to_atom( "TEXT" );
	    static Atom xa_compound_text = *qt_xdnd_str_to_atom( "COMPOUND_TEXT" );
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

		evt.xselection.property = None;

		if ( multi ) {
		    target = multi[imulti].target;
		    property = multi[imulti].property;
		    imulti++;
		} else {
		    target = req->target;
		    property = req->property;
		}

		if ( target == xa_targets ) {
		    int atoms = 0;
		    while (d->source()->format(atoms)) atoms++;
		    if (d->source()->provides("image/ppm")) atoms++;
		    if (d->source()->provides("image/pbm")) atoms++;
		    if (d->source()->provides("text/plain")) atoms+=3;

#if defined(QCLIPBOARD_DEBUG_VERBOSE)
		    qDebug("qclipboard_x11.cpp:%d: %d provided types", __LINE__, atoms);
#endif

		    // for 64 bit cleanness... XChangeProperty expects long* for data
		    // with format == 32
		    data = QByteArray(atoms * sizeof(long));
		    long *atarget = (long *) data.data();

		    int n = 0;
		    while ((fmt=d->source()->format(n)) && n < atoms) {
			Atom *dnd = qt_xdnd_str_to_atom(fmt);

			// compiler should handle the Atom to long conversion implicitly
			atarget[n++] = *dnd;
		    }

#if defined(QCLIPBOARD_DEBUG_VERBOSE)
		    qDebug("qclipboard_x11.cpp:%d: before standard, n(%d) atoms(%d)",
			   __LINE__, n, atoms);
#endif

		    if ( d->source()->provides("image/ppm") )
			atarget[n++] = XA_PIXMAP;
		    if ( d->source()->provides("image/pbm") )
			atarget[n++] = XA_BITMAP;
		    if ( d->source()->provides("text/plain") ) {
			atarget[n++] = xa_utf8_string;
			atarget[n++] = xa_compound_text;
			atarget[n++] = XA_STRING;
		    }

#if defined(QCLIPBOARD_DEBUG_VERBOSE)
		    for (int index = 0; index < n; index++) {
			qDebug("qclipboard_x11.cpp: atom %d: 0x%lx (%s)",
			       index, atarget[index],
			       XGetAtomName(dpy, atarget[index]));
		    }

		    qDebug("qclipboard_x11.cpp:%d: after standard, n(%d) atoms(%d)",
			   __LINE__, n, atoms);
		    qDebug("qclipboard_x11.cpp:%d: size %d %d",
			   __LINE__, n * sizeof(long), data.size());
#endif

		    XChangeProperty ( dpy, req->requestor, property, xa_targets, 32,
				      PropModeReplace, (uchar *) data.data(), n );
		    evt.xselection.property = property;
		    if ( multi )
			delete[] multi;
		} else {
		    bool already_done = FALSE;
		    if ( target == XA_STRING) {
			// the ICCCM states that STRING is latin1 plus newline and tab
			// see section 2.6.2
			fmt = "text/plain;charset=ISO-8859-1";
		    } else if ( target == xa_utf8_string ) {
			// proposed UTF8_STRING conversion type
			fmt = "text/plain;charset=UTF-8";
		    } else if ( target == xa_text || target == xa_compound_text ) {
			// the ICCCM states that TEXT and COMPOUND_TEXT are in the
			// encoding of choice, so we choose the encoding of the locale
			fmt = "text/plain";
			data = d->source()->encodedData( fmt );
			char *list[] = { data.data() };
			XICCEncodingStyle style;
			if ( target == xa_compound_text )
			    style = XCompoundTextStyle;
			else
			    style = XStdICCTextStyle;
			XTextProperty textprop;
			if ( XmbTextListToTextProperty( dpy, list, 1, style,
							&textprop ) == Success ) {
			    data.duplicate( (const char *) textprop.value,
					    textprop.nitems );
			    XFree( textprop.value );
			    XChangeProperty( dpy, req->requestor, property,
					     xa_compound_text, 8, PropModeReplace,
					     (unsigned char *) data.data(),
					     data.size() );
			    evt.xselection.property = property;
			}
			already_done = TRUE;
		    } else if ( target == XA_PIXMAP ) {
			fmt = "image/ppm";
			data = d->source()->encodedData(fmt);
			QPixmap pm;
			pm.loadFromData(data);
			Pixmap ph = pm.handle();
			XChangeProperty ( dpy, req->requestor, property,
					  target, 8,
					  PropModeReplace,
					  (uchar *)&ph,
					  sizeof(Pixmap));
			evt.xselection.property = property;
			d->addTransferredPixmap(pm);
			already_done = TRUE;
		    } else if ( target == XA_BITMAP ) {
			fmt = "image/pbm";
			data = d->source()->encodedData(fmt);
			QPixmap pm;
			QImage img;
			img.loadFromData(data);
			if ( img.depth() == 1 ) {
			    pm.convertFromImage(img);
			    Pixmap ph = pm.handle();
			    XChangeProperty ( dpy, req->requestor, property,
					      target, 8,
					      PropModeReplace,
					      (uchar *)&ph,
					      sizeof(Pixmap));
			    evt.xselection.property = property;
			    d->addTransferredPixmap(pm);
			} else {
			    pm.convertFromImage(img.convertDepth(1));
			    Pixmap ph = pm.handle();
			    XChangeProperty ( dpy, req->requestor, property,
					      target, 8,
					      PropModeReplace,
					      (uchar *)&ph,
					      sizeof(Pixmap));
			    evt.xselection.property = property;
			    d->addTransferredPixmap(pm);
			}
			already_done = TRUE;
		    } else {
			fmt = qt_xdnd_atom_to_str(target);
			if ( fmt && !d->source()->provides(fmt) ) {
			    fmt = 0; // Not a MIME type we can produce
			}
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

#if defined(QCLIPBOARD_DEBUG)
		qDebug("qclipboard_x11.cpp: SelectionNotify to 0x%lx\n"
		       "                    property 0x%lx (%s)",
		       req->requestor, evt.xselection.property,
		       XGetAtomName(dpy, evt.xselection.property));
#endif

		XSendEvent( dpy, req->requestor, False, 0, &evt );
		if ( !nmulti )
		    break;
	    }
	}
	break;
    }

    return TRUE;
}






QClipboardWatcher::QClipboardWatcher( QClipboard::Mode mode )
{
    switch ( mode ) {
    case QClipboard::Selection:
	atom = XA_PRIMARY;
	break;

    case QClipboard::Clipboard:
	atom = qt_xa_clipboard;
	break;

#ifdef QT_CHECK_RANGE
    default:
	qWarning( "QClipboardWatcher::empty: internal error!" );
	break;
#endif // QT_CHECK_RANGE
    }

    setupOwner();
}

QClipboardWatcher::~QClipboardWatcher()
{
    if( selection_watcher == this )
        selection_watcher = 0;
    if( clipboard_watcher == this )
        clipboard_watcher = 0;
}

bool QClipboardWatcher::empty() const
{
    Display *dpy = owner->x11Display();
    Window win = XGetSelectionOwner( dpy, atom );
    return win == None;
}

const char* QClipboardWatcher::format( int n ) const
{
    if ( empty() )
	return 0;

    if (! formatList.count()) {
	// get the list of targets from the current clipboard owner - we do this
	// once so that multiple calls to this function don't require multiple
	// server round trips...
	static Atom xa_targets = *qt_xdnd_str_to_atom( "TARGETS" );

	QClipboardWatcher *that = (QClipboardWatcher *) this;
	QByteArray ba = getDataInFormat(xa_targets);
	Atom *unsorted_target = (Atom *) ba.data();
	static Atom xa_utf8_string = *qt_xdnd_str_to_atom( "UTF8_STRING" );
	static Atom xa_text = *qt_xdnd_str_to_atom( "TEXT" );
	static Atom xa_compound_text = *qt_xdnd_str_to_atom( "COMPOUND_TEXT" );
	int i, size = ba.size() / sizeof(Atom);

	// sort TARGETS to prefer some types over others.  some apps
	// will report XA_STRING before COMPOUND_TEXT, and we want the
	// latter, not the former (if it is present).
	QByteArray sorted( ba.size() + 4 );
	sorted.fill( 0 );
	Atom *target = (Atom *) sorted.data();
	for ( i = 0; i < sorted.size(); ++i ) {
	    if ( unsorted_target[i] == xa_utf8_string )
		target[0] = unsorted_target[i];
	    else if ( unsorted_target[i] == xa_compound_text )
		target[1] = unsorted_target[i];
	    else if ( unsorted_target[i] == xa_text )
		target[2] = unsorted_target[i];
	    else if ( unsorted_target[i] == XA_STRING )
		target[3] = unsorted_target[i];
	    else
		target[i + 4] = unsorted_target[i];
	}

	for (i = 0; i < size; ++i) {
	    if ( target[i] == 0 ) continue;

	    if ( target[i] == XA_PIXMAP )
		that->formatList.append("image/ppm");
	    else if ( target[i] == XA_STRING )
		that->formatList.append( "text/plain;charset=ISO-8859-1" );
	    else if ( target[i] == xa_utf8_string )
		that->formatList.append( "text/plain;charset=UTF-8" );
	    else if ( target[i] == xa_text ||
		      target[i] == xa_compound_text )
		that->formatList.append( "text/plain" );
	    else
		that->formatList.append(qt_xdnd_atom_to_str(target[i]));
	}
    }

    if (n >= 0 && n < (signed) formatList.count())
	return formatList[n];
    if (n == 0)
	return "text/plain";
    return 0;
}

QByteArray QClipboardWatcher::encodedData( const char* fmt ) const
{
    if ( !fmt || empty() )
	return QByteArray( 0 );

    Atom fmtatom = 0;

    if ( 0==qstricmp(fmt,"text/plain;charset=iso-8859-1") ) {
	// ICCCM section 2.6.2 says STRING is latin1 text
	fmtatom = XA_STRING;
    } else if ( 0==qstricmp(fmt,"text/plain;charset=utf-8") ) {
	// proprosed UTF8_STRING conversion type
	fmtatom = *qt_xdnd_str_to_atom( "UTF8_STRING" );
    } else if ( 0==qstrcmp(fmt,"text/plain") ) {
   	fmtatom = *qt_xdnd_str_to_atom( "COMPOUND_TEXT" );
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
	fmtatom = *qt_xdnd_str_to_atom(fmt);
    }
    return getDataInFormat(fmtatom);
}

QByteArray QClipboardWatcher::getDataInFormat(Atom fmtatom) const
{
    QByteArray buf;

    Window   win   = owner->winId();
    Display *dpy   = owner->x11Display();

#ifdef QCLIPBOARD_DEBUG
    qDebug( "QClipboardWatcher::getDataInFormat: format %s",
	    XGetAtomName( dpy, fmtatom ) );
#endif // QCLIPBOARD_DEBUG

    XConvertSelection( dpy, atom,
		       fmtatom, qt_selection_property, win, CurrentTime );
    XFlush( dpy );

    XEvent xevent;
    if ( !qt_xclb_wait_for_event(dpy,win,SelectionNotify,&xevent,1000) )
	return buf;

    Atom   type;

    if ( qt_xclb_read_property(dpy,win,qt_selection_property,TRUE,
			       &buf,0,&type,0,FALSE) ) {
	if ( type == qt_x_incr ) {
	    int nbytes = buf.size() >= 4 ? *((int*)buf.data()) : 0;
	    buf = qt_xclb_read_incremental_property( dpy, win,
						     qt_selection_property,
						     nbytes, FALSE );
	}
    }

    return buf;
}


QMimeSource* QClipboard::data( Mode mode ) const
{
    QClipboardData *d;
    switch ( mode ) {
    case Selection: d = selectionData(); break;
    case Clipboard: d = clipboardData(); break;

    default:
#ifdef QT_CHECK_RANGE
	qWarning( "QClipboard::data: invalid mode '%d'", mode );
#endif // QT_CHECK_RANGE
	return 0;
    }

    if ( ! d->source() && ! timer_event_clear ) {
	if ( mode == Selection ) {
	    if ( ! selection_watcher )
		selection_watcher = new QClipboardWatcher( mode );
	    d->setSource( selection_watcher );
	} else {
	    if ( ! clipboard_watcher )
		clipboard_watcher = new QClipboardWatcher( mode );
	    d->setSource( clipboard_watcher );
	}

	if (! timer_id) {
	    // start a zero timer - we will clear cached data when the timer
	    // times out, which will be the next time we hit the event loop...
	    // that way, the data is cached long enough for calls within a single
	    // loop/function, but the data doesn't linger around in case the selection
	    // changes
	    QClipboard *that = ((QClipboard *) this);
	    timer_id = that->startTimer(0);
	}
    }

    return d->source();
}


void QClipboard::setData( QMimeSource* src, Mode mode )
{
    Atom atom, sentinel_atom;
    QClipboardData *d;
    switch ( mode ) {
    case Selection:
	atom = XA_PRIMARY;
	sentinel_atom = qt_selection_sentinel;
	d = selectionData();
	break;

    case Clipboard:
	atom = qt_xa_clipboard;
	sentinel_atom = qt_clipboard_sentinel;
	d = clipboardData();
	break;

    default:
#ifdef QT_CHECK_RANGE
	qWarning( "QClipboard::data: invalid mode '%d'", mode );
#endif // QT_CHECK_RANGE
	return;
    }

    setupOwner();
    Window   win   = owner->winId();
    Display *dpy   = owner->x11Display();

    d->setSource( src );
    if ( mode == Selection )
	emit selectionChanged();
    else
	emit dataChanged();

    Window prevOwner = XGetSelectionOwner( dpy, atom );
    XSetSelectionOwner( dpy, atom, win, qt_x_time );

    // ### perhaps this should be QT_CHECK_RANGE ?
    if ( XGetSelectionOwner(dpy, atom) != win ) {
#if defined(QCLIPBOARD_DEBUG)
	qDebug( "QClipboard::setData: Cannot set X11 selection owner for %s",
		XGetAtomName(dpy, atom));
#endif
	return;
    }

    // Signal to other Qt processes that the selection has changed
    Window owners[2];
    owners[0] = win;
    owners[1] = prevOwner;
    XChangeProperty( dpy, QApplication::desktop()->winId(),
		     sentinel_atom, XA_WINDOW, 32, PropModeReplace,
		     (unsigned char*)&owners, 2 );
}


/*
  Called by the main event loop in qapplication_x11.cpp when the
  _QT_SELECTION_SENTINEL property has been changed (i.e. when some Qt
  process has performed QClipboard::setData(). If it returns TRUE, the
  QClipBoard dataChanged() signal should be emitted.
*/

bool qt_check_selection_sentinel( XEvent* )
{
    bool doIt = TRUE;
    if ( owner ) {
	/*
	  Since the X selection mechanism cannot give any signal when
	  the selection has changed, we emulate it (for Qt processes) here.
	  The notification should be ignored in case of either
	  a) This is the process that did setData (because setData()
	  then has already emitted dataChanged())
	  b) This is the process that owned the selection when dataChanged()
	  was called (because we have then received a SelectionClear event,
	  and have already emitted dataChanged() as a result of that)
	*/

	Window* owners, rootwindow;
	Atom actualType;
	int actualFormat;
	ulong nitems;
	ulong bytesLeft;

	rootwindow = QApplication::desktop()->screen(owner->x11Screen())->winId();
	if (XGetWindowProperty( owner->x11Display(),
				rootwindow,
				qt_selection_sentinel, 0, 2, False, XA_WINDOW,
				&actualType, &actualFormat, &nitems,
				&bytesLeft, (unsigned char**)&owners ) == Success) {
	    if ( actualType == XA_WINDOW && actualFormat == 32 && nitems == 2 ) {
		Window win = owner->winId();
		if ( owners[0] == win || owners[1] == win )
		    doIt = FALSE;
	    }

	    XFree( owners );
	}
    }

    if (doIt)
	selectionData()->setSource(0);

    return doIt;
}


bool qt_check_clipboard_sentinel( XEvent* )
{
    bool doIt = TRUE;
    if (owner) {
	Window *owners, rootwindow;
	Atom actualType;
	int actualFormat;
	unsigned long nitems, bytesLeft;

	rootwindow = QApplication::desktop()->screen(owner->x11Screen())->winId();
	if (XGetWindowProperty(owner->x11Display(),
			       rootwindow,
			       qt_clipboard_sentinel, 0, 2, False, XA_WINDOW,
			       &actualType, &actualFormat, &nitems, &bytesLeft,
			       (unsigned char **) &owners) == Success) {
	    if (actualType == XA_WINDOW && actualFormat == 32 && nitems == 2) {
		Window win = owner->winId();
		if (owners[0] == win ||
		    owners[1] == win)
		    doIt = FALSE;
	    }

	    XFree(owners);
	}
    }

    if (doIt)
	clipboardData()->setSource(0);

    return doIt;
}

#endif // QT_NO_CLIPBOARD
