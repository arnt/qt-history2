/****************************************************************************
** $Id$
**
** Implementation of QPixmap class for Win32
**
** Created : 940501
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

#include "qbitmap.h"
#include "qimage.h"
#include "qpaintdevicemetrics.h"
#include "qwmatrix.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qt_windows.h"
#include <limits.h>


extern const uchar *qt_get_bitflip_array();		// defined in qimage.cpp

#define DATA_HBM	 data->hbm_or_mcpi.hbm
#define DATA_MCPI	 data->hbm_or_mcpi.mcpi
#define DATA_MCPI_MCP	 data->hbm_or_mcpi.mcpi->mcp
#define DATA_MCPI_OFFSET data->hbm_or_mcpi.mcpi->offset

static bool mcp_system_unstable = FALSE;

/*!
  \class QPixmap::QMCPI
  \brief The QPixmap::QMCPI class is an internal class.
  \internal
*/

/*
  The QMultiCellPixmap class is strictly internal and used to
  implement the setOptimization(MemoryOptim) feature for Win9x.
*/

struct QMCPFreeNode {
    QMCPFreeNode( short o, short s ) : offset(o), size(s) {}
    short offset;
    short size;
};

typedef QPtrList<QMCPFreeNode> QMCPFreeList;

class QMultiCellPixmap {
public:
    QMultiCellPixmap( int width, int depth, int maxHeight );
   ~QMultiCellPixmap();
    bool isEmpty() const
    {
	QMCPFreeNode *n = free_list->last();
	return n && n->offset == 0 && n->size == max_height;
    }
    QPixmap *sharedPixmap() const { return pixmap; }
    HDC	     handle()	    const { return pixmap->handle(); }
    HBITMAP  hbm()	    const { return pixmap->hbm(); }
    int	     allocCell( int height );
    void     freeCell( int offset, int height );
    void     debugger(); // for debugging during development
private:
    QPixmap	  *pixmap;
    int		   max_height;
    QMCPFreeList *free_list;
};


static inline HDC alloc_mem_dc( HBITMAP hbm )
{
    HDC hdc = CreateCompatibleDC( qt_display_dc() );
    if ( !hdc ) {
#if defined(QT_CHECK_NULL)
	qSystemWarning( "alloc_mem_dc: CreateCompatibleDC failed!" );
#endif
	return hdc;
    }
    if ( QColor::hPal() ) {
	SelectPalette( hdc, QColor::hPal(), FALSE );
	RealizePalette( hdc );
    }
    SelectObject( hdc, hbm );
    return hdc;
}


void QPixmap::init( int w, int h, int d, bool bitmap, Optimization optim )
{
    static int serial = 0;
    int dd = defaultDepth();

    if ( optim == DefaultOptim )		// use default optimization
	optim = defOptim;

    data = new QPixmapData;
    Q_CHECK_PTR( data );

    memset( data, 0, sizeof(QPixmapData) );
    data->count  = 1;
    data->uninit = TRUE;
    data->bitmap = bitmap;
    data->ser_no = ++serial;
    data->optim	 = optim;
    data->hasRealAlpha = FALSE;

    bool make_null = w == 0 || h == 0;		// create null pixmap
    if ( d == 1 )				// monocrome pixmap
	data->d = 1;
    else if ( d < 0 || d == dd )		// compatible pixmap
	data->d = dd;
    if ( make_null || w < 0 || h < 0 || data->d == 0 ) {
	hdc = 0;
	DATA_HBM = 0;
#if defined(QT_CHECK_RANGE)
	if ( !make_null )			// invalid parameters
	    qWarning( "QPixmap: Invalid pixmap parameters" );
#endif
	return;
    }
    data->w = w;
    data->h = h;
    if ( data->optim == MemoryOptim && ( qt_winver & WV_DOS_based ) ) {
	hdc = 0;
	if ( allocCell() >= 0 )			// successful
	    return;
    }
    if ( data->d == dd )			// compatible bitmap
	DATA_HBM = CreateCompatibleBitmap( qt_display_dc(), w, h );
    else					// monocrome bitmap
	DATA_HBM = CreateBitmap( w, h, 1, 1, 0 );
    if ( !DATA_HBM ) {
	data->w = 0;
	data->h = 0;
	hdc = 0;
#if defined(QT_CHECK_NULL)
	qSystemWarning( "QPixmap: Pixmap allocation failed" );
#endif
	return;
    }
    hdc = alloc_mem_dc( DATA_HBM );
}


void QPixmap::deref()
{
    if ( data && data->deref() ) {		// last reference lost
	if ( data->mcp ) {
	    if ( mcp_system_unstable ) {	// all mcp's gone
		data->mcp = FALSE;
		delete DATA_MCPI;
		DATA_MCPI = 0;
		DATA_HBM  = 0;
	    } else {
		freeCell( TRUE );
	    }
	}
	if ( data->mask )
	    delete data->mask;
	if ( data->bits )
	    delete [] data->bits;
	if ( data->maskpm )
	    delete data->maskpm;
	if ( hdc ) {
	    DeleteDC( hdc );
	    hdc = 0;
	}
	if ( DATA_HBM )
	    DeleteObject( DATA_HBM );
	delete data;
    }
}


