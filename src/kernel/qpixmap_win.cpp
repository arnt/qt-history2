/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap_win.cpp#23 $
**
** Implementation of QPixmap class for Windows
**
** Author  : Haavard Nord
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
#include <windows.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qpixmap_win.cpp#23 $")


bool QPixmap::optimAll = TRUE;


void QPixmap::init( int w, int h, int d )
{
    static int serial = 0;
    int dd = defaultDepth();

    data = new QPixmapData;
    CHECK_PTR( data );

    data->dirty	 = FALSE;
    data->optim	 = optimAll;
    data->uninit = TRUE;
    data->bitmap = FALSE;
    data->ser_no = ++serial;
    data->mask	 = 0;
    data->hbm	 = 0;

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
    }
    else					// monocrome bitmap
	data->hbm = CreateBitmap( w, h, 1, 1, 0 );
    if ( data->optim )
	allocMemDC();
}


QPixmap::QPixmap()
    : QPaintDevice( PDT_PIXMAP )
{
    init( 0, 0, 0 );
}

QPixmap::QPixmap( int w, int h, int depth )
    : QPaintDevice( PDT_PIXMAP )
{
    init( w, h, depth );
}

QPixmap::QPixmap( const QSize &size, int depth )
    : QPaintDevice( PDT_PIXMAP )
{
    init( size.width(), size.height(), depth );
}

