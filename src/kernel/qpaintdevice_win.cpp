/****************************************************************************
** $Id$
**
** Implementation of QPaintDevice class for Win32
**
** Created : 940801
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qt_windows.h"


QPaintDevice::QPaintDevice( uint devflags )
{
    if ( !qApp ) {				// global constructor
#if defined(QT_CHECK_STATE)
	qFatal( "QPaintDevice: Must construct a QApplication before a "
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
#if defined(QT_CHECK_STATE)
    if ( paintingActive() )
	qWarning( "QPaintDevice: Cannot destroy paint device that is being "
		  "painted.  Be sure to QPainter::end() painters!" );
#endif
}

HDC QPaintDevice::handle() const
{
    return hdc;
}

bool QPaintDevice::cmd( int, QPainter *, QPDevCmdParam * )
{
#if defined(QT_CHECK_STATE)
    qWarning( "QPaintDevice::cmd: Device has no command interface" );
#endif
    return FALSE;
}

int QPaintDevice::metric( int ) const
{
#if defined(QT_CHECK_STATE)
    qWarning( "QPaintDevice::metrics: Device has no metric information" );
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


Q_EXPORT bool qt_bitblt_bsm = FALSE;		// use black source method
uint qt_bitblt_foreground = 0;			// bitBlt foreground color


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

#if 0
    //### background colors get modified
    if ( destIsPixmap ) {			// blt directly into pixmap
	hdc = hdc_dest;
	hdc_buf = 0;
	nx = dx;
	ny = dy;
    } else {
#else
Q_UNUSED( destIsPixmap )
#endif					// use off-screen buffer
	hdc_buf = CreateCompatibleDC( hdc_dest );
	hbm_buf = CreateCompatibleBitmap( hdc_dest, sw, sh );
	hbm_buf_old = (HBITMAP)SelectObject( hdc_buf, hbm_buf );
	BitBlt( hdc_buf, 0, 0, sw, sh, hdc_dest, dx, dy, SRCCOPY );
	hdc = hdc_buf;
	nx = ny = 0;
#if 0
    }
#endif

    QPixmap *bs = *blackSourcePixmap;
    bool newPixmap = bs == 0;
    if ( newPixmap ) {
	bs = new QPixmap( src_width, src_height, src_depth,
			  QPixmap::NormalOptim );
	Q_CHECK_PTR( bs );
	BitBlt( bs->handle(), 0, 0, src_width, src_height,
		hdc_src, 0, src_offset, SRCCOPY );
	QBitmap masknot( src_width, src_height, FALSE, QPixmap::NormalOptim );
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

#ifndef Q_OS_TEMP
// For alpha blending, we must load the AlphaBlend() function at run time.
#if !defined(AC_SRC_ALPHA)
#define AC_SRC_ALPHA 0x01
#endif
typedef BOOL (WINAPI *ALPHABLEND)( HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION );
static HINSTANCE msimg32Lib = 0;
static ALPHABLEND alphaBlend = 0;
static bool loadAlphaBlendFailed = FALSE;
static void cleanup_msimg32Lib()
{
    if ( msimg32Lib != 0 ) {
	FreeLibrary( msimg32Lib );
	msimg32Lib = 0;
    }
}
#endif

/*
   Try to do an AlphaBlend(). If it fails for some reasons, use BitBlt()
   instead. The arguments are like in the BitBlt() call.
*/
void qt_AlphaBlend( HDC dst_dc, int dx, int dy, int sw, int sh, HDC src_dc, int sx, int sy, DWORD rop )
{
#ifndef Q_OS_TEMP
    BLENDFUNCTION blend = {
	AC_SRC_OVER,
	0,
	255,
	AC_SRC_ALPHA
    };
    if ( alphaBlend != 0 ) {
	alphaBlend( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, sw, sh, blend );
    } else {
	if ( !loadAlphaBlendFailed ) {
	    // try to load msimg32.dll and get the function
	    // AlphaBlend()
	    loadAlphaBlendFailed = TRUE;
	    msimg32Lib = LoadLibraryA( "msimg32" );
	    if ( msimg32Lib != 0 ) {
		qAddPostRoutine( cleanup_msimg32Lib );
		alphaBlend = (ALPHABLEND) GetProcAddress( msimg32Lib, "AlphaBlend" );
		if ( alphaBlend != 0 ) {
		    loadAlphaBlendFailed = FALSE;
		}
	    }
	}
	if ( loadAlphaBlendFailed )
	    alphaBlend( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, sw, sh, blend );
	else
	    BitBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, rop );
    }