QPixmap::QPixmap( int w, int h, const uchar *bits, bool isXbitmap )
    : QPaintDevice( QInternal::Pixmap )
{						// for bitmaps only
    init( 0, 0, 0, FALSE, NormalOptim );
    if ( w <= 0 || h <= 0 )			// create null pixmap
	return;

    data->uninit = FALSE;
    data->w = w;
    data->h = h;
    data->d = 1;

    int bitsbpl = (w+7)/8;			// original # bytes per line
    int bpl	= ((w+15)/16)*2;		// bytes per scanline
    uchar *newbits = new uchar[bpl*h];
    uchar *p	= newbits;
    int x, y, pad;
    pad = bpl - bitsbpl;
    if ( isXbitmap ) {				// flip and invert
	const uchar *f = qt_get_bitflip_array();
	for ( y=0; y<h; y++ ) {
	    for ( x=0; x<bitsbpl; x++ )
		*p++ = ~f[*bits++];
	    for ( x=0; x<pad; x++ )
		*p++ = 0;
	}
    } else {					// invert all bits
	for ( y=0; y<h; y++ ) {
	    for ( x=0; x<bitsbpl; x++ )
		*p++ = ~(*bits++);
	    for ( x=0; x<pad; x++ )
		*p++ = 0;
	}
    }
    DATA_HBM = CreateBitmap( w, h, 1, 1, newbits );
    hdc = alloc_mem_dc( DATA_HBM );
    delete [] newbits;
    if ( defOptim != NormalOptim )
	setOptimization( defOptim );
}


void QPixmap::detach()
{
    if ( data->uninit || data->count == 1 )
	data->uninit = FALSE;
    else
	*this = copy();
    // reset the cache data
    if ( data->bits ) {
	delete [] data->bits;
	data->bits = 0;
    }
    if ( data->maskpm ) {
	delete data->maskpm;
	data->maskpm = 0;
    }
}


int QPixmap::defaultDepth()
{
    static int dd = 0;
    if ( dd == 0 )
	dd = GetDeviceCaps( qt_display_dc(), BITSPIXEL );
    return dd;
}


void QPixmap::setOptimization( Optimization optimization )
{
    if ( optimization == data->optim )
	return;
    detach();
    data->optim = optimization == DefaultOptim ?
	    defOptim : optimization;
    if ( data->optim == MemoryOptim ) {
	if ( data->bits ) {
	    delete [] data->bits;
	    data->bits = 0;
	}
	if ( data->maskpm ) {
	    delete data->maskpm;
	    data->maskpm = 0;
	}
	if ( qt_winver & WV_DOS_based )
	    allocCell();
    } else {
	if ( data->mcp )
	    freeCell();
    }
}


void QPixmap::fill( const QColor &fillColor )
{
    if ( isNull() )
	return;
    detach();					// detach other references
    HDC dc;
    int sy;
    if ( data->mcp ) {				// multi-cell pixmap
	dc = DATA_MCPI_MCP->handle();
	sy = DATA_MCPI_OFFSET;
    } else {
	dc = hdc;
	sy = 0;
    }
    if ( fillColor == black ) {
	PatBlt( dc, 0, sy, data->w, data->h, BLACKNESS );
    } else if ( fillColor == white ) {
	PatBlt( dc, 0, sy, data->w, data->h, WHITENESS );
    } else {
	HBRUSH hbrush = CreateSolidBrush( fillColor.pixel() );
	HBRUSH hb_old = (HBRUSH)SelectObject( dc, hbrush );
	PatBlt( dc, 0, sy, width(), height(), PATCOPY );
	DeleteObject( SelectObject(dc, hb_old) );
    }
}


int QPixmap::metric( int m ) const
{
    if ( m == QPaintDeviceMetrics::PdmWidth ) {
	return width();
    } else if ( m == QPaintDeviceMetrics::PdmHeight ) {
	return height();
    } else {
	int val;
	HDC dc;
	QPixmap *spm;
	if ( data->mcp ) {
	    spm = DATA_MCPI_MCP->sharedPixmap();
	    dc  = spm->handle();
	} else {
	    spm = 0;
	    dc  = handle();
	}
	switch ( m ) {
	    case QPaintDeviceMetrics::PdmDpiX:
		val = GetDeviceCaps( dc, LOGPIXELSX );
		break;
	    case QPaintDeviceMetrics::PdmDpiY:
		val = GetDeviceCaps( dc, LOGPIXELSY );
		break;
	    case QPaintDeviceMetrics::PdmWidthMM:
		val = width()
			* GetDeviceCaps( dc, HORZSIZE )
			/ GetDeviceCaps( dc, HORZRES );
		if ( spm )
		    val = val * width() / spm->width();
		break;
	    case QPaintDeviceMetrics::PdmHeightMM:
		val = height()
			* GetDeviceCaps( dc, VERTSIZE )
			/ GetDeviceCaps( dc, VERTRES );
		if ( spm )
		    val = val * height() / spm->height();
		break;
	    case QPaintDeviceMetrics::PdmNumColors:
		if ( GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE )
		    val = GetDeviceCaps( dc, SIZEPALETTE );
		else
		    val = GetDeviceCaps( dc, NUMCOLORS );
		break;
	    case QPaintDeviceMetrics::PdmDepth:
		val = depth();
		break;
	    default:
		val = 0;
#if defined(QT_CHECK_RANGE)
		qWarning( "QPixmap::metric: Invalid metric command" );
#endif
	}
	return val;
    }
}


