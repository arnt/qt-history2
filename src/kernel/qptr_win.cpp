/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptr_win.cpp#83 $
**
** Implementation of QPainter class for Win32
**
** Created : 940112
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpainter.h"
#include "qpaintdc.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qpmcache.h"
#include "qlist.h"
#include "qintdict.h"
#include <stdlib.h>
#include <math.h>

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

extern WindowsVersion qt_winver;		// defined in qapp_win.cpp

RCSTAG("$Id: //depot/qt/main/src/kernel/qptr_win.cpp#83 $");


/*
  QWinFont holds extra font settings for the painter.
*/

struct QWinFont
{
    bool	killFont;
    HANDLE	hfont;
    TEXTMETRIC	tm;
};


#define COLOR_VALUE(c) ((flags & RGBColor) ? c.rgb() : c.pixel())


/*****************************************************************************
  QPainter internal pen and brush cache

  The cache makes a significant contribution to speeding up drawing.
  Setting a new pen or brush specification will make the painter look for
  an existing pen or brush with the same attributes instead of creating
  a new pen or brush. The cache structure is optimized for fast lookup.
  Only solid line pens with line width 0 and solid brushes are cached.
 *****************************************************************************/

struct QHDCObj					// cached pen or brush
{
    HANDLE  obj;
    uint    pix;
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
static HANDLE stock_sysfont;

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


static bool obtain_obj( void **ref, HANDLE *obj, uint pix, QHDCObj **cache,
			bool is_pen )
{
    if ( !cache_init )
	init_cache();

    int	     k = (pix % cache_size) * 4;
    QHDCObj *h = cache[k];
    QHDCObj *prev = 0;

#define NOMATCH (h->obj && h->pix != pix)

    if ( NOMATCH ) {
	prev = h;
	h = cache[++k];
	if ( NOMATCH ) {
	    prev = h;
	    h = cache[++k];
	    if ( NOMATCH ) {
		prev = h;
		h = cache[++k];
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
			cache[k]   = prev;
			cache[k-1] = h;
			*ref = (void *)h;
			*obj = h->obj;
			return TRUE;
		    } else {			// all objects in use
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
	    cache[k]   = prev;
	    cache[k-1] = h;
	}
    } else {					// create new pen/brush
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
    ((QHDCObj*)ref)->count--;
}

static inline bool obtain_pen( void **ref, HANDLE *pen, uint pix )
{ return obtain_obj( ref, pen, pix, pen_cache, TRUE ); }

static inline bool obtain_brush( void **ref, HANDLE *brush, uint pix )
{ return obtain_obj( ref, brush, pix, brush_cache, FALSE ); }

#define release_pen	release_obj
#define release_brush	release_obj


/*****************************************************************************
  QPainter member functions
 *****************************************************************************/

const int TxNone      = 0;			// transformation codes
const int TxTranslate = 1;			// also in qpainter.cpp
const int TxScale     = 2;
const int TxRotShear  = 3;


void QPainter::initialize()
{
    stock_nullPen    = GetStockObject( NULL_PEN );
    stock_blackPen   = GetStockObject( BLACK_PEN );
    stock_whitePen   = GetStockObject( WHITE_PEN );
    stock_nullBrush  = GetStockObject( NULL_BRUSH );
    stock_blackBrush = GetStockObject( BLACK_BRUSH );
    stock_whiteBrush = GetStockObject( WHITE_BRUSH );
    stock_sysfont    = GetStockObject( SYSTEM_FONT );
    init_cache();
}

void QPainter::cleanup()
{
    cleanup_cache();
}


typedef Q_DECLARE(QIntDictM,QPaintDevice) QPaintDeviceDict;
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
    if ( replacement ) {
	pdev_dict->insert( (long)pdev, replacement );
    } else {
	pdev_dict->remove( (long)pdev );
	if ( pdev_dict->count() == 0 ) {
	    delete pdev_dict;
	    pdev_dict = 0;
	}
    }
}


void QPainter::init()
{
    flags = IsStartingUp;
    bg_col = white;				// default background color
    bg_mode = TransparentMode;			// default background mode
    rop = CopyROP;				// default ROP
    tabstops = 0;				// default tabbing
    tabarray = 0;
    tabarraylen = 0;
    ps_stack = 0;
    pdev = 0;
    hdc = hpen = hbrush = hbrushbm = 0;
    txop = txinv = 0;
    pixmapBrush = nocolBrush = FALSE;
    penRef = brushRef = 0;
    winFont = 0;
}


void QPainter::setFont( const QFont &font )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setFont: Will be reset by begin()" );
#endif
    if ( cfont.d != font.d ) {
	cfont = font;
	setf(DirtyFont);
    }
}


void *QPainter::textMetric()
{
    if ( !isActive() )
	return 0;
    if ( winFont == 0 || testf(DirtyFont) )
	updateFont();
    return &winFont->tm;
}


void QPainter::updateFont()
{
    clearf(DirtyFont);
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].font = &cfont;
	if ( !pdev->cmd(PDC_SETFONT,this,param) || !hdc )
	    return;
    }
    HANDLE hfont;
    bool   ownFont = pdev->devType() == PDT_PRINTER;
    bool   killFont;
    if ( ownFont ) {
	bool stockFont;
	hfont = cfont.create( &stockFont, testf(VxF|WxF) ? 0 : hdc );
	killFont = !stockFont;
    } else {
	hfont = cfont.handle();
	killFont = FALSE;
    }
    SelectObject( hdc, hfont );
    if ( winFont ) {
	if ( winFont->killFont )
	    DeleteObject( winFont->hfont );
    } else {
	winFont = new QWinFont;
	CHECK_PTR( winFont );
    }
    winFont->killFont = killFont;
    winFont->hfont = hfont;
    if ( ownFont )
	GetTextMetrics( hdc, &winFont->tm );
    else
	memcpy( &winFont->tm, cfont.textMetric(), sizeof(TEXTMETRIC) );
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
    uint   pix	   = COLOR_VALUE(cpen.data->color);
    bool   cacheIt = ps == NoPen || (ps == SolidLine && cpen.width() == 0);
    HANDLE hpen_old;

    if ( penRef ) {
	release_pen( penRef );
	penRef = 0;
	hpen_old = 0;
    } else {
	hpen_old = hpen;
    }
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
    static short d1_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static short d2_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static short d3_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static short d4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static short d5_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static short d6_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static short d7_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static short *dense_patterns[]
	= { d1_pat, d2_pat, d3_pat, d4_pat, d5_pat, d6_pat, d7_pat };

    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].brush = &cbrush;
	if ( !pdev->cmd(PDC_SETBRUSH,this,param) || !hdc )
	    return;
    }

    int	   bs	   = cbrush.style();
    uint   pix	   = COLOR_VALUE(cbrush.data->color);
    bool   cacheIt = bs == NoBrush || bs == SolidPattern;
    HANDLE hbrush_old;

    if ( brushRef ) {
	release_brush( brushRef );
	brushRef = 0;
	hbrush_old = 0;
    } else {
	hbrush_old = hbrush;
    }
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
	if ( obtain_brush(&brushRef, &hbrush, pix) ) {
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

    if ( bs == SolidPattern ) {			// create solid brush
	hbrush = CreateSolidBrush( pix );
    } else if ( (bs >= Dense1Pattern && bs <= Dense7Pattern ) ||
		(bs == CustomPattern) ) {
	if ( bs == CustomPattern ) {
	    hbrushbm = cbrush.pixmap()->hbm();
	    pixmapBrush = TRUE;
	    nocolBrush = cbrush.pixmap()->depth() == 1;
	} else {
	    short *bm = dense_patterns[ bs - Dense1Pattern ];
	    hbrushbm = CreateBitmap( 8, 8, 1, 1, bm );
	    nocolBrush = TRUE;
	}
	hbrush = CreatePatternBrush( hbrushbm );
    } else {					// one of the hatch brushes
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
	hbrush = CreateHatchBrush( s, pix );
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
	warning( "QPainter::begin: Painter is already active."
		 "\n\tYou must end() the painter before a second begin()" );
#endif
	return FALSE;
    }
    if ( pd == 0 ) {
#if defined(CHECK_NULL)
	warning( "QPainter::begin: Paint device cannot be null" );
#endif
	return FALSE;
    }

    QWidget *copyFrom = 0;
    if ( pdev_dict ) {				// redirected paint device?
	pdev = pdev_dict->find( (long)pd );
	if ( pdev ) {
	    if ( pd->devType() == PDT_WIDGET )
		copyFrom = (QWidget *)pd;	// copy widget settings
	} else {
	    pdev = (QPaintDevice *)pd;
	}
    } else {
	pdev = (QPaintDevice *)pd;
    }

    if ( pdev->paintingActive() ) {		// somebody else is already painting
#if defined(CHECK_STATE)
	warning( "QPainter::begin: Another QPainter is already painting "
		 "this device;\n\tA paint device can only be painted by "
		 "one QPainter at a time" );
#endif
	return FALSE;
    }

    bool reinit = flags != IsStartingUp;	// 2nd, 3rd,.... time called
    flags = IsActive;				// init flags
    int dt = pdev->devType();			// get the device type

    if ( (pdev->devFlags & PDF_EXTDEV) != 0 )	// this is an extended device
	setf(ExtDev);
    else if ( dt == PDT_PIXMAP )		// device is a pixmap
	((QPixmap*)pdev)->detach();		// will modify it

    hdc = 0;
    holdpal = 0;

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
	txop = txinv = 0;
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
	cfont = w->font();			// use widget font
	cpen = QPen( w->foregroundColor() );	// use widget fg color
	if ( reinit ) {
	    QBrush defaultBrush;
	    cbrush = defaultBrush;
	}
	bg_col	= w->backgroundColor();		// use widget bg color
	ww = vw = w->width();			// default view size
	wh = vh = w->height();
	if ( w->testWFlags(WState_PaintEvent) ) {
	    hdc = w->hdc;			// during paint event
	} else {
	    if ( w->testWFlags(WPaintUnclipped) )
		hdc = GetWindowDC( w->winId() );
	    else
		hdc = GetDC( w->winId() );
	    w->hdc = hdc;
	}
    } else if ( dt == PDT_PIXMAP ) {		// device is a pixmap
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
    } else if ( dt == PDT_PRINTER ) {		// device is a printer
	if ( pdev->handle() )
	    hdc = pdev->handle();
	flags |= (NoCache | RGBColor);
    }
    if ( testf(ExtDev) ) {
	ww = vw = pdev->metric( PDM_WIDTH );
	wh = vh = pdev->metric( PDM_HEIGHT );
    }
    if ( ww == 0 )
	ww = wh = vw = vh = 1024;
    if ( copyFrom ) {				// copy redirected widget
	cfont = copyFrom->font();
	cpen = QPen( copyFrom->foregroundColor() );
	bg_col = copyFrom->backgroundColor();
    }
    if ( testf(ExtDev) && hdc == 0 ) {		// external device
	setBackgroundColor( bg_col );		// default background color
	setBackgroundMode( TransparentMode );	// default background mode
	setRasterOp( CopyROP );			// default raster operation
    }
    if ( hdc ) {				// initialize hdc
	if ( QColor::hPal() && dt != PDT_PRINTER ) {
	    holdpal = SelectPalette( hdc, QColor::hPal(), TRUE );
	    RealizePalette( hdc );
	}
	SetBkColor( hdc, COLOR_VALUE(bg_col) );
	SetBkMode( hdc, TRANSPARENT );
	SetROP2( hdc, R2_COPYPEN );
	SetTextAlign( hdc, TA_BASELINE );
	SetStretchBltMode( hdc, COLORONCOLOR );
    }
    updatePen();
    updateBrush();
    setf(DirtyFont);
    return TRUE;
}

