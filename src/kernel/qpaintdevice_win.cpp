/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice_win.cpp#3 $
**
** Implementation of QPaintDevice class for Windows + NT
**
** Author  : Haavard Nord
** Created : 940801
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qpaintd.h"
#include "qwidget.h"
#include "qpixmap.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpaintdevice_win.cpp#3 $";
#endif


QPaintDevice::QPaintDevice()
{
    devFlags = PDT_UNDEF;
}

QPaintDevice::~QPaintDevice()
{
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
	{ SRCCOPY, SRCPAINT, SRCINVERT, 0x00220326 /* DSna */,
	  NOTSRCCOPY, MERGEPAINT, 0x00990066 /* DSnx */,
	  SRCAND, DSTINVERT };
    if ( !(rop >= CopyROP && rop <= NotROP) ) {
#if defined(CHECK_RANGE)
	warning( "QPaintDevice::bitBlt: Invalid ROP code" );
#endif
	return;
    }
    HDC src_dc = hdc, dest_dc = dest->hdc;
    bool src_tmp = FALSE, dest_tmp = FALSE;
    if ( !src_dc ) {
	switch ( ts ) {
	    case PDT_WIDGET:
		src_dc = GetDC( ((QWidget*)this)->id() );
		break;
	    case PDT_PIXMAP:
		src_dc = ((QPixMap*)this)->allocMemDC();
		break;
	}
	src_tmp = TRUE;
    }
    if ( !dest_dc ) {
	switch ( td ) {
	    case PDT_WIDGET:
		dest_dc = GetDC( ((QWidget*)dest)->id() );
		break;
	    case PDT_PIXMAP:
		dest_dc = ((QPixMap*)dest)->allocMemDC();
		break;
	}
	dest_tmp = TRUE;
    }
    if ( !(src_dc && dest_dc) )			// not ready, (why?)
	return;
    BitBlt( dest_dc, dx, dy, sw, sh, src_dc, sx, sy, ropCodes[rop] );
    if ( src_tmp ) {
	switch ( ts ) {
	    case PDT_WIDGET:
		ReleaseDC( ((QWidget*)this)->id(), src_dc );
		break;
	    case PDT_PIXMAP:
		((QPixMap*)this)->freeMemDC();
		break;
	}
    }
    if ( dest_tmp ) {
	switch ( td ) {
	    case PDT_WIDGET:
		ReleaseDC( ((QWidget*)dest)->id(), dest_dc );
		break;
	    case PDT_PIXMAP:
		((QPixMap*)dest)->freeMemDC();
		break;
	}
    }

}
