/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptr_win.cpp#25 $
**
** Implementation of QPainter class for Windows
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpainter.h"
#include "qpaintdc.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qpmcache.h"
#include "qlist.h"
#include "qintdict.h"
#include <math.h>
#include <windows.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qptr_win.cpp#25 $")


// --------------------------------------------------------------------------
// QPainter internal pen and brush cache
//
// The cache makes a significant contribution to speeding up drawing.
// Setting a new pen or brush specification will make the painter look for
// an existing pen or brush with the same attributes instead of creating
// a new pen or brush.
//
// Only solid line pens with line width 0 and solid brushes will be cached.
//
// The cache structure is not ideal, but lookup speed is essential here.
// Experiments show that the cache is very effective under normal use.
//

struct QHDCObj					// cached pen or brush
{
    HANDLE  obj;
    ulong   pix;
    int	    count;
    int	    hits;
};

const int	cache_size = 29;		// multiply by 4
static QHDCObj *pen_cache_buf;
static QHDCObj *pen_cache[4*cache_size];
static QHDCObj *brush_cache_buf;
static QHDCObj *brush_cache[4*cache_size];
static bool	cache_init = FALSE;

static HANDLE stock_nullPen;
static HANDLE stock_blackPen;
static HANDLE stock_whitePen;
static HANDLE stock_nullBrush;
static HANDLE stock_blackBrush;
static HANDLE stock_whiteBrush;
static HANDLE stock_font;

static QHDCObj stock_dummy;
static void  *stock_ptr = (void *)&stock_dummy;


static void init_cache()
{
    if ( cache_init )
	return;
    int i;
    QHDCObj *h;
    cache_init = TRUE;
    h = pen_cache_buf = new QHDCObj[4*cache_size];
    memset( h, 0, 4*cache_size*sizeof(QHDCObj) );
    for ( i=0; i<4*cache_size; i++ )
	pen_cache[i] = h++;
    h = brush_cache_buf = new QHDCObj[4*cache_size];
    memset( h, 0, 4*cache_size*sizeof(QHDCObj) );
    for ( i=0; i<4*cache_size; i++ )
	brush_cache[i] = h++;
}


//  #define CACHE_STAT
#if defined(CACHE_STAT)
#include "qtstream.h"

static int c_numhits	= 0;
static int c_numcreates = 0;
static int c_numfaults	= 0;
#endif


static void cleanup_cache()
{
    if ( !cache_init )
	return;
    int i;
#if defined(CACHE_STAT)
    debug( "Number of cache hits = %d", c_numhits );
    debug( "Number of cache creates = %d", c_numcreates );
    debug( "Number of cache faults = %d", c_numfaults );
    debug( "PEN CACHE" );
    for ( i=0; i<cache_size; i++ ) {
	QString	    str;
	QTextStream s(str,IO_WriteOnly);
	s << i << ": ";
	for ( int j=0; j<4; j++ ) {
	    QHDCObj *h = pen_cache[i*4+j];
	    s << (h->obj ? 'X' : '-') << ',' << h->hits << ','
	      << h->count << '\t';
	}
	s << '\0';
	debug( str );
    }
    debug( "BRUSH CACHE" );
    for ( i=0; i<cache_size; i++ ) {
	QString	    str;
	QTextStream s(str,IO_WriteOnly);
	s << i << ": ";
	for ( int j=0; j<4; j++ ) {
	    QHDCObj *h = brush_cache[i*4+j];
	    s << (h->obj ? 'X' : '-') << ',' << h->hits << ','
	      << h->count << '\t';
	}
	s << '\0';
	debug( str );
    }
#endif
    for ( i=0; i<4*cache_size; i++ ) {
	if ( pen_cache[i]->obj )
	    DeleteObject( pen_cache[i]->obj );
	if ( brush_cache[i]->obj )
	    DeleteObject( brush_cache[i]->obj );
    }
    delete [] pen_cache_buf;
    delete [] brush_cache_buf;
    cache_init = FALSE;
}


static bool obtain_obj( void **ref, HANDLE *obj, ulong pix, QHDCObj **cache,
			bool is_pen )
{
    if ( !cache_init )
	init_cache();

    int	     k = (pix % cache_size) * 4;
    QHDCObj *h = cache[k++];
    QHDCObj *prev = 0;

#define NOMATCH (h->obj && h->pix != pix)

    if ( NOMATCH ) {
	prev = h;
	h = cache[k++];
	if ( NOMATCH ) {
	    prev = h;
	    h = cache[k++];
	    if ( NOMATCH ) {
		prev = h;
		h = cache[k++];
		if ( NOMATCH ) {
		    if ( h->count == 0 ) {	// steal this pen/brush
#if defined(CACHE_STAT)
			c_numcreates++;
#endif
			h->pix	 = pix;
			h->count = 1;
			h->hits	 = 1;
			DeleteObject( h->obj );
			if ( is_pen )
			    h->obj = CreatePen( PS_SOLID, 0, pix );
			else
			    h->obj = CreateSolidBrush( pix );
			cache[k-1] = prev;
			cache[k-2] = h;
			*ref = (void *)h;
			*obj = h->obj;
			return TRUE;
		    }
		    else {			// all objects in use
#if defined(CACHE_STAT)
			c_numfaults++;
#endif
			*ref = 0;
			return FALSE;
		    }
		}
	    }
	}
    }

#undef NOMATCH

    *ref = (void *)h;

    if ( h->obj ) {				// reuse existing pen/brush
#if defined(CACHE_STAT)
	c_numhits++;
#endif
	*obj = h->obj;
	h->count++;
	h->hits++;
	if ( prev && h->hits > prev->hits ) {	// maintain LRU order
	    cache[k-1] = prev;
	    cache[k-2] = h;
	}
    }
    else {					// create new pen/brush
#if defined(CACHE_STAT)
	c_numcreates++;
#endif
	if ( is_pen )
	    h->obj = CreatePen( PS_SOLID, 0, pix );
	else
	    h->obj = CreateSolidBrush( pix );
	h->pix	 = pix;
	h->count = 1;
	h->hits	 = 1;
	*obj = h->obj;
    }
    return TRUE;
}

static inline void release_obj( void *ref )
{
#if defined(DEBUG)
    ASSERT( cache_init && ref );
#endif
    ((QHDCObj*)ref)->count--;
}

static inline bool obtain_pen( void **ref, HANDLE *pen, ulong pix )
{ return obtain_obj( ref, pen, pix, pen_cache, TRUE ); }

static inline bool obtain_brush( void **ref, HANDLE *brush, ulong pix )
{ return obtain_obj( ref, brush, pix, brush_cache, FALSE ); }

#define release_pen	release_obj
#define release_brush	release_obj


// --------------------------------------------------------------------------
// QPainter member functions
//

void QPainter::initialize()
{
    stock_nullPen    = GetStockObject( NULL_PEN );
    stock_blackPen   = GetStockObject( BLACK_PEN );
    stock_whitePen   = GetStockObject( WHITE_PEN );
    stock_nullBrush  = GetStockObject( NULL_BRUSH );
    stock_blackBrush = GetStockObject( BLACK_BRUSH );
    stock_whiteBrush = GetStockObject( WHITE_BRUSH );
    stock_font	     = GetStockObject( SYSTEM_FONT );
    init_cache();
}

void QPainter::cleanup()
{
    cleanup_cache();
}


typedef declare(QIntDictM,QPaintDevice) QPaintDeviceDict;
static QPaintDeviceDict *pdev_dict = 0;


void QPainter::redirect( QPaintDevice *pdev, QPaintDevice *replacement )
{
    if ( pdev_dict == 0 ) {
	if ( replacement == 0 )
	    return;
	pdev_dict = new QPaintDeviceDict;
	CHECK_PTR( pdev_dict );
    }
#if defined(CHECK_NULL)
    if ( pdev == 0 )
	warning( "QPainter::redirect: The pdev argument cannot be 0" );
#endif
    if ( replacement )
	pdev_dict->insert( (long)pdev, replacement );
    else {
	pdev_dict->remove( (long)pdev );
	if ( pdev_dict->count() == 0 ) {
	    delete pdev_dict;
	    pdev_dict = 0;
	}
    }
}


