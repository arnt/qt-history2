/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptd_x11.cpp#12 $
**
** Implementation of QPaintDevice class for X11
**
** Author  : Haavard Nord
** Created : 940721
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpaintd.h"
#include "qpaintdc.h"
#include "qwidget.h"
#include "qpixmap.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qptd_x11.cpp#12 $";
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

    if ( paintingActive() && (dest->devFlags & PDF_EXTDEV) ) {
	QPixMap *pm;				// output to picture/printer
	QWidget *w;
	if ( ts == PDT_PIXMAP )
	    pm = (QPixMap*)this;
	else if ( ts == PDT_WIDGET ) {
	    w = (QWidget*)this;			// bitBlt to temp pixmap
	    pm = new QPixMap( w->clientWidth(), w->clientHeight() );
	    CHECK_PTR( pm );
	    w->bitBlt( 0, 0, w->clientWidth(), w->clientHeight(), pm, 0, 0,
		       CopyROP );
	}
	else {
#if defined(CHECK_RANGE)
	    warning( "QPaintDevice::bitBlt: Cannot bitBlt from device" );
#endif
	    return;
	}
	QPDevCmdParam param[3];
	QRect  r(sx,sy,sw,sh);
	QPoint p(dx,dy);
	param[0].rect   = &r;
	param[1].point  = &p;
	param[2].pixmap = pm;
	cmd( PDC_DRAWPIXMAP, param );
	if ( ts == PDT_WIDGET )
	    delete pm;
	return;
    }

    if ( !(ts <= PDT_PIXMAP && td <= PDT_PIXMAP) ) {
#if defined(CHECK_RANGE)
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
    GC        gc = qXGetTempGC();
    XGCValues gcvals;
    ulong     gcflags = 0;
    bool      copy_plane  = FALSE;

    if ( rop != CopyROP ) {			// use non-default ROP code
	gcflags |= GCFunction;
	gcvals.function = ropCodes[rop];
    }
    if ( td == PDT_WIDGET ) {			// set GC colors
	QWidget *w = (QWidget *)dest;
	gcflags |= GCBackground | GCForeground;
	gcvals.background = w->backgroundColor().pixel();
	gcvals.foreground = w->foregroundColor().pixel();
    }
    if ( gcflags != 0 )
	XChangeGC( dpy, gc, gcflags, &gcvals );

    if ( ts == PDT_PIXMAP )
	copy_plane = ((QPixMap*)this)->depth() == 1;
    if ( td == PDT_PIXMAP ) {
	bool singleplane = ((QPixMap*)dest)->depth() == 1;
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
    if ( gcflags & GCFunction )			// reset gc function
	XSetFunction( dpy, gc, GXcopy );
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
    return 0;
}
