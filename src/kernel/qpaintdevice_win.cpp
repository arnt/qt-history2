/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice_win.cpp#66 $
**
** Implementation of QPaintDevice class for Win32
**
** Created : 940801
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qt_windows.h"


extern Qt::WindowsVersion qt_winver;		// defined in qapplication_win.cpp

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
    painters = 0;
}

QPaintDevice::~QPaintDevice()
{
#if defined(CHECK_STATE)
    if ( paintingActive() )
	warning( "QPaintDevice: Cannot destroy paint device that is being "
		 "painted.  Be sure to QPainter::end() painters!" );
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

int QPaintDevice::fontMet( QFont *, int, const char*, int ) const
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
  The src_offset and mask_offset parameters are for multi cell
  pixmaps.  sy includes src_offset.
*/

static void qDrawTransparentPixmap( HDC hdc_dest, bool destIsPixmap,
				    int dx, int dy,
				    HDC hdc_src,
				    int src_width, int src_height,
				    int src_depth,
				    HDC hdc_mask,
				    int sx, int sy, int sw, int sh,
				    int src_offset, int mask_offset,
				    QPixmap **blackSourcePixmap )
{
    HDC	     hdc;
    HDC	     hdc_buf;
    HBITMAP  hbm_buf, hbm_buf_old;
    int	     nx, ny;

    if ( destIsPixmap ) {			// blt directly into pixmap
	hdc = hdc_dest;
	hdc_buf = 0;
	nx = dx;
	ny = dy;
    } else {					// use off-screen buffer
	hdc_buf = CreateCompatibleDC( hdc_dest );
	hbm_buf = CreateCompatibleBitmap( hdc_dest, sw, sh );
	hbm_buf_old = (HBITMAP)SelectObject( hdc_buf, hbm_buf );
	BitBlt( hdc_buf, 0, 0, sw, sh, hdc_dest, dx, dy, SRCCOPY );
	hdc = hdc_buf;
	nx = ny = 0;
    }

    QPixmap *bs = *blackSourcePixmap;
    bool newPixmap = bs == 0;
    if ( newPixmap ) {
	bs = new QPixmap( src_width, src_height, src_depth );
	CHECK_PTR( bs );
	bs->setOptimization( QPixmap::NormalOptim );
	BitBlt( bs->handle(), 0, 0, src_width, src_height,
		hdc_src, 0, src_offset, SRCCOPY );
	QBitmap masknot( src_width, src_height );
	masknot.setOptimization( QPixmap::NormalOptim );
	BitBlt( masknot.handle(), 0, 0, src_width, src_height,
		hdc_mask, 0, mask_offset, NOTSRCCOPY );
	BitBlt( bs->handle(), 0, 0, src_width, src_height,
		masknot.handle(), 0, 0, SRCAND );
    }
    BitBlt( hdc, nx, ny, sw, sh, hdc_mask, sx, sy-src_offset+mask_offset,
	    SRCAND );
    BitBlt( hdc, nx, ny, sw, sh, bs->handle(), sx, sy-src_offset, SRCPAINT );
    *blackSourcePixmap = bs;

    if ( hdc_buf ) {				// blt off-screen buffer
	BitBlt( hdc_dest, dx, dy, sw, sh, hdc_buf, 0, 0, SRCCOPY );
	DeleteObject( SelectObject(hdc_buf,hbm_buf_old) );
	DeleteDC( hdc_buf );
    }
}


void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx, int sy, int sw, int sh,
	     Qt::RasterOp rop, bool ignoreMask  )
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
	    sw = src->metric(QPaintDeviceMetrics::PdmWidth) - sx;
	else
	    return;
    }
    if ( sh <= 0 ) {				// special height
	if ( sh < 0 )
	    sh = src->metric(QPaintDeviceMetrics::PdmHeight) - sy;
	else
	    return;
    }

    if ( dst->paintingActive() && dst->isExtDev() ) {
	QPixmap *pm;				// output to picture/printer
	bool	 tmp_pm = TRUE;
	if ( ts == QInternal::Pixmap ) {
	    pm = (QPixmap*)src;
	    if ( sx != 0 || sy != 0 ||
		 sw != pm->width() || sh != pm->height() ) {
		QPixmap *pm_new = new QPixmap( sw, sh, pm->depth() );
		bitBlt( pm_new, 0, 0, pm, sx, sy, sw, sh );
		pm = pm_new;
	    } else {
		tmp_pm = FALSE;
	    }
	} else if ( ts == QInternal::Widget ) {	// bitBlt to temp pixmap
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
	dst->cmd( QPaintDevice::PdcDrawPixmap, 0, param );
	if ( tmp_pm )
	    delete pm;
	return;
    }

    if ( !(ts <= QInternal::Pixmap && td <= QInternal::Pixmap) ) {
#if defined(CHECK_RANGE)
	warning( "bitBlt: Cannot bitBlt to or from device" );
#endif
	return;
    }
    static uint ropCodes[] = {			// ROP translation table
	SRCCOPY, SRCPAINT, SRCINVERT, 0x00220326 /* DSna */,
	NOTSRCCOPY, MERGEPAINT, 0x00990066 /* DSnx */, SRCAND,
	DSTINVERT, BLACKNESS, WHITENESS, 0x00AA0029 /* D */,
	SRCERASE, 0x00DD0228 /* SDno */, 0x007700E6 /* DSan */, NOTSRCERASE
    };
    if ( rop > Qt::LastROP ) {
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

    if ( td == QInternal::Pixmap )
	((QPixmap*)dst)->detach();		// changes shared pixmap

    HDC	 src_dc = src->hdc, dst_dc = dst->hdc;
    bool src_tmp = FALSE, dst_tmp = FALSE;
    int  src_offset = 0;

    QPixmap *src_pm, *dst_pm;
    QBitmap *mask;
    if ( ts == QInternal::Pixmap ) {
	src_pm = (QPixmap *)src;
	mask = ignoreMask ? 0 : (QBitmap *)src_pm->mask();
	if ( src_pm->isMultiCellPixmap() ) {
	    src_dc = src_pm->multiCellHandle();
	    src_offset = src_pm->multiCellOffset();
	    sy += src_offset;
	}
    } else {
	src_pm = 0;
	mask   = 0;
	if ( !src_dc && ts == QInternal::Widget ) {
	    src_dc = GetDC( ((QWidget*)src)->winId() );
	    src_tmp = TRUE;
	}
    }
    if ( td == QInternal::Pixmap ) {
	dst_pm = (QPixmap *)dst;
	if ( dst_pm->isMultiCellPixmap() ) {
	    dst_dc = dst_pm->multiCellHandle();
	    dy += dst_pm->multiCellOffset();
	}
    } else {
	dst_pm = 0;
	if ( !dst_dc && td == QInternal::Widget ) {
	    if ( ((QWidget*)dst)->testWFlags(Qt::WPaintUnclipped) )
		dst_dc = GetWindowDC( ((QWidget*)dst)->winId() );
	    else
		dst_dc = GetDC( ((QWidget*)dst)->winId() );
	    dst_tmp = TRUE;
	}
    }
#if defined(CHECK_NULL)
    ASSERT( src_dc && dst_dc );
#endif

    if ( mask ) {
	if ( src_pm->data->selfmask ) {
	    HBRUSH b = CreateSolidBrush( Qt::black.pixel() );
	    COLORREF tc, bc;
	    b = (HBRUSH)SelectObject( dst_dc, b );
	    tc = SetTextColor( dst_dc, Qt::black.pixel() );
	    bc = SetBkColor( dst_dc, Qt::white.pixel() );
	    BitBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, 0x00b8074a );
	    SetBkColor( dst_dc, bc );
	    SetTextColor( dst_dc, tc );
	    DeleteObject( SelectObject(dst_dc, b) );
	} else if ( qt_winver == Qt::WV_95 || qt_winver == Qt::WV_98 ||
		    qt_bitblt_bsm ) {
	    HDC mask_dc;
	    int mask_offset;
	    if ( mask->isMultiCellPixmap() ) {
		mask_dc = mask->multiCellHandle();
		mask_offset = mask->multiCellOffset();
	    } else {
		mask_dc = mask->handle();
		mask_offset = 0;
	    }
	    qDrawTransparentPixmap( dst_dc, td == QInternal::Pixmap,
				    dx, dy, src_dc, src_pm->width(),
				    src_pm->height(), src_pm->depth(),
				    mask_dc, sx, sy, sw, sh,
				    src_offset, mask_offset,
				    &src_pm->data->maskpm );
	    if ( src_pm->optimization() != QPixmap::BestOptim ) {
		// Don't keep black source pixmap
		delete src_pm->data->maskpm;
		src_pm->data->maskpm = 0;
	    }
	} else {
	    MaskBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, mask->hbm(),
		     sx, sy, MAKEROP4(0x00aa0000,ropCodes[rop]) );
	}
    } else {
	BitBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, ropCodes[rop] );
    }
    if ( src_tmp )
	ReleaseDC( ((QWidget*)src)->winId(), src_dc );
    if ( dst_tmp )
	ReleaseDC( ((QWidget*)dst)->winId(), dst_dc );
}