QPainter::QPainter()
{
    flags = IsStartingUp;
    bg_col = white;				// default background color
    bg_mode = TransparentMode;			// default background mode
    rop = CopyROP;				// default ROP
    tabstops = 0;				// default tabbing
    tabarray = 0;
    tabarraylen = 0;
    ps_stack = 0;
    hdc = hpen = hbrush = hbrushbm = 0;
    pixmapBrush = nocolBrush = FALSE;
    penRef = brushRef = 0;
    tm = 0;
}

QPainter::~QPainter()
{
    if ( isActive() ) {
#if defined(CHECK_STATE)
	warning( "QPainter: Painting wasn't properly end()'ed" );
#endif
	end();
    }
    if ( tabarray )				// delete tab array
	delete tabarray;
    if ( ps_stack )
	killPStack();
}


void QPainter::setFont( const QFont &font )
{
    if ( cfont.d != font.d ) {
	cfont = font;
	updateFont();
    }
}


void QPainter::updateFont()
{
    if ( tm ) {					// delete old text metrics
	delete tm;
	tm = 0;
    }
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].font = &cfont;
	if ( !pdev->cmd(PDC_SETFONT,this,param) || !hdc )
	    return;
    }
    bool give_hdc = pdev->devType() == PDT_PRINTER && !testf(VxF|WxF);
    SelectObject( hdc, cfont.handle(give_hdc ? hdc : 0) );
}


void QPainter::updatePen()
{
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].pen = &cpen;
	if ( !pdev->cmd(PDC_SETPEN,this,param) || !hdc )
	    return;
    }

    int	   ps	   = cpen.style();
    ulong  pix	   = cpen.color().pixel();
    bool   cacheIt = ps == NoPen || (ps == SolidLine && cpen.width() == 0);
    HANDLE hpen_old;

    if ( penRef ) {
	release_pen( penRef );
	penRef = 0;
	hpen_old = 0;
    }
    else
	hpen_old = hpen;

    if ( cacheIt ) {
	if ( ps == NoPen ) {
	    hpen = stock_nullPen;
	    penRef = stock_ptr;
	    SelectObject( hdc, hpen );
	    SetTextColor( hdc, pix );
	    if ( hpen_old )
		DeleteObject( hpen_old );
	    return;
	}
	if ( obtain_pen(&penRef, &hpen, pix) ) {
	    SelectObject( hdc, hpen );
	    SetTextColor( hdc, pix );
	    if ( hpen_old )
		DeleteObject( hpen_old );
	    return;
	}
    }

    int s;

    switch ( ps ) {
	case SolidLine:
	    s = PS_SOLID;
	    break;
	case DashLine:
	    s = PS_DASH;
	    break;
	case DotLine:
	    s = PS_DOT;
	    break;
	case DashDotLine:
	    s = PS_DASHDOT;
	    break;
	case DashDotDotLine:
	    s = PS_DASHDOTDOT;
	    break;
	default:
	    s = PS_SOLID;
#if defined(DEBUG)
	    warning( "QPainter::updatePen: Invalid pen style" );
#endif
    }

    hpen = CreatePen( s, cpen.width(), pix );
    SetTextColor( hdc, pix );			// pen color is also text color
    SelectObject( hdc, hpen );
    if ( hpen_old )				// delete last pen
	DeleteObject( hpen_old );
}


void QPainter::updateBrush()
{
    static short d1_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static short d2_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static short d3_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static short d4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static short d5_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static short d6_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static short d7_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static short *dense_patterns[]
	= { d1_pat, d2_pat, d3_pat, d4_pat, d5_pat, d6_pat, d7_pat };

    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].brush = &cbrush;
	if ( !pdev->cmd(PDC_SETBRUSH,this,param) || !hdc )
	    return;
    }

    int	   bs	   = cbrush.style();
    QColor c	   = cbrush.color();
    bool   cacheIt = bs == NoBrush || bs == SolidPattern;
    HANDLE hbrush_old;

    if ( brushRef ) {
	release_brush( brushRef );
	brushRef = 0;
	hbrush_old = 0;
    }
    else
	hbrush_old = hbrush;

    if ( cacheIt ) {
	if ( bs == NoBrush ) {
	    hbrush = stock_nullBrush;
	    brushRef = stock_ptr;
	    SelectObject( hdc, hbrush );
	    if ( hbrush_old ) {
		DeleteObject( hbrush_old );
		if ( hbrushbm && !pixmapBrush )
		    DeleteObject( hbrushbm );
		hbrushbm = 0;
		pixmapBrush = nocolBrush = FALSE;
	    }
	    return;
	}
	if ( obtain_brush(&brushRef, &hbrush, c.pixel()) ) {
	    SelectObject( hdc, hbrush );
	    if ( hbrush_old ) {
		DeleteObject( hbrush_old );
		if ( hbrushbm && !pixmapBrush )
		    DeleteObject( hbrushbm );
		hbrushbm = 0;
		pixmapBrush = nocolBrush = FALSE;
	    }
	    return;
	}
    }

    HANDLE hbrushbm_old	   = hbrushbm;
    bool   pixmapBrush_old = pixmapBrush;

    pixmapBrush = nocolBrush = FALSE;
    hbrushbm = 0;

    if ( bs == SolidPattern )			// create solid brush
	hbrush = CreateSolidBrush( c.pixel() );
    else if ( (bs >= Dense1Pattern && bs <= Dense7Pattern ) ||
	      (bs == CustomPattern) ) {
	if ( bs == CustomPattern ) {
	    hbrushbm = cbrush.pixmap()->hbm();
	    pixmapBrush = TRUE;
	    nocolBrush = cbrush.pixmap()->depth() == 1;
	}
	else {
	    short *bm = dense_patterns[ bs - Dense1Pattern ];
	    hbrushbm = CreateBitmap( 8, 8, 1, 1, bm );
	    nocolBrush = TRUE;
	}
	hbrush = CreatePatternBrush( hbrushbm );
    }
    else {					// one of the hatch brushes
	int s;
	switch ( bs ) {
	    case HorPattern:
		s = HS_HORIZONTAL;
		break;
	    case VerPattern:
		s = HS_VERTICAL;
		break;
	    case CrossPattern:
		s = HS_CROSS;
		break;
	    case BDiagPattern:
		s = HS_BDIAGONAL;
		break;
	    case FDiagPattern:
		s = HS_FDIAGONAL;
		break;
	    case DiagCrossPattern:
		s = HS_DIAGCROSS;
		break;
	    default:
		s = HS_HORIZONTAL;
	}
	hbrush = CreateHatchBrush( s, c.pixel() );
    }

    SelectObject( hdc, hbrush );

    if ( hbrush_old ) {
	DeleteObject( hbrush_old );		// delete last brush
	if ( hbrushbm_old && !pixmapBrush_old )
	    DeleteObject( hbrushbm_old );	// delete last brush pixmap
    }
}