QImage QPixmap::convertToImage() const
{
    if ( isNull() ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QPixmap::convertToImage: Cannot convert a null pixmap" );
#endif
	QImage nullImage;
	return nullImage;
    }

    int	w = width();
    int	h = height();
    int	d = depth();
    int	ncols = 2;
    const QBitmap *m = mask();

    if ( d > 1 && d <= 8 || d == 1 && m ) {	// set to nearest valid depth
	d = 8;					//   2..7 ==> 8
	ncols = 256;
    } else if ( d > 8 ) {
	d = 32;					//   > 8  ==> 32
	ncols = 0;
    }

    QImage image( w, h, d, ncols, QImage::BigEndian );
    int	  bmi_data_len = sizeof(BITMAPINFO)+sizeof(RGBQUAD)*ncols;
    char *bmi_data = new char[bmi_data_len];
    memset( bmi_data, 0, bmi_data_len );
    BITMAPINFO	     *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize		  = sizeof(BITMAPINFOHEADER);
    bmh->biWidth	  = w;
    bmh->biHeight	  = -h;			// top-down bitmap
    bmh->biPlanes	  = 1;
    bmh->biBitCount	  = d;
    bmh->biCompression	  = BI_RGB;
    bmh->biSizeImage	  = image.numBytes();
    bmh->biClrUsed	  = ncols;
    bmh->biClrImportant	  = 0;
    QRgb *coltbl = (QRgb*)(bmi_data + sizeof(BITMAPINFOHEADER));

    bool mcp = data->mcp;
    if ( mcp )					// disable multi cell
	((QPixmap*)this)->freeCell();
#ifndef Q_OS_TEMP
    GetDIBits( qt_display_dc(), DATA_HBM, 0, h, image.bits(), bmi,
	       DIB_RGB_COLORS );
#endif
    if ( data->hasRealAlpha ) {
	// Windows has premultiplied alpha, so revert it
	image.setAlphaBuffer( TRUE );
	int l = image.numBytes();
	uchar *b = image.bits();
	// ### is it right to assume that we have 32bpp?
	for ( int i=0; i+3<l; i+=4 ) {
	    if ( b[i+3] == 0 )
		continue;
	    b[i]   = ((int)b[i]  *255)/b[i+3];
	    b[i+1] = ((int)b[i+1]*255)/b[i+3];
	    b[i+2] = ((int)b[i+2]*255)/b[i+3];
	}
    }

    if ( mcp )
	((QPixmap*)this)->allocCell();

    for ( int i=0; i<ncols; i++ ) {		// copy color table
	RGBQUAD *r = (RGBQUAD*)&coltbl[i];
	if ( m )
	    image.setColor( i, qRgba(r->rgbRed,
				r->rgbGreen,
				r->rgbBlue,255) );
	else
	    image.setColor( i, qRgb(r->rgbRed,
				r->rgbGreen,
				r->rgbBlue) );
    }

    if ( m ) {
	image.setAlphaBuffer(TRUE);
	QImage msk = m->convertToImage();

	switch ( d ) {
	  case 8: {
	    int used[256];
	    memset( used, 0, sizeof(int)*256 );
	    uchar* p = image.bits();
	    int l = image.numBytes();
	    while (l--) {
		used[*p++]++;
	    }
	    int trans=0;
	    int bestn=INT_MAX;
	    for ( int i=0; i<256; i++ ) {
		if ( used[i] < bestn ) {
		    bestn = used[i];
		    trans = i;
		    if ( !bestn )
			break;
		}
	    }
	    image.setColor( trans, image.color(trans)&0x00ffffff );
	    for ( int y=0; y<image.height(); y++ ) {
		uchar* mb = msk.scanLine(y);
		uchar* ib = image.scanLine(y);
		uchar bit = 0x80;
		int i=image.width();
		while (i--) {
		    if ( !(*mb & bit) )
			*ib = trans;
		    bit /= 2; if ( !bit ) mb++,bit = 0x80; // ROL
		    ib++;
		}
	    }
	  } break;
	  case 32: {
	    for ( int y=0; y<image.height(); y++ ) {
		uchar* mb = msk.scanLine(y);
		QRgb* ib = (QRgb*)image.scanLine(y);
		uchar bit = 0x80;
		int i=image.width();
		while (i--) {
		    if ( *mb & bit )
			*ib |= 0xff000000;
		    else
			*ib &= 0x00ffffff;
		    bit /= 2; if ( !bit ) mb++,bit = 0x80; // ROL
		    ib++;
		}
	    }
	  } break;
	}
    }

    if ( d == 1 ) {
	// Make image bit 0 come from color0, image bit 1 come form color1
	image.invertPixels();
	QRgb c0 = image.color(0);
	image.setColor(0,image.color(1));
	image.setColor(1,c0);
    }
    delete [] bmi_data;
    return image;
}


