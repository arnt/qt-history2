/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpm_win.cpp#43 $
**
** Implementation of QPixmap class for Win32
**
** Created : 940501
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qbitmap.h"
#include "qimage.h"
#include "qpaintdc.h"
#include "qwmatrix.h"
#include "qapp.h"

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qpm_win.cpp#43 $");


extern uchar *qt_get_bitflip_array();		// defined in qimage.cpp

bool QPixmap::optimAll = TRUE;


void QPixmap::init( int w, int h, int d )
{
    static int serial = 0;
    int dd = defaultDepth();

    data = new QPixmapData;
    CHECK_PTR( data );

    data->dirty	   = FALSE;
    data->optim	   = optimAll;
    data->uninit   = TRUE;
    data->bitmap   = FALSE;
    data->selfmask = FALSE;
    data->ser_no   = ++serial;
    data->mask	   = 0;
    data->hbm	   = 0;

    bool make_null = w == 0 || h == 0;		// create null pixmap
    if ( d == 1 )				// monocrome pixmap
	data->d = 1;
    else if ( d < 0 || d == dd )		// compatible pixmap
	data->d = dd;
    else
	data->d = 0;
    if ( make_null || w < 0 || h < 0 || data->d == 0 ) {
	data->w = data->h = 0;
	data->d = 0;
#if defined(CHECK_RANGE)
	if ( !make_null )			// invalid parameters
	    warning( "QPixmap: Invalid pixmap parameters" );
#endif
	return;
    }
    data->w = w;
    data->h = h;
    data->bits = 0;

    if ( data->d == dd ) {			// compatible bitmap
	HANDLE hdc = GetDC( 0 );
#if 1
	data->hbm  = CreateCompatibleBitmap( hdc, w, h );
	data->bits = 0;
#else
	BITMAPINFOHEADER b;
	memset( &b, 0, sizeof(b) );
	b.biSize = sizeof(BITMAPINFOHEADER);
	b.biWidth = w;
	b.biHeight = h;
	b.biPlanes = 1;
	b.biBitCount = data->d;
	b.biCompression = BI_RGB;
	data->hbm = CreateDIBSection( hdc, (BITMAPINFO*)&b, DIB_PAL_COLORS,
				      &data->bits, 0, 0 );
#endif
	ReleaseDC( 0, hdc );
    } else {					// monocrome bitmap
	data->hbm = CreateBitmap( w, h, 1, 1, 0 );
    }
    if ( data->optim )
	allocMemDC();
}