bool QPainter::end()
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
	warning( "QPainter::end: Missing begin() or begin() failed" );
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
	} else {
	    DeleteObject( hpen );
	}
	hpen = 0;
    }
    if ( hbrush ) {
	SelectObject( hdc, stock_nullBrush );
	if ( brushRef ) {
	    release_brush( brushRef );
	    brushRef = 0;
	} else {
	    DeleteObject( hbrush );
	    if ( hbrushbm && !pixmapBrush )
		DeleteObject( hbrushbm );
	}
	hbrush = hbrushbm = 0;
	pixmapBrush = nocolBrush = FALSE;
    }
    if ( winFont ) {
	SelectObject( hdc, stock_sysfont );
	if ( winFont->killFont )
	    DeleteObject( winFont->hfont );
	delete winFont;
	winFont = 0;
    }
    if ( holdpal ) {
	SelectPalette( hdc, holdpal, TRUE );
	RealizePalette( hdc );
    }

    if ( testf(ExtDev) )
	pdev->cmd( PDC_END, this, 0 );

    if ( pdev->devType() == PDT_WIDGET ) {
	if ( !((QWidget*)pdev)->testWFlags(WState_PaintEvent) ) {
	    QWidget *w = (QWidget*)pdev;
	    ReleaseDC( w->winId(), hdc );
	    w->hdc = 0;
	}
    } else if ( pdev->devType() == PDT_PIXMAP ) {
	QPixmap *pm = (QPixmap*)pdev;
	pm->freeMemDC();
	if ( pm->isOptimized() )
	    pm->allocMemDC();
    }

    flags = 0;
    pdev->devFlags &= ~PDF_PAINTACTIVE;
    pdev = 0;
    hdc	 = 0;
    return TRUE;
}