bool QPainter::begin( const QPaintDevice *pd )
{
    if ( isActive() ) {				// already active painting
#if defined(CHECK_STATE)
	warning( "QPainter::begin: Painter is already active" );
#endif
	return FALSE;
    }
    else if ( pd == 0 ) {
#if defined(CHECK_NULL)
	warning( "QPainter::begin: Paint device cannot be null" );
#endif
	return FALSE;
    }

    bool reinit = flags != IsStartingUp;	// 2nd, 3rd,.... time called
    flags = IsActive;				// init flags

    if ( pdev_dict ) {				// redirected paint device?
	pdev = pdev_dict->find( (long)pd );
	if ( !pdev )				// no
	    pdev = (QPaintDevice *)pd;
    }
    else
	pdev = (QPaintDevice *)pd;

    int dt = pdev->devType();			// get the device type

    if ( (pdev->devFlags & PDF_EXTDEV) != 0 )	// this is an extended device
	setf(ExtDev);
    else if ( dt == PDT_PIXMAP )		// device is a pixmap
	((QPixmap*)pdev)->detach();		// will modify pixmap

    xfFont = FALSE;
    hdc = 0;

    if ( testf(ExtDev) ) {			// external device
	if ( !pdev->cmd(PDC_BEGIN,this,0) ) {	// could not begin painting
	    pdev = 0;
	    return FALSE;
	}
	if ( tabstops )				// update tabstops for device
	    setTabStops( tabstops );
	if ( tabarray )				// update tabarray for device
	    setTabArray( tabarray );
    }

    pdev->devFlags |= PDF_PAINTACTIVE;		// also tell paint device
    bro = QPoint( 0, 0 );
    if ( reinit ) {
	bg_mode = TransparentMode;		// default background mode
	rop = CopyROP;				// default ROP
	wxmat.reset();				// reset world xform matrix
	if ( dt != PDT_WIDGET ) {
	    QFont  defaultFont;			// default drawing tools
	    QPen   defaultPen;
	    QBrush defaultBrush;
	    cfont  = defaultFont;		// set these drawing tools
	    cpen   = defaultPen;
	    cbrush = defaultBrush;
	    bg_col = white;			// default background color
	}
    }
    wx = wy = vx = vy = 0;			// default view origins
    ww = 0;

    if ( dt == PDT_WIDGET ) {			// device is a widget
	QWidget *w = (QWidget*)pdev;
	cfont	= w->font();			// use widget font
	cpen = QPen( w->foregroundColor() );	// use widget fg color
	if ( reinit ) {
	    QBrush defaultBrush;
	    cbrush = defaultBrush;
	}
	bg_col	= w->backgroundColor();		// use widget bg color
	ww = vw = w->width();			// default view size
	wh = vh = w->height();
	if ( w->testWFlags(WState_Paint) )	// during paint event
	    hdc = w->hdc;
	else
	    hdc = GetDC( w->id() );
	if ( w->testWFlags(WPaintUnclipped) ) { // paint direct on device
	    // !!!hanord todo
	}
    }
    else if ( dt == PDT_PIXMAP ) {		// device is a pixmap
	QPixmap *pm = (QPixmap*)pdev;
	if ( pm->isNull() ) {
#if defined(CHECK_NULL)
	    warning( "QPainter::begin: Cannot paint null pixmap" );
#endif
	    end();
	    return FALSE;
	}

	pm->freeMemDC();
	pm->allocMemDC();
	hdc = pm->handle();
	ww = vw = pm->width();			// default view size
	wh = vh = pm->height();
	if ( pm->depth() == 1 ) {		// monochrome pixmap
	    setf( MonoDev );
	    bg_col = color0;
	    cpen.setColor( color1 );
	}
    }
    else if ( dt == PDT_PRINTER ) {		// device is a printer
	if ( pdev->handle() ) {
	    hdc = pdev->handle();
	}
    }
    if ( testf(ExtDev) ) {
	ww = vw = pdev->metric( PDM_WIDTH );
	wh = vh = pdev->metric( PDM_HEIGHT );
    }
    if ( ww == 0 )
	ww = wh = vw = vh = 1024;

    if ( testf(ExtDev) && hdc == 0 ) {		// external device
	setBackgroundColor( bg_col );		// default background color
	setBackgroundMode( TransparentMode );	// default background mode
	setRasterOp( CopyROP );			// default raster operation
    }
    if ( hdc ) {				// initialize hdc
	SetBkColor( hdc, bg_col.pixel() );	// set background color
	SetBkMode( hdc, TRANSPARENT );		// set background mode
	SetROP2( hdc, R2_COPYPEN );		// set raster operation
	SetTextAlign( hdc, TA_BASELINE );	// baseline-aligned text
	SetStretchBltMode( hdc, COLORONCOLOR ); // pixmap stretch mode
	if ( QColor::hPal() ) {			// realize global palette
	    SelectPalette( hdc, QColor::hPal(), FALSE );
	    RealizePalette( hdc );
	}
    }
    updatePen();
    updateBrush();
    updateFont();
    return TRUE;
}

bool QPainter::end()
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
	warning( "QPainter::end: No begin()" );
#endif
	return FALSE;
    }
    if ( testf(FontMet) )			// remove references to this
	QFontMetrics::reset( this );
    if ( testf(FontInf) )			// remove references to this
	QFontInfo::reset( this );

    if ( hpen ) {
	SelectObject( hdc, stock_nullPen );
	if ( penRef ) {
	    release_pen( penRef );
	    penRef = 0;
	}
	else
	    DeleteObject( hpen );
	hpen = 0;
    }
    if ( hbrush ) {
	SelectObject( hdc, stock_nullBrush );
	if ( brushRef ) {
	    release_brush( brushRef );
	    brushRef = 0;
	}
	else {
	    DeleteObject( hbrush );
	    if ( hbrushbm && !pixmapBrush )
		DeleteObject( hbrushbm );
	}
	hbrush = hbrushbm = 0;
	pixmapBrush = nocolBrush = FALSE;
    }

    if ( testf(ExtDev) )
	pdev->cmd( PDC_END, this, 0 );

    if ( pdev->devType() == PDT_WIDGET ) {
	if ( !((QWidget*)pdev)->testWFlags(WState_Paint) )
	    ReleaseDC( ((QWidget*)pdev)->id(), hdc );
    }
    else if ( pdev->devType() == PDT_PIXMAP ) {
	QPixmap *pm = (QPixmap*)pdev;
	pm->freeMemDC();
	if ( pm->isOptimized() )
	    pm->allocMemDC();
    }

    if ( tm ) {					// delete old text metrics
	delete tm;
	tm = 0;
    }

    flags = 0;
    pdev->devFlags &= ~PDF_PAINTACTIVE;
    pdev = 0;
    hdc	 = 0;
    return TRUE;
}


void QPainter::setBackgroundColor( const QColor &c )
{
    if ( !isActive() )
	return;
    bg_col = c;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].color = &bg_col;
	if ( !pdev->cmd(PDC_SETBKCOLOR,this,param) || !hdc )
	    return;
    }
    SetBkColor( hdc, c.pixel() );
}

void QPainter::setBackgroundMode( BGMode m )
{
    if ( !isActive() )
	return;
    if ( m != TransparentMode && m != OpaqueMode ) {
#if defined(CHECK_RANGE)
	warning( "QPainter::setBackgroundMode: Invalid mode" );
#endif
	return;
    }
    bg_mode = m;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = m;
	if ( !pdev->cmd(PDC_SETBKMODE,this,param) || !hdc )
	    return;
    }
    SetBkMode( hdc, m == TransparentMode ? TRANSPARENT : OPAQUE );
}

void QPainter::setRasterOp( RasterOp r )
{
    static short ropCodes[] =
	{ R2_COPYPEN, R2_MERGEPEN, R2_XORPEN, R2_MASKNOTPEN,
	  R2_NOTCOPYPEN, R2_MERGENOTPEN, R2_NOTXORPEN, R2_MASKPEN,
	  R2_NOT };
    if ( !isActive() )
	return;
    if ( (uint)r > NotROP ) {
#if defined(CHECK_RANGE)
	warning( "QPainter::setRasterOp: Invalid ROP code" );
#endif
	return;
    }
    rop = r;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = r;
	if ( !pdev->cmd(PDC_SETROP,this,param) || !hdc )
	    return;
    }
    SetROP2( hdc, ropCodes[rop] );
}

void QPainter::setBrushOrigin( int x, int y )
{
    if ( !isActive() )
	return;
    bro = QPoint(x,y);
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].point = &bro;
	if ( !pdev->cmd(PDC_SETBRUSHORIGIN,this,param) || !hdc )
	    return;
    }
#if defined(_WS_WIN32_)
    POINT dummy;
    SetBrushOrgEx( hdc, x, y, &dummy );
#else
    SetBrushOrg( hdc, x, y );
#endif
}


// #define OUR_XFORM

