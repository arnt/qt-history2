/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcur_x11.cpp#16 $
**
** Implementation of QCursor class for X11
**
** Author  : Haavard Nord
** Created : 940219
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcursor.h"
#include "qapp.h"
#include "qbitmap.h"
#include "qdstream.h"
#include "qmemchk.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcur_x11.cpp#16 $";
#endif


/*!
\class QCursor qcursor.h
\brief This class provides a mouse cursor with arbitrary shape and color.

The QCursor class represents the window system's mouse curor.  There
are a number of <a name=cursors>predefined cursor shapes</a>:
<dl compact>
<dt> arrowCursor <dd> standard arrow cursor
<dt> upArrowCursor <dd> upwards arrow
<dt> crossCursor <dd> crosshair
<dt> hourGlassCursor <dd> hourglass/watch
<dt> ibeamCursor <dd> ibeam/text entry
<dt> sizeVerCursor <dd> vertical resize
<dt> sizeHorCursor <dd> horizontal resize
<dt> sizeBDiagCursor <dd> diagonal resize (/)
<dt> sizeFDiagCursor <dd> diagonal resize (\)
<dt> sizeAllCursor <dd> all directions resize
</dl>

It is also possible to define custom shapes.

Members are provided to \link setShape() change shape \endlink,
\link pos() get the cursor position \endlink and so on.
*/


// --------------------------------------------------------------------------
// Global cursors
//

const QCursor arrowCursor;
const QCursor upArrowCursor;
const QCursor crossCursor;
const QCursor hourGlassCursor;
const QCursor ibeamCursor;
const QCursor sizeVerCursor;
const QCursor sizeHorCursor;
const QCursor sizeBDiagCursor;
const QCursor sizeFDiagCursor;
const QCursor sizeAllCursor;


// --------------------------------------------------------------------------
// Internal QCursorData class
//

struct QCursorData : QShared {			// internal cursor data
    QCursorData();
   ~QCursorData();
    int	      cshape;
    QBitmap  *bm, *bmm;
    short     hx, hy;
    Cursor    hcurs;
    Pixmap    pm, pmm;
};

QCursorData::QCursorData()
{
    bm = bmm = 0;
    pm = pmm = 0;
}

QCursorData::~QCursorData()
{
    Display *dpy = qt_xdisplay();
    if ( hcurs )
	XFreeCursor( dpy, hcurs );
    if ( pm )
	XFreePixmap( dpy, pm );
    if ( pmm )
	XFreePixmap( dpy, pmm );
    if ( bm )
	delete bm;
    if ( bmm )
	delete bmm;
}


// --------------------------------------------------------------------------
// QCursor member functions
//

static QCursor *cursorTable[] = {		// the order is important!!
    (QCursor*)&arrowCursor,
    (QCursor*)&upArrowCursor,
    (QCursor*)&crossCursor,
    (QCursor*)&hourGlassCursor,
    (QCursor*)&ibeamCursor,
    (QCursor*)&sizeVerCursor,
    (QCursor*)&sizeHorCursor,
    (QCursor*)&sizeBDiagCursor,
    (QCursor*)&sizeFDiagCursor,
    (QCursor*)&sizeAllCursor,
    0
};

void QCursor::initialize()			// initialize standard cursors
{
    int shape = ArrowCursor;
    while ( cursorTable[shape] ) {
	cursorTable[shape]->data->cshape = shape;
	shape++;
    }
}

void QCursor::cleanup()				// cleanup standard cursors
{
    int shape = ArrowCursor;
#if defined(DEBUG)
    bool mc = memchkSetReporting( FALSE );	// get rid of stupid messages
#endif
    while ( cursorTable[shape] ) {
	delete cursorTable[shape]->data;
	cursorTable[shape]->data = 0;
	shape++;
    }
#if defined(DEBUG)
    memchkSetReporting( mc );
#endif
}


QCursor *QCursor::locate( int shape )		// get global cursor
{
    return (uint)shape <= SizeAllCursor ? cursorTable[shape] : 0;
}


/*!
Constructs a cursor with the default arrow shape.
*/

QCursor::QCursor()				// default arrow cursor
{
    if ( qApp ) {				// not initializing
	data = arrowCursor.data;		//   then make shallow copy
	data->ref();
    }
    else {					// this is a standard cursor
	data = new QCursorData;
	CHECK_PTR( data );
	data->hcurs = 0;
	data->cshape = 0;
    }
}

/*!
Constructs a cursor with the specified \e shape.
*/

QCursor::QCursor( int shape )			// cursor with shape
{
    QCursor *c = locate( shape );
    if ( !c )					// not found
	c = (QCursor *)&arrowCursor;		//   then use arrowCursor
    c->data->ref();
    data = c->data;
}


/*!
Constructs a custom bitmap cursor.

\arg \e bitmap and
\arg \e mask make up the bitmap
\arg \e hotX and
\arg \e hotY define the hot spot of this cursor
*/

