/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpm_win.cpp#4 $
**
** Implementation of QPixmap class for Windows
**
** Author  : Haavard Nord
** Created : 940501
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpixmap.h"
#include "qimage.h"
#include "qpaintdc.h"
#include "qwmatrix.h"
#include "qapp.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpm_win.cpp#4 $";
#endif


// --------------------------------------------------------------------------
// Internal functions
//

static int defaultPixmapDepth()
{
    static int defDepth = 0;
    if ( defDepth == 0 )
	defDepth = GetDeviceCaps( qt_display_dc(), BITSPIXEL );
    return defDepth;
}

static uchar *flip_bits( uchar *bits, int len ) // flip bits in bitmap
{
    extern char *qt_get_bitflip_array();	// defined in qimage.cpp
    register uchar *p = bits;
    uchar *end = p + len;
    uchar *newdata = new uchar[len];
    uchar *b = newdata;
    uchar *f = (uchar *)qt_get_bitflip_array();
    while ( p < end )
	*b++ = f[*p++];
    return newdata;
}


// --------------------------------------------------------------------------
// QPixmap member functions
//

bool QPixmap::optimAll = TRUE;


void QPixmap::init()
{
    data = new QPixmapData;
    CHECK_PTR( data );
    data->dirty	 = FALSE;
    data->optim	 = optimAll;
    data->uninit = TRUE;
    data->bitmap = FALSE;
    data->hbm = 0;
}


QPixmap::QPixmap()
    : QPaintDevice( PDT_PIXMAP )
{
    init();
    data->w = data->h = 0;
    data->d = 0;
    hdc = 0;
}

QPixmap::QPixmap( int w, int h, int depth )
    : QPaintDevice( PDT_PIXMAP )
{
    init();
    int dd = defaultPixmapDepth();
    bool make_null = w == 0 || h == 0;		// create null pixmap
    if ( depth == 1 )				// monocrome pixmap
	data->d = 1;
    else if ( depth < 0 || depth == dd )	// compatible pixmap
	data->d = dd;
    else
	data->d = 0;
    if ( make_null || w < 0 || h < 0 || data->d == 0 ) {
	data->w = data->h = 0;
	data->d = 0;
	data->hbm = 0;
#if defined(CHECK_RANGE)
	if ( !make_null )			// invalid parameters
	    warning( "QPixmap: Invalid pixmap parameters" );
#endif
	return;
    }
    data->w = w;
    data->h = h;
    if ( data->d == dd )			// compatible bitmap
	data->hbm = CreateCompatibleBitmap( qt_display_dc(), w, h );
    else					// monocrome bitmap
	data->hbm = CreateBitmap( w, h, 1, 1, 0 );
    if ( data->optim )
	allocMemDC();
}

QPixmap::QPixmap( const QSize &size, int depth )
    : QPaintDevice( PDT_PIXMAP )
{
    int w = size.width();
    int h = size.height();
    init();
    int dd = defaultPixmapDepth();
    bool make_null = w == 0 || h == 0;		// create null pixmap
    if ( depth == 1 )				// monocrome pixmap
	data->d = 1;
    else if ( depth < 0 || depth == dd )	// compatible pixmap
	data->d = dd;
    else
	data->d = 0;
    if ( make_null || w < 0 || h < 0 || data->d == 0 ) {
	data->w = data->h = 0;
	data->d = 0;
	data->hbm = 0;
#if defined(CHECK_RANGE)
	if ( !make_null )			// invalid parameters
	    warning( "QPixmap: Invalid pixmap parameters" );
#endif
	return;
    }
    data->w = w;
    data->h = h;
    if ( data->d == dd )			// compatible bitmap
	data->hbm = CreateCompatibleBitmap( qt_display_dc(), w, h );
    else					// monocrome bitmap
	data->hbm = CreateBitmap( w, h, 1, 1, 0 );
    if ( data->optim )
	allocMemDC();
}

QPixmap::QPixmap( int w, int h, const char *bits, bool isXbitmap )
    : QPaintDevice( PDT_PIXMAP )
{						// for bitmaps only
    init();
    data->uninit = FALSE;
    if ( w <= 0 ) w = 1;
    if ( h <= 0 ) h = 1;
    data->w = w;  data->h = h;	data->d = 1;
    uchar *flipped_bits;
    if ( !isXbitmap )
	flipped_bits = 0;
    else {					// X bitmap -> flip bits
	flipped_bits = flip_bits( (uchar *)bits, ((w+7)/8)*h );
	bits = (const char *)flipped_bits;
    }
    data->hbm = CreateBitmap( w, h, 1, 1, bits );
    if ( data->optim )
	allocMemDC();
    delete flipped_bits;
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
    if( data->deref() ) {			// last reference lost
	freeMemDC();
	if ( data->hbm )
	    DeleteObject( data->hbm );
	delete data;
    }
}


