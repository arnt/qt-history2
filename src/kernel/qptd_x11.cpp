/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptd_x11.cpp#26 $
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
#include "qapp.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qptd_x11.cpp#26 $";
#endif


/*!
\class QPaintDevice qpaintd.h
\brief The QPaintDevice is the base class of objects that can be painted.

A paint device is an abstraction of a two-dimensional space that can be
drawn into using a painter (see QPainter).
The drawing capabilities are implemented by the subclasses: QWidget,
QPixmap, QPicture and QPrinter.

The default coordinate system of a paint device has its origin located
at the top left position. X increases to the left and Y increases to the
bottom. The unit is one pixel.
A user-defined coordinate system can be specified to the painter (see
Q2DMatrix).

Here is an example how to draw on a paint device:
\code
  QPainter p;				\/ our painter
  QWidget  w;				\/ out paint device
  p.begin( &w );			\/ start painting
  p.setPen( red );			\/ blue outline
  p.setBrush( yellow );			\/ yellow fill
  p.drawEllipse( 10,20, 100,100 );	\/ 100x100 ellipse at 10,20
  p.end();				\/ painting done
\endcode

The bit block transfer is an extremely useful operation for copying pixels
from one paint device to another (or to itself).
It is implemented as the global function bitBlt().

This code demonstrates how to scroll the contents of a widget 10 pixels
to the right:
\code
    QWidget  w;
    bitBlt( &w, 10, 0, &w, 0, 0, -1, -1 );
\endcode

\warning Qt requires that a QApplication object must exist before any paint
devices can be created.  Paint devices access window system resources, and 
these resources are not initialized before an application object is created.
*/


/*!
Constructs a paint device with internal flags \e devflags.
This constructor can only be invoked from subclasses of QPaintDevice.
*/

QPaintDevice::QPaintDevice( uint devflags )
{
    if ( !qApp ) {				// global constructor
#if defined(CHECK_STATE)
	fatal( "QPaintDevice: Global paint device objects are not allowed" );
#endif
	return;
    }    
    devFlags = devflags;
    dpy = qt_xdisplay();
    hd  = 0;
}

/*!
Destroys the paint device and frees window system resources.
*/

QPaintDevice::~QPaintDevice()
{
#if defined(CHECK_STATE)
    if ( !qt_xdisplay() )			// this is a static object
	warning( "QPaintDevice: Static (local) paint device objects do not "
		 "release window system resources" );
#endif
}


/*!
\fn int QPaintDevice::devType() const
Returns the device type identifier: \c PDT_WIDGET, \c PDT_PIXMAP,
\c PDT_PRINTER or \c PDT_PICTURE.
*/

/*!
\fn bool QPaintDevice::isExtDev() const
Returns TRUE if the device is a so-called external paint device.

External paint devices cannot be bitBlt'ed from.
*/

/*!
\fn HDC QPaintDevice::handle() const
Returns the window system handle of the paint device (Windows only).
*/

/*!
\fn HPS QPaintDevice::handle() const
Returns the window system handle of the paint device (OS/2 PM only).
*/

/*!
\fn WId QPaintDevice::handle() const
Returns the window system handle of the paint device (X-Windows only).
*/

/*!
\fn Display *display() const
Returns a pointer to the X display (X-Windows only).
*/


/*!
Internal virtual function that interprets drawing commands from the painter.

Implemented by subclasses that have no direct support for drawing graphics
(for instance QPicture).
*/

bool QPaintDevice::cmd( int, QPDevCmdParam * )
{
#if defined(CHECK_STATE)
    warning( "QPaintDevice::cmd: Device has no command interface" );
#endif
    return FALSE;
}

/*!
Internal virtual function that returns paint device metrics.

Implemented by all subclasses.
*/

long QPaintDevice::metric( int ) const
{
#if defined(CHECK_STATE)
    warning( "QPaintDevice::metrics: Device has no metric information" );
#endif
    return 0;
}