QCursor::QCursor( const QBitmap &bitmap, const QBitmap &mask,
		  int hotX, int hotY )
{						// define own cursor
    data = new QCursorData;
    CHECK_PTR( data );
    data->bm  = new QBitmap( bitmap );
    data->bmm = new QBitmap( mask );
    data->hcurs = 0;
    data->cshape = BitmapCursor;
    data->hx = hotX >= 0 ? hotX : bitmap.width()/2;
    data->hy = hotY >= 0 ? hotY : bitmap.height()/2;
}

/*!
Constructs a copy of the cursor \e c.
*/

QCursor::QCursor( const QCursor &c )
{
    data = c.data;				// shallow copy
    data->ref();
}

/*!
Dereferences the cursor and destroys the cursor data if it was the last
reference.
*/

QCursor::~QCursor()
{
    if ( data && data->deref() )		// data == 0 for std cursors
	delete data;
}

/*!
Assigns \e c to this cursor and returns a reference to the cursor.
*/

QCursor &QCursor::operator=( const QCursor &c )
{
    c.data->ref();				// avoid c = c
    if ( data->deref() )
	delete data;
    data = c.data;
    return *this;
}

/*!
Returns a deep copy of the cursor.

\todo Copy bitmap data
*/

QCursor QCursor::copy() const
{
    QCursor c( data->cshape );
    return c;
}

/*!
This function returns the cursor shape (one of the
<a href=#cursors>global cursors</a> or one of the custom cursors.
*/

int QCursor::shape() const			// get cursor shape
{
    return data->cshape;
}

/*!
This function sets the cursor shape.  If the argument isn't a
valid shape number, an arrow cursor will be substituted.
*/

bool QCursor::setShape( int shape )		// set cursor shape
{
    QCursor *c = locate( shape );		// find one of the global ones
    bool res = c != 0;
    if ( !res )					// not found
	c = (QCursor *)&arrowCursor;		//   then use arrowCursor
    c->data->ref();
    if ( data->deref() )			// make shallow copy
	delete data;
    data = c->data;
    return res;
}


/*!
Returns the position of the cursor on the screen in the global
coordinate system.

You can use QWidget::mapFromGlobal() to translate it to widget
coordinates.

The return value is the coordinate of the hot spot.

\sa setPos(), QWidget::mapFromGlobal() and QWidget::mapToGlobal().
*/

QPoint QCursor::pos()				// get cursor position
{
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint buttons;
    XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
		   &root_x, &root_y, &win_x, &win_y, &buttons );
    return QPoint( root_x, root_y );
}

/*!
Warps the cursor to the global screen position \e (x,y).

You can use QWidget::mapToGlobal() to translate widget coordinates
to global screen coordinates.

\sa pos().
*/

void QCursor::setPos( int x, int y )		// set cursor position
{
    XWarpPointer( qt_xdisplay(), None, qt_xrootwin(), 0, 0, 0, 0, x, y );
}


/*!
\internal Creates the cursor internals.
*/