void QPainter::flush()
{
    GdiFlush();
}


void QPainter::setBackgroundColor( const QColor &c )
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
	warning( "QPainter::setBackgroundColor: Call begin() first" );
#endif
	return;
    }
    bg_col = c;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].color = &bg_col;
	if ( !pdev->cmd(PDC_SETBKCOLOR,this,param) || !hdc )
	    return;
    }
    SetBkColor( hdc, COLOR_VALUE(c) );
}

void QPainter::setBackgroundMode( BGMode m )
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
	warning( "QPainter::setBackgroundMode: Call begin() first" );
#endif
	return;
    }
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

    if ( !isActive() ) {
#if defined(CHECK_STATE)
	warning( "QPainter::setRasterOp: Call begin() first" );
#endif
	return;
    }
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
    if ( !isActive() ) {
#if defined(CHECK_STATE)
	warning( "QPainter::setBrushOrigin: Call begin() first" );
#endif
	return;
    }
    bro = QPoint(x,y);
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].point = &bro;
	if ( !pdev->cmd(PDC_SETBRUSHORIGIN,this,param) || !hdc )
	    return;
    }
#if defined(_WS_WIN32_)
    SetBrushOrgEx( hdc, x, y, 0 );
#else
    SetBrushOrg( hdc, x, y );