bool QPixmap::convertFromImage( const QImage &img, int conversion_flags )
{
    if ( img.isNull() ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QPixmap::convertFromImage: Cannot convert a null image" );
#endif
	return FALSE;
    }
    QImage image = img;
    int	   d     = image.depth();
    int    dd    = defaultDepth();
    bool force_mono = (dd == 1 || isQBitmap() ||
		       (conversion_flags & ColorMode_Mask)==MonoOnly );

    if ( force_mono ) {				// must be monochrome
	if ( d != 1 ) {				// dither
	    image = image.convertDepth( 1, conversion_flags );
	    d = 1;
	}
    } else {					// can be both
	bool conv8 = FALSE;
	if ( d > 8 && dd <= 8 ) {		// convert to 8 bit
	    if ( (conversion_flags & DitherMode_Mask) == AutoDither )
		conversion_flags = (conversion_flags & ~DitherMode_Mask)
					| PreferDither;
	    conv8 = TRUE;
	} else if ( (conversion_flags & ColorMode_Mask) == ColorOnly ) {
	    conv8 = d == 1;			// native depth wanted
	} else if ( d == 1 ) {
	    if ( image.numColors() == 2 ) {
		QRgb c0 = image.color(0);	// Auto: convert to best
		QRgb c1 = image.color(1);
		conv8 = QMIN(c0,c1) != qRgb(0,0,0) || QMAX(c0,c1) != qRgb(255,255,255);
	    } else {
		// eg. 1-color monochrome images (they do exist).
		conv8 = TRUE;
	    }
	}
	if ( conv8 ) {
	    image = image.convertDepth( 8, conversion_flags );
	    d = 8;
	}
    }

    if ( d == 1 )				// 1 bit pixmap (bitmap)
	image = image.convertBitOrder( QImage::BigEndian );

    int w = image.width();
    int h = image.height();

    if ( width() == w && height() == h && ( (d == 1 && depth() == 1) ||
					    (d != 1 && depth() != 1) ) ) {
	// same size etc., use the existing pixmap
	detach();
	if ( data->mask ) {			// get rid of the mask
	    delete data->mask;
	    data->mask = 0;
	}
    } else {
	// different size or depth, make a new pixmap
	QPixmap pm(w, h, d == 1 ? 1 : -1, data->bitmap, data->optim);
	*this = pm;
    }

    int	  ncols	   = image.numColors();
    char *bmi_data = new char[sizeof(BITMAPINFO)+sizeof(QRgb)*ncols];
    BITMAPINFO	     *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize		  = sizeof(BITMAPINFOHEADER);
    bmh->biWidth	  = w;
    bmh->biHeight	  = -h;
    bmh->biPlanes	  = 1;
    bmh->biBitCount	  = d;
    bmh->biCompression	  = BI_RGB;
    bmh->biSizeImage	  = image.numBytes();
    bmh->biXPelsPerMeter  = 0;
    bmh->biYPelsPerMeter  = 0;
    bmh->biClrUsed	  = ncols;
    bmh->biClrImportant	  = ncols;
    QRgb *coltbl = (QRgb*)(bmi_data + sizeof(BITMAPINFOHEADER));
    bool doAlloc = ( QApplication::colorSpec() == QApplication::CustomColor
		     && QColor::hPal() );
    for ( int i=0; i<ncols; i++ ) {		// copy color table
	RGBQUAD *r = (RGBQUAD*)&coltbl[i];
	QRgb	 c = image.color(i);
	r->rgbBlue  = qBlue ( c );
	r->rgbGreen = qGreen( c );
	r->rgbRed   = qRed  ( c );
	r->rgbReserved = 0;
	if ( doAlloc ) {
	    QColor cl( c );
	    cl.alloc();
	}
    }

    HDC dc;
    int sy;
    if ( data->mcp ) {
	dc = DATA_MCPI_MCP->handle();
	sy = DATA_MCPI_OFFSET;
    } else {
	dc = handle();
	sy = 0;
    }

#ifndef Q_OS_TEMP
    data->hasRealAlpha = //FALSE && // ### there are some problems with alpha, so don't do it at the moment
	img.hasAlphaBuffer() &&
	d==32 && // ### can we have alpha channel with depth<32bpp?
	( QApplication::winVersion() == Qt::WV_98 ||
	  QApplication::winVersion() == Qt::WV_2000 ||
	  QApplication::winVersion() == Qt::WV_XP );

    if ( data->hasRealAlpha ) {
	// Windows expects premultiplied alpha
	int l = image.numBytes();
	uchar *b = new uchar[l];
	memcpy( b, image.bits(), l );
	bool hasRealAlpha = FALSE;
	for ( int i=0; i+3<l; i+=4 ) {
	    if ( b[i+3]!=0 && b[i+3]!=255 ) {
		hasRealAlpha = TRUE;
	    }
	    b[i]   = (b[i]  *b[i+3]) / 255;
	    b[i+1] = (b[i+1]*b[i+3]) / 255;
	    b[i+2] = (b[i+2]*b[i+3]) / 255;
	}
	if ( hasRealAlpha ) {
	    void *ppvBits;
	    HBITMAP hBitmap = CreateDIBSection( dc, bmi, DIB_RGB_COLORS, &ppvBits, NULL, 0 );
	    // ### the old DATA_HBM should probably be deleted with a DeleteObject() call
	    DATA_HBM = (HBITMAP)SelectObject( dc, hBitmap );
	    memcpy( ppvBits, b, l );
	    DeleteObject( hBitmap );
	} else {
	    data->hasRealAlpha = FALSE;
	}
	delete [] b;
    }
#else
    data->hasRealAlpha = FALSE;
#endif
    if ( !data->hasRealAlpha ) {
	// "else case" of the above if (but the above can change
	// data->hasAlpha(), so we need another if for it)
#ifndef Q_OS_TEMP
	if ( dc )
	    SetDIBitsToDevice( dc, 0, sy, w, h, 0, 0, 0, h,
			       image.bits(), bmi, DIB_RGB_COLORS );
#else
	void *ppvBits;
	HDC hdcSrc = CreateCompatibleDC( dc );
	HBITMAP hBitmap = CreateDIBSection( hdcSrc, bmi, DIB_RGB_COLORS, &ppvBits, NULL, 0 );
	memcpy( ppvBits, image.bits(), image.numBytes() );
	SelectObject( hdcSrc, hBitmap );
	BitBlt( dc, 0, sy, w, h, hdcSrc, 0, 0, SRCCOPY );
	DeleteObject( hBitmap );
	DeleteDC( hdcSrc );
#endif
	if ( img.hasAlphaBuffer() ) {
	    QBitmap m;
	    m = img.createAlphaMask( conversion_flags );
	    setMask( m );
	}
    }

    delete [] bmi_data;
    data->uninit = FALSE;

    return TRUE;
}