QPixmap::QPixmap( int w, int h, const char *bits, bool isXbitmap )
    : QPaintDevice( PDT_PIXMAP )
{						// for bitmaps only
    extern char *qt_get_bitflip_array();	// defined in qimage.cpp
    init( 0, 0, 0 );
    if ( w <= 0 || h <= 0 )			// create null pixmap
	return;

    data->uninit = FALSE;
    data->w = w;  data->h = h;	data->d = 1;

    int bitsbpl = (w+7)/8;			// original # bytes per line
    int bpl	= ((w+15)/16)*2;		// 16-bit aligned bpl
    int nbytes	= bpl*h;
    char *newbits = new char[nbytes];
    char *p	= newbits;
    int x, y, pad;
    pad = bpl - bitsbpl;
    if ( isXbitmap ) {				// flip and invert
	char *f = qt_get_bitflip_array();
	if ( pad == 0 ) {
	    while ( nbytes-- )
		*p++ = ~f[*bits++];
	}
	else {
	    for ( y=0; y<h; y++ ) {
		for ( x=0; x<bitsbpl; x++ )
		    *p++ = ~f[*bits++];
		for ( x=0; x<pad; x++ )
		    *p++ = 0;
	    }
	}
    }
    else {					// invert all bits
	if ( pad == 0 ) {
	    while ( nbytes-- )
		*p++ = ~(*bits++);
	}
	else {
	    for ( y=0; y<h; y++ ) {
		for ( x=0; x<bitsbpl; x++ )
		    *p++ = ~(*bits++);
		for ( x=0; x<pad; x++ )
		    *p++ = 0;
	    }
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
    data = pixmap.data;
    data->ref();
    devFlags = pixmap.devFlags;			// copy QPaintDevice flags
    hdc = pixmap.hdc;				// copy QPaintDevice hdc
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
    pixmap.data->ref();				// avoid 'x = x'
    if ( data->deref() ) {			// last reference lost
	freeMemDC();
	if ( data->mask )
	    delete data->mask;
	if ( data->hbm )
	    DeleteObject( data->hbm );
	delete data;
    }
    data = pixmap.data;
    devFlags = pixmap.devFlags;			// copy QPaintDevice flags
    hdc = pixmap.hdc;				// copy QPaintDevice drawable
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


bool QPixmap::isOptimized() const
{
    return data->optim;
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
    if ( fillColor == black )
	PatBlt( hdc, 0, 0, data->w, data->h, BLACKNESS );
    else if ( fillColor == white )
	PatBlt( hdc, 0, 0, data->w, data->h, WHITENESS );
    else {
	HANDLE hbrush = CreateSolidBrush( fillColor.pixel() );
	HANDLE hb_old = SelectObject( hdc, hbrush );
	PatBlt( hdc, 0, 0, width(), height(), PATCOPY );
	SelectObject( hdc, hb_old );
	DeleteObject( hbrush );
    }
    if ( tmp_hdc )
	freeMemDC();
}


long QPixmap::metric( int m ) const		// get metric information
{
    long val;
    if ( m == PDM_WIDTH || m == PDM_HEIGHT ) {
	if ( m == PDM_WIDTH )
	    val = width();
	else
	    val = height();
    }
    else {
	HDC gdc = GetDC( 0 );
	switch ( m ) {
	    // !!!hanord: return widget mm width/height
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
		warning( "QWidget::metric: Invalid metric command" );
#endif
	}
	ReleaseDC( 0, gdc );
    }
    return val;
}


QImage QPixmap::convertToImage() const
{
    if ( isNull() ) {
#if defined(CHECK_NULL)
	warning( "QPixmap::convertToImage: Cannot convert a null pixmap" );
#endif
	QImage nullImage;
	return nullImage;
    }

    int	 w = width();
    int	 h = height();
    int	 d = depth();
    int	 ncols = 2;

    if ( d > 1 && d <= 8 ) {			// set to nearest valid depth
	d = 8;					//   2..7 ==> 8
	ncols = 256;
    }
    else if ( d > 8 ) {
	d = 24;					//   > 8  ==> 24
	ncols = 0;
    }

    QImage image( w, h, d, ncols, QImage::BigEndian );
    uchar *bits;
    int	   bpl	= ((w*d+31)/32)*4;
    int	   ibpl = image.bytesPerLine();
    int	   nbytes = bpl*h;
    bool   newBits = bpl != ibpl;

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
    bmh->biSizeImage	  = nbytes;
    bmh->biClrUsed	  = ncols;
    bmh->biClrImportant	  = 0;
    ulong *coltbl = (ulong*)(bmi_data + sizeof(BITMAPINFOHEADER));

    if ( newBits )
	bits = new uchar[nbytes];
    else
	bits = image.bits();
    GetDIBits( handle(), hbm(), 0, h, bits, bmi, DIB_RGB_COLORS );

    if ( newBits ) {
	uchar *p = bits;
	for ( int y=0; y<h; y++ ) {
	    memcpy( image.scanLine(y), p, ibpl );
	    p += bpl;
	}
	delete [] bits;
    }

    for ( int i=0; i<ncols; i++ ) {		// copy color table
	RGBQUAD *r = (RGBQUAD*)&coltbl[i];
	image.setColor( i, QRGB(r->rgbRed,
				r->rgbGreen,
				r->rgbBlue) );
    }

    delete [] bmi_data;
    return image;
}


extern bool qt_image_did_turn_scanlines();	// defined in qpixmap.cpp


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
    }
    else {					// can be both
	bool conv8 = FALSE;
	if ( mode == Color )			// native depth wanted
	    conv8 = d == 1;
	else if ( d == 1 && image.numColors() == 2 ) {
	    ulong c0 = image.color(0);		// mode==Auto: convert to best
	    ulong c1 = image.color(1);
	    conv8 = QMIN(c0,c1) != 0 || QMAX(c0,c1) != QRGB(255,255,255);
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

    uchar *bits;
    int	   bpl	= ((w*d+31)/32)*4;
    int	   ibpl = image.bytesPerLine();
    bool   newBits = bpl != ibpl;
    int	   numBytes;
    bool   turn = qt_image_did_turn_scanlines();

    if ( newBits ) {				// must align to 32 bits
	numBytes = bpl*h;
	bits	 = new uchar[numBytes];
	uchar *p = bits;
	for ( int y=0; y<h; y++ ) {
	    memcpy( p, image.scanLine(y), ibpl );
	    p += bpl;
	}
    }
    else {					// 32-bits aligned image data
	numBytes = image.numBytes();
	bits	 = image.bits();
    }

    int	  ncols	   = image.numColors();
    char *bmi_data = new char[sizeof(BITMAPINFO)+sizeof(ulong)*ncols];
    BITMAPINFO	     *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize		  = sizeof(BITMAPINFOHEADER);
    bmh->biWidth	  = w;
    bmh->biHeight	  = turn ? h : -h;
    bmh->biPlanes	  = 1;
    bmh->biBitCount	  = d;
    bmh->biCompression	  = BI_RGB;
    bmh->biSizeImage	  = numBytes;
    bmh->biXPelsPerMeter  = 0;
    bmh->biYPelsPerMeter  = 0;
    bmh->biClrUsed	  = ncols;
    bmh->biClrImportant	  = ncols;
    ulong *coltbl = (ulong*)(bmi_data + sizeof(BITMAPINFOHEADER));
    for ( int i=0; i<ncols; i++ ) {		// copy color table
	RGBQUAD *r = (RGBQUAD*)&coltbl[i];
	ulong c = image.color(i);
	r->rgbBlue  = QBLUE( c );
	r->rgbGreen = QGREEN( c );
	r->rgbRed   = QRED( c );
	r->rgbReserved = 0;
    }

    SetDIBitsToDevice( pm.handle(), 0, 0, w, h, 0, 0, 0, h, bits,
		       bmi, DIB_RGB_COLORS );
    delete [] bmi_data;
    if ( newBits )
	delete [] bits;

    pm.data->uninit = FALSE;
    if ( tmp_dc )
	pm.freeMemDC();
    *this = pm;
    return TRUE;
}


QPixmap QPixmap::grabWindow( WId window, int x, int y, int w, int h )
{
    QPixmap null;
    return null;
}


QPixmap QPixmap::xForm( const QWMatrix &matrix ) const
{
    int	  w, h;					// size of target pixmap
    int	  ws, hs;				// size of source pixmap
    POINT p[3];					// plg points
    bool  plg;

    if ( isNull() )				// this is a null pixmap
	return copy();

    ws = width();
    hs = height();

    if ( matrix.m12() == 0.0F  && matrix.m21() == 0.0F &&
	 matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F ) {
	plg = FALSE;
	w = qRound( matrix.m11()*ws );
	h = qRound( matrix.m22()*hs );
    }
    else {					// rotation/shearing
	plg = TRUE;
	const float dt = 0.0001F;
	float x1,y1, x2,y2, x3,y3, x4,y4;	// get corners
	float xx = (float)ws - 1;
	float yy = (float)hs - 1;

	matrix.map( dt, dt, &x1, &y1 );
	matrix.map( xx, dt, &x2, &y2 );
	matrix.map( xx, yy, &x3, &y3 );
	matrix.map( dt, yy, &x4, &y4 );

	float ymin = y1;			// lowest y value
	if ( y2 < ymin ) ymin = y2;
	if ( y3 < ymin ) ymin = y3;
	if ( y4 < ymin ) ymin = y4;
	float xmin = x1;			// lowest x value
	if ( x2 < xmin ) xmin = x2;
	if ( x3 < xmin ) xmin = x3;
	if ( x4 < xmin ) xmin = x4;

	QWMatrix mat( 1.0F, 0.0F, 0.0F, 1.0F, -xmin, -ymin );
	mat = matrix * mat;

	QPointArray a( QRect(0,0,ws,hs) );
	a = mat.map( a );
	QRect r = a.boundingRect().normalize();
	h = r.height();
	w = r.width();

	bool invertible;
	mat = mat.invert( &invertible );	// invert matrix

	if ( !invertible ) {			// not invertible
	    w = 0;
	}
	else {
	    p[0].x = qRound(x1 - xmin);
	    p[0].y = qRound(y1 - ymin);
	    p[1].x = qRound(x2 - xmin);
	    p[1].y = qRound(y2 - ymin);
	    p[2].x = qRound(x4 - xmin);
	    p[2].y = qRound(y4 - ymin);
	}
    }

    if ( w == 0 || h == 0 ) {			// invalid result
	QPixmap pm;
	pm.data->bitmap = data->bitmap;
	return pm;
    }

    bool src_tmp;
    QPixmap *self = (QPixmap*)this;		// mutable
    if ( !handle() ) {
	self->allocMemDC();
	src_tmp = TRUE;
    }
    else
	src_tmp = FALSE;
    QPixmap pm( w, h, depth() );
    bool dst_tmp;
    if ( !pm.handle() ) {
	pm.allocMemDC();
	dst_tmp = TRUE;
    }
    else
	dst_tmp = FALSE;

    SetStretchBltMode( pm.handle(), COLORONCOLOR );
    if ( plg ) {				// rotate/shear
	RECT r;
	r.left = r.top = 0;
	r.right	 = w;
	r.bottom = h;
	FillRect( pm.handle(), &r, GetStockObject(WHITE_BRUSH) );
	PlgBlt( pm.handle(), p, handle(), 0, 0, ws, hs, 0, 0, 0 );
    }
    else					// scale
	StretchBlt( pm.handle(), 0, 0, w, h, handle(), 0, 0, ws, hs,
		    SRCCOPY );

    if ( dst_tmp )
	pm.freeMemDC();
    if ( src_tmp )
	self->freeMemDC();

    if ( data->mask ) {
	if ( data->mask->data == data )		// pixmap == mask
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
