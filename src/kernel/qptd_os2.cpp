/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptd_os2.cpp#12 $
**
** Implementation of QPaintDevice class for OS/2 PM
**
** Created : 940802
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpaintdevice.h"
#include "qwidget.h"
#include "qpixmap.h"
#define	 INCL_PM
#include <os2.h>

QPaintDevice::QPaintDevice()
{
    devFlags = PDT_UNDEF;
}

QPaintDevice::~QPaintDevice()
{
}


static int get_dev_height( QPaintDevice *d )
{
    QSize s(0,0);
    switch ( d->devType() ) {
	case PDT_WIDGET:
	    s = ((QWidget*)d)->clientSize();
	    break;
	case PDT_PIXMAP:
	    s = ((QPixMap*)d)->size();
	    break;
    }
    return s.height();
}


void QPaintDevice::bitBlt( int sx, int sy, int sw, int sh, QPaintDevice *dest,
			   int dx, int dy, RasterOp rop )
{
    int ts = devType();				// from device type
    int td = dest->devType();			// to device type

// NOTE!!!  PS printer/metafile pixmap output not yet supported...
    if ( (td == PDT_PRINTER || td == PDT_METAFILE) && dest->paintingActive() ){
//	cmd( PDC_DRAWPIXMAP, ... );   // !!!!!!!!!!
	return;
    }

    if ( ts == PDT_PRINTER || ts == PDT_METAFILE ) {
#if defined(CHECK_STATE)
	warning( "QPaintDevice::bitBlt: Cannot bitBlt from device" );
#endif
	return;
    }
    static long ropCodes[] =			// ROP translation table
	{ ROP_SRCCOPY, ROP_SRCPAINT, ROP_SRCINVERT, 0x0022L /* D & ~S */,
	  ROP_NOTSRCCOPY, ROP_MERGEPAINT, 0x0099L /* D ^ ~S */,
	  ROP_SRCAND, ROP_DSTINVERT };
    if ( !(rop >= CopyROP && rop <= NotROP) ) {
#if defined(CHECK_RANGE)
	warning( "QPaintDevice::bitBlt: Invalid ROP code" );
#endif
	return;
    }
    HDC srchps = hps, desthps = dest->hps;
    if ( !(srchps && desthps) )			// not ready, (why?)
	return;
    int srcdh  = get_dev_height( (QPaintDevice *)this );
    int destdh = get_dev_height( dest );
    POINTL pts[3];
    pts[0].x = dx;				// target, lower left
    pts[0].y = destdh - dy;
    pts[1].x = dx + sw;				// target, upper right
    pts[1].y = destdh - dy - sh;
    pts[2].x = sx;				// source, lower left
    pts[2].y = srcdh - sy;
    GpiBitBlt( desthps, srchps, 3, pts, ropCodes[rop], BBO_AND );
}