#endif
}


void QPainter::nativeXForm( bool enable )
{
    if ( enable ) {
	XFORM m;
	QWMatrix mtx;
	if ( testf(VxF) ) {
	    mtx.translate( vx, vy );
	    mtx.scale( 1.0*vw/ww, 1.0*vh/wh );
	    mtx.translate( -wx, -wy );
	    mtx = wxmat * mtx;
	} else {
	    mtx = wxmat;
	}
	m.eM11 = mtx.m11();
	m.eM12 = mtx.m12();
	m.eM21 = mtx.m21();
	m.eM22 = mtx.m22();
	m.eDx  = mtx.dx();
	m.eDy  = mtx.dy();
	SetGraphicsMode( hdc, GM_ADVANCED );
	SetWorldTransform( hdc, &m );
    } else {
	SetGraphicsMode( hdc, GM_ADVANCED );
	ModifyWorldTransform( hdc, 0, MWT_IDENTITY );
    }
}


void QPainter::setClipping( bool enable )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setClipping: Will be reset by begin()" );
#endif
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
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setClipRegion: Will be reset by begin()" );
#endif
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


void QPainter::drawPolyInternal( const QPointArray &a, bool close )
{
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cbrush.data->color) );
    if ( close ) {
	Polygon( hdc, (POINT*)a.data(), a.size() );
    } else {
	Polyline( hdc, (POINT*)a.data(), a.size() );	
    }
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cpen.data->color) );
}


void QPainter::drawPoint( int x, int y )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QPoint p( x, y );
	    param[0].point = &p;
	    if ( !pdev->cmd(PDC_DRAWPOINT,this,param) || !hdc )
		return;
	}
	map( x, y, &x, &y );
    }
    if ( cpen.style() != NoPen )
	SetPixelV( hdc, x, y, COLOR_VALUE(cpen.data->color) );
}


void QPainter::drawPoints( const QPointArray& a, int index, int npoints )
{
    if ( npoints < 0 )
	npoints = a.size() - index;
    if ( index + npoints > (int)a.size() )
	npoints = a.size() - index;
    if ( !isActive() || npoints < 1 || index < 0 )
	return;
    QPointArray pa = a;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    for (int i=0; i<npoints; i++) {
		QPoint p( pa[index+i].x(), pa[index+i].y() );
		param[0].point = &p;
		if ( !pdev->cmd(PDC_DRAWPOINT,this,param))
		    return;
	    }
	    if ( !hdc ) return;
	}
	if ( txop != TxNone ) {
	    pa = xForm( a, index, npoints );
	    if ( pa.size() != a.size() ) {
		index = 0;
		npoints = pa.size();
	    }		
	}
    }
    if ( cpen.style() != NoPen ) {
	for (int i=0; i<npoints; i++) {
	    SetPixelV( hdc, pa[index+i].x(), pa[index+i].y(),
		       COLOR_VALUE(cpen.data->color) );
	}
    }
}


void QPainter::moveTo( int x, int y )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QPoint p( x, y );
	    param[0].point = &p;
	    if ( !pdev->cmd(PDC_MOVETO,this,param) || !hdc )
		return;
	}
	map( x, y, &x, &y );
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
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QPoint p( x, y );
	    param[0].point = &p;
	    if ( !pdev->cmd(PDC_LINETO,this,param) || !hdc )
		return;
	}
	map( x, y, &x, &y );
    }
    LineTo( hdc, x, y );
    if ( cpen.style() != NoPen )
	SetPixelV( hdc, x, y, COLOR_VALUE(cpen.data->color) );
}