void QPainter::updateXForm()
{
#if defined(OUR_XFORM)

    QWMatrix m;
    if ( testf(VxF) ) {
	m.translate( vx, vy );
	m.scale( 1.0*vw/ww, 1.0*vh/wh );
	m.translate( -wx, -wy );
	m = wxmat * m;
    }
    else
	m = wxmat;
    wm11 = qRound((double)m.m11()*65536.0);
    wm12 = qRound((double)m.m12()*65536.0);
    wm21 = qRound((double)m.m21()*65536.0);
    wm22 = qRound((double)m.m22()*65536.0);
    wdx	 = qRound((double)m.dx() *65536.0);
    wdy	 = qRound((double)m.dy() *65536.0);
    bool invertible;
    m = m.invert( &invertible );		// invert matrix
    im11 = qRound((double)m.m11()*65536.0);
    im12 = qRound((double)m.m12()*65536.0);
    im21 = qRound((double)m.m21()*65536.0);
    im22 = qRound((double)m.m22()*65536.0);
    idx	 = qRound((double)m.dx() *65536.0);
    idy	 = qRound((double)m.dy() *65536.0);

#else

    if ( testf(VxF) ) {				// view xform enabled
	SetMapMode( hdc, MM_ANISOTROPIC );
#if defined(_WS_WIN32_)
	SetWindowExtEx( hdc, ww, wh, 0 );
	SetWindowOrgEx( hdc, wx, wy, 0 );
	SetViewportExtEx( hdc, vw, vh, 0 );
	SetViewportOrgEx( hdc, vx, vy, 0 );
#else
	SetWindowExt( hdc, ww, wh );
	SetWindowOrg( hdc, wx, wy );
	SetViewportExt( hdc, vw, vh );
	SetViewportOrg( hdc, vx, vy );
#endif
    }
    else {					// view xform disabled
	SetMapMode( hdc, MM_TEXT );
#if defined(_WS_WIN32_)
	SetWindowExtEx( hdc, 1, 1, 0 );
	SetWindowOrgEx( hdc, 0, 0, 0 );
	SetViewportExtEx( hdc, 1, 1, 0 );
	SetViewportOrgEx( hdc, 0, 0, 0 );
#else
	SetWindowExt( hdc, 1, 1 );
	SetWindowOrg( hdc, 0, 0 );
	SetViewportExt( hdc, 1, 1 );
	SetViewportOrg( hdc, 0, 0 );
#endif
    }
    if ( testf(WxF) ) {
#if defined(_WS_WIN32_)
	XFORM m;
	m.eM11 = wxmat.m11();
	m.eM12 = wxmat.m12();
	m.eM21 = wxmat.m21();
	m.eM22 = wxmat.m22();
	m.eDx  = wxmat.dx();
	m.eDy  = wxmat.dy();
//	SetGraphicsMode( hdc, GM_ADVANCED );
	SetGraphicsMode( hdc, GM_COMPATIBLE );
	SetWorldTransform( hdc, &m );
#endif
    }
    else {
#if defined(_WS_WIN32_)
	XFORM m;
	m.eM11 = (FLOAT)1.0;
	m.eM12 = (FLOAT)0.0;
	m.eM21 = (FLOAT)0.0;
	m.eM22 = (FLOAT)1.0;
	m.eDx  = (FLOAT)0.0;
	m.eDy  = (FLOAT)0.0;
	SetWorldTransform( hdc, &m );
	SetGraphicsMode( hdc, GM_COMPATIBLE );
#endif
    }
    bool xff = testf(VxF|WxF);
    if ( xff != (bool)xfFont && pdev->devType() == PDT_PRINTER ) {
	int ps = cfont.pointSize();		// must reload font
	cfont.setPointSize( ps+1 );
	cfont.setPointSize( ps );
	SelectObject( hdc, cfont.handle(xff ? 0 : hdc) );
	xfFont = xff;
    }

#endif // OUR_XFORM
}


#if defined(OUR_XFORM)

// xForm macros, use with care...

#define VXFORM_P(x,y)						\
    { x = (vw*(x-wx))/ww + vx; y = (vh*(y-wy))/wh + vy; }

#define VXFORM_R(x,y,w,h)					\
    { x = (vw*(x-wx))/ww + vx; y = (vh*(y-wy))/wh + vy;		\
      w = (vw*w)/ww; h = (vh*h)/wh;				\
      if ( w < 0 ) { w = -w; x -= w; }				\
      if ( h < 0 ) { h = -h; y -= h; } }

#define WXFORM_P(x,y)						\
    { int xx = wm11*x+wm21*y+wdx;				\
      xx += xx>0 ? 32768 : -32768;				\
      y = wm12*x+wm22*y+wdy;					\
      y += y>0 ? 32768 : -32768;				\
      x = xx/65536;  y /= 65536; }

#define WXFORM_R(x,y,w,h)					\
    { x = wm11*x+wdx;						\
      y = wm22*y+wdy;						\
      w = wm11*w;						\
      h = wm22*h;						\
      x += x>0 ? 32768 : -32768;				\
      y += y>0 ? 32768 : -32768;				\
      w += w>0 ? 32768 : -32768;				\
      h += h>0 ? 32768 : -32768;				\
      x/=65536; y/=65536; w/=65536; h/=65536; }

#endif // OUR_XFORM


QPoint QPainter::xForm( const QPoint &pv ) const
{						// map point, virtual -> device
#if defined(OUR_XFORM)
    int x=pv.x(), y=pv.y();
    if ( testf(WxF) ) {				// world xform
	WXFORM_P( x, y );
    }
    else if ( testf(VxF) ) {			// view xform
	VXFORM_P( x, y );
    }
    return QPoint( x, y );
#else
    if ( !hdc ) {
	return pv;	// !!!hanord what about ext devs???
    }
    POINT p;
    p.x = pv.x();
    p.y = pv.y();
    LPtoDP( hdc, &p, 1 );
    return QPoint( p.x, p.y );
#endif
}

QRect QPainter::xForm( const QRect &rv ) const
{						// map rect, virtual -> device
#if defined(OUR_XFORM)
    if ( !testf(VxF|WxF) )
	return rv;
    int x, y, w, h;
    rv.rect( &x, &y, &w, &h );
    if ( testf(WxF) ) {				// world xform
	if ( wm12 == 0 && wm21 == 0 ) {		// scaling+translation only
	    WXFORM_R(x,y,w,h);
	}
	else {					// return bounding rect
	    QPointArray a( rv );
	    a = xForm( a );
	    return a.boundingRect();
	}
    }
    else if ( testf(VxF) ) {			// view xform
	VXFORM_P(x,y);
	w = (vw*w)/ww;
	h = (vh*h)/wh;
    }
    return QRect( x, y, w, h );
#else
    if ( !hdc ) {
	return rv;	// !!!hanord what about ext devs???
    }
    if ( testf(WxF) ) {
	QPointArray a( rv );
	a = xForm( a );
	return a.boundingRect();
    }
    RECT r;
    SetRect( &r, rv.left(), rv.top(), rv.right(), rv.bottom() );
    LPtoDP( hdc, (POINT*)&r, 2 );
    return QRect( QPoint(r.left,  r.top),
		  QPoint(r.right, r.bottom) );
#endif
}

QPointArray QPainter::xForm( const QPointArray &av ) const
{						// map point array, v -> d
#if defined(OUR_XFORM)
    if ( !testf(VxF|WxF) )
	return av;
    QPointArray a = av.copy();
    int x, y;
    for ( int i=0; i<(int)a.size(); i++ ) {
	a.point( i, &x, &y );
	if ( testf(WxF) )
	    WXFORM_P( x, y )
	else if ( testf(VxF) )
	    VXFORM_P( x, y )
	a.setPoint( i, x, y );
    }
    return a;
#else
    if ( !hdc ) {
	return av.copy(); // !!!hanord what about ext devs???
    }
    QPointArray a = av.copy();
    LPtoDP( hdc, (POINT*)a.data(), a.size() );
    return a;
#endif
}

QPoint QPainter::xFormDev( const QPoint &pd ) const
{						// map point, device -> virtual
#if defined(OUR_XFORM)
    int x=pd.x(), y=pd.y();
    if ( testf(WxF) ) {
	int xx = im11*x+im21*y+idx;
	xx += xx > 0 ? 32768 : -32768;
	int yy = im12*x+im22*y+idy;
	yy += yy > 0 ? 32768 : -32768;
	x = xx/65536;
	y = yy/65536;
    }
    else if ( testf(VxF) ) {
	x = (ww*(x-vx))/vw + wx;
	y = (wh*(y-vy))/vh + wy;
    }
    return QPoint( x, y );
#else
    if ( !hdc ) {
	return pd;	// !!!hanord what about ext devs???
    }
    POINT p;
    p.x = pd.x();
    p.y = pd.y();
    DPtoLP( hdc, &p, 1 );
    return QPoint( p.x, p.y );
#endif
}

