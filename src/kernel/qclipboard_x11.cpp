/****************************************************************************
** $Id$
**
** Implementation of QClipboard class for X11
**
** Created : 960430
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
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

#ifdef QCLIPBOARD_DEBUG
#  define DEBUG qDebug
#else
#  define DEBUG if (FALSE) qDebug
#endif

#ifdef QCLIPBOARD_DEBUG_VERBOSE
#  define VDEBUG qDebug
#else
#  define VDEBUG if (FALSE) qDebug
#endif

#include "qplatformdefs.h"

// POSIX Large File Support redefines open -> open64
#if defined(open)
# undef open
#endif

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

#include "qapplication.h"
#include "qeventloop.h"
#include "qbitmap.h"
#include "qdatetime.h"
#include "qdragobject.h"
#include "qbuffer.h"
#include "qtextcodec.h"
#include "qlist.h"
#include "qt_x11_p.h"
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
extern Atom qt_utf8_string;
extern Atom* qt_xdnd_str_to_atom( const char *mimeType );
extern const char* qt_xdnd_atom_to_str( Atom );


static QWidget * owner = 0;
static QWidget *requestor = 0;
static bool timer_event_clear = FALSE;
static int timer_id = 0;

static int pending_timer_id = 0;
static bool pending_clipboard_changed = FALSE;
static bool pending_selection_changed = FALSE;

// event capture mechanism for qt_xclb_wait_for_event
static bool waiting_for_data = FALSE;
static bool has_captured_event = FALSE;
static Window capture_event_win = None;
static int capture_event_type = -1;
static XEvent captured_event;

class QClipboardWatcher; // forward decl
static QClipboardWatcher *selection_watcher = 0;
static QClipboardWatcher *clipboard_watcher = 0;

static void cleanup()
{
    delete owner;
    delete requestor;
    owner = 0;
    requestor = 0;
}

static
void setupOwner()
{
    if ( owner )
	return;
    owner = new QWidget( 0, "internal clipboard owner" );
    requestor = new QWidget(0, "internal clipboard requestor");
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
    QList<const char *> formatList;
};



class QClipboardData
{
public:
    QClipboardData();
    ~QClipboardData();

    void setSource(QMimeSource* s)
    {
	delete src;
	src = s;
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

    QMimeSource *src;
    Time timestamp;

    QPixmap transferred[2];
    int tindex;
};

QClipboardData::QClipboardData()
{
    src = 0;
    timestamp = CurrentTime;
    tindex=0;
}

QClipboardData::~QClipboardData()
{ clear(); }

void QClipboardData::clear()
{
    delete src;
    src = 0;
    timestamp = CurrentTime;
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
{ setData(0, mode); }


/*!
    Returns TRUE if the clipboard supports mouse selection; otherwise
    returns FALSE.
*/
bool QClipboard::supportsSelection() const
{ return TRUE; }


/*!
    Returns TRUE if this clipboard object owns the mouse selection
    data; otherwise returns FALSE.
*/
bool QClipboard::ownsSelection() const
{ return selectionData()->timestamp != CurrentTime; }

/*!
    Returns TRUE if this clipboard object owns the clipboard data;
    otherwise returns FALSE.
*/
bool QClipboard::ownsClipboard() const
{ return clipboardData()->timestamp != CurrentTime; }


// event filter function... captures interesting events while
// qt_xclb_wait_for_event is running the event loop
static int qt_xclb_event_filter(XEvent *event)
{
    if (event->xany.type == capture_event_type &&
	event->xany.window == capture_event_win) {
	VDEBUG( "qt_xclb_event_filter: caught event type %d", event->type );
	has_captured_event = TRUE;
	captured_event = *event;
	return 1;
    }

    return 0;
}

bool qt_xclb_wait_for_event( Display *dpy, Window win, int type, XEvent *event,
			     int timeout )
{
    if ( waiting_for_data )
	qFatal( "QClipboard: internal error, qt_xclb_wait_for_event recursed" );

    waiting_for_data = TRUE;

    QTime started = QTime::currentTime();
    QTime now = started;

    has_captured_event = FALSE;
    capture_event_win = win;
    capture_event_type = type;

    // from qapplication_x11.cpp
    typedef int (*QX11EventFilter) (XEvent*);
    extern QX11EventFilter qt_set_x11_event_filter (QX11EventFilter filter);
    QX11EventFilter old_event_filter = qt_set_x11_event_filter(qt_xclb_event_filter);

    do {
	if ( XCheckTypedWindowEvent(dpy,win,type,event) ) {
	    waiting_for_data = FALSE;
	    qt_set_x11_event_filter(old_event_filter);
	    return TRUE;
	}

	now = QTime::currentTime();
	if ( started > now )			// crossed midnight
	    started = now;

	// 0x08 == ExcludeTimers for X11 only
	qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput |
					  QEventLoop::ExcludeSocketNotifiers |
					  QEventLoop::WaitForMore | 0x08 );

	if ( has_captured_event ) {
	    waiting_for_data = FALSE;
	    *event = captured_event;
	    qt_set_x11_event_filter(old_event_filter);
	    return TRUE;
	}
    } while ( started.msecsTo(now) < timeout );

    waiting_for_data = FALSE;
    qt_set_x11_event_filter(old_event_filter);

    return FALSE;
}


