/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice_x11.cpp#7 $
**
** Implementation of QPaintDevice class for X11
**
** Author  : Haavard Nord
** Created : 940721
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpaintd.h"
#include "qwidget.h"
#include "qpixmap.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpaintdevice_x11.cpp#7 $";
#endif


QPaintDevice::QPaintDevice()
{
    devFlags = PDT_UNDEF;
    dpy = qXDisplay();
    hd = 0;
}

QPaintDevice::~QPaintDevice()
{
}


void QPaintDevice::bitBlt( int sx, int sy, int sw, int sh, QPaintDevice *dest,
			   int dx, int dy, RasterOp rop )
{
    int ts = devType();				// from device type
    int td = dest->devType();			// to device type

/*
    NOTE!!!  PS printer pixmap output not yet supported...
    if ( td == PDT_PRINTER && paintingActive() )
    	cmd( PDC_DRAWPIXMAP, ... );   // or something similar...
*/

    if ( !(ts <= PDT_PIXMAP && td <= PDT_PIXMAP) ) {
#if defined(CHECK_STATE)
	warning( "QPaintDevice::bitBlt: Cannot bitBlt to or from device" );
#endif
	return;
    }
    static short ropCodes[] =			// ROP translation table
	{ GXcopy, GXor, GXxor, GXandInverted,
	  GXcopyInverted, GXorInverted, GXequiv, GXand, GXinvert };
    if ( !(rop >= CopyROP && rop <= NotROP) ) {
#if defined(CHECK_RANGE)
	warning( "QPaintDevice::bitBlt: Invalid ROP code" );
#endif
	return;
    }
    GC gc;
    bool new_gc = FALSE;
    bool copy_plane = FALSE;
    if ( td == PDT_WIDGET )			// borrow dest widget's gc
	gc = ((QWidget*)dest)->getGC();
    else if ( ts == PDT_WIDGET )		// borrow source widget's gc
	gc = ((QWidget*)this)->getGC();
    else {					// create dedicated GC
	gc = XCreateGC( dpy, hd, 0, 0 );
	new_gc = TRUE;
    }
    if ( rop != CopyROP )
	XSetFunction( dpy, gc, ropCodes[rop] );
    if ( ts == PDT_PIXMAP )
	copy_plane = ((QPixMap*)this)->planes() == 1;
    if ( td == PDT_PIXMAP ) {
	bool singleplane = ((QPixMap*)dest)->planes() == 1;
	if ( singleplane && !copy_plane ) {	
#if defined(CHECK_RANGE)
		warning( "QPaintDevice::bitBlt: Incompatible destination pixmap" );
#endif
		return;			// dest is 1-bit pixmap, source is not
	}
	copy_plane ^= singleplane;
    }
    if ( copy_plane )
	XCopyPlane( dpy, hd, dest->hd, gc, sx, sy, sw, sh, dx, dy, 1 );
    else
	XCopyArea( dpy, hd, dest->hd, gc, sx, sy, sw, sh, dx, dy );
    if ( new_gc )
	XFreeGC( dpy, gc );
    else if ( rop != CopyROP )			// restore ROP for borrowed gc
	XSetFunction( dpy, gc, ropCodes[CopyROP] );
}


bool QPaintDevice::cmd( int, QPDevCmdParam * )
{
#if defined(CHECK_STATE)
    warning( "QPaintDevice::cmd: Device has no command interface" );
#endif
    return FALSE;
}

long QPaintDevice::metric( int ) const
{
#if defined(CHECK_STATE)
    warning( "QPaintDevice::metrics: Device has no metric information" );
#endif
}