void QPainter::drawLine( int x1, int y1, int x2, int y2 )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[2];
	    QPoint p1(x1, y1), p2(x2, y2);
	    param[0].point = &p1;
	    param[1].point = &p2;
	    if ( !pdev->cmd(PDC_DRAWLINE,this,param) || !hdc )
		return;
	}
	map( x1, y1, &x1, &y1 );
	map( x2, y2, &x2, &y2 );
    }
    POINT pts[2];
    bool plot_pixel = FALSE;
    if ( x1 == x2 ) {				// vertical
	if ( y1 < y2 )
	    y2++;
	else
	    y2--;
    } else if ( y1 == y2 ) {			// horizontal
	if ( x1 < x2 )
	    x2++;
	else
	    x2--;
    } else {
	plot_pixel = cpen.style() == SolidLine; // plot last pixel
    }
    pts[0].x = x1;  pts[0].y = y1;
    pts[1].x = x2;  pts[1].y = y2;
    Polyline( hdc, pts, 2 );
    if ( plot_pixel )
	SetPixelV( hdc, x2, y2, COLOR_VALUE(cpen.data->color) );
}


void QPainter::drawRect( int x, int y, int w, int h )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    if ( !pdev->cmd(PDC_DRAWRECT,this,param) || !hdc )
		return;
	}
	if ( txop == TxRotShear ) {		// rotate/shear polygon
	    QPointArray a( QRect(x,y,w,h) );
	    drawPolyInternal( xForm(a) );
	    return;
	}
	map( x, y, w, h, &x, &y, &w, &h );
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
    if ( nocolBrush ) {
	if ( bg_mode == TransparentMode )
	{
 	    if ( cbrush.color() == color0 )
		// DPna  dest = dest AND NOT pattern
		PatBlt( hdc, x, y, w, h, 0x000A0329 );
	    else
		// DPo   dest = dest OR pattern
		PatBlt( hdc, x, y, w, h, 0x00FA0089 );
	} else {
	    SetTextColor( hdc, COLOR_VALUE(cbrush.data->color) );
	    Rectangle( hdc, x, y, x+w, y+h );
	    SetTextColor( hdc, COLOR_VALUE(cpen.data->color) );
	}
    } else {
        Rectangle( hdc, x, y, x+w, y+h );
    }
}


void QPainter::drawWinFocusRect( int x, int y, int w, int h, const QColor & )
{
    drawWinFocusRect( x, y, w, h );
}

void QPainter::drawWinFocusRect( int x, int y, int w, int h )
{
    if ( !isActive() || txop == TxRotShear )
	return;

    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    if ( !pdev->cmd(PDC_DRAWRECT,this,param) || !hdc )
		return;
	}
	map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }

    RECT r;
    r.left   = x;
    r.right  = x + w;
    r.top    = y;
    r.bottom = y + h;
    DrawFocusRect( hdc, &r );
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
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[3];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    param[1].ival = xRnd;
	    param[2].ival = yRnd;
	    if ( !pdev->cmd(PDC_DRAWROUNDRECT,this,param) || !hdc )
		return;
	}
	if ( txop == TxRotShear ) {		// rotate/shear polygon
	    QPointArray a;
	    if ( w <= 0 || h <= 0 )
		fix_neg_rect( &x, &y, &w, &h );
	    w--;
	    h--;
	    int rxx = w*xRnd/200;
	    int ryy = h*yRnd/200;
	    int rxx2 = 2*rxx;
	    int ryy2 = 2*ryy;
	    int xx, yy;
	    a.makeEllipse( x, y, rxx2, ryy2 );
	    int s = a.size()/4;
	    int i = 0;
	    while ( i < s ) {
		a.point( i, &xx, &yy );
		xx += w - rxx2;
		a.setPoint( i++, xx, yy );
	    }
	    i = 2*s;
	    while ( i < 3*s ) {
		a.point( i, &xx, &yy );
		yy += h - ryy2;
		a.setPoint( i++, xx, yy );
	    }
	    while ( i < 4*s ) {
		a.point( i, &xx, &yy );
		xx += w - rxx2;
		yy += h - ryy2;
		a.setPoint( i++, xx, yy );
	    }
	    drawPolyInternal( xForm(a) );
	    return;
	}
	map( x, y, w, h, &x, &y, &w, &h );
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
	SetTextColor( hdc, COLOR_VALUE(cbrush.data->color) );
    RoundRect( hdc, x, y, x+w, y+h, w*xRnd/100, h*yRnd/100 );
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cpen.data->color) );

}