QRect QPainter::xFormDev( const QRect &rd ) const
{						// map rect, device -> virtual
#if defined(OUR_XFORM)
    if ( !testf(VxF|WxF) )
	return rd;
    int x, y, w, h;
    rd.rect( &x, &y, &w, &h );
    if ( testf(WxF) ) {
	int x1 = im11*x+im21*y+idx;
	int y1 = im12*x+im22*y+idy;
	int x2 = im11*(x+w-1)+im21*(y+h-1)+idx;
	int y2 = im12*(x+w-1)+im22*(y+h-1)+idy;
	x1 += x1>0 ? 32768 : -32768;
	y1 += y1>0 ? 32768 : -32768;
	x2 += x2>0 ? 32768 : -32768;
	y2 += y2>0 ? 32768 : -32768;
	x=x1/65536; y=y1/65536; w=(x2-x1)/65536+1; h=(y2-y1)/65536+1;
    }
    else if ( testf(VxF) ) {
	x = (ww*(x-vx))/vw + wx;
	y = (wh*(y-vy))/vh + wy;
	w = (ww*w)/vw;
	h = (wh*h)/vh;
    }
    return QRect( x, y, w, h );
#else
    if ( !hdc ) {
	return rd;	// !!!hanord what about ext devs???
    }
    RECT r;
    SetRect( &r, rd.left(), rd.top(), rd.right(), rd.bottom() );
    DPtoLP( hdc, (POINT*)&r, 2 );
    return QRect( QPoint(r.left, r.top),
		  QPoint(r.right,r.bottom) );
#endif
}

QPointArray QPainter::xFormDev( const QPointArray &ad ) const
{
#if defined(OUR_XFORM)
    if ( !testf(VxF|WxF) )
	return ad;
    QPointArray a = ad.copy();
    int x, y;
    for ( int i=0; i<(int)a.size(); i++ ) {
	a.point( i, &x, &y );
	if ( testf(WxF) ) {
	    int xx = im11*x+im21*y+idx;
	    xx += xx > 0 ? 32768 : -32768;
	    y = im12*x+im22*y+idy;
	    y += y > 0 ? 32768 : -32768;
	    x = xx/65536;
	    y /= 65536;
	}
	else if ( testf(VxF) ) {
	    x = (ww*(x-vx))/vw + wx;
	    y = (wh*(y-vy))/vh + wy;
	}
	a.setPoint( i, x, y );
    }
    return a;
#else
    if ( !hdc ) {
	return ad.copy(); // !!!hanord what about ext devs???
    }
    QPointArray a = ad.copy();
    DPtoLP( hdc, (POINT*)a.data(), a.size() );
    return a;
#endif
}


void QPainter::setClipping( bool enable )
{
    if ( !isActive() || enable == testf(ClipOn) )
	return;
    setf( ClipOn, enable );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = enable;
	if ( !pdev->cmd(PDC_SETCLIP,this,param) || !hdc )
	    return;
    }
    if ( testf(ClipOn) )
	SelectClipRgn( hdc, crgn.handle() );
    else
	SelectClipRgn( hdc, 0 );
}


void QPainter::setClipRect( const QRect &r )
{
    QRegion rgn( r );
    setClipRegion( rgn );
}

void QPainter::setClipRegion( const QRegion &rgn )
{
    crgn = rgn;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].rgn = &crgn;
	if ( !pdev->cmd(PDC_SETCLIPRGN,this,param) || !hdc )
	    return;
    }
    clearf( ClipOn );				// be sure to update clip rgn
    setClipping( TRUE );
}


void QPainter::drawPoint( int x, int y )
{
    if ( !isActive() || cpen.style() == NoPen )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	QPoint p( x, y );
	param[0].point = &p;
	if ( !pdev->cmd(PDC_DRAWPOINT,this,param) || !hdc )
	    return;
    }
    SetPixel( hdc, x, y, cpen.color().pixel() );
}


void QPainter::moveTo( int x, int y )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	QPoint p( x, y );
	param[0].point = &p;
	if ( !pdev->cmd(PDC_MOVETO,this,param) || !hdc )
	    return;
    }
#if defined(_WS_WIN32_)
    MoveToEx( hdc, x, y, 0 );
#else
    MoveTo( hdc, x, y );
#endif
}


void QPainter::lineTo( int x, int y )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	QPoint p( x, y );
	param[0].point = &p;
	if ( !pdev->cmd(PDC_LINETO,this,param) || !hdc )
	    return;
    }
    LineTo( hdc, x, y );
    SetPixel( hdc, x, y, cpen.color().pixel() );
}


void QPainter::drawLine( int x1, int y1, int x2, int y2 )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	QPoint p1( x1, y1 ), p2( x2, y2 );
	param[0].point = &p1;
	param[1].point = &p2;
	if ( !pdev->cmd(PDC_DRAWLINE,this,param) || !hdc )
	    return;
    }
#if defined(_WS_WIN32_)
    MoveToEx( hdc, x1, y1, 0 );
#else
    MoveTo( hdc, x1, y1 );
#endif
    if ( x1 == x2 ) {				// vertical
	if ( y1 < y2 )
	    y2++;
	else
	    y2--;
    }
    else if ( y1 == y2 ) {			// horizontal
	if ( x1 < x2 )
	    x2++;
	else
	    x2--;
    }
    else if ( cpen.style() == SolidLine )	// draw last pixel
	SetPixel( hdc, x2, y2, cpen.color().pixel() );
    LineTo( hdc, x2, y2 );
}


static void fix_neg_rect( int *x, int *y, int *w, int *h )
{
    if ( *w < 0 ) {
	*w = -*w;
	*x -= *w - 1;
    }
    if ( *h < 0 ) {
	*h = -*h;
	*y -= *h - 1;
    }
}


void QPainter::drawRect( int x, int y, int w, int h )
{
    if ( !isActive() )
	return;
#if defined(OUR_XFORM)
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    if ( !pdev->cmd(PDC_DRAWRECT,this,param) || !hdc )
		return;
	}
	if ( testf(WxF) ) {			// world transform
	    if ( wm12 == 0 && wm21 == 0 ) {	// scaling+translation only
		WXFORM_R(x,y,w,h);
	    }
	    else {
		QPointArray a( QRect(x,y,w,h) );// rectangle polygon
		a = xForm( a );			// xform polygon
		uint tmpf = flags;
		flags = IsActive | SafePolygon; // fake flags to speed up
		drawPolygon( a );
		flags = tmpf;
		return;
	    }
	}
	else if ( testf(VxF) )
	    VXFORM_R( x, y, w, h );
    }
#else
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	QRect r( x, y, w, h );
	param[0].rect = &r;
	if ( !pdev->cmd(PDC_DRAWRECT,this,param) || !hdc )
	    return;
    }
#endif
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() == NoPen ) {
	w++;
	h++;
    }
    if ( nocolBrush )
	SetTextColor( hdc, cbrush.color().pixel() );
    Rectangle( hdc, x, y, x+w, y+h );
    if ( nocolBrush )
	SetTextColor( hdc, cpen.color().pixel() );
}


void QPainter::drawRoundRect( int x, int y, int w, int h, int xRnd, int yRnd )
{
    if ( !isActive() )
	return;
    if ( xRnd <= 0 || yRnd <= 0 ) {
	drawRect( x, y, w, h );			// draw normal rectangle
	return;
    }
    if ( xRnd >= 100 )				// fix ranges
	xRnd = 99;
    if ( yRnd >= 100 )
	yRnd = 99;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	param[0].rect = &r;
	param[1].ival = xRnd;
	param[2].ival = yRnd;
	if ( !pdev->cmd(PDC_DRAWROUNDRECT,this,param) || !hdc )
	    return;
    }
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() == NoPen ) {
	w++;
	h++;
    }
    if ( nocolBrush )
	SetTextColor( hdc, cbrush.color().pixel() );
    RoundRect( hdc, x, y, x+w, y+h,
	       (int)(((long)w)*xRnd/100L),
	       (int)(((long)h)*yRnd/100L) );
    if ( nocolBrush )
	SetTextColor( hdc, cpen.color().pixel() );

}


void QPainter::drawEllipse( int x, int y, int w, int h )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	QRect r( x, y, w, h );
	param[0].rect = &r;
	if ( !pdev->cmd(PDC_DRAWELLIPSE,this,param) || !hdc )
	    return;
    }
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
    if ( nocolBrush )
	SetTextColor( hdc, cbrush.color().pixel() );
    Ellipse( hdc, x, y, x+w, y+h );
    if ( nocolBrush )
	SetTextColor( hdc, cpen.color().pixel() );
}


