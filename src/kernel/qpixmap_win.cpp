/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap_win.cpp#2 $
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
#include "q2matrix.h"
#include "qapp.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpixmap_win.cpp#2 $";
#endif


// --------------------------------------------------------------------------
// Internal functions
//

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

void QPixmap::init()
{
    data = new QPixmapData;
    CHECK_PTR( data );
    data->dirty	 = data->optim = FALSE;
    data->uninit = TRUE;
    data->bitmap = FALSE;
    data->hbm = 0;
}


/*!
  Constructs a null pixmap.
  \sa isNull()
*/

QPixmap::QPixmap()
    : QPaintDevice( PDT_PIXMAP )
{
    init();
    data->w = data->h = 0;
    data->d = 0;
    hdc = 0;
}


static int defaultPixmapDepth()
{
    static int defDepth = 0;
    if ( defDepth == 0 ) {
	HDC gdc = GetDC( 0 );
	defDepth = GetDeviceCaps( gdc, PLANES );
	ReleaseDC( 0, gdc );
    }
    return defDepth;
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
    if ( data->d == dd ) {			// compatible bitmap
	HDC gdc = GetDC( 0 );
	data->hbm = CreateCompatibleBitmap( gdc, w, h );
	ReleaseDC( 0, gdc );	
    }
    else					// monocrome bitmap
	data->hbm = CreateBitmap( w, h, 1, 1, 0 );
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
    if ( data->d == dd ) {			// compatible bitmap
	HDC gdc = GetDC( 0 );
	data->hbm = CreateCompatibleBitmap( gdc, w, h );
	ReleaseDC( 0, gdc );	
    }
    else					// monocrome bitmap
	data->hbm = CreateBitmap( w, h, 1, 1, 0 );
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
    delete flipped_bits;
}

QPixmap::QPixmap( const QPixmap &pixmap )
    : QPaintDevice( PDT_PIXMAP )
{
    data = pixmap.data;
    data->ref();
    devFlags = pixmap.devFlags;			// copy QPaintDevice flags
    hdc	= pixmap.hdc;				// copy QPaintDevice hdc
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
    if ( !hdc ) {
	HANDLE h = GetDC( 0 );
	hdc = CreateCompatibleDC( hdc );
	SelectObject( hdc, data->hbm );
	ReleaseDC( 0, h );
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
    hd	= pixmap.hdc;				// copy QPaintDevice drawable
    return *this;
}


bool QPixmap::enableImageCache( bool enable )
{
    bool v = data->optim;
    data->optim = enable ? 1 : 0;
    data->dirty = FALSE;		// !!!hanord to be implemented
    return v;
}


void QPixmap::fill( const QColor &fillColor )	// fill pixmap contents
{
    if ( isNull() )
	return;
    detach();					// detach other references
    bool tmp_hdc = hdc == 0;
    if ( tmp_hdc )
	allocMemDC();
    RECT r;
    r.left = r.top = 0;		// !!!hanord SetDIBits/SetBitmapBits
    r.right  = width();
    r.bottom = height();
    HANDLE hbrush = CreateSolidBrush( fillColor.pixel() );
    FillRect( hdc, &r, hbrush );
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
	switch ( m ) {
	HDC gdc = GetDC( 0 );
	switch ( m ) {
	    // !!!hanord: return widget mm width/height
	    case PDM_WIDTHMM:
		val = GetDeviceCaps( gdc, HORSIZE );
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


bool QPixmap::convertFromImage( const QImage &img )
{
    if ( img.isNull() ) {
#if defined(CHECK_NULL)
	warning( "QPixmap::convertFromImage: Cannot convert a null image" );
#endif
	return FALSE;
    }
    return FALSE;
}


QPixmap QPixmap::grabWindow( WId window, int x, int y, int w, int h )
{
    QPixmap null;
    return null;
}


QPixmap QPixmap::xForm( const Q2DMatrix &matrix ) const
{
    QPixmap null;
    return null;
}


Q2DMatrix QPixmap::trueMatrix( const Q2DMatrix &matrix, int w, int h )
{						// get true wxform matrix
    const float dt = 0.0001;
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

    Q2DMatrix mat( 1, 0, 0, 1, -xmin, -ymin );	// true matrix
    mat = matrix * mat;
    return mat;
}