static inline int maxSelectionIncr( Display *dpy )
{ return XMaxRequestSize(dpy) > 65536 ? 65536*4 : XMaxRequestSize(dpy)*4 - 100; }

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
    if (r != Success || (type && *type == None)) {
	buffer->resize( 0 );
	return FALSE;
    }
    XFree( (char*)data );

    int  offset = 0, buffer_offset = 0, format_inc = 1, proplen = bytes_left;

    VDEBUG("qt_xclb_read_property: initial property length: %d", proplen);

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

    int newSize = proplen + (nullterm ? 1 : 0);
    buffer->resize(newSize);

    bool ok = (buffer->size() == newSize);
    VDEBUG("qt_xclb_read_property: buffer resized to %d", buffer->size());

    if (ok) {
	// could allocate buffer

	while ( bytes_left ) {
	    // more to read...

	    r = XGetWindowProperty( dpy, win, property, offset, maxsize/4,
				    False, AnyPropertyType, type, format,
				    &length, &bytes_left, &data );
	    if (r != Success || (type && *type == None))
		break;

	    offset += length / (32 / *format);
	    length *= format_inc * (*format) / 8;

	    // Here we check if we get a buffer overflow and tries to
	    // recover -- this shouldn't normally happen, but it doesn't
	    // hurt to be defensive
	    if ((int)(buffer_offset + length) > buffer->size()) {
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
	    buffer[buffer_offset] = '\0';
    }

    // correct size, not 0-term.
    if ( size )
	*size = buffer_offset;

    VDEBUG("qt_xclb_read_property: buffer size %d, buffer offset %d, offset %d",
	   buffer->size(), buffer_offset, offset);

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

    if ( nbytes > 0 ) {
	// Reserve buffer + zero-terminator (for text data)
	// We want to complete the INCR transfer even if we cannot
	// allocate more memory
	buf.resize(nbytes+1);
	alloc_error = buf.size() != nbytes+1;
    }

    for (;;) {
	XFlush( dpy );
	if ( !qt_xclb_wait_for_event(dpy,win,PropertyNotify,
				     (XEvent*)&event,5000) )
	    break;
	if ( event.xproperty.atom != property ||
	     event.xproperty.state != PropertyNewValue )
	    continue;
	if ( qt_xclb_read_property(dpy, win, property, TRUE, &tmp_buf,
				   &length,0, 0, FALSE) ) {
	    if ( length == 0 ) {		// no more data, we're done
		if ( nullterm ) {
		    buf.resize( offset+1 );
		    buf[offset] = '\0';
		} else {
		    buf.resize(offset);
		}
		return buf;
	    } else if ( !alloc_error ) {
		if ( offset+length > (int)buf.size() ) {
		    buf.resize(offset+length+65535);
		    if ( buf.size() != offset+length+65535 ) {
			alloc_error = TRUE;
			length = buf.size() - offset;
		    }
		}
		memcpy( buf.data()+offset, tmp_buf.constData(), length );
		tmp_buf.resize( 0 );
		offset += length;
	    }
	} else {
	    break;
	}
    }

    // timed out ... create a new requestor window, otherwise the requestor
    // could consider next request to be still part of this timed out request
    delete requestor;
    requestor = new QWidget( 0, "internal clipboard requestor" );

    return QByteArray();
}

static void qt_xclb_send_incremental_property(Display *dpy, const Window window,
					      const Atom property, const Atom xtarget,
					      const int xformat, const QByteArray &data,
					      const unsigned int increment)
{
    DEBUG("QClipboard: starting INCR transfer, %d bytes", data.size());

    unsigned int offset = 0;
    unsigned int bytes_left = data.size();
    bool proto_error = FALSE;
    while (bytes_left > 0) {
	XEvent event;
	if (! qt_xclb_wait_for_event(dpy, window, PropertyNotify,
				     &event, 5000)) {
	    DEBUG("QClipboard: INCR: timeout waiting for PropertyDelete");
	    proto_error = TRUE;
	    break;
	}

	if (event.xproperty.state != PropertyDelete ||
	    event.xproperty.atom != property)
	    continue;

	unsigned int xfer = QMIN(increment, bytes_left);

	DEBUG("QClipboard: sending %d bytes", xfer);

	XChangeProperty(dpy, window, property, xtarget, xformat,
			PropModeReplace, (uchar *) data.constData() + offset, xfer);
	XFlush(dpy);

	offset += xfer;
	bytes_left -= xfer;

	VDEBUG("QClipboard: INCR: %d bytes remaining", bytes_left);
    }

    if (! proto_error) {
	DEBUG("QClipboard: finished sending INCR data");

	bool finished = FALSE;
	while (! finished) {
	    XEvent event;
	    if (! qt_xclb_wait_for_event(dpy, window, PropertyNotify, &event, 5000)) {
		DEBUG("QClipboard: INCR: timeout waiting for PropertyDelete");
		break;
	    }

	    if (event.xproperty.state != PropertyDelete ||
		event.xproperty.atom != property)
		continue;

	    VDEBUG("QClipboard: INCR: sending 0 bytes (final XChangeProperty)");

	    XChangeProperty(dpy, window, property, xtarget, xformat,
			    PropModeReplace, (uchar *) data.constData(), 0);
	    finished = TRUE;
	}
    }

    XSelectInput(dpy, window, NoEventMask);
}

static Atom send_targets_selection(QClipboardData *d, Window window, Atom property)
{
    int atoms = 0;
    while (d->source()->format(atoms)) atoms++;
    if (d->source()->provides("image/ppm")) atoms++;
    if (d->source()->provides("image/pbm")) atoms++;
    if (d->source()->provides("text/plain")) atoms+=4;

    VDEBUG("qclipboard_x11.cpp: send_targets_selection()\n"
	   "    %d provided types", atoms);

    // for 64 bit cleanness... XChangeProperty expects long* for data with format == 32
    QByteArray data((atoms+3) * sizeof(long)); // plus TARGETS, MULTIPLE and TIMESTAMP
    long *atarget = (long *) data.data();

    const char *fmt;
    int n = 0;
    while ((fmt=d->source()->format(n)) && n < atoms)
	atarget[n++] = *qt_xdnd_str_to_atom(fmt);

    VDEBUG("    before standard, n(%d) atoms(%d)", n, atoms);

    static Atom xa_text = *qt_xdnd_str_to_atom("TEXT");
    static Atom xa_compound_text = *qt_xdnd_str_to_atom("COMPOUND_TEXT");
    static Atom xa_targets = *qt_xdnd_str_to_atom("TARGETS");
    static Atom xa_multiple = *qt_xdnd_str_to_atom("MULTIPLE");
    static Atom xa_timestamp = *qt_xdnd_str_to_atom("TIMESTAMP");

    if (d->source()->provides("image/ppm"))
	atarget[n++] = XA_PIXMAP;
    if (d->source()->provides("image/pbm"))
	atarget[n++] = XA_BITMAP;
    if (d->source()->provides("text/plain")) {
	atarget[n++] = qt_utf8_string;
	atarget[n++] = xa_text;
	atarget[n++] = xa_compound_text;
	atarget[n++] = XA_STRING;
    }

    atarget[n++] = xa_targets;
    atarget[n++] = xa_multiple;
    atarget[n++] = xa_timestamp;

#if defined(QCLIPBOARD_DEBUG_VERBOSE)
    for (int index = 0; index < n; index++) {
	VDEBUG("    atom %d: 0x%lx (%s)", index, atarget[index],
	       qt_xdnd_atom_to_str(atarget[index]));
    }

    VDEBUG("    after standard, n(%d) atoms(%d)", n, atoms);
    VDEBUG("    size %d %d", n * sizeof(long), data.size());
#endif

    XChangeProperty(QPaintDevice::x11AppDisplay(), window, property, xa_targets, 32,
		    PropModeReplace, (uchar *) data.data(), n);
    return property;
}

static Atom send_string_selection(QClipboardData *d, Atom target, Window window, Atom property)
{
    static Atom xa_text = *qt_xdnd_str_to_atom("TEXT");
    static Atom xa_compound_text = *qt_xdnd_str_to_atom("COMPOUND_TEXT");

    DEBUG("qclipboard_x11.cpp: send_string_selection()\n"
	  "    property type %lx\n"
	  "    property name '%s'",
	  target, qt_xdnd_atom_to_str(target));

    if (target == xa_text || target == xa_compound_text) {
	// the ICCCM states that TEXT and COMPOUND_TEXT are in the
	// encoding of choice, so we choose the encoding of the locale
	QByteArray data = d->source()->encodedData("text/plain");
	data.resize(data.size() + 1);
	data[data.size() - 1] = '\0';
	char *list[] = { data.data(), NULL };

	XICCEncodingStyle style =
	    (target == xa_compound_text) ? XCompoundTextStyle : XStdICCTextStyle;
	XTextProperty textprop;
	if (list[0] != NULL
	    && XmbTextListToTextProperty(QPaintDevice::x11AppDisplay(),
					 list, 1, style, &textprop) == Success) {
	    DEBUG("    textprop type %lx\n"
		  "    textprop name '%s'\n"
		  "    format %d\n"
		  "    %ld items",
		  textprop.encoding, qt_xdnd_atom_to_str(textprop.encoding),
		  textprop.format, textprop.nitems);

	    XChangeProperty(QPaintDevice::x11AppDisplay(), window, property,
			    textprop.encoding, textprop.format, PropModeReplace,
			    textprop.value, textprop.nitems);
	    XFree(textprop.value);
	    return property;
	}

	return None;
    }

    Atom xtarget = None;
    const char *fmt = 0;
    if (target == XA_STRING
	|| (target == xa_text && QTextCodec::codecForLocale()->mibEnum() == 4)) {
	// the ICCCM states that STRING is latin1 plus newline and tab
	// see section 2.6.2
	fmt = "text/plain;charset=ISO-8859-1";
	xtarget = XA_STRING;
    } else if (target == qt_utf8_string) {
	// proposed UTF8_STRING conversion type
	fmt = "text/plain;charset=UTF-8";
	xtarget = qt_utf8_string;
    }

    if (xtarget == None) // should not happen
	return None;

    QByteArray data = d->source()->encodedData(fmt);

    DEBUG("    format 8\n    %d bytes", data.size());

    XChangeProperty(QPaintDevice::x11AppDisplay(), window, property, xtarget, 8,
		    PropModeReplace, (uchar *) data.data(), data.size());
    return property;
}

static Atom send_pixmap_selection(QClipboardData *d, Atom target, Window window, Atom property)
{
    QPixmap pm;

    if ( target == XA_PIXMAP ) {
	QByteArray data = d->source()->encodedData("image/ppm");
	pm.loadFromData(data);
    } else if ( target == XA_BITMAP ) {
	QByteArray data = d->source()->encodedData("image/pbm");
	QImage img;
	img.loadFromData(data);
	if ( img.depth() != 1 )
	    img = img.convertDepth(1);
    }

    if (pm.isNull()) // should never happen
	return None;

    Pixmap handle = pm.handle();
    XChangeProperty(QPaintDevice::x11AppDisplay(), window, property,
		    target, 32, PropModeReplace, (uchar *) &handle, 1);
    d->addTransferredPixmap(pm);
    return property;
}

static Atom send_selection(QClipboardData *d, Atom target, Window window, Atom property,
			   Atom selection, Time timestamp, bool allow_incr)
{
    Atom xtarget = 0;
    int xformat = 0;
    const char *fmt = 0;

    fmt = qt_xdnd_atom_to_str(target);
    xtarget = target;
    xformat = 8;
    if (fmt && !d->source()->provides(fmt)) // Not a MIME type we can produce
	return None;

    QByteArray data = d->source()->encodedData(fmt);

    int sz;
    switch (xformat) {
    default:
    case 8:  sz = sizeof( char); break;
    case 16: sz = sizeof(short); break;
    case 32: sz = sizeof( long); break;
    }

    DEBUG("qclipboard_x11.cpp: send_selection()\n"
	  "    property type %lx\n"
	  "    property name '%s'\n"
	  "    format %d\n"
	  "    %d bytes",
	  xtarget, qt_xdnd_atom_to_str(xtarget), xformat, data.size());

    const int increment =
	(XMaxRequestSize(QPaintDevice::x11AppDisplay()) >= 65536 ?
	 65536 : XMaxRequestSize(QPaintDevice::x11AppDisplay()));
    if ( data.size() > increment && allow_incr) {
	// only support INCR when not using INCR
	long bytes = data.size();
	XChangeProperty(QPaintDevice::x11AppDisplay(), window, property,
			qt_x_incr, 32, PropModeReplace, (uchar *) &bytes, 1);

	XEvent event;
	event.xselection.type      = SelectionNotify;
	event.xselection.display   = QPaintDevice::x11AppDisplay();
	event.xselection.requestor = window;
	event.xselection.selection = selection;
	event.xselection.target    = xtarget;
	event.xselection.property  = property;
	event.xselection.time      = timestamp;

	XSelectInput(QPaintDevice::x11AppDisplay(), window, PropertyChangeMask);
	XSendEvent(QPaintDevice::x11AppDisplay(), window, False, NoEventMask, &event);
	XFlush(QPaintDevice::x11AppDisplay());

	qt_xclb_send_incremental_property(QPaintDevice::x11AppDisplay(), window, property,
					  xtarget, xformat, data, increment);

	return qt_x_incr;
    }

    // make sure we can perform the XChangeProperty in a single request
    if ((int)data.size() >= XMaxRequestSize(QPaintDevice::x11AppDisplay()) - 100)
	return None; // ### perhaps use several XChangeProperty calls w/ PropModeAppend?

    // use a single request to transfer data
    XChangeProperty(QPaintDevice::x11AppDisplay(), window, property, xtarget,
		    xformat, PropModeReplace, (uchar *) data.data(), data.size() / sz);
    return property;
}

/*! \internal
    Internal cleanup for Windows.
*/
void QClipboard::ownerDestroyed()
{ }


/*! \internal
    Internal optimization for Windows.
*/
void QClipboard::connectNotify( const char * )
{ }


/*! \reimp
 */
bool QClipboard::event( QEvent *e )
{
    if ( e->type() == QEvent::Timer ) {
	QTimerEvent *te = (QTimerEvent *) e;

	if ( waiting_for_data ) // should never happen
	    return FALSE;

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
	} else if ( te->timerId() == pending_timer_id ) {
	    // I hate klipper
	    killTimer( pending_timer_id );
	    pending_timer_id = 0;

	    if ( pending_clipboard_changed ) {
		pending_clipboard_changed = FALSE;
		clipboardData()->clear();
		emit dataChanged();
	    }
	    if ( pending_selection_changed ) {
		pending_selection_changed = FALSE;
		selectionData()->clear();
		emit selectionChanged();
	    }

	    return TRUE;
	} else {
	    return QObject::event( e );
	}
    } else if ( e->type() != QEvent::Clipboard ) {
	return QObject::event( e );
    }

    XEvent *xevent = (XEvent *)(((QCustomEvent *)e)->data());
    Display *dpy = QPaintDevice::x11AppDisplay();

    if ( !xevent )
	return TRUE;

    switch ( xevent->type ) {

    case SelectionClear:
	// new selection owner
	if (xevent->xselectionclear.selection == XA_PRIMARY) {
	    QClipboardData *d = selectionData();

	    // ignore the event if it was generated before we gained selection ownership
	    if (d->timestamp != CurrentTime && xevent->xselectionclear.time < d->timestamp)
		break;

	    DEBUG("QClipboard: new selection owner 0x%lx at time %lx (ours %lx)",
		  XGetSelectionOwner(dpy, XA_PRIMARY),
		  xevent->xselectionclear.time, d->timestamp);

	    if ( ! waiting_for_data ) {
		d->clear();
		emit selectionChanged();
	    } else {
		pending_selection_changed = TRUE;
		if ( ! pending_timer_id )
		    pending_timer_id = QApplication::clipboard()->startTimer( 0 );
	    }
	} else if (xevent->xselectionclear.selection == qt_xa_clipboard) {
	    QClipboardData *d = clipboardData();

	    // ignore the event if it was generated before we gained selection ownership
	    if (d->timestamp != CurrentTime && xevent->xselectionclear.time < d->timestamp)
		break;

	    DEBUG("QClipboard: new clipboard owner 0x%lx at time %lx (%lx)",
		  XGetSelectionOwner(dpy, qt_xa_clipboard),
		  xevent->xselectionclear.time, d->timestamp);

	    if ( ! waiting_for_data ) {
		d->clear();
		emit dataChanged();
	    } else {
		pending_clipboard_changed = TRUE;
		if ( ! pending_timer_id )
		    pending_timer_id = QApplication::clipboard()->startTimer( 0 );
	    }
	} else {
#ifdef QT_CHECK_STATE
	    qWarning("QClipboard: Unknown SelectionClear event received.");
#endif
	    return FALSE;
	}
	break;

    case SelectionNotify:
	/*
	  Something has delivered data to us, but this was not caught
	  by QClipboardWatcher::getDataInFormat()

	  Just skip the event to prevent Bad Things (tm) from
	  happening later on...
	*/
	break;

    case SelectionRequest:
	{
	    // someone wants our data
	    XSelectionRequestEvent *req = &xevent->xselectionrequest;

	    if (req->requestor == requestor->winId()) break;

	    XEvent event;
	    event.xselection.type      = SelectionNotify;
	    event.xselection.display   = req->display;
	    event.xselection.requestor = req->requestor;
	    event.xselection.selection = req->selection;
	    event.xselection.target    = req->target;
	    event.xselection.property  = None;
	    event.xselection.time      = req->time;

	    DEBUG("qclipboard_x11.cpp: selection requested\n"
		  "                    from 0x%lx\n"
		  "                    to 0x%lx (%s) 0x%lx (%s)",
		  req->requestor,
		  req->selection,
		  qt_xdnd_atom_to_str(req->selection),
		  req->target,
		  qt_xdnd_atom_to_str(req->target));

	    QClipboardData *d;
	    Mode m;

	    if ( req->selection == XA_PRIMARY ) {
		m = Selection;
		d = selectionData();
	    } else if ( req->selection == qt_xa_clipboard ) {
		m = Clipboard;
		d = clipboardData();
	    } else {
#ifdef QT_CHECK_RANGE
		qWarning( "QClipboard::event: unknown selection request '%lx'",
			  req->selection );
#endif // QT_CHECK_RANGE

		XSendEvent(dpy, req->requestor, False, NoEventMask, &event);
		break;
	    }

	    if (! d->source()) {
#ifdef QT_CHECK_STATE
		qWarning("QClipboard: cannot transfer data, no data available");
#endif // QT_CHECK_STATE

		XSendEvent(dpy, req->requestor, False, NoEventMask, &event);
		break;
	    }

	    DEBUG("QClipboard: SelectionRequest at time %lx (ours %lx)",
		  req->time, d->timestamp);

	    if (d->timestamp == CurrentTime // we don't own the selection anymore
		|| (req->time != CurrentTime && req->time < d->timestamp)) {
		DEBUG("QClipboard: SelectionRequest too old");
		XSendEvent(dpy, req->requestor, False, NoEventMask, &event);
		break;
	    }

	    static Atom xa_text = *qt_xdnd_str_to_atom("TEXT");
	    static Atom xa_compound_text = *qt_xdnd_str_to_atom("COMPOUND_TEXT");
	    static Atom xa_targets = *qt_xdnd_str_to_atom("TARGETS");
	    static Atom xa_multiple = *qt_xdnd_str_to_atom("MULTIPLE");
	    static Atom xa_timestamp = *qt_xdnd_str_to_atom("TIMESTAMP");
	    static Atom motif_clip_temporary = *qt_xdnd_str_to_atom("CLIP_TEMPORARY");

	    struct AtomPair { Atom target; Atom property; } *multi = 0;
	    Atom multi_type = None;
	    int multi_format = 0;
	    int nmulti = 0;
	    int imulti = -1;
	    bool multi_writeback = FALSE;

	    if ( req->target == xa_multiple ) {
		QByteArray multi_data;
		if (req->property == None
		    || !qt_xclb_read_property(dpy, req->requestor, req->property,
					      FALSE, &multi_data, 0, &multi_type, &multi_format, 0)
		    || multi_format != 32) {
		    // MULTIPLE property not formatted correctly
		    XSendEvent(dpy, req->requestor, False, NoEventMask, &event);
		    break;
		}
		nmulti = multi_data.size()/sizeof(*multi);
		multi = new AtomPair[nmulti];
		memcpy(multi,multi_data.data(),multi_data.size());
		imulti = 0;
	    }

	    for (; imulti < nmulti; ++imulti) {
		Atom target;
		Atom property;

		if ( multi ) {
		    target = multi[imulti].target;
		    property = multi[imulti].property;
		} else {
		    target = req->target;
		    property = req->property;
		    if (property == None) // obsolete client
			property = target;
		}

		Atom ret = None;
		if (target == None || property == None) {
		    ;
		} else if (target == xa_timestamp) {
		    if (d->timestamp != CurrentTime) {
			XChangeProperty(dpy, req->requestor, property, xa_timestamp, 32,
					PropModeReplace, (uchar *) &d->timestamp, 1);
			ret = property;
		    } else {

#ifdef QT_CHECK_STATE
			qWarning("QClipboard: invalid data timestamp");
#endif // QT_CHECK_STATE
		    }
		} else if (target == xa_targets) {
		    ret = send_targets_selection(d, req->requestor, property);
		} else if (target == XA_STRING
			   || target == xa_text
			   || target == xa_compound_text
			   || target == qt_utf8_string) {
		    ret = send_string_selection(d, target, req->requestor, property);
		} else if (target == XA_PIXMAP
			   || target == XA_BITMAP) {
		    ret = send_pixmap_selection(d, target, req->requestor, property);
		} else {
		    // don't allow INCR transfers when using MULTIPLE or to
		    // Motif clients (since Motif doesn't support INCR)
		    bool allow_incr = nmulti == 0 && req->property != motif_clip_temporary;
		    ret = send_selection(d, target, req->requestor, property,
					 req->selection, req->time, allow_incr);
		}

		if (nmulti > 0) {
		    if (ret == None) {
			multi[imulti].property = None;
			multi_writeback = TRUE;
		    }
		} else {
		    event.xselection.property = ret;
		    break;
		}
	    }

	    if (nmulti > 0) {
		if (multi_writeback) {
		    // according to ICCCM 2.6.2 says to put None back
		    // into the original property on the requestor window
		    XChangeProperty(dpy, req->requestor, req->property, multi_type, 32,
				    PropModeReplace, (uchar *) multi, nmulti * 2);
		}

		delete [] multi;
		event.xselection.property = req->property;
	    }

	    if (event.xselection.property != qt_x_incr) {
		// send selection notify to requestor
		XSendEvent(dpy, req->requestor, False, NoEventMask, &event);

		DEBUG("qclipboard_x11.cpp: SelectionNotify to 0x%lx\n"
		      "                    property 0x%lx (%s)",
		      req->requestor, event.xselection.property,
		      qt_xdnd_atom_to_str(event.xselection.property));
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
	qWarning( "QClipboardWatcher: internal error, unknown clipboard mode" );
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
    Display *dpy = QPaintDevice::x11AppDisplay();
    Window win = XGetSelectionOwner( dpy, atom );

#ifdef QT_CHECK_STATE
    if( win == requestor->winId()) {
        qWarning( "QClipboardWatcher::empty: internal error, app owns the selection" );
        return TRUE;
    }
#endif // QT_CHECK_STATE

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
	if (ba.size() == 0) // no clipboard data available
	    return 0;

	Atom *unsorted_target = (Atom *) ba.data();
	static Atom xa_text = *qt_xdnd_str_to_atom( "TEXT" );
	static Atom xa_compound_text = *qt_xdnd_str_to_atom( "COMPOUND_TEXT" );
	int i, size = ba.size() / sizeof(Atom);

	// sort TARGETS to prefer some types over others.  some apps
	// will report XA_STRING before COMPOUND_TEXT, and we want the
	// latter, not the former (if it is present).
	Atom* target = new Atom[size+4];
	memset( target, 0, (size+4) * sizeof(Atom) );

	for ( i = 0; i < size; ++i ) {
	    if ( unsorted_target[i] == qt_utf8_string )
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

	for (i = 0; i < size + 4; ++i) {
	    if ( target[i] == 0 ) continue;

	    if ( target[i] == XA_PIXMAP )
		that->formatList.append("image/ppm");
	    else if ( target[i] == XA_STRING )
		that->formatList.append( "text/plain;charset=ISO-8859-1" );
	    else if ( target[i] == qt_utf8_string )
		that->formatList.append( "text/plain;charset=UTF-8" );
	    else if ( target[i] == xa_text ||
		      target[i] == xa_compound_text )
		that->formatList.append( "text/plain" );
	    else
		that->formatList.append(qt_xdnd_atom_to_str(target[i]));
	}
	delete []target;

	DEBUG("QClipboardWatcher::format: %d formats available",
	      that->formatList.count());
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
	return QByteArray();

    DEBUG("QClipboardWatcher::encodedData: fetching format '%s'", fmt);

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
	    Display *dpy = QPaintDevice::x11AppDisplay();
	    Window r;
	    int x,y;
	    uint w,h,bw,d;
	    if ( ! xpm )
		return QByteArray();
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

    Display *dpy = QPaintDevice::x11AppDisplay();
    Window   win = requestor->winId();

    DEBUG("QClipboardWatcher::getDataInFormat: selection '%s' format '%s'",
	  qt_xdnd_atom_to_str(atom), qt_xdnd_atom_to_str(fmtatom));

    XSelectInput(dpy, win, NoEventMask); // don't listen for any events

    XDeleteProperty(dpy, win, qt_selection_property);
    XConvertSelection(dpy, atom, fmtatom, qt_selection_property, win, qt_x_time);
    XFlush(dpy);

    VDEBUG("QClipboardWatcher::getDataInFormat: waiting for SelectionNotify event");

    XEvent xevent;
    if ( !qt_xclb_wait_for_event(dpy,win,SelectionNotify,&xevent,5000) ||
	 xevent.xselection.property == None ) {
	DEBUG("QClipboardWatcher::getDataInFormat: format not available");
	return buf;
    }

    VDEBUG("QClipboardWatcher::getDataInFormat: fetching data...");

    Atom   type;
    XSelectInput(dpy, win, PropertyChangeMask);

    if ( qt_xclb_read_property(dpy,win,qt_selection_property,TRUE,
			       &buf,0,&type,0,FALSE) ) {
	if ( type == qt_x_incr ) {
	    int nbytes = buf.size() >= 4 ? *((int*)buf.data()) : 0;
	    buf = qt_xclb_read_incremental_property( dpy, win,
						     qt_selection_property,
						     nbytes, FALSE );
	}
    }

    XSelectInput(dpy, win, NoEventMask);

    DEBUG("QClipboardWatcher::getDataInFormat: %d bytes received", buf.size());

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

    Display *dpy = QPaintDevice::x11AppDisplay();
    Window newOwner;

    if (! src) { // no data, clear clipboard contents
	newOwner = None;
	d->clear();
    } else {
	setupOwner();

	newOwner = owner->winId();

	d->setSource(src);
	d->timestamp = qt_x_time;
    }

    Window prevOwner = XGetSelectionOwner( dpy, atom );
    XSetSelectionOwner(dpy, atom, newOwner, d->timestamp);

    if ( mode == Selection )
	emit selectionChanged();
    else
	emit dataChanged();

    if (XGetSelectionOwner(dpy, atom) != newOwner) {
#ifdef QT_CHECK_STATE
	qWarning("QClipboard::setData: Cannot set X11 selection owner for %s",
		 qt_xdnd_atom_to_str(atom));
#endif // QT_CHECK_STATE

	d->clear();
	return;
    }

    // Signal to other Qt processes that the selection has changed
    Window owners[2];
    owners[0] = newOwner;
    owners[1] = prevOwner;
    XChangeProperty( dpy, QApplication::desktop()->screen(0)->winId(),
		     sentinel_atom, XA_WINDOW, 32, PropModeReplace,
		     (unsigned char*)&owners, 2 );
}


/*
  Called by the main event loop in qapplication_x11.cpp when the
  _QT_SELECTION_SENTINEL property has been changed (i.e. when some Qt
  process has performed QClipboard::setData(). If it returns TRUE, the
  QClipBoard dataChanged() signal should be emitted.
*/

bool qt_check_selection_sentinel()
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

	Window* owners;
	Atom actualType;
	int actualFormat;
	ulong nitems;
	ulong bytesLeft;

	if (XGetWindowProperty(QPaintDevice::x11AppDisplay(),
			       QApplication::desktop()->screen(0)->winId(),
			       qt_selection_sentinel, 0, 2, False, XA_WINDOW,
			       &actualType, &actualFormat, &nitems,
			       &bytesLeft, (unsigned char**)&owners) == Success) {
	    if ( actualType == XA_WINDOW && actualFormat == 32 && nitems == 2 ) {
		Window win = owner->winId();
		if ( owners[0] == win || owners[1] == win )
		    doIt = FALSE;
	    }

	    XFree( owners );
	}
    }

    if (doIt) {
	if ( waiting_for_data ) {
	    pending_selection_changed = TRUE;
	    if ( ! pending_timer_id )
		pending_timer_id = QApplication::clipboard()->startTimer( 0 );
	    doIt = FALSE;
	} else {
	    selectionData()->clear();
	}
    }

    return doIt;
}


bool qt_check_clipboard_sentinel()
{
    bool doIt = TRUE;
    if (owner) {
	Window *owners;
	Atom actualType;
	int actualFormat;
	unsigned long nitems, bytesLeft;

	if (XGetWindowProperty(QPaintDevice::x11AppDisplay(),
			       QApplication::desktop()->screen(0)->winId(),
			       qt_clipboard_sentinel, 0, 2, False, XA_WINDOW,
			       &actualType, &actualFormat, &nitems, &bytesLeft,
			       (unsigned char **) &owners) == Success) {
	    if (actualType == XA_WINDOW && actualFormat == 32 && nitems == 2) {
		Window win = owner->winId();
		if (owners[0] == win || owners[1] == win)
		    doIt = FALSE;
	    }

	    XFree(owners);
	}
    }

    if (doIt) {
	if ( waiting_for_data ) {
	    pending_clipboard_changed = TRUE;
	    if ( ! pending_timer_id )
		pending_timer_id = QApplication::clipboard()->startTimer( 0 );
	    doIt = FALSE;
	} else {
	    clipboardData()->clear();
	}
    }

    return doIt;
}

#endif // QT_NO_CLIPBOARD