HANDLE QPixmap::allocMemDC()
{
    if ( !hdc && !isNull() ) {
	hdc = CreateCompatibleDC( qt_display_dc() );
	SelectObject( hdc, data->hbm );
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
    if( data->deref() ) {			// last reference lost
	freeMemDC();
	if ( data->hbm )
	    DeleteObject( data->hbm );
	delete data;
    }
    data = pixmap.data;
    devFlags = pixmap.devFlags;			// copy QPaintDevice flags
    hdc = pixmap.hdc;				// copy QPaintDevice drawable
    return *this;
}


bool QPixmap::optimized() const
{
    return data->optim;
}

void QPixmap::setOptimization( bool enable )
{
    if ( paintingActive() ) {
#if defined(DEBUG)
	warning( "QPixmap::setOptimization: Cannot be done when painting" );
#endif
	return;
    }
    if ( enable == (bool)data->optim )
	return;
    data->optim = enable;
    data->dirty = FALSE;
    if ( enable )
	allocMemDC();
    else
	freeMemDC();
}

bool QPixmap::optimizedAll()
{
    return optimAll;
}

void QPixmap::setOptimizationAll( bool enable )
{
    optimAll = enable;
}


void QPixmap::fill( const QColor &fillColor )	// fill pixmap contents
{
    if ( isNull() )
	return;
    detach();					// detach other references
    bool tmp_hdc = hdc == 0;
    if ( tmp_hdc )
	allocMemDC();
    bool   stockBrush = TRUE;
    HANDLE hbrush;
    if ( fillColor == black )
	hbrush = GetStockObject( BLACK_BRUSH );
    else if ( fillColor == white )
	hbrush == GetStockObject( WHITE_BRUSH );
    else {
	hbrush = CreateSolidBrush( fillColor.pixel() );
	stockBrush = FALSE;
    }
    RECT r;
    r.left = r.top = 0;
    r.right  = width();
    r.bottom = height();
    FillRect( hdc, &r, hbrush );
    if ( !stockBrush )
	DeleteObject( hbrush );
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
	    case PDM_NUMPLANES:
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
    QImage image;
    if ( isNull() ) {
#if defined(CHECK_NULL)
	warning( "QPixmap::convertToImage: Cannot convert a null pixmap" );
#endif
	return image;
    }
    return image;
}


extern bool qt_image_did_turn_scanlines();	// defined in qpixmap.cpp


bool QPixmap::convertFromImage( const QImage &image )
{
    if ( image.isNull() ) {
#if defined(CHECK_NULL)
	warning( "QPixmap::convertFromImage: Cannot convert a null image" );
#endif
	return FALSE;
    }
    int d = image.depth() == 1 ? 1 : -1;
    int w = image.width();
    int h = image.height();
    QPixmap pm( w, h, d );
    bool tmp_dc = pm.handle() == 0;
    if ( tmp_dc )
	pm.allocMemDC();

    uchar *bits;
    bool   turn = !qt_image_did_turn_scanlines();
    if ( turn ) {				// turn scanlines
	int bpl = image.bytesPerLine();
	bits = new uchar[image.numBytes()];
	uchar *d = bits;
	uchar *s = image.scanline( h-1 );
	for ( int i=0; i<h; i++ ) {
	    memcpy( d, s, bpl );
	    d += bpl;
	    s -= bpl;
	}
    }
    else {					// scanlines ok
	bits = image.bits();
    }

    int	  ncols	   = image.numColors();
    char *bmi_data = new char[sizeof(BITMAPINFO)+sizeof(ulong)*ncols];
    BITMAPINFO	     *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize		  = sizeof(BITMAPINFOHEADER);
    bmh->biWidth	  = w;
    bmh->biHeight	  = h;
    bmh->biPlanes	  = 1;
    bmh->biBitCount	  = image.depth();
    bmh->biCompression	  = BI_RGB;
    bmh->biSizeImage	  = image.numBytes();
    bmh->biXPelsPerMeter  = 0;
    bmh->biYPelsPerMeter  = 0;
    bmh->biClrUsed	  = ncols;
    bmh->biClrImportant	  = ncols;
    ulong *coltbl = (ulong*)(bmi_data + sizeof(BITMAPINFOHEADER));
    for ( int i=0; i<ncols; i++ ) {
	RGBQUAD *r = (RGBQUAD*)&coltbl[i];
	ulong c = image.color(i);
	r->rgbBlue  = QBLUE( c );
	r->rgbGreen = QGREEN( c );
	r->rgbRed   = QRED( c );
	r->rgbReserved = 0;
    }
    SetDIBitsToDevice( pm.handle(), 0, 0, w, h, 0, 0, 0, h, bits,
		       bmi, DIB_RGB_COLORS );
    delete bmi_data;
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


static inline int d2i_round( double d )		// double -> int, rounded
{
    return d > 0 ? int(d+0.5) : int(d-0.5);
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

    if ( matrix.m12() == 0.0F && matrix.m21() == 0.0F &&
	 matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F ) {
	plg = FALSE;
	w = d2i_round( matrix.m11()*ws );
	h = d2i_round( matrix.m22()*hs );
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

	if ( mat.m12() != 0.0 || mat.m21() != 0.0 ) {
	    QPointArray a( QRect(0,0,ws,hs) );
	    a = mat.map( a );
	    QRect r = a.boundingRect();
	    r.fixup();
	    h = r.height();
	    w = r.width();
	}
	else {					// no rotation/shearing
	    h = d2i_round( mat.m22()*hs );
	    w = d2i_round( mat.m11()*ws );
	    h = QABS( h );
	    w = QABS( w );
	}
	bool invertible;
	mat = mat.invert( &invertible );	// invert matrix

	if ( !invertible )			// not invertible
	    w = 0;

	else {
	    p[0].x = d2i_round(x1 - xmin);
	    p[0].y = d2i_round(y1 - ymin);
	    p[1].x = d2i_round(x2 - xmin);
	    p[1].y = d2i_round(y2 - ymin);
	    p[2].x = d2i_round(x4 - xmin);
	    p[2].y = d2i_round(y4 - ymin);
	}
    }

    if ( w == 0 || h == 0 ) {			// invalid result
	QPixmap pm;
	pm.data->bitmap = data->bitmap;
	return pm;
    }

    bool src_tmp;
    QPixmap *self = (QPixmap*)this;
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