void QPainter::drawEllipse( int x, int y, int w, int h )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    if ( !pdev->cmd(PDC_DRAWELLIPSE,this,param) || !hdc )
		return;
	}
	if ( txop == TxRotShear ) {		// rotate/shear polygon
	    QPointArray a;
	    a.makeEllipse( x, y, w, h );
	    drawPolyInternal( xForm(a) );
	    return;
	}
	map( x, y, w, h, &x, &y, &w, &h );
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
	SetTextColor( hdc, COLOR_VALUE(cbrush.data->color) );
    Ellipse( hdc, x, y, x+w, y+h );
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cpen.data->color) );
}


void QPainter::drawArc( int x, int y, int w, int h, int a, int alen )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[3];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    param[1].ival = a;
	    param[2].ival = alen;
	    if ( !pdev->cmd(PDC_DRAWARC,this,param) || !hdc )
		return;
	}
	if ( txop == TxRotShear ) {		// rotate/shear
	    QPointArray pa;
	    pa.makeArc( x, y, w, h, a, alen );	// arc polyline
	    drawPolyInternal( xForm(pa), FALSE );
	    return;
	}
	map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if ( alen < 0.0 ) {				// swap angles
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
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[3];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    param[1].ival = a;
	    param[2].ival = alen;
	    if ( !pdev->cmd(PDC_DRAWPIE,this,param) || !hdc )
		return;
	}
	if ( txop == TxRotShear ) {		// rotate/shear
	    QPointArray pa;
	    pa.makeArc( x, y, w, h, a, alen );	// arc polyline
	    int n = pa.size();
	    pa.resize( n+2 );
	    pa.setPoint( n, x+w/2, y+h/2 );	// add legs
	    pa.setPoint( n+1, pa.at(0) );
	    drawPolyInternal( xForm(pa) );
	    return;
	}
	map( x, y, w, h, &x, &y, &w, &h );
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
    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if ( alen < 0.0 ) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    double w2 = 0.5*w;
    double h2 = 0.5*h;
    float r = (float)(w2+h2);
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cbrush.data->color) );
    Pie( hdc, x, y, x+w, y+h,
	 qRound(w2 + (cos(ra1)*r) + x),
	 qRound(h2 - (sin(ra1)*r) + y),
	 qRound(w2 + (cos(ra2)*r) + x),
	 qRound(h2 - (sin(ra2)*r) + y) );
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cpen.data->color) );
}


void QPainter::drawChord( int x, int y, int w, int h, int a, int alen )
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[3];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    param[1].ival = a;
	    param[2].ival = alen;
	    if ( !pdev->cmd(PDC_DRAWCHORD,this,param) || !hdc )
		return;
	}
	if ( txop == TxRotShear ) {		// rotate/shear
	    QPointArray pa;
	    pa.makeArc( x, y, w-1, h-1, a, alen ); // arc polygon
	    int n = pa.size();
	    pa.resize( n+1 );
	    pa.setPoint( n, pa.at(0) );		// connect endpoints
	    drawPolyInternal( xForm(pa) );
	    return;
	}
	map( x, y, w, h, &x, &y, &w, &h );
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
	SetTextColor( hdc, COLOR_VALUE(cbrush.data->color) );
    Chord( hdc, x, y, x+w, y+h,
	   qRound(w2 + (cos(ra1)*r) + x),
	   qRound(h2 - (sin(ra1)*r) + y),
	   qRound(w2 + (cos(ra2)*r) + x),
	   qRound(h2 - (sin(ra2)*r) + y) );
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cpen.data->color) );
}


