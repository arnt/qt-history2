/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptd_x11.cpp#16 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qptd_x11.cpp#16 $";
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


void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx, int sy, int sw, int sh,
	     RasterOp rop )
{
    int ts = src->devType();			// from device type
    int td = dst->devType();			// to device type
    Display *dpy = src->display();

    if ( sw <= 0 )				// use device width
	sw = src->metric( PDM_WIDTH );
    if ( sh <= 0 )				// use device height
	sh = src->metric( PDM_HEIGHT );

    if ( src->paintingActive() && dst->isExtDev() ) {
	QPixMap *pm;				// output to picture/printer
	if ( ts == PDT_PIXMAP )
	    pm = (QPixMap*)src;
	else if ( ts == PDT_WIDGET ) {		// bitBlt to temp pixmap
	    pm = new QPixMap( sw, sh );
	    CHECK_PTR( pm );
	    bitBlt( pm, 0, 0, src, sx, sy, sw, sh, CopyROP );
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
	dst->cmd( PDC_DRAWPIXMAP, param );
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
    if ( rop > NotROP ) {
#if defined(CHECK_RANGE)
	warning( "QPaintDevice::bitBlt: Invalid ROP code" );
#endif
	return;
    }

    bool copy_plane = FALSE;
    bool mono = FALSE;

    if ( ts == PDT_PIXMAP )
	copy_plane = ((QPixMap*)src)->depth() == 1;
    if ( td == PDT_PIXMAP ) {
	bool single_plane = ((QPixMap*)dst)->depth() == 1;
	if ( single_plane && !copy_plane ) {	
#if defined(CHECK_RANGE)
		warning( "QPaintDevice::bitBlt: Incompatible destination pixmap" );
#endif
		return;			// dest is 1-bit pixmap, source is not
	}
	mono = copy_plane && single_plane;
	copy_plane ^= single_plane;
    }
    GC        gc = qXGetTempGC( mono );
    XGCValues gcvals;
    ulong     gcflags = GCBackground | GCForeground;

    if ( rop != CopyROP ) {			// use non-default ROP code
	gcflags |= GCFunction;
	gcvals.function = ropCodes[rop];
    }
    if ( td == PDT_WIDGET ) {			// set GC colors
	QWidget *w = (QWidget *)dst;
	gcvals.background = w->backgroundColor().pixel();
	gcvals.foreground = w->foregroundColor().pixel();
    }
    else {
	gcvals.background = white.pixel();
	gcvals.foreground = black.pixel();
    }
    XChangeGC( dpy, gc, gcflags, &gcvals );

    if ( copy_plane )
	XCopyPlane( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		    dx, dy, 1 );
    else
	XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		   dx, dy );
    if ( rop != CopyROP )			// reset gc function
	XSetFunction( dpy, gc, GXcopy );
}