QPixmap QPixmap::grabWindow( WId window, int x, int y, int w, int h )
{
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 ) {
	    QPixmap nullPixmap;
	    return nullPixmap;
	}
	RECT r;
	GetWindowRect( window, &r );
	if ( w < 0 )
	    w = (r.right - r.left) + 1;
	if ( h < 0 )
	    h = (r.bottom - r.top) + 1;
    }
    QPixmap pm( w, h );
    HDC dc;
    int sy;
    if ( pm.data->mcp ) {
	dc = pm.DATA_MCPI_MCP->handle();
	sy = pm.DATA_MCPI_OFFSET;
    } else {
	dc = pm.handle();
	sy = 0;
    }
    HDC src_dc = GetDC( window );
    BitBlt( dc, 0, sy, w, h, src_dc, x, y, SRCCOPY );
    ReleaseDC( window, src_dc );
    return pm;
}


QPixmap QPixmap::xForm( const QWMatrix &matrix ) const
{
    int	   w, h;				// size of target pixmap
    int	   ws, hs;				// size of source pixmap
    uchar *dptr;				// data in target pixmap
    int	   dbpl, dbytes;			// bytes per line/bytes total
    uchar *sptr;				// data in original pixmap
    int	   sbpl;				// bytes per line in original
    int	   bpp;					// bits per pixel
    bool   depth1 = depth() == 1;
    int	   y;

    if ( isNull() )				// this is a null pixmap
	return copy();

    ws = width();
    hs = height();

    QWMatrix mat = trueMatrix( matrix, ws, hs ); // true matrix

    if ( matrix.m12() == 0.0F  && matrix.m21() == 0.0F &&
	 matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F ) {
	if ( mat.m11() == 1.0F && mat.m22() == 1.0F )
	    return *this;			// identity matrix
	h = qRound( mat.m22()*hs );
	w = qRound( mat.m11()*ws );
	h = QABS( h );
	w = QABS( w );
	HDC dc;
	int sy;
	if ( data->mcp ) {
	    dc = DATA_MCPI_MCP->handle();
	    sy = DATA_MCPI_OFFSET;
	} else {
	    dc = handle();
	    sy = 0;
	}
	QPixmap pm( w, h, depth(), optimization() );
	HDC pm_dc;
	int pm_sy;
	if ( pm.data->mcp ) {
	    pm_dc = pm.multiCellHandle();
	    pm_sy = pm.multiCellOffset();
	} else {
	    pm_dc = pm.handle();
	    pm_sy = 0;
	}
#ifndef Q_OS_TEMP	
	SetStretchBltMode( pm_dc, COLORONCOLOR );
#endif
	StretchBlt( pm_dc, 0, pm_sy, w, h,	// scale the pixmap
		    dc, 0, sy, ws, hs, SRCCOPY );
	if ( data->mask ) {
	    QBitmap bm =
		data->selfmask ? *((QBitmap*)(&pm)) :
					 data->mask->xForm(matrix);
	    pm.setMask( bm );
	}
	return pm;
    } else {					// rotation or shearing
	QPointArray a( QRect(0,0,ws,hs) );
	a = mat.map( a );
	QRect r = a.boundingRect().normalize();
	w = r.width();
	h = r.height();
    }
    bool invertible;
    mat = mat.invert( &invertible );		// invert matrix

    if ( h == 0 || w == 0 || !invertible ) {	// error, return null pixmap
	QPixmap pm;
	pm.data->bitmap = data->bitmap;
	return pm;
    }

    bpp  = depth();
    if ( bpp > 1 && bpp < 8 )
	bpp = 8;

    sbpl = ((ws*bpp+31)/32)*4;
    sptr = new uchar[hs*sbpl];
    int ncols;
    if ( bpp <= 8 ) {
	ncols = 1 << bpp;
    } else {
	ncols = 0;
    }

    int	  bmi_data_len = sizeof(BITMAPINFO)+sizeof(RGBQUAD)*ncols;
    char *bmi_data = new char[bmi_data_len];
    memset( bmi_data, 0, bmi_data_len );
    BITMAPINFO	     *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize		  = sizeof(BITMAPINFOHEADER);
    bmh->biWidth	  = ws;
    bmh->biHeight	  = -hs;		// top-down bitmap
    bmh->biPlanes	  = 1;
    bmh->biBitCount	  = bpp;
    bmh->biCompression	  = BI_RGB;
    bmh->biSizeImage	  = sbpl*hs;
    bmh->biClrUsed	  = ncols;
    bmh->biClrImportant	  = 0;

    bool mcp = data->mcp;
    if ( mcp )
	((QPixmap*)this)->freeCell();
#ifndef Q_OS_TEMP
    int result = GetDIBits( qt_display_dc(), DATA_HBM, 0, hs,
			    sptr, bmi, DIB_RGB_COLORS );
#else
    int result = 0;
#endif
    if ( mcp )
	((QPixmap*)this)->allocCell();

    if ( !result ) {				// error, return null pixmap
	return QPixmap( 0, 0, 0, data->bitmap, NormalOptim );
    }

    dbpl   = ((w*bpp+31)/32)*4;
    dbytes = dbpl*h;

    dptr = new uchar[ dbytes ];			// create buffer for bits
    Q_CHECK_PTR( dptr );
    if ( depth1 )
	memset( dptr, 0xff, dbytes );
    else if ( bpp == 8 )
	memset( dptr, white.pixel(), dbytes );
    else
	memset( dptr, 0xff, dbytes );

    int m11 = qRound((double)mat.m11()*65536.0);
    int m12 = qRound((double)mat.m12()*65536.0);
    int m21 = qRound((double)mat.m21()*65536.0);
    int m22 = qRound((double)mat.m22()*65536.0);
    int dx  = qRound((double)mat.dx() *65536.0);
    int dy  = qRound((double)mat.dy() *65536.0);

    int	  m21ydx = dx, m22ydy = dy;
    uint  trigx, trigy;
    uint  maxws = ws<<16, maxhs=hs<<16;
    uchar *p	= dptr;
    int	  xbpl, p_inc;

    if ( depth1 ) {
	xbpl  = (w+7)/8;
	p_inc = dbpl - xbpl;
    } else {
	xbpl  = (w*bpp)/8;
	p_inc = dbpl - xbpl;
    }

    for ( y=0; y<h; y++ ) {			// for each target scanline
	trigx = m21ydx;
	trigy = m22ydy;
	uchar *maxp = p + xbpl;
	if ( !depth1 ) {
	    switch ( bpp ) {
		case 8:				// 8 bpp transform
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs )
			*p = *(sptr+sbpl*(trigy>>16)+(trigx>>16));
		    trigx += m11;
		    trigy += m12;
		    p++;
		}
		break;

		case 16:			// 16 bpp transform
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs )
			*((ushort*)p) = *((ushort *)(sptr+sbpl*(trigy>>16) +
						     ((trigx>>16)<<1)));
		    trigx += m11;
		    trigy += m12;
		    p++;
		    p++;
		}
		break;

		case 24: {			// 24 bpp transform
		uchar *p2;
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs ) {
			p2 = sptr+sbpl*(trigy>>16) + ((trigx>>16)*3);
			p[0] = p2[0];
			p[1] = p2[1];
			p[2] = p2[2];
		    }
		    trigx += m11;
		    trigy += m12;
		    p += 3;
		}
		}
		break;

		case 32:			// 32 bpp transform
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs )
			*((uint*)p) = *((uint *)(sptr+sbpl*(trigy>>16) +
						   ((trigx>>16)<<2)));
		    trigx += m11;
		    trigy += m12;
		    p += 4;
		}
		break;

		default: {
#if defined(QT_CHECK_RANGE)
		qWarning( "QPixmap::xForm: Display not supported (bpp=%d)",bpp);
#endif
		return QPixmap( 0, 0, 0, data->bitmap, data->optim );
		}
	    }
	} else {				// mono bitmap LSB first
	    while ( p < maxp ) {
#undef IWX
#define IWX(b,t) if ( trigx < maxws && trigy < maxhs ) {		      \
		    if ( (*(sptr+sbpl*(trigy>>16)+(trigx>>19)) &	      \
			 (1 << (7-((trigx>>16)&7)))) == 0 )		      \
			*p &= ~t;					      \
		}							      \
		trigx += m11;						      \
		trigy += m12;
	// END OF MACRO
		IWX(1,128);
		IWX(2,64);
		IWX(4,32);
		IWX(8,16);
		IWX(16,8);
		IWX(32,4);
		IWX(64,2);
		IWX(128,1);
		p++;
	    }
	}
	m21ydx += m21;
	m22ydy += m22;
	p += p_inc;
    }

    delete [] sptr;

    QPixmap pm( w, h, depth(), data->bitmap, optimization() );
    HDC pm_dc;
    int pm_sy;
    if ( pm.data->mcp ) {
	pm_dc = pm.multiCellHandle();
	pm_sy = pm.multiCellOffset();
    } else {
	pm_dc = pm.handle();
	pm_sy = 0;
    }
    pm.data->uninit = FALSE;
    bmh->biWidth  = w;
    bmh->biHeight = -h;
    bmh->biSizeImage = dbytes;