#else
    BitBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, rop );
#endif
}

void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx, int sy, int sw, int sh,
	     Qt::RasterOp rop, bool ignoreMask  )
{
    if ( !src || !dst ) {
#if defined(QT_CHECK_NULL)
	Q_ASSERT( src != 0 );
	Q_ASSERT( dst != 0 );
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
	    Q_CHECK_PTR( pm );
	    bitBlt( pm, 0, 0, src, sx, sy, sw, sh );
	} else {
#if defined(QT_CHECK_RANGE)
	    qWarning( "bitBlt: Cannot bitBlt from device" );
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

    switch ( ts ) {
	case QInternal::Widget:
	case QInternal::Pixmap:
	case QInternal::System:			// OK, can blt from these
	    break;
	default:
#if defined(QT_CHECK_RANGE)
	    qWarning( "bitBlt: Cannot bitBlt from device type %x", ts );
#endif
	    return;
    }
    switch ( td ) {
	case QInternal::Widget:
	case QInternal::Pixmap:
	case QInternal::System:			// OK, can blt to these
	    break;
	default:
#if defined(QT_CHECK_RANGE)
	    qWarning( "bitBlt: Cannot bitBlt to device type %x", td );
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
#if defined(QT_CHECK_RANGE)
	qWarning( "bitBlt: Invalid ROP code" );
#endif
	return;
    }

    if ( dst->isExtDev() ) {
#if defined(QT_CHECK_NULL)
	qWarning( "bitBlt: Cannot bitBlt to device" );
#endif
	return;
    }

    if ( td == QInternal::Pixmap )
	((QPixmap*)dst)->detach();		// changes shared pixmap

    HDC	 src_dc = src->hdc, dst_dc = dst->hdc;
    bool src_tmp = FALSE, dst_tmp = FALSE;
    int  src_offset = 0;

    QPixmap *src_pm;
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
	QPixmap *dst_pm = (QPixmap *)dst;
	if ( dst_pm->isMultiCellPixmap() ) {
	    dst_dc = dst_pm->multiCellHandle();
	    dy += dst_pm->multiCellOffset();
	}
    } else {
	if ( !dst_dc && td == QInternal::Widget ) {
	    if ( ((QWidget*)dst)->testWFlags(Qt::WPaintUnclipped) )
		dst_dc = GetWindowDC( ((QWidget*)dst)->winId() );
	    else
		dst_dc = GetDC( ((QWidget*)dst)->winId() );
	    dst_tmp = TRUE;
	}
    }
#if defined(QT_CHECK_NULL)
    Q_ASSERT( src_dc && dst_dc );
#endif

    if ( mask ) {
	if ( src_pm->data->selfmask ) {
	    uint   c = dst->paintingActive() ? qt_bitblt_foreground
					     : Qt::black.pixel();
	    HBRUSH b = CreateSolidBrush( c );
	    COLORREF tc, bc;
	    b = (HBRUSH)SelectObject( dst_dc, b );
	    tc = SetTextColor( dst_dc, Qt::black.pixel() );
	    bc = SetBkColor( dst_dc, Qt::white.pixel() );
	    BitBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, 0x00b8074a );
	    SetBkColor( dst_dc, bc );
	    SetTextColor( dst_dc, tc );
	    DeleteObject( SelectObject(dst_dc, b) );
	} else if ( (qt_winver & Qt::WV_DOS_based) || qt_bitblt_bsm ) {
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
	    // We can safely access hbm() here since multi cell pixmaps
	    // are not used under NT.
	    MaskBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, mask->hbm(),
		     sx, sy, MAKEROP4(0x00aa0000,ropCodes[rop]) );
	}
    } else {
	if ( src_pm && src_pm->data->hasRealAlpha ) {
	    qt_AlphaBlend( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, ropCodes[rop] );
	} else {
	    BitBlt( dst_dc, dx, dy, sw, sh, src_dc, sx, sy, ropCodes[rop] );
	}
    }
    if ( src_tmp )
	ReleaseDC( ((QWidget*)src)->winId(), src_dc );
    if ( dst_tmp )
	ReleaseDC( ((QWidget*)dst)->winId(), dst_dc );
}


void QPaintDevice::setResolution( int )
{
}
 
int QPaintDevice::resolution() const
{
    return metric( QPaintDeviceMetrics::PdmDpiY );
}
