/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor_x11.cpp#2 $
**
** Implementation of QCursor class for X11
**
** Author  : Haavard Nord
** Created : 940219
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcursor.h"
#include "qapp.h"
#include "qbitmap.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcursor_x11.cpp#2 $";
#endif


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

static QCursor *cursorTable[] = {		// order is important!!
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

void QCursor::initialize()			// initialize all cursors
{
    int shape = ArrowCursor;
    while ( cursorTable[shape] ) {
	cursorTable[shape]->cshape = shape;
	shape++;
    }
}

void QCursor::cleanup()
{
    Display *dpy = qXDisplay();
    int shape = ArrowCursor;
    register QCursor *c;
    while ( c=cursorTable[shape] ) {
	if ( c->cursor )			// cursor was used
	    XFreeCursor( dpy, c->cursor );
	if ( c->pm && c->pmm ) {		// cursor has pixmap
	    XFreePixmap( dpy, c->pm );
	    XFreePixmap( dpy, c->pmm );
	}
	shape++;
    }
}


QCursor *QCursor::locate( int shape )		// get global cursor
{
    return shape >= ArrowCursor && shape <= SizeAllCursor ?
	   cursorTable[shape] : 0;
}


QCursor::QCursor()				// default arrow cursor
{
    cursor = 0;
    cshape = 0;
    if ( qApp )					// when not initializing
	setShape( ArrowCursor );
}

QCursor::QCursor( int shape )			// cursor with shape
{
    cursor = 0;
    cshape = 0;
    setShape( shape );
}

QCursor::QCursor( QBitMap *bitmap, QBitMap *mask, int hotX, int hotY )
{						// define own cursor
    bm = bitmap;
    bmm = mask;
    cursor = 0;
    cshape = BitMapCursor;
    hx = hotX >= 0 ? hotX : bitmap->size().width()/2;
    hy = hotY >= 0 ? hotY : bitmap->size().height()/2;
}

QCursor::~QCursor()
{
    if ( cshape == BitMapCursor ) {		// free bitmap cursor
	delete bm;
	delete bmm;
    }
}


bool QCursor::setShape( int shape )		// set cursor shape
{
    if ( cshape == BitMapCursor ) {		// free bitmap cursor
	delete bm;
	delete bmm;
    }
    cshape = shape;
    QCursor *c = locate( shape );		// find one of the global ones
    if ( c ) {
	c->update();
	cursor = c->cursor;			// copy attributes
	pm  = c->pm;
	pmm = c->pmm;
    }
    return c != 0;
}


QPoint QCursor::pos()				// get cursor position
{
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint buttons;
    XQueryPointer( qXDisplay(), qXRootWin(), &root, &child,
		   &root_x, &root_y, &win_x, &win_y, &buttons );
    return QPoint( root_x, root_y );
}

void QCursor::setPos( int x, int y )		// set cursor position
{
    XWarpPointer( qXDisplay(), None, qXRootWin(), 0, 0, 0, 0, x, y );
}


void QCursor::update()				// update/load cursor
{
    if ( cursor )				// already loaded
	return;

  // Non-standard X11 cursors are created from bitmaps

    static char cur_ver_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0xc0, 0x03, 0xe0, 0x07, 0xf0, 0x0f,
	0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0xf0, 0x0f,
	0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00 };
    static char mcur_ver_bits[] = {
	0x00, 0x00, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x3f,
	0xfc, 0x7f, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xfc, 0x7f, 0xf8, 0x3f,
	0xf0, 0x1f, 0xe0, 0x0f, 0xc0, 0x07, 0x80, 0x03 };
    static char cur_hor_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x08, 0x30, 0x18,
	0x38, 0x38, 0xfc, 0x7f, 0xfc, 0x7f, 0x38, 0x38, 0x30, 0x18, 0x20, 0x08,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static char mcur_hor_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0x60, 0x0c, 0x70, 0x1c, 0x78, 0x3c,
	0xfc, 0x7f, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfc, 0x7f, 0x78, 0x3c,
	0x70, 0x1c, 0x60, 0x0c, 0x40, 0x04, 0x00, 0x00 };
    static char cur_bdiag_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x3e,
	0x00, 0x37, 0x88, 0x23, 0xd8, 0x01, 0xf8, 0x00, 0x78, 0x00, 0xf8, 0x00,
	0xf8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static char mcur_bdiag_bits[] = {
	0x00, 0x00, 0xc0, 0x7f, 0x80, 0x7f, 0x00, 0x7f, 0x00, 0x7e, 0x04, 0x7f,
	0x8c, 0x7f, 0xdc, 0x77, 0xfc, 0x63, 0xfc, 0x41, 0xfc, 0x00, 0xfc, 0x01,
	0xfc, 0x03, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00 };
    static char cur_fdiag_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x01, 0xf8, 0x00, 0x78, 0x00,
	0xf8, 0x00, 0xd8, 0x01, 0x88, 0x23, 0x00, 0x37, 0x00, 0x3e, 0x00, 0x3c,
	0x00, 0x3e, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00 };
    static char mcur_fdiag_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xfc, 0x03, 0xfc, 0x01, 0xfc, 0x00,
	0xfc, 0x41, 0xfc, 0x63, 0xdc, 0x77, 0x8c, 0x7f, 0x04, 0x7f, 0x00, 0x7e,
	0x00, 0x7f, 0x80, 0x7f, 0xc0, 0x7f, 0x00, 0x00 };

    static char *cursor_bits[] = {
	cur_ver_bits, mcur_ver_bits, cur_hor_bits, mcur_hor_bits,
	cur_bdiag_bits, mcur_bdiag_bits, cur_fdiag_bits, mcur_fdiag_bits };

    Display *dpy = qXDisplay();
    uint sh;
    if ( cshape == BitMapCursor ) {
	XColor bg, fg;				// ignore stupid CFront message
	bg.red = bg.green = bg.blue = 255 << 8;
	fg.red = fg.green = fg.blue = 0;
	cursor = XCreatePixmapCursor( dpy, bm->hd, bmm->hd, &fg, &bg, hx, hy );
	return;
    }
    if ( cshape >= SizeVerCursor && cshape < SizeAllCursor ) {
	XColor bg, fg;				// ignore stupid CFront message
	bg.red = bg.green = bg.blue = 255 << 8;
	fg.red = fg.green = fg.blue = 0;
	int i = (cshape - SizeVerCursor)*2;
	Window rootwin = qXRootWin();
	pm  = XCreateBitmapFromData( dpy, rootwin, cursor_bits[i], 16, 16 );
	pmm = XCreateBitmapFromData( dpy, rootwin, cursor_bits[i+1], 16, 16 );
	cursor = XCreatePixmapCursor( dpy, pm, pmm, &fg, &bg, 8, 8 );
	return;
    }
    switch ( cshape ) {
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
	    warning( "QCursor::update: Invalid cursor shape %d", cshape );
#endif
	    return;
    }
    cursor = XCreateFontCursor( qXDisplay(), sh );
}