#ifndef Q_OS_TEMP
    SetDIBitsToDevice( pm_dc, 0, pm_sy, w, h, 0, 0, 0, h,
		       dptr, bmi, DIB_RGB_COLORS );
#else
    void *ppvBits;
    HDC hdcSrc = CreateCompatibleDC( pm.handle() );
    HBITMAP hBitmap = CreateDIBSection( hdcSrc, bmi, DIB_RGB_COLORS, &ppvBits, NULL, 0 );
    memcpy( ppvBits, dptr, dbytes );
    SelectObject( hdcSrc, hBitmap );
    BitBlt( pm.handle(), 0, 0, w, h, hdcSrc, 0, 0, SRCCOPY );
    DeleteObject( hBitmap );
    DeleteDC( hdcSrc );
#endif
    delete [] bmi_data;
    delete [] dptr;
    if ( data->mask ) {
	QBitmap bm = data->selfmask ? *((QBitmap*)(&pm)) :
				     data->mask->xForm(matrix);
	pm.setMask( bm );
    }
    return pm;
}

/*!
  \fn HBITMAP QPixmap::hbm() const
  \internal
*/

/*!
  \fn bool QPixmap::isMultiCellPixmap() const
  \internal
*/

/*!
  \internal
*/
HDC QPixmap::multiCellHandle() const
{
    return data->mcp ? DATA_MCPI_MCP->handle() : 0;
}