void QPainter::drawLineSegments( const QPointArray &a, int index, int nlines )
{
    if ( nlines < 0 )
	nlines = a.size()/2 - index/2;
    if ( index + nlines*2 > (int)a.size() )
	nlines = (a.size() - index)/2;
    if ( !isActive() || nlines < 1 || index < 0 )
	return;
    QPointArray pa = a;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    if ( nlines != (int)pa.size()/2 ) {
		pa.resize( nlines*2 );
		for ( int i=0; i<nlines*2; i++ )
		    pa.setPoint( i, a.point(index+i) );
		index = 0;
	    }
	    QPDevCmdParam param[1];
	    param[0].ptarr = (QPointArray*)&pa;
	    if ( !pdev->cmd(PDC_DRAWLINESEGS,this,param) || !hdc )
		return;
	}
	if ( txop != TxNone ) {
	    pa = xForm( a, index, nlines*2 );
	    if ( pa.size() != a.size() ) {
		index  = 0;
		nlines = pa.size()/2;
	    }		
	}
    }

    int	 x1, y1, x2, y2;
    uint i = index;
    bool solid = cpen.style() == SolidLine;
    uint pixel = COLOR_VALUE(cpen.data->color);

    while ( nlines-- ) {
	pa.point( i++, &x1, &y1 );
	pa.point( i++, &x2, &y2 );
	if ( x1 == x2 ) {			// vertical
	    if ( y1 < y2 )
		y2++;
	    else
		y2--;
	} else if ( y1 == y2 ) {		// horizontal
	    if ( x1 < x2 )
		x2++;
	    else
		x2--;
	} else if ( solid )			// draw last pixel
	    SetPixelV( hdc, x2, y2, pixel );
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
    QPointArray pa = a;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    if ( npoints != (int)pa.size() ) {
		pa.resize( npoints );
		for ( int i=0; i<npoints; i++ )
		    pa.setPoint( i, a.point(index+i) );
		index = 0;
	    }
	    QPDevCmdParam param[1];
	    param[0].ptarr = (QPointArray*)&pa;
	    if ( !pdev->cmd(PDC_DRAWPOLYLINE,this,param) || !hdc )
		return;
	}
	if ( txop != TxNone ) {
	    pa = xForm( a, index, npoints );
	    if ( pa.size() != a.size() ) {
		index   = 0;
		npoints = pa.size();
	    }		
	}
    }
    int x1, y1, x2, y2, xsave, ysave;
    pa.point( index+npoints-2, &x1, &y1 );	// last line segment
    pa.point( index+npoints-1, &x2, &y2 );
    xsave = x2; ysave = y2;
    bool plot_pixel = FALSE;
    if ( x1 == x2 ) {				// vertical
	if ( y1 < y2 )
	    y2++;
	else
	    y2--;
    } else if ( y1 == y2 ) {			// horizontal
	if ( x1 < x2 )
	    x2++;
	else
	    x2--;
    } else {
	plot_pixel = cpen.style() == SolidLine; // plot last pixel
    }
    if ( plot_pixel ) {
	Polyline( hdc, (POINT*)(pa.data()+index), npoints );
	SetPixelV( hdc, x2, y2, COLOR_VALUE(cpen.data->color) );
    } else {
	pa.setPoint( index+npoints-1, x2, y2 );
	Polyline( hdc, (POINT*)(pa.data()+index), npoints );
	pa.setPoint( index+npoints-1, xsave, ysave );
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
    QPointArray pa = a;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    if ( npoints != (int)a.size() ) {
		pa.resize( npoints );
		for ( int i=0; i<npoints; i++ )
		    pa.setPoint( i, a.point(index+i) );
	    }
	    QPDevCmdParam param[2];
	    param[0].ptarr = (QPointArray*)&pa;
	    param[1].ival = winding;
	    if ( !pdev->cmd(PDC_DRAWPOLYGON,this,param) || !hdc )
		return;
	}
	if ( txop != TxNone ) {
	    pa = xForm( a, index, npoints );
	    if ( pa.size() != a.size() ) {
		index   = 0;
		npoints = pa.size();
	    }		
	}
    }
    if ( winding )				// set to winding fill mode
	SetPolyFillMode( hdc, WINDING );
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cbrush.data->color) );
    Polygon( hdc, (POINT*)(pa.data()+index), npoints );
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cpen.data->color) );
    if ( winding )				// set to normal fill mode
	SetPolyFillMode( hdc, ALTERNATE );
}


void QPainter::drawQuadBezier( const QPointArray &a, int index )
{
    if ( !isActive() )
	return;
    if ( (int)a.size() - index < 4 ) {
#if defined(CHECK_RANGE)
	warning( "QPainter::drawQuadBezier: Cubic Bezier needs 4 control "
		 "points" );
#endif
	return;
    }
    QPointArray pa( a );
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( index != 0 || a.size() > 4 ) {
	    pa = QPointArray( 4 );
	    for ( int i=0; i<4; i++ )
		pa.setPoint( i, a.point(index+i) );
	    index = 0;
	}
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    param[0].ptarr = (QPointArray*)&pa;
	    if ( !pdev->cmd(PDC_DRAWQUADBEZIER,this,param) || !hdc )
		return;
	}
	if ( txop != TxNone )
	    pa = xForm( pa );
    }
    PolyBezier( hdc, (POINT*)(pa.data()+index), 4 );
}


