/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcur_x11.cpp#19 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcur_x11.cpp#19 $";
#endif


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
    hx = hy  = 0;
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

static QCursor *find_cur( int shape )		// find predefined cursor
{
    return (uint)shape <= SizeAllCursor ? cursorTable[shape] : 0;
}


/*!
  Internal function that initializes the predefined cursors.
*/

void QCursor::initialize()
{
    int shape = ArrowCursor;
    while ( cursorTable[shape] ) {
	cursorTable[shape]->data->cshape = shape;
	shape++;
    }
}

/*!
  Internal function that cleans up the predefined cursors.
*/

void QCursor::cleanup()
{
    int shape = ArrowCursor;
#if defined(CHECK_MEMORY)
    bool mc = memchkSetReporting( FALSE );	// get rid of stupid messages
#endif
    while ( cursorTable[shape] ) {
	delete cursorTable[shape]->data;
	cursorTable[shape]->data = 0;
	shape++;
    }
#if defined(CHECK_MEMORY)
    memchkSetReporting( mc );
#endif
}


/*!
  Constructs a cursor with the default arrow shape.
*/

QCursor::QCursor()
{
    if ( QApplication::startingUp() ) {		// this is a global cursor
	data = new QCursorData;
	CHECK_PTR( data );
	data->cshape = 0;
	data->hcurs = 0;
    }
    else {					// default arrow cursor
	data = arrowCursor.data;
	data->ref();
    }
}

/*!
  Constructs a cursor with the specified \e shape.
*/

QCursor::QCursor( int shape )			// cursor with shape
{
    QCursor *c = find_cur( shape );
    if ( !c )					// not found
	c = (QCursor *)&arrowCursor;		//   then use arrowCursor
    c->data->ref();
    data = c->data;
}


/*!
  Constructs a custom bitmap cursor.

  \arg \e bitmap and
  \arg \e mask make up the bitmap.
  \arg \e hotX and
  \arg \e hotY define the hot spot of this cursor.
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
  Destroys the cursor.
*/

QCursor::~QCursor()
{
    if ( data && data->deref() )
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
  Returns the cursor shape identifer.
  \sa setShape()
*/

int QCursor::shape() const			// get cursor shape
{
    return data->cshape;
}

/*!
  Sets the cursor shape identifier to \e shape.
  We recommend using one of the <a href=#cursors>predefined cursors</a>
  instead.
  \sa shape()
*/

void QCursor::setShape( int shape )		// set cursor shape
{
    QCursor *c = find_cur( shape );		// find one of the global ones
    if ( !c )					// not found
	c = (QCursor *)&arrowCursor;		//   then use arrowCursor
    c->data->ref();
    if ( data->deref() )			// make shallow copy
	delete data;
    data = c->data;
}


/*!
  Returns the cursor bitmap, or 0 if it is one of the standard cursors.
*/

const QBitmap *QCursor::bitmap() const
{
    return data->bm;
}

/*!
  Returns the cursor bitmap mask, or 0 if it is one of the standard cursors.
*/

const QBitmap *QCursor::mask() const
{
    return data->bmm;
}

/*!
  Returns the cursor hot spot, or (0,0) if it is one of the standard cursors.
*/

QPoint QCursor::hotSpot() const
{
    return QPoint( data->hx, data->hy );
}


/*!
  Returns the window system cursor handle.

  \warning 
  Portable in principle, but if you use it you are probably about to do
  something non-portable. Be careful.
*/

HANDLE QCursor::handle() const
{
    if ( !data->hcurs )
	update();
    return data->hcurs;
}


/*!
  Returns the position of the cursor (hot spot) in global screen
  coordinates.

  You can call QWidget::mapFromGlobal() to translate it to widget
  coordinates.
  \sa setPos(), QWidget::mapFromGlobal(), QWidget::mapToGlobal()
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
  Warps the cursor (hot spot) to the global screen position \e (x,y).

  You can call QWidget::mapToGlobal() to translate widget coordinates
  to global screen coordinates.
  \sa pos(), QWidget::mapFromGlobal(), QWidget::mapToGlobal()
*/

void QCursor::setPos( int x, int y )		// set cursor position
{
    XWarpPointer( qt_xdisplay(), None, qt_xrootwin(), 0, 0, 0, 0, x, y );
}


/*!
  \internal Creates the cursor.
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