/*!
  \internal
*/
HBITMAP QPixmap::multiCellBitmap() const
{
    return data->mcp ? DATA_MCPI_MCP->hbm() : 0;
}

/*!
  \internal
*/
int QPixmap::multiCellOffset() const
{
    return data->mcp ? DATA_MCPI_OFFSET : 0;
}


/*
  Implementation of internal QMultiCellPixmap class.
*/

QMultiCellPixmap::QMultiCellPixmap( int width, int depth, int maxHeight )
{
    // Start with a small pixmap first and expand as needed
    int height = width * 4;
    // Set def optim to normal to avoid recursion
    pixmap = new QPixmap( width, height, depth, QPixmap::NormalOptim );
    pixmap->detach();				// clears uninit flag
    max_height = maxHeight;
    free_list = new QMCPFreeList;
    free_list->setAutoDelete( TRUE );
    // The whole pixmap area can be allocated
    free_list->append( new QMCPFreeNode(0,max_height) );
}

QMultiCellPixmap::~QMultiCellPixmap()
{
    delete free_list;
    delete pixmap;
}

int QMultiCellPixmap::allocCell( int height )
{
    QMCPFreeNode *n = free_list->first();
    while ( n && n->size < height )		// find free space
	n = free_list->next();
    if ( !n )					// not enough space
	return -1;
    int offset = n->offset;
    if ( n->size > height ) {			// alloc part of free space
	n->offset += height;
	n->size -= height;
    } else {					// perfect fit, height == size
	Q_ASSERT( n->size == height );
	free_list->remove();			// remove the node
    }
    int pm_height = pixmap->height();
    while ( offset + height > pm_height )
	pm_height *= 2;
    if ( pm_height > pixmap->height() )		// expand pixmap
	pixmap->resize( pixmap->width(), pm_height );
    return offset;
}

void QMultiCellPixmap::freeCell( int offset, int size )
{
    QMCPFreeNode *n = free_list->first();
    QMCPFreeNode *p = 0;
    while ( n && n->offset < offset ) {
	p = n;
	n = free_list->next();
    }
    if ( p && p->offset + p->size == offset ) {
	// The previous free node is adjacent to the cell we are freeing up,
	// then expand the size of the prev node to include this space.
	p->size += size;
	if ( n && p->offset + p->size == n->offset ) {
	    // If the next node comes after the prev node, collapse them.
	    p->size += n->size;
	    free_list->remove();	// removes the current node (i.e. n)
	}
    } else if ( n ) {
	// We have found the first free node after the cell.
	if ( offset + size == n->offset ) {
	    // The next free node comes right after the freed up area, then
	    // include this area.
	    n->offset -= size;
	    n->size   += size;
	} else {
	    // Insert a new free node before this one.
	    free_list->insert( free_list->at(),
			       new QMCPFreeNode(offset,size) );
	}
    } else {
	// n == 0, this means the free_list is empty or that the cell is
	// after the last free block (but still not adjacent to p),
	// then append a new free node.
	free_list->append( new QMCPFreeNode(offset,size) );
    }
}


/*
  We have internal lists of QMultiCellPixmaps; 4 for color pixmaps and
  4 for mono pixmaps/bitmaps.  There is one list for pixmaps with a
  width 1..16, 17..32, 33..64 and 65..128.
*/

typedef QPtrList<QMultiCellPixmap>  QMultiCellPixmapList;
typedef QMultiCellPixmapList   *pQMultiCellPixmapList;

static const int mcp_num_lists  = 8;
static bool	 mcp_lists_init = FALSE;
static pQMultiCellPixmapList mcp_lists[mcp_num_lists];

static void cleanup_mcp()
{
    if ( mcp_lists_init ) {
	mcp_system_unstable = TRUE;		// tell QPixmap::deref()
	mcp_lists_init = FALSE;
	for ( int i=0; i<mcp_num_lists; i++ ) {
	    delete mcp_lists[i];
	    mcp_lists[i] = 0;
	}	
    }
}

static void init_mcp()
{
    if ( !mcp_lists_init ) {
	mcp_lists_init = TRUE;
	for ( int i=0; i<mcp_num_lists; i++ )
	    mcp_lists[i] = 0;
	qAddPostRoutine( cleanup_mcp );
    }
}