void QPainter::drawArc( int x, int y, int w, int h, int a, int alen )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	param[0].rect = &r;
	param[1].ival = a;
	param[2].ival = alen;
	if ( !pdev->cmd(PDC_DRAWARC,this,param) || !hdc )
	    return;
    }
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if ( ra2 < 0.0 ) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    double w2 = 0.5*w;
    double h2 = 0.5*h;
    float r = (float)(w2+h2);
    Arc( hdc, x, y, x+w, y+h,
	 qRound(w2 + (cos(ra1)*r) + x),
	 qRound(h2 - (sin(ra1)*r) + y),
	 qRound(w2 + (cos(ra2)*r) + x),
	 qRound(h2 - (sin(ra2)*r) + y) );
}


void QPainter::drawPie( int x, int y, int w, int h, int a, int alen )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	param[0].rect = &r;
	param[1].ival = a;
	param[2].ival = alen;
	if ( !pdev->cmd(PDC_DRAWPIE,this,param) || !hdc )
	    return;
    }
    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if ( ra2 < 0.0 ) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    double w2 = 0.5*w;
    double h2 = 0.5*h;
    float r = (float)(w2+h2);
    if ( nocolBrush )
	SetTextColor( hdc, cbrush.color().pixel() );
    Pie( hdc, x, y, x+w, y+h,
	 qRound(w2 + (cos(ra1)*r) + x),
	 qRound(h2 - (sin(ra1)*r) + y),
	 qRound(w2 + (cos(ra2)*r) + x),
	 qRound(h2 - (sin(ra2)*r) + y) );
    if ( nocolBrush )
	SetTextColor( hdc, cpen.color().pixel() );
}


void QPainter::drawChord( int x, int y, int w, int h, int a, int alen )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	param[0].rect = &r;
	param[1].ival = a;
	param[2].ival = alen;
	if ( !pdev->cmd(PDC_DRAWPIE,this,param) || !hdc )
	    return;
    }
    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if ( ra2 < 0.0 ) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    double w2 = 0.5*w;
    double h2 = 0.5*h;
    float r = (float)(w2+h2);
    if ( nocolBrush )
	SetTextColor( hdc, cbrush.color().pixel() );
    Chord( hdc, x, y, x+w, y+h,
	   qRound(w2 + (cos(ra1)*r) + x),
	   qRound(h2 - (sin(ra1)*r) + y),
	   qRound(w2 + (cos(ra2)*r) + x),
	   qRound(h2 - (sin(ra2)*r) + y) );
    if ( nocolBrush )
	SetTextColor( hdc, cpen.color().pixel() );
}


void QPainter::drawLineSegments( const QPointArray &a, int index, int nlines )
{
    if ( nlines < 0 )
	nlines = a.size()/2 - index/2;
    if ( index + nlines*2 > (int)a.size() )
	nlines = (a.size() - index)/2;
    if ( !isActive() || nlines < 1 || index < 0 )
	return;
    if ( testf(ExtDev) ) {
	QPointArray tmp;
	if ( nlines == (int)a.size()/2 )
	    tmp = a;
	else {
	    tmp.resize( nlines*2 );
	    for ( int i=0; i<nlines*2; i++ )
		tmp.setPoint( i, a.point(index+i) );
	}
	QPDevCmdParam param[1];
	param[0].ptarr = (QPointArray*)&tmp;
	if ( !pdev->cmd(PDC_DRAWLINESEGS,this,param) || !hdc )
	    return;
    }

    int x1, y1, x2, y2;
    uint i = index;
    bool  solid = cpen.style() == SolidLine;
    ulong pixel = cpen.color().pixel();

    while ( nlines-- ) {
	a.point( i++, &x1, &y1 );
	a.point( i++, &x2, &y2 );
	if ( x1 == x2 ) {			// vertical
	    if ( y1 < y2 )
		y2++;
	    else
		y2--;
	}
	else
	if ( y1 == y2 ) {			// horizontal
	    if ( x1 < x2 )
		x2++;
	    else
		x2--;
	}
	else if ( solid )			// draw last pixel
	    SetPixel( hdc, x2, y2, pixel );
#if defined(_WS_WIN32_)
	MoveToEx( hdc, x1, y1, 0 );
#else
	MoveTo( hdc, x1, y1 );
#endif
	LineTo( hdc, x2, y2 );
    }
}


void QPainter::drawPolyline( const QPointArray &a, int index, int npoints )
{
    if ( npoints < 0 )
	npoints = a.size() - index;
    if ( index + npoints > (int)a.size() )
	npoints = a.size() - index;
    if ( !isActive() || npoints < 2 || index < 0 )
	return;
    if ( testf(ExtDev) ) {
	QPointArray tmp;
	if ( npoints == (int)a.size() )
	    tmp = a;
	else {
	    tmp.resize( npoints );
	    for ( int i=0; i<npoints; i++ )
		tmp.setPoint( i, a.point(index+i) );
	}
	QPDevCmdParam param[1];
	param[0].ptarr = (QPointArray*)&tmp;
	if ( !pdev->cmd(PDC_DRAWPOLYLINE,this,param) || !hdc )
	    return;
    }
    Polyline( hdc, (POINT*)(a.data()+index), npoints );
    if ( cpen.style() == SolidLine ) {
	QPoint p = a.point( index+npoints-1 );	// plot last point
	SetPixel( hdc, p.x(), p.y(), cpen.color().pixel() );
    }
}


void QPainter::drawPolygon( const QPointArray &a, bool winding, int index,
			    int npoints )
{
    if ( npoints < 0 )
	npoints = a.size() - index;
    if ( index + npoints > (int)a.size() )
	npoints = a.size() - index;
    if ( !isActive() || npoints < 2 || index < 0 )
	return;
    if ( testf(ExtDev) ) {
	QPointArray tmp;
	if ( npoints == (int)a.size() )
	    tmp = a;
	else {
	    tmp.resize( npoints );
	    for ( int i=0; i<npoints; i++ )
		tmp.setPoint( i, a.point(index+i) );
	}
	QPDevCmdParam param[2];
	param[0].ptarr = (QPointArray*)&tmp;
	param[1].ival = winding;
	if ( !pdev->cmd(PDC_DRAWPOLYGON,this,param) || !hdc )
	    return;
    }
    if ( winding )				// set to winding fill mode
	SetPolyFillMode( hdc, WINDING );
    if ( nocolBrush )
	SetTextColor( hdc, cbrush.color().pixel() );
    Polygon( hdc, (POINT*)(a.data()+index), npoints );
    if ( nocolBrush )
	SetTextColor( hdc, cpen.color().pixel() );
    if ( winding )				// set to normal fill mode
	SetPolyFillMode( hdc, ALTERNATE );
}


void QPainter::drawBezier(  const QPointArray &a, int index, int npoints )
{
    if ( npoints < 0 )
	npoints = a.size() - index;
    if ( index + npoints > (int)a.size() )
	npoints = a.size() - index;
    if ( !isActive() || npoints < 2 || index < 0 )
	return;
    if ( testf(ExtDev) ) {
	QPointArray tmp;
	if ( npoints == (int)a.size() )
	    tmp = a;
	else {
	    tmp.resize( npoints );
	    for ( int i=0; i<npoints; i++ )
		tmp.setPoint( i, a.point(index+i) );
	}
	QPDevCmdParam param[1];
	param[0].ptarr = (QPointArray*)&tmp;
	if ( !pdev->cmd(PDC_DRAWBEZIER,this,param) || !hdc )
	    return;
    }
    PolyBezier( hdc, (POINT*)(a.data()+index), npoints );
}