/*!
\relates QPaintDevice
This function copies a block of pixels from one paint device to another
(bitBlt means bit block transfer).

\arg \e dst is the paint device to copy to.
\arg \e dx and \e dy is the position to copy to.
\arg \e src is the paint device to copy from.
\arg \e sx and \e sy is the position to copy from.
\arg \e sw and \e sh is the width and height of the block to be copied.
\arg \e rop defines the raster operation to be used when copying.

If \e sw is 0 or \e sh is 0, then bitBlt will do nothing.<br>

If \e sw is negative, then bitBlt calculates
<code>sw = src->width - sx.</code><br>
If \e sh is negative, then bitBlt calculates
<code>sh = src->height - sy.</code><br>

The \e rop parameter can be one of:
<ul>
<li> \c CopyROP:     dst = src.
<li> \c OrROP:       dst = dst OR src.
<li> \c XorROP:      dst = dst XOR src.
<li> \c EraseROP:    dst = (NOT src) AND dst
<li> \c NotCopyROP:  dst = NOT src
<li> \c NotOrROP:    dst = (NOT src) OR dst
<li> \c NotXorROP:   dst = (NOT src) XOR dst
<li> \c NotEraseROP: dst = src AND dst
<li> \c NotROP:      dst = NOT dst
</ul>

There are some restrictions:
<ol>
<li> The \e src device must be QWidget or QPixmap.  You cannot copy pixels
from a picture or a printer (external device).
<li> The \e src device may not have pixel depth greater than \e src.
You cannot copy from an 8 bit pixmap to a 1 bit pixmap.
</ol>
*/

void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx, int sy, int sw, int sh,
	     RasterOp rop )
{
    if ( src->handle() == 0 ) {
#if defined(CHECK_NULL)
	warning( "bitBlt: Cannot bitBlt from device" );
#endif
	return;
    }
    int ts = src->devType();			// from device type
    int td = dst->devType();			// to device type
    Display *dpy = src->display();

    if ( sw <= 0 ) {				// special width
	if ( sw < 0 )
	    sw = src->metric( PDM_WIDTH ) - sx;
	else
	    return;
    }
    if ( sh <= 0 ) {				// special height
	if ( sh < 0 )
	    sh = src->metric( PDM_HEIGHT ) - sy;
	else
	    return;
    }

    if ( dst->paintingActive() && dst->isExtDev() ) {
	QPixmap *pm;				// output to picture/printer
	if ( ts == PDT_PIXMAP )
	    pm = (QPixmap*)src;
	else if ( ts == PDT_WIDGET ) {		// bitBlt to temp pixmap
	    pm = new QPixmap( sw, sh );
	    CHECK_PTR( pm );
	    bitBlt( pm, 0, 0, src, sx, sy, sw, sh, CopyROP );
	}
	else {
#if defined(CHECK_RANGE)
	    warning( "bitBlt: Cannot bitBlt from device" );
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
	warning( "bitBlt: Cannot bitBlt to or from device" );
#endif
	return;
    }
    static short ropCodes[] =			// ROP translation table
	{ GXcopy, GXor, GXxor, GXandInverted,
	  GXcopyInverted, GXorInverted, GXequiv, GXand, GXinvert };
    if ( rop > NotROP ) {
#if defined(CHECK_RANGE)
	warning( "bitBlt: Invalid ROP code" );
#endif
	return;
    }

    if ( dst->handle() == 0 ) {
#if defined(CHECK_NULL)
	warning( "bitBlt: Cannot bitBlt to device" );
#endif
	return;
    }

    bool copy_plane = FALSE;
    bool mono = FALSE;

    if ( ts == PDT_PIXMAP )
	copy_plane = ((QPixmap*)src)->depth() == 1;
    if ( td == PDT_PIXMAP ) {
	bool single_plane = ((QPixmap*)dst)->depth() == 1;
	if ( single_plane && !copy_plane ) {	
#if defined(CHECK_RANGE)
		warning( "bitBlt: Incompatible destination pixmap" );
#endif
		return;			// dest is 1-bit pixmap, source is not
	}
	mono = copy_plane && single_plane;
	copy_plane ^= single_plane;
	((QPixmap*)dst)->detach();   // we will change (possibly) shared pixmap
    }
    GC        gc = qt_xget_temp_gc( mono );
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


/*!
\relates QPaintDevice
\fn void bitBlt( QPaintDevice *dst, const QPoint &dp, const QPaintDevice *src, const QRect &sr, RasterOp rop=CopyROP )
Synonymous bitBlt with the destination point \e dp and source rectangle \e sr.
*/