static int index_of_mcp_list( int width, bool mono, int *size=0 )
{
    if ( width <= 0 )
	return -1;
    int i;
    int s = 16;
    for ( i=0; i<4; i++ ) {			// try s=16,32,64,128
	if ( width <= s )			// index= 0, 1, 2,  3
	    break;
	s *= 2;
    }
    if ( i == 4 )				// too big pixmap
	return -1;
    if ( mono )					// mono: index 4, 5, ...
	i += 4;
    if ( size )
	*size = s;
    return i;
}


/*!
  \internal
*/
int QPixmap::allocCell()
{
    if ( qt_winver & WV_NT_based )		// only for NT based systems
	return -1;
    if ( !mcp_lists_init )
	init_mcp();
    if ( data->mcp )				// cell already alloc'd
	freeCell();
    int s;
    int i = index_of_mcp_list(width(), depth() == 1, &s);
    if ( i < 0 )				// too large width
	return -1;
    if ( !mcp_lists[i] ) {
	mcp_lists[i] = new QMultiCellPixmapList;
	mcp_lists[i]->setAutoDelete( TRUE );
    }
    QMultiCellPixmapList *list = mcp_lists[i];
    QMultiCellPixmap     *mcp  = list->first();
    int offset = -1;
    while ( mcp && offset < 0 ) {
	offset = mcp->allocCell( height() );
	if ( offset < 0 )
	    mcp = list->next();
    }
    if ( offset < 0 ) {				// could not alloc
	mcp = new QMultiCellPixmap( s, depth(), 2048 );
	Q_CHECK_PTR( mcp );
	offset = mcp->allocCell( height() );
	if ( offset < 0 ) {			// height() > total height
	    delete mcp;
	    return offset;
	}
	list->append( mcp );
    }
    if ( hdc ) {				// copy into multi cell pixmap
	BitBlt( mcp->handle(), 0, offset, width(), height(), hdc,
		0, 0, SRCCOPY );
	DeleteDC( hdc );
	hdc = 0;
	DeleteObject( DATA_HBM );
	DATA_HBM = 0;
    }
    data->mcp = TRUE;
    DATA_MCPI = new QMCPI;
    Q_CHECK_PTR( DATA_MCPI );
    DATA_MCPI_MCP = mcp;
    DATA_MCPI_OFFSET = offset;
    return offset;
}


/*!
  \internal
*/
void QPixmap::freeCell( bool terminate )
{
    if ( !mcp_lists_init || !data->mcp )
	return;
    QMultiCellPixmap *mcp = DATA_MCPI_MCP;
    int offset = DATA_MCPI_OFFSET;
    data->mcp = FALSE;
    delete DATA_MCPI;
    DATA_MCPI = 0;
    Q_ASSERT( hdc == 0 );
    if ( terminate ) {				// pixmap is being destroyed
	DATA_HBM = 0;
    } else {
	if ( data->d == defaultDepth() )
	    DATA_HBM = CreateCompatibleBitmap( qt_display_dc(), data->w, data->h);
	else
	    DATA_HBM = CreateBitmap( data->w, data->h, 1, 1, 0 );
	hdc = alloc_mem_dc( DATA_HBM );
	BitBlt( hdc, 0, 0, data->w, data->h, mcp->handle(), 0, offset, SRCCOPY );
    }
    mcp->freeCell( offset, data->h );
    if ( mcp->isEmpty() ) {			// no more cells left
	int i = index_of_mcp_list(width(),depth()==1,0);
	Q_ASSERT( i >= 0 && mcp_lists[i] );
	if ( mcp_lists[i]->count() > 1 ) {	// don't remove the last one
	    mcp_lists[i]->remove( mcp );
	    if ( mcp_lists[i]->isEmpty() ) {
		delete mcp_lists[i];
		mcp_lists[i] = 0;
	    }
	}
    }
}

void QMultiCellPixmap::debugger()
{
    qDebug( "  Multi cell pixmap %d x %d x %d (%p)",
	   pixmap->width(), max_height, pixmap->depth(), this );
    qDebug( "    Actual pixmap height = %d", pixmap->height() );
    qDebug( "    Free List" );
    QMCPFreeNode *n = free_list->first();
    while ( n  ) {
	qDebug( "      Offset %4d, Size %3d", n->offset, n->size );
	n = free_list->next();
    }
    qDebug( "      Num free nodes = %d", free_list->count() );
}


Q_EXPORT void qt_mcp_debugger()
{
    int i, s=16;
    const char *info = "pixmaps";
    bool nothing = TRUE;
    qDebug( "MCP DEBUGGER" );
    qDebug( "------------" );
    for ( i=0; i<mcp_num_lists; i++ ) {
	if ( i == 5 ) {
	    s = 16;
	    info = "mono pixmaps";
	}
	if ( mcp_lists[i] ) {
	    nothing = FALSE;
	    qDebug( "Multi cell list %d, %s, size<=%d, number of lists = %d",
		   i, info, s, mcp_lists[i]->count() );
	    QMultiCellPixmap *mcp = mcp_lists[i]->first();
	    while ( mcp ) {
		mcp->debugger();
		mcp = mcp_lists[i]->next();
	    }
	}
    }
    if ( nothing )
	qDebug( "No internal info" );
    qDebug( "MCP DONE\n" );
}