QPixmap::QPixmap( int w, int h, const uchar *bits, bool isXbitmap )
    : QPaintDevice( PDT_PIXMAP )
{						// for bitmaps only
    init( 0, 0, 0 );
    if ( w <= 0 || h <= 0 )			// create null pixmap
	return;

    data->uninit = FALSE;
    data->w = w;  data->h = h;	data->d = 1;

    int bitsbpl = (w+7)/8;			// original # bytes per line
    int bpl	= ((w+15)/16)*2;		// bytes per scanline
    uchar *newbits = new uchar[bpl*h];
    uchar *p	= newbits;
    int x, y, pad;
    pad = bpl - bitsbpl;
    if ( isXbitmap ) {				// flip and invert
	uchar *f = qt_get_bitflip_array();
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
    data->hbm = CreateBitmap( w, h, 1, 1, newbits );
    if ( data->optim )
	allocMemDC();
    delete [] newbits;
}

QPixmap::QPixmap( const QPixmap &pixmap )
    : QPaintDevice( PDT_PIXMAP )
{
    if ( pixmap.paintingActive() ) {		// make a deep copy
	data = 0;
	operator=( pixmap );
    } else {
	data = pixmap.data;
	data->ref();
	devFlags = pixmap.devFlags;		// copy QPaintDevice flags
	hdc = pixmap.hdc;			// copy QPaintDevice hdc
    }
}

QPixmap::~QPixmap()
{
    if ( data->deref() ) {			// last reference lost
	freeMemDC();
	if ( data->mask )
	    delete data->mask;
	if ( data->hbm )
	    DeleteObject( data->hbm );
	delete data;
    }
}


HANDLE QPixmap::allocMemDC()
{
    if ( !hdc && !isNull() ) {
	HANDLE hdcScreen = GetDC( 0 );
	hdc = CreateCompatibleDC( hdcScreen );
	if ( QColor::hPal() ) {
	    SelectPalette( hdc, QColor::hPal(), FALSE );
	    RealizePalette( hdc );
	}
	SelectObject( hdc, data->hbm );
	ReleaseDC( 0, hdcScreen );
    }
    return hdc;
}

void QPixmap::freeMemDC()
{
    if ( hdc ) {
	DeleteDC( hdc );
	hdc = 0;
    }
}


QPixmap &QPixmap::operator=( const QPixmap &pixmap )
{
    if ( paintingActive() ) {
#if defined(CHECK_STATE)
	warning("QPixmap::operator=: Cannot assign to pixmap during painting");
#endif
	return *this;
    }
    pixmap.data->ref();				// avoid 'x = x'
    if ( data && data->deref() ) {		// last reference lost
	freeMemDC();
	if ( data->mask )
	    delete data->mask;
	if ( data->hbm )
	    DeleteObject( data->hbm );
	delete data;
    }
    if ( pixmap.paintingActive() ) {		// make a deep copy
	init( pixmap.width(), pixmap.height(), pixmap.depth() );
	data->dirty  = FALSE;
	data->uninit = FALSE;
	data->optim  = pixmap.data->optim;	// copy optim flag
	data->bitmap = pixmap.data->bitmap;	// copy bitmap flag
	if ( !isNull() ) {
	    bitBlt( this, 0, 0, &pixmap, pixmap.width(), pixmap.height(),
		    CopyROP, TRUE );
	    if ( pixmap.mask() )
		setMask( *pixmap.mask() );
	}
	pixmap.data->deref();
    } else {
	data = pixmap.data;
	devFlags = pixmap.devFlags;		// copy QPaintDevice flags
	hdc = pixmap.hdc;			// copy QPaintDevice drawable
    }
    return *this;
}


int QPixmap::defaultDepth()
{
    static int dd = 0;
    if ( dd == 0 ) {
	HANDLE hdc = GetDC( 0 );
	dd = GetDeviceCaps( hdc, BITSPIXEL );
	ReleaseDC( 0, hdc );
    }
    return dd;
}


void QPixmap::optimize( bool enable )
{
    if ( enable == (bool)data->optim )
	return;
    data->optim = enable;
    data->dirty = FALSE;
    if ( paintingActive() )			// wait until QPainter::end()
	return;
    if ( enable )
	allocMemDC();
    else
	freeMemDC();
}

bool QPixmap::isGloballyOptimized()
{
    return optimAll;
}

void QPixmap::optimizeGlobally( bool enable )
{
    optimAll = enable;
}


void QPixmap::fill( const QColor &fillColor )
{
    if ( isNull() )
	return;
    detach();					// detach other references
    bool tmp_hdc = hdc == 0;
    if ( tmp_hdc )
	allocMemDC();
    if ( fillColor == black ) {
	PatBlt( hdc, 0, 0, data->w, data->h, BLACKNESS );
    } else if ( fillColor == white ) {
	PatBlt( hdc, 0, 0, data->w, data->h, WHITENESS );
    } else {
	HANDLE hbrush = CreateSolidBrush( fillColor.pixel() );
	HANDLE hb_old = SelectObject( hdc, hbrush );
	PatBlt( hdc, 0, 0, width(), height(), PATCOPY );
	SelectObject( hdc, hb_old );
	DeleteObject( hbrush );
    }
    if ( tmp_hdc )
	freeMemDC();
}


int QPixmap::metric( int m ) const
{
    int val;
    if ( m == PDM_WIDTH || m == PDM_HEIGHT ) {
	if ( m == PDM_WIDTH )
	    val = width();
	else
	    val = height();
    } else {
	HDC gdc = GetDC( 0 );
	switch ( m ) {
	    case PDM_WIDTHMM:
		val = GetDeviceCaps( gdc, HORZSIZE );
		break;
	    case PDM_HEIGHTMM:
		val = GetDeviceCaps( gdc, VERTSIZE );
		break;
	    case PDM_NUMCOLORS:
		if ( GetDeviceCaps(gdc, RASTERCAPS) & RC_PALETTE )
		    val = GetDeviceCaps( gdc, SIZEPALETTE );
		else
		    val = GetDeviceCaps( gdc, NUMCOLORS );
		break;
	    case PDM_DEPTH:
		val = GetDeviceCaps( gdc, PLANES );
		break;
	    default:
		val = 0;
#if defined(CHECK_RANGE)
		warning( "QPixmap::metric: Invalid metric command" );
#endif
	}
	ReleaseDC( 0, gdc );
    }
    return val;
}


extern bool qt_image_did_native_bmp();		// defined in qpixmap.cpp


QImage QPixmap::convertToImage() const
{
    if ( isNull() ) {
#if defined(CHECK_NULL)
	warning( "QPixmap::convertToImage: Cannot convert a null pixmap" );
#endif
	QImage nullImage;
	return nullImage;
    }

    int	w = width();
    int	h = height();
    int	d = depth();
    int	ncols = 2;

    if ( d > 1 && d <= 8 ) {			// set to nearest valid depth
	d = 8;					//   2..7 ==> 8
	ncols = 256;
    } else if ( d > 8 ) {
	d = 32;					//   > 8  ==> 32
	ncols = 0;
    }

    QImage image( w, h, d, ncols, QImage::BigEndian );
    bool   native = qt_image_did_native_bmp();

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

    GetDIBits( handle(), hbm(), 0, h, image.bits(), bmi, DIB_RGB_COLORS );
    if ( !native && d == 32 ) {
	for ( int i=0; i<image.height(); i++ ) {
	    uint *p = (uint*)image.scanLine(i);
	    uint *end = p + image.width();
	    while ( p < end ) {
		*p = ((*p << 16) & 0xff0000) | ((*p >> 16) & 0xff) |
		    *p & 0xff00;
		p++;
	    }
	}
    }

    for ( int i=0; i<ncols; i++ ) {		// copy color table
	RGBQUAD *r = (RGBQUAD*)&coltbl[i];
	image.setColor( i, qRgb(r->rgbRed,
				r->rgbGreen,
				r->rgbBlue) );
    }
    delete [] bmi_data;
    return image;
}


bool QPixmap::convertFromImage( const QImage &img, ColorMode mode )
{
    if ( img.isNull() ) {
#if defined(CHECK_NULL)
	warning( "QPixmap::convertFromImage: Cannot convert a null image" );
#endif
	return FALSE;
    }
    QImage image = img;
    int	   d = image.depth();
    bool   force_mono = (isQBitmap() || mode == Mono);

    if ( force_mono ) {				// must be monochrome
	if ( d != 1 ) {
	    image = image.convertDepth( 1 );	// dither
	    d = 1;
	}
    } else {					// can be both
	bool conv8 = FALSE;
	if ( mode == Color ) {			// native depth wanted
	    conv8 = d == 1;
	} else if ( d == 1 && image.numColors() == 2 ) {
	    QRgb c0 = image.color(0);		// mode==Auto: convert to best
	    QRgb c1 = image.color(1);
	    conv8 = QMIN(c0,c1) != 0 || QMAX(c0,c1) != qRgb(255,255,255);
	}
	if ( conv8 ) {
	    image = image.convertDepth( 8 );
	    d = 8;
	}
    }

    if ( d == 1 )				// 1 bit pixmap (bitmap)
	image = image.convertBitOrder( QImage::BigEndian );

    int w = image.width();
    int h = image.height();

    QPixmap pm( w, h, d == 1 ? 1 : -1 );
    bool tmp_dc = pm.handle() == 0;
    if ( tmp_dc )
	pm.allocMemDC();
    pm.data->optim  = data->optim;		// keep optimization flag
    pm.data->bitmap = data->bitmap;		// keep is-a flag
    if ( pm.data->optim )
	tmp_dc = FALSE;				// don't do pm.freeMemDC()

    bool native = qt_image_did_native_bmp();

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
    for ( int i=0; i<ncols; i++ ) {		// copy color table
	RGBQUAD *r = (RGBQUAD*)&coltbl[i];
	QRgb	 c = image.color(i);
	r->rgbBlue  = qBlue ( c );
	r->rgbGreen = qGreen( c );
	r->rgbRed   = qRed  ( c );
	r->rgbReserved = 0;
    }
    if ( !native && d == 32 ) {
	for ( int i=0; i<image.height(); i++ ) {
	    uint *p = (uint*)image.scanLine(i);
	    uint *end = p + image.width();
	    while ( p < end ) {
		*p = ((*p << 16) & 0xff0000) | ((*p >> 16) & 0xff) |
		    *p & 0xff00;
		p++;
	    }
	}
    }
    SetDIBitsToDevice( pm.handle(), 0, 0, w, h, 0, 0, 0, h,
		       image.bits(), bmi, DIB_RGB_COLORS );
    if ( !native && d == 32 ) {
	for ( int i=0; i<image.height(); i++ ) {
	    uint *p = (uint*)image.scanLine(i);
	    uint *end = p + image.width();
	    while ( p < end ) {
		*p = ((*p << 16) & 0xff0000) | ((*p >> 16) & 0xff) |
		    *p & 0xff00;
		p++;
	    }
	}
    }
    delete [] bmi_data;

    pm.data->uninit = FALSE;
    if ( tmp_dc )
	pm.freeMemDC();
    *this = pm;

    if ( image.hasAlphaBuffer() ) {
	QBitmap m;
	m = image.createAlphaMask();
	setMask( m );
    }

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
    bool tmp_hdc = pm.hdc == 0;
    if ( tmp_hdc )
	pm.allocMemDC();
    HANDLE src_dc = GetDC( window );
    BitBlt( pm.hdc, 0, 0, w, h, src_dc, x, y, SRCCOPY );
    ReleaseDC( window, src_dc );
    if ( tmp_hdc )
	pm.freeMemDC();
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
    bool   src_tmp, dst_tmp;

    if ( isNull() )				// this is a null pixmap
	return copy();

    ws = width();
    hs = height();

    const float dt = 0.0001;
    float x1,y1, x2,y2, x3,y3, x4,y4;		// get corners
    float xx = (float)ws - 1;
    float yy = (float)hs - 1;

    matrix.map( dt, dt, &x1, &y1 );
    matrix.map( xx, dt, &x2, &y2 );
    matrix.map( xx, yy, &x3, &y3 );
    matrix.map( dt, yy, &x4, &y4 );

    float ymin = y1;				// lowest y value
    if ( y2 < ymin ) ymin = y2;
    if ( y3 < ymin ) ymin = y3;
    if ( y4 < ymin ) ymin = y4;
    float xmin = x1;				// lowest x value
    if ( x2 < xmin ) xmin = x2;
    if ( x3 < xmin ) xmin = x3;
    if ( x4 < xmin ) xmin = x4;

    QWMatrix mat( 1, 0, 0, 1, -xmin, -ymin );	// true matrix
    mat = matrix * mat;

    QPixmap *self = (QPixmap *)this;

    if ( matrix.m12() == 0.0F  && matrix.m21() == 0.0F &&
	 matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F ) {
	if ( mat.m11() == 1.0F && mat.m22() == 1.0F )
	    return *this;			// identity matrix
	h = qRound( mat.m22()*hs );
	w = qRound( mat.m11()*ws );
	h = QABS( h );
	w = QABS( w );
	if ( !self->handle() ) {
	    self->allocMemDC();
	    src_tmp = TRUE;
	} else {
	    src_tmp = FALSE;
	}
	QPixmap pm( w, h, depth() );		// scale the pixmap
	if ( !pm.handle() ) {
	    pm.allocMemDC();
	    dst_tmp = TRUE;
	} else {
	    dst_tmp = FALSE;
	}
	SetStretchBltMode( pm.handle(), COLORONCOLOR );
	StretchBlt( pm.handle(), 0, 0, w, h, self->handle(),
		    0, 0, ws, hs, SRCCOPY );
	if ( dst_tmp )
	    pm.freeMemDC();
	if ( src_tmp )
	    self->freeMemDC();
	QPixmap *m = data->mask;
	if ( m ) {
	    if ( m->data == data )		// pixmap == mask
		pm.setMask( *((QBitmap*)(&pm)) );
	    else
		pm.setMask( data->mask->xForm(matrix) );
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

    if ( !self->handle() ) {
	self->allocMemDC();
	src_tmp = TRUE;
    } else {
	src_tmp = FALSE;
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
    QRgb *coltbl = (QRgb*)(bmi_data + sizeof(BITMAPINFOHEADER));

    int result;
    result = GetDIBits( handle(), hbm(), 0, hs, sptr, bmi, DIB_RGB_COLORS );

    if ( src_tmp )
	self->freeMemDC();

    if ( !result ) {				// error, return null pixmap
	QPixmap pm;
	pm.data->bitmap = data->bitmap;
	return pm;
    }

    dbpl   = ((w*bpp+31)/32)*4;
    dbytes = dbpl*h;

    dptr = new uchar[ dbytes ];			// create buffer for bits
    CHECK_PTR( dptr );
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
    uchar *flip = qt_get_bitflip_array();
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
#if defined(CHECK_RANGE)
		warning( "QPixmap::xForm: Display not supported (bpp=%d)",bpp);
#endif
		QPixmap pm;
		return pm;
		}
	    }
	} else {				// mono bitmap LSB first
	    while ( p < maxp ) {
#undef IWX
#define IWX(b,t) if ( trigx < maxws && trigy < maxhs ) {		      \
		    if ( (*(sptr+sbpl*(trigy>>16)+(trigx>>19)) &	      \
			 (1 << 7-((trigx>>16)&7))) == 0 )		      \
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

    QPixmap pm( w, h, depth() );
    pm.data->uninit = FALSE;
    pm.data->bitmap = data->bitmap;
    if ( !pm.handle() ) {
	pm.allocMemDC();
	dst_tmp = TRUE;
    } else {
	dst_tmp = FALSE;
    }
    bmh->biWidth  = w;
    bmh->biHeight = -h;
    bmh->biSizeImage = dbytes;
    SetDIBitsToDevice( pm.handle(), 0, 0, w, h, 0, 0, 0, h,
		       dptr, bmi, DIB_RGB_COLORS );
    delete [] bmi_data;
    delete [] dptr;
    if ( dst_tmp )
	pm.freeMemDC();
    if ( data->mask ) {
	if ( data->selfmask )			// pixmap == mask
	    pm.setMask( *((QBitmap*)(&pm)) );
	else
	    pm.setMask( data->mask->xForm(matrix) );
    }
    return pm;
}


QWMatrix QPixmap::trueMatrix( const QWMatrix &matrix, int w, int h )
{						// get true wxform matrix
    const float dt = 0.0001F;
    float x1,y1, x2,y2, x3,y3, x4,y4;		// get corners
    float xx = (float)w - 1;
    float yy = (float)h - 1;

    matrix.map( dt, dt, &x1, &y1 );
    matrix.map( xx, dt, &x2, &y2 );
    matrix.map( xx, yy, &x3, &y3 );
    matrix.map( dt, yy, &x4, &y4 );

    float ymin = y1;				// lowest y value
    if ( y2 < ymin ) ymin = y2;
    if ( y3 < ymin ) ymin = y3;
    if ( y4 < ymin ) ymin = y4;
    float xmin = x1;				// lowest x value
    if ( x2 < xmin ) xmin = x2;
    if ( x3 < xmin ) xmin = x3;
    if ( x4 < xmin ) xmin = x4;

    QWMatrix mat( 1.0F, 0.0F, 0.0F, 1.0F, -xmin, -ymin );
    mat = matrix * mat;
    return mat;
}