void QPainter::drawPixmap( int x, int y, const QPixmap &pixmap,
			   int sx, int sy, int sw, int sh )
{
    if ( !isActive() || pixmap.isNull() )
	return;
    if ( sw < 0 )
	sw = pixmap.width();
    if ( sh < 0 )
	sh = pixmap.height();
    if ( testf(ExtDev) ) {
	if ( !hdc && (sx != 0 || sy != 0 ||
	     sw != pixmap.width() || sh != pixmap.height()) ) {
	    QPixmap tmp( sw, sh, pixmap.depth() );
	    bitBlt( &tmp, 0, 0, &pixmap, sx, sy, sw, sh );
	    drawPixmap( x, y, tmp );
	    return;
	}
	QPDevCmdParam param[2];
	QPoint p(x,y);
	param[0].point	= &p;
	param[1].pixmap = &pixmap;
	if ( !pdev->cmd(PDC_DRAWPIXMAP,this,param) || !hdc )
	    return;
    }
    QPixmap *pm = (QPixmap*)&pixmap;
    bool tmp_dc = pm->handle() == 0;
    if ( tmp_dc )
	pm->allocMemDC();
#if defined(MASK_WILL_BE_IN_PIXMAP)
    if ( pm->depth() == 1 && bg_mode == TransparentMode )
	MaskBlt( hdc, x, y, sw, sh, pm->handle(), sx, sy, pm->hbm(),
		 sx, sy, 0xccaa0000 );
#if 0
	SetBkColor( pm->handle(), RGB(255,255,255) );
	BitBlt( hdc, x, y, sw, sh, pm->handle(), sx, sy, MERGEPAINT );
	BitBlt( hdc, x, y, sw, sh, pm->handle(), sx, sy, SRCPAINT );
#endif
    else
#endif
	BitBlt( hdc, x, y, sw, sh, pm->handle(), sx, sy, SRCCOPY );
    if ( tmp_dc )
	pm->freeMemDC();
}


void QPainter::drawText( int x, int y, const char *str, int len )
{
    if ( !isActive() )
	return;
    if ( len < 0 )
	len = strlen( str );
    if ( len == 0 )				// empty string
	return;

    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	QPoint p( x, y );
	QString newstr = str;
	newstr.truncate( len );
	param[0].point = &p;
	param[1].str = newstr.data();
	if ( !pdev->cmd(PDC_DRAWTEXT,this,param) || !hdc )
	    return;
    }

    TextOut( hdc, x, y, str, len );
}


void QPainter::drawText( int x, int y, int w, int h, int tf,
			 const char *str, int len, QRect *brect,
			 char **internal )
{
    if ( !isActive() )
	return;
    if ( len < 0 )
	len = strlen( str );
    if ( len == 0 )				// empty string
	return;

    if ( testf(ExtDev) && (tf & DontPrint) == 0 ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	QString newstr = str;
	newstr.truncate( len );
	param[0].rect = &r;
	param[1].ival = tf;
	param[2].str = newstr.data();
	if ( !pdev->cmd(PDC_DRAWTEXTFRMT,this,param) || !hdc )
	    return;
    }

    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }

    QFontMetrics fm = fontMetrics();		// get font metrics
    bool wordbreak  = (tf & WordBreak)	!= 0;
    bool expandtabs = (tf & ExpandTabs) != 0;
    bool singleline = (tf & SingleLine) != 0;
    bool showprefix = (tf & ShowPrefix) != 0;
    bool containsnl;

    if ( singleline )
	containsnl = FALSE;
    else {
	if ( str[len] == '\0' )
	    containsnl = strchr(str,'\n') != 0;
	else {					// there's no strnchr
	    const char *p = str;
	    containsnl = FALSE;
	    for ( int i=0; i<len; i++ ) {
		if ( p[i] == '\n' ) {
		    containsnl = TRUE;
		    break;
		}
	    }
	}
    }

    if ( !wordbreak && !expandtabs && !containsnl && 0 ) {
	RECT r;
	r.left = x;
	r.top  = y;
	r.right	 = x + w - 1;
	r.bottom = y + h - 1;
	uint f = DT_SINGLELINE;
	if ( !showprefix )
	    f |= DT_NOPREFIX;
	if ( (tf & AlignVCenter) != 0 )		// vertically centered text
	    f |= DT_VCENTER;
	else if ( (tf & AlignBottom) != 0 )	// bottom aligned
	    f |= DT_BOTTOM;
	else					// top aligned
	    f |= DT_TOP;
	if ( (tf & AlignRight) != 0 )		// right aligned
	    f |= DT_RIGHT;
	else if ( (tf & AlignHCenter) != 0 )	// horizontally centered text
	    f |= DT_CENTER;
	else
	    f |= DT_LEFT;			// left aligned
	if ( (tf & DontClip) != 0 )
	    f |= DT_NOCLIP;
	if ( (tf & DontPrint) == 0 )
	    DrawText( hdc, str, len, &r, f );
	else {
	    DrawText( hdc, str, len, &r, f | DT_CALCRECT );
	    if ( brect )
		*brect = QRect( QPoint(r.left,r.top),
				QPoint(r.right,r.bottom) );
	}
	return;
    }

    struct text_info {				// internal text info
	char  tag[4];				// contains "qptr"
	int   w;				// width
	int   h;				// height
	int   tf;				// flags (alignment etc.)
	int   len;				// text length
	int   maxwidth;				// max text width
	int   nlines;				// number of lines
	int   codelen;				// length of encoding
    };

    ushort codearray[200];
    int	   codelen    = 200;
    bool   code_alloc = FALSE;
    ushort *codes     = codearray;
    ushort cc	      = 0;			// character code
    bool   decode     = internal && *internal;	// decode from internal data
    bool   encode     = internal && !*internal; // build internal data

    if ( len > 150 && !decode ) {		// need to alloc code array
	codelen = len + len/2;
	codes	= (ushort *)malloc( codelen*sizeof(ushort) );
	code_alloc = TRUE;
    }

    const BEGLINE  = 0x8000;			// encoding 0x8zzz, zzz=width
    const TABSTOP  = 0x4000;			// encoding 0x4zzz, zzz=tab pos
    const PREFIX   = 0x2000;			// encoding 0x20zz, zz=char
    const WIDTHBITS= 0x1fff;			// bits for width encoding
    const MAXWIDTH = 0x1fff;			// max width value

    char *p = (char *)str;
    int nlines;					// number of lines
    int index;					// index for codes
    int begline;				// index at beginning of line
    int breakindex;				// index where to break
    int breakwidth;				// width of text at breakindex
    int maxwidth;				// maximum width of a line
    int bcwidth;				// width of break char
    int tabindex;				// tab array index
    int cw;					// character width
    int k;					// index for p
    int tw;					// text width
    short charwidth[255];			// TO BE REMOVED LATER!!!
    memset( charwidth, -1, 255*sizeof(short) );