void QPainter::drawPixmap( int x, int y, const QPixmap &pixmap,
			   int sx, int sy, int sw, int sh )
{
    if ( !isActive() || pixmap.isNull() )
	return;
    bool nat_xf = qt_winver == WV_NT && txop == TxRotShear;
    if ( sw < 0 )
	sw = pixmap.width() - sx;
    if ( sh < 0 )
	sh = pixmap.height() - sy;

    // Sanity-check clipping
    if ( sx < 0 ) {
	x -= sx;
	sx = 0;
    }
    if ( sw + sx > pixmap.width() )
	sw = pixmap.width() - sx;
    if ( sy < 0 ) {
	y -= sy;
	sy = 0;
    }
    if ( sh + sy > pixmap.height() )
	sh = pixmap.height() - sy;

    if ( testf(ExtDev|VxF|WxF) ) {
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
	    param[0].point  = &p;
	    param[1].pixmap = &pixmap;
	    if ( !pdev->cmd(PDC_DRAWPIXMAP,this,param) || !hdc )
		return;
	}
	if ( nat_xf )
	    nativeXForm( TRUE );
	else if ( txop == TxTranslate )
	    map( x, y, &x, &y );
    }

    if ( txop <= TxTranslate ) {		// use optimized bitBlt
	if ( pixmap.mask() && pixmap.data->selfmask ) {
	    QPixmap *pm	= (QPixmap*)&pixmap;
	    bool tmp_dc	= pm->handle() == 0;
	    if ( tmp_dc )
		pm->allocMemDC();
	    HBRUSH b = CreateSolidBrush( COLOR_VALUE(cpen.data->color) );
	    COLORREF tc, bc;
	    b = SelectObject( hdc, b );
	    tc = SetTextColor( hdc, COLOR_VALUE(black) );
	    bc = SetBkColor( hdc, COLOR_VALUE(white) );
	    BitBlt( hdc, x, y, sw, sh, pm->handle(), sx, sy, 0x00b8074a );
	    SetBkColor( hdc, bc );
	    SetTextColor( hdc, tc );
	    DeleteObject( SelectObject(hdc, b) );
	    if ( tmp_dc )
		pm->freeMemDC();
	} else {
	    bitBlt( pdev, x, y, &pixmap, sx, sy, sw, sh, (RasterOp)rop );
	}
	return;
    }

    QPixmap *pm	  = (QPixmap*)&pixmap;
    QBitmap *mask = (QBitmap*)pm->mask();
    bool tmp_dc	  = pm->handle() == 0;

    if ( tmp_dc )
	pm->allocMemDC();

    if ( mask ) {
	if ( qt_winver == WV_NT ) {
	    MaskBlt( hdc, x, y, sw, sh, pm->handle(), sx, sy, mask->hbm(),
		     sx, sy, MAKEROP4(0x00aa0029,SRCCOPY) );
	} else {
	    if ( pm->data->selfmask ) {
		HBRUSH b = CreateSolidBrush( COLOR_VALUE(cpen.data->color) );
		COLORREF tc, bc;
		b = SelectObject( hdc, b );
		tc = SetTextColor( hdc, COLOR_VALUE(black) );
		bc = SetBkColor( hdc, COLOR_VALUE(white) );
		BitBlt( hdc, x, y, sw, sh, pm->handle(), sx, sy, 0x00b8074a );
		SetBkColor( hdc, bc );
		SetTextColor( hdc, tc );
		DeleteObject( SelectObject(hdc, b) );
	    } else {
		BitBlt( hdc, x, y, sw, sh, pm->handle(), sx, sy, SRCCOPY );
	    }
	}
    } else {
	if ( txop == TxNone || txop == TxTranslate || nat_xf ) {
	    BitBlt( hdc, x, y, sw, sh, pm->handle(), sx, sy, SRCCOPY );
	} else if ( txop == TxScale ) {
	    int w, h;
	    map( x, y, sw, sh, &x, &y, &w, &h );
	    StretchBlt( hdc, x, y, w, h, pm->handle(), sx,sy, sw,sh, SRCCOPY );
	} else {				// rotate/shear, Win95
	    BitBlt( hdc, x, y, sw, sh, pm->handle(), sx, sy, SRCCOPY );
	}
    }

    if ( tmp_dc )
	pm->freeMemDC();
    if ( nat_xf )
	nativeXForm( FALSE );
}


void QPainter::drawText( int x, int y, const char *str, int len )
{
    if ( !isActive() )
	return;
    bool nat_xf = qt_winver == WV_NT && txop >= TxScale;
    if ( len < 0 )
	len = strlen( str );
    if ( len == 0 )				// empty string
	return;

    if ( testf(DirtyFont|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyFont) )
	    updateFont();
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
	if ( nat_xf )
	    nativeXForm( TRUE );
	else if ( txop == TxTranslate )
	    map( x, y, &x, &y );
    }

    TextOut( hdc, x, y, str, len );
    if ( nat_xf )
	nativeXForm( FALSE );
}
