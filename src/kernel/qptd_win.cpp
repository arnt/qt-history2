/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptd_win.cpp#35 $
**
** Implementation of QPaintDevice class for Win32
**
** Created : 940801
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpaintd.h"
#include "qpaintdc.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapp.h"

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

extern WindowsVersion qt_winver;		// defined in qapp_win.cpp

RCSTAG("$Id: //depot/qt/main/src/kernel/qptd_win.cpp#35 $");


QPaintDevice::QPaintDevice( uint devflags )
{
    if ( !qApp ) {				// global constructor
#if defined(CHECK_STATE)
	fatal( "QPaintDevice: Must construct a QApplication before a "
	       "QPaintDevice" );
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

int QPaintDevice::metric( int ) const
{
#if defined(CHECK_STATE)
    warning( "QPaintDevice::metrics: Device has no metric information" );
#endif
    return 0;
}

int QPaintDevice::fontMet( QFont *, int, const char *, int ) const
{
    return 0;
}

int QPaintDevice::fontInf( QFont *, int ) const
{
    return 0;
}


bool qt_bitblt_bsm = FALSE;			// ######### experimental

/*
  Draw transparent pixmap using the black source method.
*/

static void qDrawTransparentPixmap( HDC hdc_dest, bool destIsPixmap,
				    int dx, int dy,
				    const QPixmap *pixmap,
				    const QBitmap *mask,
				    int sx, int sy, int sw, int sh,
				    QPixmap **blackSourcePixmap )
{
#if defined(DEBUG)
    ASSERT( sw > 0 && sh > 0 && pixmap->handle() && mask->handle() );
#endif
    HDC	     hdc;
    HDC	     hdc_buf = 0;
    HBITMAP  hbm_buf, hbm_buf_old;
    int	     nx, ny;

    if ( destIsPixmap ) {			// blt directly into pixmap
	hdc = hdc_dest;
	nx = dx;
	ny = dy;
    } else {					// use off-screen buffer
	hdc_buf = CreateCompatibleDC( hdc_dest );
	hbm_buf = CreateCompatibleBitmap( hdc_dest, sw, sh );
	hbm_buf_old = SelectObject( hdc_buf, hbm_buf );
	BitBlt( hdc_buf, 0, 0, sw, sh, hdc_dest, dx, dy, SRCCOPY );
	hdc = hdc_buf;
	nx = ny = 0;
    }

    QPixmap *bs = *blackSourcePixmap;
    bool newPixmap = bs == 0;
    if ( newPixmap ) {
	bs = new QPixmap( sw, sh, pixmap->depth() );
	CHECK_PTR( bs );
	bs->setOptimization( QPixmap::NormalOptim );
	BitBlt( bs->handle(), 0, 0, sw, sh, pixmap->handle(),
		sx, sy, SRCCOPY );
	QBitmap masknot( sw, sh );
	masknot.setOptimization( QPixmap::NormalOptim );
	BitBlt( masknot.handle(), 0, 0, sw, sh, mask->handle(),
		sx, sy, NOTSRCCOPY );
	BitBlt( bs->handle(), 0, 0, sw, sh, masknot.handle(),
		sx, sy, SRCAND );
    }
    BitBlt( hdc, nx, ny, sw, sh, mask->handle(), sx, sy, SRCAND );
    BitBlt( hdc, nx, ny, sw, sh, bs->handle(), sx, sy, SRCPAINT );
    *blackSourcePixmap = bs;

    if ( hdc_buf ) {				// blt off-screen buffer
	BitBlt( hdc_dest, dx, dy, sw, sh, hdc_buf, 0, 0, SRCCOPY );
	DeleteObject( SelectObject(hdc_buf,hbm_buf_old) );
	DeleteDC( hdc_buf );
    }
}


void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx, int sy, int sw, int sh,
	     RasterOp rop, bool ignoreMask  )
{
    if ( !src || !dst ) {
#if defined(CHECK_NULL)
	ASSERT( src != 0 );
	ASSERT( dst != 0 );
#endif
	return;
    }
    if ( src->isExtDev() )
	return;

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
	bool	 tmp_pm = TRUE;
	if ( ts == PDT_PIXMAP ) {
	    pm = (QPixmap*)src;
	    if ( sx != 0 || sy != 0 ||
		 sw != pm->width() || sh != pm->height() ) {
		QPixmap *pm_new = new QPixmap( sw, sh, pm->depth() );
		bitBlt( pm_new, 0, 0, pm, sx, sy, sw, sh );
		pm = pm_new;
	    } else {
		tmp_pm = FALSE;
	    }
	} else if ( ts == PDT_WIDGET ) {	// bitBlt to temp pixmap
	    pm = new QPixmap( sw, sh );
	    CHECK_PTR( pm );
	    bitBlt( pm, 0, 0, src, sx, sy, sw, sh );
	} else {
#if defined(CHECK_RANGE)
	    warning( "bitBlt: Cannot bitBlt from device" );
#endif
	    return;
	}
	QPDevCmdParam param[3];
	QPoint p(dx,dy);
	param[0].point	= &p;
	param[1].pixmap = pm;
	dst->cmd( PDC_DRAWPIXMAP, 0, param );
	if ( tmp_pm )
	    delete pm;
	return;
    }

    if ( !(ts <= PDT_PIXMAP && td <= PDT_PIXMAP) ) {
#if defined(CHECK_RANGE)
	warning( "bitBlt: Cannot bitBlt to or from device" );
#endif
	return;
    }
    static uint ropCodes[] =			// ROP translation table
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

    QPixmap *src_pm;
    QBitmap *mask;
    if ( ts == PDT_PIXMAP ) {
	src_pm = (QPixmap *)src;
	mask   = ignoreMask ? 0 : (QBitmap *)src_pm->mask();
    } else {
	src_pm = 0;
	mask   = 0;
    }

    if ( !src_dc ) {
	switch ( ts ) {
	    case PDT_WIDGET:
		src_dc = GetDC( ((QWidget*)src)->winId() );
		break;
	    case PDT_PIXMAP:
		src_dc = src_pm->allocMemDC();
		break;
	}
	src_tmp = TRUE;
    }
    if ( !dst_dc ) {
	switch ( td ) {
	    case PDT_WIDGET:
		if ( ((QWidget*)dst)->testWFlags(WPaintUnclipped) )
		    dst_dc = GetWindowDC( ((QWidget*)dst)->winId() );
		else
		    dst_dc = GetDC( ((QWidget*)dst)->winId() );
		break;
	    case PDT_PIXMAP:
		dst_dc = ((QPixmap*)dst)->allocMemDC();
		break;
	}
	dst_tmp = TRUE;
    }
    if ( !(src_dc && dst_dc) )			// not ready, (why?)
	return;

    if ( mask ) {
	if ( src_pm->data->selfmask ) {
	    HBRUSH b = CreateSolidBrush( black.pixel() );
	    COLORREF tc, bc;
	    b = SelectObject( dst_dc, b );
	    tc = SetTextColor( dst_dc, black.pixel() );
	    bc = SetBkColor( dst_dc, white.pixel() );
	    BitBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, 0x00b8074a );
	    SetBkColor( dst_dc, bc );
	    SetTextColor( dst_dc, tc );
	    DeleteObject( SelectObject(dst_dc, b) );
	} else if ( qt_winver == WV_95 || qt_bitblt_bsm ) {
	    bool mask_tmp = mask->handle() == 0;
	    if ( mask_tmp )
		mask->allocMemDC();
	    qDrawTransparentPixmap( dst_dc, td == PDT_PIXMAP,
				    dx, dy, src_pm, mask,
				    sx, sy, sw, sh, &src_pm->data->maskpm );
	    if ( src_pm->data->opt != QPixmap::BestOptim ) {
		delete src_pm->data->maskpm;
		src_pm->data->maskpm = 0;
	    }
	    if ( mask_tmp )
		mask->freeMemDC();
	} else {
	    MaskBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, mask->hbm(),
		     sx, sy, MAKEROP4(0x00aa0000,ropCodes[rop]) );
	}
    } else {
	BitBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, ropCodes[rop] );
    }
    if ( src_tmp ) {
	switch ( ts ) {
	    case PDT_WIDGET:
		ReleaseDC( ((QWidget*)src)->winId(), src_dc );
		break;
	    case PDT_PIXMAP:
		src_pm->freeMemDC();
		break;
	}
    }
    if ( dst_tmp ) {
	switch ( td ) {
	    case PDT_WIDGET:
		ReleaseDC( ((QWidget*)dst)->winId(), dst_dc );
		break;
	    case PDT_PIXMAP:
		((QPixmap*)dst)->freeMemDC();
		break;
	}
    }

}
