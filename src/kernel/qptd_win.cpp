/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptd_win.cpp#10 $
**
** Implementation of QPaintDevice class for Windows
**
** Author  : Haavard Nord
** Created : 940801
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpaintd.h"
#include "qpaintdc.h"
#include "qwidget.h"
#include "qpixmap.h"
#include "qapp.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qptd_win.cpp#10 $";
#endif


QPaintDevice::QPaintDevice( uint devflags )
{
    if ( !qApp ) {				// global constructor
#if defined(CHECK_STATE)
	fatal( "QPaintDevice: Must construct a QApplication before a QPaintDevice" );
#endif
	return;
    }
    devFlags = devflags;
    hdc = 0;
}

QPaintDevice::~QPaintDevice()
{
#if defined(CHECK_STATE)
    if ( paintingActive() )
	warning( "QPaintDevice: Cannot destroy paint device that is being "
		 "painted" );
#endif
}


bool QPaintDevice::cmd( int, QPainter *, QPDevCmdParam * )
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
    if ( src->isExtDev() ) {
#if defined(CHECK_NULL)
	warning( "bitBlt: Cannot bitBlt from device" );
#endif
	return;
    }
    int ts = src->devType();			// from device type
    int td = dst->devType();			// to device type

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
	param[0].rect	= &r;
	param[1].point	= &p;
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
    static long ropCodes[] =			// ROP translation table
	{ SRCCOPY, SRCPAINT, SRCINVERT, 0x00220326 /* DSna */,
	  NOTSRCCOPY, MERGEPAINT, 0x00990066 /* DSnx */,
	  SRCAND, DSTINVERT };
    if ( rop > NotROP ) {
#if defined(CHECK_RANGE)
	warning( "bitBlt: Invalid ROP code" );
#endif
	return;
    }

    if ( dst->isExtDev() ) {
#if defined(CHECK_NULL)
	warning( "bitBlt: Cannot bitBlt to device" );
#endif
	return;
    }

    if ( td == PDT_PIXMAP )
	((QPixmap*)dst)->detach();		// changes shared pixmap
    HDC	 src_dc	 = src->hdc, dst_dc  = dst->hdc;
    bool src_tmp = FALSE,    dst_tmp = FALSE;
    if ( !src_dc ) {
	switch ( ts ) {
	    case PDT_WIDGET:
		src_dc = GetDC( ((QWidget*)src)->id() );
		break;
	    case PDT_PIXMAP:
		src_dc = ((QPixmap*)src)->allocMemDC();
		break;
	}
	src_tmp = TRUE;
    }
    if ( !dst_dc ) {
	switch ( td ) {
	    case PDT_WIDGET:
		dst_dc = GetDC( ((QWidget*)dst)->id() );
		break;
	    case PDT_PIXMAP:
		dst_dc = ((QPixmap*)dst)->allocMemDC();
		break;
	}
	dst_tmp = TRUE;
    }
    if ( !(src_dc && dst_dc) )			// not ready, (why?)
	return;
    BitBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, ropCodes[rop] );
    if ( src_tmp ) {
	switch ( ts ) {
	    case PDT_WIDGET:
		ReleaseDC( ((QWidget*)src)->id(), src_dc );
		break;
	    case PDT_PIXMAP:
		((QPixmap*)src)->freeMemDC();
		break;
	}
    }
    if ( dst_tmp ) {
	switch ( td ) {
	    case PDT_WIDGET:
		ReleaseDC( ((QWidget*)dst)->id(), dst_dc );
		break;
	    case PDT_PIXMAP:
		((QPixmap*)dst)->freeMemDC();
		break;
	}
    }

}