#define CWIDTH(x) (charwidth[x]>=0 ? charwidth[x] : (charwidth[x]=fm.width(x)))

    int	 spacewidth = CWIDTH( ' ' );		// width of space char

    nlines = 0;
    index  = 1;					// first index contains BEGLINE
    begline = breakindex = breakwidth = maxwidth = bcwidth = tabindex = 0;
    k = tw = 0;

    if ( decode )				// skip encoding
	k = len;

    while ( k < len ) {				// convert string to codes

	if ( *p > 32 ) {			// printable character
	    if ( *p == '&' && showprefix ) {
		cc = '&';			// assume ampersand
		if ( k < len-1 ) {
		    k++;
		    p++;
		    if ( *p != '&' && isprint(*p) )
			cc = PREFIX | *p;	// use prefix char
		}
	    }
	    else
		cc = *p;
	    cw = CWIDTH( cc & 0xff );
	}

	else {					// not printable (except ' ')

	    if ( *p == 32 ) {			// the space character
		cc = ' ';
		cw = spacewidth;
	    }

	    else if ( *p == '\n' ) {		// newline
		if ( singleline ) {
		    cc = ' ';			// convert newline to space
		    cw = spacewidth;
		}
		else {
		    cc = BEGLINE;
		    cw = 0;
		}
	    }

	    else if ( *p == '\t' ) {		// TAB character
		if ( expandtabs ) {
		    cw = 0;
		    if ( tabarray ) {		// use tab array
			while ( tabindex < tabarraylen ) {
			    if ( tabarray[tabindex] > tw ) {
				cw = tabarray[tabindex] - tw;
				tabindex++;
				break;
			    }
			    tabindex++;
			}
		    }
		    if ( cw == 0 && tabstops )	// use fixed tab stops
			cw = tabstops - tw%tabstops;
		    cc = TABSTOP | QMIN(tw+cw,MAXWIDTH);
		}
		else {				// convert TAB to space
		    cc = ' ';
		    cw = spacewidth;
		}
	    }

	    else {				// ignore character
		k++;
		p++;
		continue;
	    }

	    if ( wordbreak ) {			// possible break position
		breakindex = index;
		breakwidth = tw;
		bcwidth = cw;
	    }
	}

	if ( wordbreak && breakindex > 0 && tw+cw > w ) {
	    if ( index == breakindex ) {	// break at current index
		cc = BEGLINE;
		cw = 0;
	    }
	    else {				// break at breakindex
		codes[begline] = BEGLINE | QMIN(breakwidth,MAXWIDTH);
		maxwidth = QMAX(maxwidth,breakwidth);
		begline = breakindex;
		nlines++;
		tw -= breakwidth + bcwidth;
		breakindex = tabindex = 0;
	    }
	}

	tw += cw;				// increment text width

	if ( cc == BEGLINE ) {
	    codes[begline] = BEGLINE | QMIN(tw,MAXWIDTH);
	    maxwidth = QMAX(maxwidth,tw);
	    begline = index;
	    nlines++;
	    tw = 0;
	    breakindex = tabindex = 0;
	}
	codes[index++] = cc;
	if ( index >= codelen - 1 ) {		// grow code array
	    codelen *= 2;
	    if ( code_alloc )
		codes = (ushort *)realloc( codes, sizeof(ushort)*codelen );
	    else {
		codes = (ushort *)malloc( sizeof(ushort)*codelen );
		code_alloc = TRUE;
	    }
	}
	k++;
	p++;
    }

    if ( decode ) {				// decode from internal data
	char	  *data = *internal;
	text_info *ti	= (text_info*)data;
	if ( strncmp(ti->tag,"qptr",4)!=0 || ti->w != w || ti->h != h ||
	     ti->tf != tf || ti->len != len ) {
#if defined(CHECK_STATE)
	    warning( "QPainter::drawText: Internal text info is invalid" );
#endif
	    return;
	}
	maxwidth = ti->maxwidth;		// get internal values
	nlines	 = ti->nlines;
	codelen	 = ti->codelen;
	codes	 = (ushort *)(data + sizeof(text_info));
    }
    else {
	codes[begline] = BEGLINE | QMIN(tw,MAXWIDTH);
	maxwidth = QMAX(maxwidth,tw);
	nlines++;
	codes[index++] = 0;
	codelen = index;
    }

    if ( encode ) {				// build internal data
	char	  *data = new char[sizeof(text_info)+codelen*sizeof(ushort)];
	text_info *ti	= (text_info*)data;
	strncpy( ti->tag, "qptr", 4 );		// set tag
	ti->w	     = w;			// save parameters
	ti->h	     = h;
	ti->tf	     = tf;
	ti->len	     = len;
	ti->maxwidth = maxwidth;
	ti->nlines   = nlines;
	ti->codelen  = codelen;
	memcpy( data+sizeof(text_info), codes, codelen*sizeof(ushort) );
	*internal = data;
    }

    int	    fascent  = fm.ascent();		// get font measurements
    int	    fheight  = fm.height();
    QRegion save_rgn = crgn;			// save the current region
    bool    clip_on  = testf(ClipOn);
    int	    xp, yp;
    char    p_array[200];
    bool    p_alloc;

    if ( (tf & AlignVCenter) != 0 )		// vertically centered text
	yp = h/2 - nlines*fheight/2;
    else if ( (tf & AlignBottom) != 0 )		// bottom aligned
	yp = h - nlines*fheight;
    else					// top aligned
	yp = 0;
    if ( (tf & AlignRight) != 0 )		// right aligned
	xp = w - maxwidth;
    else if ( (tf & AlignHCenter) != 0 )	// horizontally centered text
	xp = w/2 - maxwidth/2;
    else
	xp = 0;					// left aligned

    QRect br( x+xp, y+yp, maxwidth, nlines*fheight );
    if ( brect )				// set bounding rect
	*brect = br;

    if ( (tf & DontPrint) != 0 ) {		// don't print any text
	if ( code_alloc )
	    free( codes );
	return;
    }

    if ( len > 200 ) {
	p = new char[len];			// buffer for printable string
	CHECK_PTR( p );
	p_alloc = TRUE;
    }
    else {
	p = p_array;
	p_alloc = FALSE;
    }

    if ( br.x() >= x && br.y() >= y && br.width() < w && br.height() < h )
	tf |= DontClip;				// no need to clip

    if ( (tf & DontClip) == 0 ) {		// clip text
	QRegion new_rgn;
	QRect r( x, y, w, h );
	if ( testf(WxF) ) {			// world xform active
	    QPointArray a( r );			// complex region
	    a = xForm( a );
	    new_rgn = QRegion( a );
	}
	else {
	    r = xForm( r );
	    new_rgn = QRegion( r );
	}
	if ( clip_on )				// add to existing region
	    new_rgn = new_rgn.intersect( crgn );
	setClipRegion( new_rgn );
    }

    QPixmap  *pm;
    QPainter *pp;

    if ( (tf & GrayText) == GrayText ) {	// prepare to draw gray text
	pm = new QPixmap( w, fheight );
	CHECK_PTR( pm );
	pp = new QPainter;
	CHECK_PTR( pp );
	pp->begin( pm );
	pp->setBackgroundColor( bg_col );
	pp->setFont( cfont );
	pp->setPen( cpen.color() );
	pp->updatePen();
	pp->setBrush( QBrush(bg_col, Dense4Pattern) );
	pp->updateBrush();
    }
    else {
	pm = 0;
	pp = 0;
    }

    yp += fascent;

    register ushort *cp = codes;

    while ( *cp ) {				// finally, draw the text

	tw = *cp++ & WIDTHBITS;			// text width

	if ( tw == 0 ) {			// ignore empty line
	    while ( *cp && (*cp & BEGLINE) == 0 )
		cp++;
	    yp += fheight;
	    continue;
	}

	if ( (tf & AlignRight) != 0 )		// right aligned
	    xp = w - tw;
	else if ( (tf & AlignHCenter) != 0 )	// centered text
	    xp = w/2 - tw/2;
	else					// left aligned
	    xp = 0;

	if ( pp )				// erase pixmap if gray text
	    pp->eraseRect( 0, 0, w, fheight );

	int bx = xp;				// base x position
	while ( TRUE ) {
	    k = 0;
	    while ( *cp && (*cp & (BEGLINE|TABSTOP)) == 0 ) {
		if ( (*cp & PREFIX) != 0 ) {
		    int xcpos = fm.width( p, k );
		    if ( pp )			// gray text
			pp->fillRect( xp+xcpos, fascent+fm.underlinePos(),
				      CWIDTH( *cp&0xff ), fm.lineWidth(),
				      cpen.color() );
		    else
			fillRect( x+xp+xcpos, y+yp+fm.underlinePos(),
				  CWIDTH( *cp&0xff ), fm.lineWidth(),
				  cpen.color() );
		}
		p[k++] = (char)*cp++;
		index++;
	    }
	    if ( pp )				// gray text
		pp->drawText( xp, fascent, p, k );
	    else
		drawText( x+xp, y+yp, p, k );	// draw the text
	    if ( (*cp & TABSTOP) != 0 )
		xp = bx + (*cp++ & WIDTHBITS);
	    else				// *cp == 0 || *cp == BEGLINE
		break;
	}
	if ( pp ) {				// gray text
	    pp->setPen( NoPen );
	    pp->drawRect( bx, 0, tw, fheight );
	    drawPixmap( x, y+yp-fascent, *pm );
	}

	yp += fheight;
    }

    if ( pp ) {					// gray text
	pp->end();
	delete pp;
	delete pm;
    }

    if ( (tf & DontClip) == 0 ) {		// restore clipping
	if ( clip_on )				// set original region
	    setClipRegion( save_rgn );
	else {					// clipping was off
	    crgn = save_rgn;
	    SelectClipRgn( hdc, 0 );
	}
    }

    if ( p_alloc )
	delete [] p;
    if ( code_alloc )
	free( codes );
}


QRect QPainter::boundingRect( int x, int y, int w, int h, int tf,
			      const char *str, int len, char **internal )
{
    QRect brect;
    drawText( x, y, w, h, tf | DontPrint, str, len, &brect, internal );
    return brect;
}