void QCursor::update() const			// update/load cursor
{
    register QCursorData *d = data;		// cheat const!
    if ( d->hcurs )				// already loaded
	return;

  // Non-standard X11 cursors are created from bitmaps

    static uchar cur_ver_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0xc0, 0x03, 0xe0, 0x07, 0xf0, 0x0f,
	0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0xf0, 0x0f,
	0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00 };
    static uchar mcur_ver_bits[] = {
	0x00, 0x00, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x3f,
	0xfc, 0x7f, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xfc, 0x7f, 0xf8, 0x3f,
	0xf0, 0x1f, 0xe0, 0x0f, 0xc0, 0x07, 0x80, 0x03 };
    static uchar cur_hor_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x08, 0x30, 0x18,
	0x38, 0x38, 0xfc, 0x7f, 0xfc, 0x7f, 0x38, 0x38, 0x30, 0x18, 0x20, 0x08,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static uchar mcur_hor_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0x60, 0x0c, 0x70, 0x1c, 0x78, 0x3c,
	0xfc, 0x7f, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfc, 0x7f, 0x78, 0x3c,
	0x70, 0x1c, 0x60, 0x0c, 0x40, 0x04, 0x00, 0x00 };
    static uchar cur_bdiag_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x3e,
	0x00, 0x37, 0x88, 0x23, 0xd8, 0x01, 0xf8, 0x00, 0x78, 0x00, 0xf8, 0x00,
	0xf8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static uchar mcur_bdiag_bits[] = {
	0x00, 0x00, 0xc0, 0x7f, 0x80, 0x7f, 0x00, 0x7f, 0x00, 0x7e, 0x04, 0x7f,
	0x8c, 0x7f, 0xdc, 0x77, 0xfc, 0x63, 0xfc, 0x41, 0xfc, 0x00, 0xfc, 0x01,
	0xfc, 0x03, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00 };
    static uchar cur_fdiag_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x01, 0xf8, 0x00, 0x78, 0x00,
	0xf8, 0x00, 0xd8, 0x01, 0x88, 0x23, 0x00, 0x37, 0x00, 0x3e, 0x00, 0x3c,
	0x00, 0x3e, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00 };
    static uchar mcur_fdiag_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xfc, 0x03, 0xfc, 0x01, 0xfc, 0x00,
	0xfc, 0x41, 0xfc, 0x63, 0xdc, 0x77, 0x8c, 0x7f, 0x04, 0x7f, 0x00, 0x7e,
	0x00, 0x7f, 0x80, 0x7f, 0xc0, 0x7f, 0x00, 0x00 };

    static uchar *cursor_bits[] = {
	cur_ver_bits, mcur_ver_bits, cur_hor_bits, mcur_hor_bits,
	cur_bdiag_bits, mcur_bdiag_bits, cur_fdiag_bits, mcur_fdiag_bits };

    Display *dpy = qt_xdisplay();
    if ( d->cshape == BitmapCursor ) {
	XColor bg, fg;				// ignore stupid CFront message
	bg.red = bg.green = bg.blue = 255 << 8;
	fg.red = fg.green = fg.blue = 0;
	d->hcurs = XCreatePixmapCursor( dpy, d->bm->handle(), d->bmm->handle(),
					&fg, &bg, d->hx, d->hy );
	return;
    }
    if ( d->cshape >= SizeVerCursor && d->cshape < SizeAllCursor ) {
	XColor bg, fg;				// ignore stupid CFront message
	bg.red = bg.green = bg.blue = 255 << 8;
	fg.red = fg.green = fg.blue = 0;
	int i = (d->cshape - SizeVerCursor)*2;
	Window rootwin = qt_xrootwin();
	d->pm  = XCreateBitmapFromData( dpy, rootwin, (char *)cursor_bits[i],
					16, 16 );
	d->pmm = XCreateBitmapFromData( dpy, rootwin, (char *)cursor_bits[i+1],
					16,16);
	d->hcurs = XCreatePixmapCursor( dpy, d->pm, d->pmm, &fg, &bg, 8, 8 );
	return;
    }
    uint sh;
    switch ( d->cshape ) {			// map Q cursor to X cursor
	case ArrowCursor:
	    sh = XC_left_ptr;
	    break;
	case UpArrowCursor:
	    sh = XC_center_ptr;
	    break;
	case CrossCursor:
	    sh = XC_crosshair;
	    break;
	case HourGlassCursor:
	    sh = XC_watch;
	    break;
	case IbeamCursor:
	    sh = XC_xterm;
	    break;
	case SizeAllCursor:
	    sh = XC_fleur;
	    break;
	default:
#if defined(CHECK_RANGE)
	    warning( "QCursor::update: Invalid cursor shape %d", d->cshape );
#endif
	    return;
    }
    d->hcurs = XCreateFontCursor( dpy, sh );
}


/*!
\warning System dependent!  Do not use this unless you have to.

This function returns the window system's handle to the current cursor.
Rather than use it, you should <a href=mailto:qt-bugs@troll.no>tell
us</a> what the portable functions lack.

The return type for each window system is specified in qcursor.h.
*/

Cursor QCursor::handle() const
{
    if ( !data->hcurs )
	update();
    return data->hcurs;
}


// --------------------------------------------------------------------------
// QCursor stream functions
//

/*!
\relates QCursor
Writes the cursor to the stream.
*/

QDataStream &operator<<( QDataStream &s, const QCursor &c )
{
    s << (INT16)c.data->cshape;			// write shape id to stream
    if ( c.data->cshape == BitmapCursor ) {
	s << *(c.data->bm) << *(c.data->bmm);
	s << (INT16)c.data->hx << (INT16)c.data->hy;
    }
    return s;
}

/*!
\relates QCursor
Reads the cursor from the stream.
*/

QDataStream &operator>>( QDataStream &s, QCursor &c )
{
    INT16 shape;
    s >> shape;					// read shape id from stream
    if ( shape == BitmapCursor ) {		// read bitmap cursor
	if ( c.data->deref() )
	    delete c.data;
	c.data = new QCursorData;
	CHECK_PTR( c.data );
	QBitmap bm(1,1), bmm(1,1); // ###
	INT16   hx, hy;
	s >> bm >> bmm;
	s >> hx >> hy;
	CHECK_PTR( c.data->bm && c.data->bmm );
	c.data->bm  = new QBitmap( bm );
	c.data->bmm = new QBitmap( bmm );
	c.data->hcurs  = 0;
	c.data->cshape = BitmapCursor;
	c.data->hx = hx;
	c.data->hy = hy;
	c.update();
    }
    else
	c.setShape( (int)shape );		// create cursor with shape
    return s;
}
