/****************************************************************************
** $Id$
**
** Implementation of QPainter class for Win32
**
** Created : 940112
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

#include "qpainter.h"
#include "qpaintdevicemetrics.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qpaintdevicemetrics.h"
#include "qpixmapcache.h"
#include "qptrlist.h"
#include "qintdict.h"
#include "qprinter.h"
#include <stdlib.h>
#include <math.h>
#include "qapplication_p.h"
#include "qcomplextext_p.h"
#include "qfontdata_p.h"
#include "qt_windows.h"


#define COLOR_VALUE(c) ((flags & RGBColor) ? RGB(c.red(),c.green(),c.blue()) : c.pixel())


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

static HPEN   stock_nullPen;
static HPEN   stock_blackPen;
static HPEN   stock_whitePen;
static HBRUSH stock_nullBrush;
static HBRUSH stock_blackBrush;
static HBRUSH stock_whiteBrush;
static HFONT  stock_sysfont;

static QHDCObj stock_dummy;
static void  *stock_ptr = (void *)&stock_dummy;

/* paintevent magic to provide Windows semantics on Windows ;)
 */
static QRegion* paintEventClipRegion = 0;
static QPaintDevice* paintEventDevice = 0;

void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region)
{
    if ( !paintEventClipRegion )
	paintEventClipRegion = new QRegion( region );
    else
	*paintEventClipRegion = region;
    paintEventDevice = dev;
}

void qt_clear_paintevent_clipping()
{
    delete paintEventClipRegion;
    paintEventClipRegion = 0;
    paintEventDevice = 0;
}


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
#include "qtextstream.h"

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
    qDebug( "Number of cache hits = %d", c_numhits );
    qDebug( "Number of cache creates = %d", c_numcreates );
    qDebug( "Number of cache faults = %d", c_numfaults );
    qDebug( "PEN CACHE" );
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
	qDebug( str );
    }
    qDebug( "BRUSH CACHE" );
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
	qDebug( str );
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

static inline bool obtain_pen( void **ref, HPEN *pen, uint pix )
{ return obtain_obj( ref, (HANDLE*)pen, pix, pen_cache, TRUE ); }

static inline bool obtain_brush( void **ref, HBRUSH *brush, uint pix )
{ return obtain_obj( ref, (HANDLE*)brush, pix, brush_cache, FALSE ); }

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
    stock_nullPen    = (HPEN)GetStockObject( NULL_PEN );
    stock_blackPen   = (HPEN)GetStockObject( BLACK_PEN );
    stock_whitePen   = (HPEN)GetStockObject( WHITE_PEN );
    stock_nullBrush  = (HBRUSH)GetStockObject( NULL_BRUSH );
    stock_blackBrush = (HBRUSH)GetStockObject( BLACK_BRUSH );
    stock_whiteBrush = (HBRUSH)GetStockObject( WHITE_BRUSH );
    stock_sysfont    = (HFONT)GetStockObject( SYSTEM_FONT );
    init_cache();
}

void QPainter::cleanup()
{
    cleanup_cache();
}

void QPainter::destroy()
{

}


typedef QIntDict<QPaintDevice> QPaintDeviceDict;
static QPaintDeviceDict *pdev_dict = 0;


void QPainter::redirect( QPaintDevice *pdev, QPaintDevice *replacement )
{
    if ( pdev_dict == 0 ) {
	if ( replacement == 0 )
	    return;
	pdev_dict = new QPaintDeviceDict;
	Q_CHECK_PTR( pdev_dict );
    }
#if defined(QT_CHECK_NULL)
    if ( pdev == 0 )
	qWarning( "QPainter::redirect: The pdev argument cannot be 0" );
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
    d = 0;
    flags = IsStartingUp;
    bg_col = white;				// default background color
    bg_mode = TransparentMode;			// default background mode
    rop = CopyROP;				// default ROP
    tabstops = 0;				// default tabbing
    tabarray = 0;
    tabarraylen = 0;
    ps_stack = 0;
    wm_stack = 0;
    pdev = 0;
    hdc = 0;
    hpen = 0;
    hbrush = 0;
    hbrushbm = 0;
    hfont = 0;
    txop = txinv = 0;
    pixmapBrush = nocolBrush = FALSE;
    penRef = brushRef = 0;
    pfont = 0;
    block_ext = FALSE;
}


void QPainter::setFont( const QFont &font )
{
#if defined(QT_CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setFont: Will be reset by begin()" );
#endif
    if ( cfont.d != font.d || testf(VolatileDC) ) {
	cfont = font;
	setf(DirtyFont);
    }
}


void QPainter::updateFont()
{
    clearf(DirtyFont);
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].font = &cfont;
	if ( !pdev->cmd( QPaintDevice::PdcSetFont, this, param ) || !hdc )
	    return;
    }
    bool   ownFont = pdev->devType() == QInternal::Printer;
    if ( ownFont ) {
	int dw = pdev->metric( QPaintDeviceMetrics::PdmWidth );
	int dh = pdev->metric( QPaintDeviceMetrics::PdmHeight );
	// ### fix compat mode
	bool vxfScale = testf(Qt2Compat) && testf(VxF)
			&& ( dw != ww || dw != vw || dh != wh || dh != vh );

	if ( pfont ) delete pfont;
	pfont = new QFont( cfont.d, pdev );
	hfont = pfont->handle();
    } else {
	if ( pfont ) {
	    delete pfont;
	    pfont = 0;
	}
	hfont = cfont.handle();
    }
    SelectObject( hdc, hfont );
}


void QPainter::updatePen()
{
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].pen = &cpen;
	if ( !pdev->cmd(QPaintDevice::PdcSetPen,this,param) || !hdc )
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
#ifndef Q_OS_TEMP
	case DotLine:
	    s = PS_DOT;
	    break;
	case DashDotLine:
	    s = PS_DASHDOT;
	    break;
	case DashDotDotLine:
	    s = PS_DASHDOTDOT;
	    break;
#endif
	default:
	    s = PS_SOLID;
#if defined(QT_CHECK_STATE)
	    qWarning( "QPainter::updatePen: Invalid pen style" );
#endif
    }
#ifndef Q_OS_TEMP
    if ( (qt_winver & WV_NT_based) && cpen.width() > 1 ) {
	LOGBRUSH lb;
	lb.lbStyle = 0;
	lb.lbColor = pix;
	lb.lbHatch = 0;
	int pst =
		PS_GEOMETRIC |
		s;
	switch ( cpen.capStyle() ) {
	    case SquareCap:
		pst |= PS_ENDCAP_SQUARE;
		break;
	    case RoundCap:
		pst |= PS_ENDCAP_ROUND;
		break;
	    case FlatCap:
		pst |= PS_ENDCAP_FLAT;
		break;
	}
	switch ( cpen.joinStyle() ) {
	    case BevelJoin:
		pst |= PS_JOIN_BEVEL;
		break;
	    case RoundJoin:
		pst |= PS_JOIN_ROUND;
		break;
	    case MiterJoin:
		pst |= PS_JOIN_MITER;
		break;
	}
	hpen = ExtCreatePen( pst, cpen.width(), &lb, 0, 0 );
    }
    else
#endif
	{
	hpen = CreatePen( s, cpen.width(), pix );
    }
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
	if ( !pdev->cmd(QPaintDevice::PdcSetBrush,this,param) || !hdc )
	    return;
    }

    int	   bs	   = cbrush.style();
    uint   pix	   = COLOR_VALUE(cbrush.data->color);
    bool   cacheIt = bs == NoBrush || bs == SolidPattern;
    HBRUSH hbrush_old;

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

    HBITMAP hbrushbm_old    = hbrushbm;
    bool    pixmapBrush_old = pixmapBrush;

    pixmapBrush = nocolBrush = FALSE;
    hbrushbm = 0;

    if ( bs == SolidPattern ) {			// create solid brush
	hbrush = CreateSolidBrush( pix );
    } else if ( (bs >= Dense1Pattern && bs <= Dense7Pattern ) ||
		(bs == CustomPattern) ) {
	if ( bs == CustomPattern ) {
	    // The brush pixmap can never be a multi cell pixmap
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
#ifndef Q_OS_TEMP
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
#endif
    }

    SelectObject( hdc, hbrush );

    if ( hbrush_old ) {
	DeleteObject( hbrush_old );		// delete last brush
	if ( hbrushbm_old && !pixmapBrush_old )
	    DeleteObject( hbrushbm_old );	// delete last brush pixmap
    }
}


bool QPainter::begin( const QPaintDevice *pd, bool unclipped )
{
    if ( isActive() ) {				// already active painting
#if defined(QT_CHECK_STATE)
	qWarning( "QPainter::begin: Painter is already active."
		 "\n\tYou must end() the painter before a second begin()" );
#endif
	return FALSE;
    }
    if ( pd == 0 ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QPainter::begin: Paint device cannot be null" );
#endif
	return FALSE;
    }

    QWidget *copyFrom = 0;
    if ( pdev_dict ) {				// redirected paint device?
	pdev = pdev_dict->find( (long)pd );
	if ( pdev ) {
	    if ( pd->devType() == QInternal::Widget )
		copyFrom = (QWidget *)pd;	// copy widget settings
	} else {
	    pdev = (QPaintDevice *)pd;
	}
    } else {
	pdev = (QPaintDevice *)pd;
    }

    if ( pdev->paintingActive() ) {		// somebody else is already painting
#if defined(QT_CHECK_STATE)
	qWarning( "QPainter::begin: Another QPainter is already painting "
		 "this device;\n\tA paint device can only be painted by "
		 "one QPainter at a time" );
#endif
	return FALSE;
    }

    bool reinit = flags != IsStartingUp;	// 2nd, 3rd,.... time called
    flags = 0x0;				// init flags
    int dt = pdev->devType();			// get the device type

    if ( (pdev->devFlags & QInternal::ExternalDevice) != 0 )
	setf(ExtDev);				// this is an extended device
    if ( (pdev->devFlags & QInternal::CompatibilityMode) != 0 )
	setf(Qt2Compat);
    else if ( dt == QInternal::Pixmap )		// device is a pixmap
	((QPixmap*)pdev)->detach();		// will modify it

    hdc = 0;
    holdpal = 0;

    if ( testf(ExtDev) ) {			// external device
	if ( !pdev->cmd(QPaintDevice::PdcBegin,this,0) ) {
	    pdev = 0;				// could not begin
	    clearf( IsActive | DirtyFont | ExtDev );
	    return FALSE;
	}
	if ( tabstops )				// update tabstops for device
	    setTabStops( tabstops );
	if ( tabarray )				// update tabarray for device
	    setTabArray( tabarray );
    }

    setf( IsActive );
    pdev->painters++;				// also tell paint device
    Q_ASSERT(pdev->painters==1);
    bro = QPoint( 0, 0 );
    if ( reinit ) {
	bg_mode = TransparentMode;		// default background mode
	rop = CopyROP;				// default ROP
	wxmat.reset();				// reset world xform matrix
	txop = txinv = 0;
	if ( dt != QInternal::Widget ) {
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

    if ( dt == QInternal::Widget ) {		// device is a widget
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
	if ( w->testWState(Qt::WState_InPaintEvent) ) {
	    hdc = w->hdc;			// during paint event
	} else {
	    if ( unclipped || w->testWFlags( WPaintUnclipped ) ) {
		hdc = GetWindowDC( w->winId() );
		if ( w->isTopLevel() ) {
		    int dx = w->geometry().x() - w->frameGeometry().x();
		    int dy = w->geometry().y() - w->frameGeometry().y();
#ifndef Q_OS_TEMP
		    SetWindowOrgEx( hdc, -dx, -dy, 0 );
#else
//		    MoveWindow( w->winId(), w->frameGeometry().x(), w->frameGeometry().y(), w->frameGeometry().width(), w->frameGeometry().height(), FALSE );
//		    MoveWindow( w->winId(), w->frameGeometry().x() - 50, w->frameGeometry().y() - 50, w->frameGeometry().width(), w->frameGeometry().height(), FALSE );
#endif
		}
	    } else {
		hdc = GetDC( w->winId() );
	    }
	    w->hdc = hdc;
	}
    } else if ( dt == QInternal::Pixmap ) {	// device is a pixmap
	QPixmap *pm = (QPixmap*)pdev;
	if ( pm->isNull() ) {
#if defined(QT_CHECK_NULL)
	    qWarning( "QPainter::begin: Cannot paint null pixmap" );
#endif
	    end();
	    return FALSE;
	}
	if ( pm->isMultiCellPixmap() )		// disable multi cell
	    pm->freeCell();
	hdc = pm->handle();
	ww = vw = pm->width();			// default view size
	wh = vh = pm->height();
	if ( pm->depth() == 1 ) {		// monochrome pixmap
	    setf( MonoDev );
	    bg_col = color0;
	    cpen.setColor( color1 );
	}
    } else if ( dt == QInternal::Printer ) {	// device is a printer
	if ( pdev->handle() )
	    hdc = pdev->handle();
	flags |= (NoCache | RGBColor);
	if ( qt_winver & WV_DOS_based )
	    flags |= VolatileDC;
    } else if ( dt == QInternal::System ) {	// system-dependent device
	hdc = pdev->handle();
	if ( hdc ) {
	    SIZE s;
	    GetWindowExtEx( hdc, &s );
	    ww = vw = s.cx;
	    wh = vh = s.cy;
	}
    }
    if ( testf(ExtDev) ) {
	ww = vw = pdev->metric( QPaintDeviceMetrics::PdmWidth );
	wh = vh = pdev->metric( QPaintDeviceMetrics::PdmHeight );
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
	if ( QColor::hPal() && dt != QInternal::Printer ) {
	    holdpal = SelectPalette( hdc, QColor::hPal(), TRUE );
	    RealizePalette( hdc );
	}
	SetBkColor( hdc, COLOR_VALUE(bg_col) );
	SetBkMode( hdc, TRANSPARENT );
	SetROP2( hdc, R2_COPYPEN );
#ifndef Q_OS_TEMP
	SetTextAlign( hdc, TA_BASELINE );
	SetStretchBltMode( hdc, COLORONCOLOR );
#endif
    }
    updatePen();
    updateBrush();
    if ( pdev == paintEventDevice )
	SelectClipRgn( hdc, paintEventClipRegion->handle() );
    else
	SelectClipRgn( hdc, 0 );
    setf(DirtyFont);

    return TRUE;
}

bool QPainter::end()
{
    if ( !isActive() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QPainter::end: Missing begin() or begin() failed" );
#endif
	return FALSE;
    }

    killPStack();
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
	hbrush = 0;
	hbrushbm = 0;
	pixmapBrush = nocolBrush = FALSE;
    }
    if ( hfont ) {
	SelectObject( hdc, stock_sysfont );
	hfont = 0;
    }
    if ( holdpal ) {
	SelectPalette( hdc, holdpal, TRUE );
	RealizePalette( hdc );
    }
    if ( !pdev )
	return FALSE;

    if ( testf(ExtDev) )
	pdev->cmd( QPaintDevice::PdcEnd, this, 0 );

    if ( pdev->devType() == QInternal::Widget ) {
	if ( !((QWidget*)pdev)->testWState(Qt::WState_InPaintEvent) ) {
	    QWidget *w = (QWidget*)pdev;
	    ReleaseDC( w->winId(), hdc );
	    w->hdc = 0;
	}
    } else if ( pdev->devType() == QInternal::Pixmap ) {
	QPixmap *pm = (QPixmap*)pdev;
	if ( pm->optimization() == QPixmap::MemoryOptim &&
	     ( qt_winver & WV_DOS_based ) )
	    pm->allocCell();
    }

    if ( pfont ) {
	delete pfont;
	pfont = 0;
    }

    flags = 0;
    pdev->painters--;
    pdev = 0;
    hdc	 = 0;
    return TRUE;
}

void QPainter::flush(const QRegion &, CoordinateMode)
{
    flush();
}

void QPainter::flush()
{
#ifndef Q_OS_TEMP
    GdiFlush();
#endif
}


void QPainter::setBackgroundColor( const QColor &c )
{
    if ( !isActive() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QPainter::setBackgroundColor: Call begin() first" );
#endif
	return;
    }
    bg_col = c;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].color = &bg_col;
	if ( !pdev->cmd(QPaintDevice::PdcSetBkColor,this,param) || !hdc )
	    return;
    }
    SetBkColor( hdc, COLOR_VALUE(c) );
}

void QPainter::setBackgroundMode( BGMode m )
{
    if ( !isActive() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QPainter::setBackgroundMode: Call begin() first" );
#endif
	return;
    }
    if ( m != TransparentMode && m != OpaqueMode ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QPainter::setBackgroundMode: Invalid mode" );
#endif
	return;
    }
    bg_mode = m;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = m;
	if ( !pdev->cmd(QPaintDevice::PdcSetBkMode,this,param) || !hdc )
	    return;
    }
    SetBkMode( hdc, m == TransparentMode ? TRANSPARENT : OPAQUE );
}

void QPainter::setRasterOp( RasterOp r )
{
    static short ropCodes[] =
	{ R2_COPYPEN, R2_MERGEPEN, R2_XORPEN, R2_MASKNOTPEN,
	  R2_NOTCOPYPEN, R2_MERGENOTPEN, R2_NOTXORPEN, R2_MASKPEN,
	  R2_NOT, R2_BLACK, R2_WHITE, R2_NOP,
	  R2_MASKPENNOT, R2_MERGEPENNOT, R2_NOTMASKPEN, R2_NOTMERGEPEN };

    if ( !isActive() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QPainter::setRasterOp: Call begin() first" );
#endif
	return;
    }
    if ( (uint)r > LastROP ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QPainter::setRasterOp: Invalid ROP code" );
#endif
	return;
    }
    rop = r;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = r;
	if ( !pdev->cmd(QPaintDevice::PdcSetROP,this,param) || !hdc )
	    return;
    }
    SetROP2( hdc, ropCodes[rop] );
}


void QPainter::setBrushOrigin( int x, int y )
{
    if ( !isActive() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QPainter::setBrushOrigin: Call begin() first" );
#endif
	return;
    }
    bro = QPoint(x,y);
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].point = &bro;
	if ( !pdev->cmd(QPaintDevice::PdcSetBrushOrigin,this,param) || !hdc )
	    return;
    }
    SetBrushOrgEx( hdc, x, y, 0 );
}


void QPainter::nativeXForm( bool enable )
{
#ifndef Q_OS_TEMP
    XFORM m;
    if ( enable ) {
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
	m.eM11 = m.eM22 = (float)1.0;
	m.eM12 = m.eM21 = m.eDx = m.eDy = (float)0.0;
	SetGraphicsMode( hdc, GM_ADVANCED );
	ModifyWorldTransform( hdc, &m, MWT_IDENTITY );
	SetGraphicsMode( hdc, GM_COMPATIBLE );
    }
#else
    if ( enable ) {
	QWMatrix mtx;
	if ( testf(VxF) ) {
	    mtx.translate( vx, vy );
	    mtx.scale( 1.0*vw/ww, 1.0*vh/wh );
	    mtx.translate( -wx, -wy );
	    mtx = wxmat * mtx;
	} else {
	    mtx = wxmat;
	}
	SetViewportOrgEx( hdc, mtx.dx(), mtx.dy(), NULL );
    } else {
	SetViewportOrgEx( hdc, 0, 0, NULL );
    }
#endif
}


void QPainter::setClipping( bool enable )
{
    if ( !isActive() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QPainter::setClipping: Will be reset by begin()" );
#endif
	return;
    }

    if ( !isActive() || enable == testf(ClipOn) )
	return;

    setf( ClipOn, enable );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = enable;
	if ( !pdev->cmd(QPaintDevice::PdcSetClip,this,param) || !hdc )
	    return;
    }
    if ( enable ) {
	QRegion rgn = crgn;
	if ( pdev == paintEventDevice )
	    rgn = rgn.intersect( *paintEventClipRegion );
#ifndef QT_NO_PRINTER
	if ( pdev->devType() == QInternal::Printer ) {
	    double xscale = ((float)pdev->metric( QPaintDeviceMetrics::PdmPhysicalDpiX )) /
    		((float)pdev->metric( QPaintDeviceMetrics::PdmDpiX ));
	    double yscale = ((float)pdev->metric( QPaintDeviceMetrics::PdmPhysicalDpiY )) /
    		((float)pdev->metric( QPaintDeviceMetrics::PdmDpiY ));
	    double xoff = 0;
	    double yoff = 0;
	    QPrinter* printer = (QPrinter*)pdev;
	    if ( printer->fullPage() ) {	// must adjust for margins
		QSize margins = printer->margins();
		xoff = -margins.width();
		yoff = -margins.height();
	    }
	    rgn = QWMatrix( xscale, 0, 0, yscale, xoff, yoff ) * rgn;
	}
#endif
	SelectClipRgn( hdc, rgn.handle() );
    }
    else {
	if ( pdev == paintEventDevice )
	    SelectClipRgn( hdc, paintEventClipRegion->handle() );
	else
	    SelectClipRgn( hdc, 0 );
    }
}


void QPainter::setClipRect( const QRect &r, CoordinateMode m )
{
    QRegion rgn( r );
    setClipRegion( rgn, m );
}

void QPainter::setClipRegion( const QRegion &rgn, CoordinateMode m )
{
#if defined(QT_CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setClipRegion: Will be reset by begin()" );
#endif
    if ( m == CoordDevice )
	crgn = rgn;
    else
	crgn = xmat * rgn;

    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	param[0].rgn = &rgn;
	param[1].ival = m;
	if ( !pdev->cmd(QPaintDevice::PdcSetClipRegion,this,param) || !hdc )
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
	    if ( !pdev->cmd(QPaintDevice::PdcDrawPoint,this,param) || !hdc )
		return;
	}
	map( x, y, &x, &y );
    }
    if ( cpen.style() != NoPen )
#ifndef Q_OS_TEMP
	SetPixelV( hdc, x, y, COLOR_VALUE(cpen.data->color) );
#else
	SetPixel( hdc, x, y, COLOR_VALUE(cpen.data->color) );
#endif
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
		if ( !pdev->cmd(QPaintDevice::PdcDrawPoint,this,param))
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
#ifndef Q_OS_TEMP
	    SetPixelV( hdc, pa[index+i].x(), pa[index+i].y(),
		       COLOR_VALUE(cpen.data->color) );
#else
	    SetPixel( hdc, pa[index+i].x(), pa[index+i].y(),
		       COLOR_VALUE(cpen.data->color) );
#endif
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
	    if ( !pdev->cmd(QPaintDevice::PdcMoveTo,this,param) || !hdc )
		return;
	}
	map( x, y, &x, &y );
    }
#ifndef Q_OS_TEMP
    MoveToEx( hdc, x, y, 0 );
#else
    internalCurrentPos = QPoint( x, y );
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
	    if ( !pdev->cmd(QPaintDevice::PdcLineTo,this,param) || !hdc )
		return;
	}
	map( x, y, &x, &y );
    }
#ifndef Q_OS_TEMP
    LineTo( hdc, x, y );
#else
    // PolyLine from internalCurrentPos to x, y.
    POINT linePts[2] = { { internalCurrentPos.x(), internalCurrentPos.y() }, { x, y } };
    Polyline( hdc, linePts, 2 );
    internalCurrentPos = QPoint( x, y );
#endif
    if ( cpen.style() != NoPen )
#ifndef Q_OS_TEMP
	SetPixelV( hdc, x, y, COLOR_VALUE(cpen.data->color) );
#else
	SetPixel( hdc, x, y, COLOR_VALUE(cpen.data->color) );
#endif
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
	    if ( !pdev->cmd(QPaintDevice::PdcDrawLine,this,param) || !hdc )
		return;
	}
	map( x1, y1, &x1, &y1 );
	map( x2, y2, &x2, &y2 );
    }
    POINT pts[2];
    bool plot_pixel = FALSE;
    if ( qt_winver & WV_NT_based )
	plot_pixel = (cpen.width() == 0) && (cpen.style() == SolidLine);
    else
	plot_pixel = (cpen.width() <= 1) && (cpen.style() == SolidLine);
    if ( plot_pixel ) {
	if ( x1 == x2 ) {				// vertical
	    if ( y1 < y2 )
		y2++;
	    else
		y2--;
	    plot_pixel = FALSE;
	} else if ( y1 == y2 ) {			// horizontal
	    if ( x1 < x2 )
		x2++;
	    else
		x2--;
	    plot_pixel = FALSE;
	}
    }
    pts[0].x = x1;  pts[0].y = y1;
    pts[1].x = x2;  pts[1].y = y2;
    Polyline( hdc, pts, 2 );
    if ( plot_pixel )
#ifndef Q_OS_TEMP
	SetPixelV( hdc, x2, y2, COLOR_VALUE(cpen.data->color) );
#else
	SetPixel( hdc, x2, y2, COLOR_VALUE(cpen.data->color) );
#endif
#ifndef Q_OS_TEMP
    MoveToEx( hdc, x2, y2, 0 );
#else
    internalCurrentPos = QPoint( x2, y2 );
#endif
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
	    if ( !pdev->cmd(QPaintDevice::PdcDrawRect,this,param) || !hdc )
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
	if ( pdev->devType() == QInternal::Pixmap
	  && ((QPixmap*)pdev)->depth()==1
	  && bg_mode == TransparentMode )
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
	    if ( !pdev->cmd(QPaintDevice::PdcDrawRect,this,param) || !hdc )
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
	    if ( !pdev->cmd(QPaintDevice::PdcDrawRoundRect,this,param) || !hdc)
		return;
	}
	if ( txop == TxRotShear ) {		// rotate/shear polygon
	    if ( w <= 0 || h <= 0 )
		fix_neg_rect( &x, &y, &w, &h );
	    w--;
	    h--;
	    int rxx = w*xRnd/200;
	    int ryy = h*yRnd/200;
	    // were there overflows?
	    if ( rxx < 0 )
		rxx = w/200*xRnd;
	    if ( ryy < 0 )
		ryy = h/200*yRnd;
	    int rxx2 = 2*rxx;
	    int ryy2 = 2*ryy;
	    QPointArray a[4];
	    a[0].makeArc( x, y, rxx2, ryy2, 1*16*90, 16*90, xmat );
	    a[1].makeArc( x, y+h-ryy2, rxx2, ryy2, 2*16*90, 16*90, xmat );
	    a[2].makeArc( x+w-rxx2, y+h-ryy2, rxx2, ryy2, 3*16*90, 16*90, xmat );
	    a[3].makeArc( x+w-rxx2, y, rxx2, ryy2, 0*16*90, 16*90, xmat );
	    // ### is there a better way to join QPointArrays?
	    QPointArray aa;
	    aa.resize( a[0].size() + a[1].size() + a[2].size() + a[3].size() );
	    uint j = 0;
	    for ( int k=0; k<4; k++ ) {
		for ( uint i=0; i<a[k].size(); i++ ) {
		    aa.setPoint( j, a[k].point(i) );
		    j++;
		}
	    }
	    drawPolyInternal( aa );
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
	    if ( !pdev->cmd(QPaintDevice::PdcDrawEllipse,this,param) || !hdc )
		return;
	}
	if ( txop == TxRotShear ) {		// rotate/shear polygon
	    QPointArray a;
	    a.makeArc( x, y, w, h, 0, 360*16, xmat );
	    drawPolyInternal( a );
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
	    if ( !pdev->cmd(QPaintDevice::PdcDrawArc,this,param) || !hdc )
		return;
	}
	if ( txop == TxRotShear ) {		// rotate/shear
	    QPointArray pa;
	    pa.makeArc( x, y, w, h, a, alen, xmat );	// arc polyline
	    drawPolyInternal( pa, FALSE );
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
    int xS = qRound(w2 + (cos(ra1)*w) + x);
    int yS = qRound(h2 - (sin(ra1)*h) + y);
    int xE = qRound(w2 + (cos(ra2)*w) + x);
    int yE = qRound(h2 - (sin(ra2)*h) + y);
    if ( QABS(alen) < 90*16 ) {
	if ( (xS == xE) && (yS == yE) ) {
	    // don't draw a whole circle
	    return; //### should we draw a point?
	}
    }
#ifndef Q_OS_TEMP
    Arc( hdc, x, y, x+w, y+h, xS, yS, xE, yE );
#endif
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
	    if ( !pdev->cmd(QPaintDevice::PdcDrawPie,this,param) || !hdc )
		return;
	}
	if ( txop == TxRotShear ) {		// rotate/shear
	    QPointArray pa;
	    pa.makeArc( x, y, w, h, a, alen, xmat ); // arc polyline
	    int n = pa.size();
	    int cx, cy;
	    xmat.map(x+w/2, y+h/2, &cx, &cy);
	    pa.resize( n+2 );
	    pa.setPoint( n, cx, cy );	// add legs
	    pa.setPoint( n+1, pa.at(0) );
	    drawPolyInternal( pa );
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
    int xS = qRound(w2 + (cos(ra1)*w) + x);
    int yS = qRound(h2 - (sin(ra1)*h) + y);
    int xE = qRound(w2 + (cos(ra2)*w) + x);
    int yE = qRound(h2 - (sin(ra2)*h) + y);
    if ( QABS(alen) < 90*16 ) {
	if ( (xS == xE) && (yS == yE) ) {
	    // don't draw a whole circle
	    return; //### should we draw something?
	}
    }
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cbrush.data->color) );
#ifndef Q_OS_TEMP
    Pie( hdc, x, y, x+w, y+h, xS, yS, xE, yE );
#endif
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
	    if ( !pdev->cmd(QPaintDevice::PdcDrawChord,this,param) || !hdc )
		return;
	}
	if ( txop == TxRotShear ) {		// rotate/shear
	    QPointArray pa;
	    pa.makeArc( x, y, w-1, h-1, a, alen, xmat ); // arc polygon
	    int n = pa.size();
	    pa.resize( n+1 );
	    pa.setPoint( n, pa.at(0) );		// connect endpoints
	    drawPolyInternal( pa );
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
    int xS = qRound(w2 + (cos(ra1)*w) + x);
    int yS = qRound(h2 - (sin(ra1)*h) + y);
    int xE = qRound(w2 + (cos(ra2)*w) + x);
    int yE = qRound(h2 - (sin(ra2)*h) + y);
    if ( QABS(alen) < 90*16 ) {
	if ( (xS == xE) && (yS == yE) ) {
	    // don't draw a whole circle
	    return; //### should we draw something?
	}
    }
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cbrush.data->color) );
#ifndef Q_OS_TEMP
    Chord( hdc, x, y, x+w, y+h, xS, yS, xE, yE );
#endif
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
		pa = QPointArray( nlines*2 );
		for ( int i=0; i<nlines*2; i++ )
		    pa.setPoint( i, a.point(index+i) );
		index = 0;
	    }
	    QPDevCmdParam param[1];
	    param[0].ptarr = (QPointArray*)&pa;
	    if ( !pdev->cmd(QPaintDevice::PdcDrawLineSegments,this,param)
		 || !hdc )
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
    uint pixel = COLOR_VALUE(cpen.data->color);
    bool maybe_plot_pixel = FALSE;
    if ( qt_winver & WV_NT_based )
	maybe_plot_pixel = (cpen.width() == 0) && (cpen.style() == SolidLine);
    else
	maybe_plot_pixel = (cpen.width() <= 1) && (cpen.style() == SolidLine);

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
	} else if ( maybe_plot_pixel ) {	// draw last pixel
#ifndef Q_OS_TEMP
	    SetPixelV( hdc, x2, y2, pixel );
#else
	    SetPixel( hdc, x2, y2, pixel );
#endif
	}

#ifndef Q_OS_TEMP
        MoveToEx( hdc, x1, y1, 0 );
	LineTo( hdc, x2, y2 );
#else
	// PolyLine from x1, y1 to x2, y2.
	POINT linePts[2] = { { x1, y1 }, { x2, y2 } };
	Polyline( hdc, linePts, 2 );
	internalCurrentPos = QPoint( x2, y2 );
#endif
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
		pa = QPointArray( npoints );
		for ( int i=0; i<npoints; i++ )
		    pa.setPoint( i, a.point(index+i) );
		index = 0;
	    }
	    QPDevCmdParam param[1];
	    param[0].ptarr = (QPointArray*)&pa;
	    if ( !pdev->cmd(QPaintDevice::PdcDrawPolyline,this,param) || !hdc )
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
    if ( qt_winver & WV_NT_based )
	plot_pixel = (cpen.width() == 0) && (cpen.style() == SolidLine);
    else
	plot_pixel = (cpen.width() <= 1) && (cpen.style() == SolidLine);
    if ( plot_pixel ) {
	if ( x1 == x2 ) {				// vertical
	    if ( y1 < y2 )
		y2++;
	    else
		y2--;
	    plot_pixel = FALSE;
	} else if ( y1 == y2 ) {			// horizontal
	    if ( x1 < x2 )
		x2++;
	    else
		x2--;
	    plot_pixel = FALSE;
	}
    }
    if ( plot_pixel ) {
	Polyline( hdc, (POINT*)(pa.data()+index), npoints );
#ifndef Q_OS_TEMP
	SetPixelV( hdc, x2, y2, COLOR_VALUE(cpen.data->color) );
#else
	SetPixel( hdc, x2, y2, COLOR_VALUE(cpen.data->color) );
#endif
    } else {
	pa.setPoint( index+npoints-1, x2, y2 );
	Polyline( hdc, (POINT*)(pa.data()+index), npoints );
	pa.setPoint( index+npoints-1, xsave, ysave );
    }
}


void QPainter::drawConvexPolygon( const QPointArray &pa,
			     int index, int npoints )
{
    // Any efficient way?
    drawPolygon(pa,FALSE,index,npoints);
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
		pa = QPointArray( npoints );
		for ( int i=0; i<npoints; i++ )
		    pa.setPoint( i, a.point(index+i) );
	    }
	    QPDevCmdParam param[2];
	    param[0].ptarr = (QPointArray*)&pa;
	    param[1].ival = winding;
	    if ( !pdev->cmd(QPaintDevice::PdcDrawPolygon,this,param) || !hdc )
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
#ifndef Q_OS_TEMP
    if ( winding )				// set to winding fill mode
	SetPolyFillMode( hdc, WINDING );
#endif
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cbrush.data->color) );
    Polygon( hdc, (POINT*)(pa.data()+index), npoints );
    if ( nocolBrush )
	SetTextColor( hdc, COLOR_VALUE(cpen.data->color) );
#ifndef Q_OS_TEMP
    if ( winding )				// set to normal fill mode
	SetPolyFillMode( hdc, ALTERNATE );
#endif
}


void QPainter::drawCubicBezier( const QPointArray &a, int index )
{
    if ( !isActive() )
	return;
    if ( (int)a.size() - index < 4 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QPainter::drawCubicBezier: Cubic Bezier needs 4 control "
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
	    if ( !pdev->cmd(QPaintDevice::PdcDrawCubicBezier,this,param)
		 || !hdc )
		return;
	}
	if ( txop != TxNone )
	    pa = xForm( pa );
    }
#ifndef Q_OS_TEMP
    PolyBezier( hdc, (POINT*)(pa.data()+index), 4 );
#endif
}


extern uint qt_bitblt_foreground;		// in qpaintdevice_win.cpp

void QPainter::drawPixmap( int x, int y, const QPixmap &pixmap,
			   int sx, int sy, int sw, int sh )
{
    if ( !isActive() || pixmap.isNull() )
	return;
    if ( sw < 0 )
	sw = pixmap.width() - sx;
    if ( sh < 0 )
	sh = pixmap.height() - sy;

    // Sanity-check clipping
    if ( sx < 0 ) {
	x -= sx;
	sw += sx;
	sx = 0;
    }
    if ( sw + sx > pixmap.width() )
	sw = pixmap.width() - sx;
    if ( sy < 0 ) {
	y -= sy;
	sh += sy;
	sy = 0;
    }
    if ( sh + sy > pixmap.height() )
	sh = pixmap.height() - sy;

    if ( sw <= 0 || sh <= 0 )
	return;

    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) || (txop == TxScale && pixmap.mask()) ||
	     txop == TxRotShear ) {
	    if ( sx != 0 || sy != 0 ||
		 sw != pixmap.width() || sh != pixmap.height() ||
		 pixmap.isMultiCellPixmap() ) {
		QPixmap tmp( sw, sh, pixmap.depth(), QPixmap::NormalOptim );
		bitBlt( &tmp, 0, 0, &pixmap, sx, sy, sw, sh, CopyROP, TRUE );
		if ( pixmap.mask() ) {
		    QBitmap mask( sw, sh );
		    bitBlt( &mask, 0, 0, pixmap.mask(), sx, sy, sw, sh,
			    CopyROP, TRUE );
		    tmp.setMask( mask );
		}
		drawPixmap( x, y, tmp );
		return;
	    }
	    if ( testf(ExtDev) ) {
		QPDevCmdParam param[2];
		QRect r( x, y, pixmap.width(), pixmap.height() );
		param[0].rect  = &r;
		param[1].pixmap = &pixmap;
		if ( !pdev->cmd(QPaintDevice::PdcDrawPixmap,this,param)
		     || !hdc )
		    return;
	    }
	}
	if ( txop == TxTranslate )
	    map( x, y, &x, &y );
    }

    if ( txop <= TxTranslate ) {		// use optimized bitBlt
	if ( pixmap.depth() == 1 )
	    qt_bitblt_foreground = COLOR_VALUE(cpen.data->color);
	bitBlt( pdev, x, y, &pixmap, sx, sy, sw, sh, (RasterOp)rop );
	return;
    }

    QPixmap *pm	  = (QPixmap*)&pixmap;
    QBitmap *mask = (QBitmap*)pm->mask();
    HDC pm_dc;
    int pm_offset;
    if ( pm->isMultiCellPixmap() ) {
	pm_dc = pm->multiCellHandle();
	pm_offset = pm->multiCellOffset();
    } else {
	pm_dc = pm->handle();
	pm_offset = 0;
    }

    if ( txop == TxScale && !mask ) {
	// Plain scaling and no mask, then StretchBlt is fastest
	int w, h;
	map( x, y, sw, sh, &x, &y, &w, &h );
	StretchBlt( hdc, x, y, w, h, pm_dc, sx,sy+pm_offset, sw,sh, SRCCOPY );
    } else {
	// We have a complex xform or scaling with mask, then xform the
	// pixmap (and possible mask) and bitBlt again.
	QWMatrix mat( m11(), m12(),
		      m21(), m22(),
		      dx(),  dy() );
	mat = QPixmap::trueMatrix( mat, sw, sh );
	QPixmap pmx;
	if ( sx == 0 && sy == 0 &&
	     sw == pixmap.width() && sh == pixmap.height() ) {
	    pmx = pixmap;			// xform the whole pixmap
	} else {
	    pmx = QPixmap( sw, sh );		// xform subpixmap
	    bitBlt( &pmx, 0, 0, pm, sx, sy, sw, sh );
	}
	pmx = pmx.xForm( mat );
	if ( pmx.isNull() )			// xformed into nothing
	    return;
	if ( !pmx.mask() && txop == TxRotShear ) {
	    QBitmap bm_clip( sw, sh, 1 );	// make full mask, xform it
	    bm_clip.fill( color1 );
	    pmx.setMask( bm_clip.xForm(mat) );
	}
	map( x, y, &x, &y );	//### already done above? // compute position of pixmap
	int dx, dy;
	mat.map( 0, 0, &dx, &dy );
	bitBlt( pdev, x - dx, y - dy, &pmx );
    }
}


/* Internal, used by drawTiledPixmap */

static void drawTile( QPainter *p, int x, int y, int w, int h,
		      const QPixmap &pixmap, int xOffset, int yOffset )
{
    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = y;
    yOff = yOffset;
    while( yPos < y + h ) {
	drawH = pixmap.height() - yOff;    // Cropping first row
	if ( yPos + drawH > y + h )	   // Cropping last row
	    drawH = y + h - yPos;
	xPos = x;
	xOff = xOffset;
	while( xPos < x + w ) {
	    drawW = pixmap.width() - xOff; // Cropping first column
	    if ( xPos + drawW > x + w )	   // Cropping last column
		drawW = x + w - xPos;
	    p->drawPixmap( xPos, yPos, pixmap, xOff, yOff, drawW, drawH );
	    xPos += drawW;
	    xOff = 0;
	}
	yPos += drawH;
	yOff = 0;
    }
}


/* These functions are implemented in qtmain_win.cpp */
void qt_fill_tile( QPixmap *, const QPixmap & );
void qt_draw_tiled_pixmap( HDC, int, int, int, int,
			   const QPixmap *, int, int );


void QPainter::drawTiledPixmap( int x, int y, int w, int h,
				const QPixmap &pixmap, int sx, int sy )
{
    int sw = pixmap.width();
    int sh = pixmap.height();
    if (!sw || !sh )
	return;
    if ( sx < 0 )
	sx = sw - -sx % sw;
    else
	sx = sx % sw;
    if ( sy < 0 )
	sy = sh - -sy % sh;
    else
	sy = sy % sh;
    /*
      Requirements for optimizing tiled pixmaps:
       - not an external device
       - not scale or rotshear
       - no mask
    */
    QBitmap *mask = (QBitmap *)pixmap.mask();
#if 0 // Windows is too buggy, so we just use our own code (which is faster).
    if ( !testf(ExtDev) && txop <= TxTranslate && mask == 0 ) {
	if ( txop == TxTranslate )
	    map( x, y, &x, &y );
	qt_draw_tiled_pixmap( hdc, x, y, w, h, &pixmap, sx, sy );
	return;
    }
#endif
    if ( sw*sh < 8192 && sw*sh < 16*w*h ) {
	int tw = sw, th = sh;
	while ( tw*th < 32678 && tw < w/2 )
	    tw *= 2;
	while ( tw*th < 32678 && th < h/2 )
	    th *= 2;
	QPixmap tile( tw, th, pixmap.depth(), QPixmap::BestOptim );
	qt_fill_tile( &tile, pixmap );
	if ( mask ) {
	    QBitmap tilemask( tw, th, FALSE, QPixmap::NormalOptim );
	    qt_fill_tile( &tilemask, *mask );
	    tile.setMask( tilemask );
	}
	drawTile( this, x, y, w, h, tile, sx, sy );
    } else {
	drawTile( this, x, y, w, h, pixmap, sx, sy );
    }
}


//
// Generate a string that describes a transformed bitmap. This string is used
// to insert and find bitmaps in the global pixmap cache.
//

static QString gen_text_bitmap_key( const QWMatrix &m, const QFont &font,
				    const QString &str, int len )
{
    QString fk = font.key();
    int sz = 4*2 + len*2 + fk.length()*2 + sizeof(double)*6;
    QByteArray buf(sz);
    uchar *p = (uchar *)buf.data();
    *((double*)p)=m.m11();  p+=sizeof(double);
    *((double*)p)=m.m12();  p+=sizeof(double);
    *((double*)p)=m.m21();  p+=sizeof(double);
    *((double*)p)=m.m22();  p+=sizeof(double);
    *((double*)p)=m.dx();   p+=sizeof(double);
    *((double*)p)=m.dy();   p+=sizeof(double);
    QChar h1( '$' );
    QChar h2( 'q' );
    QChar h3( 't' );
    QChar h4( '$' );
    *((QChar*)p)=h1;  p+=2;
    *((QChar*)p)=h2;  p+=2;
    *((QChar*)p)=h3;  p+=2;
    *((QChar*)p)=h4;  p+=2;
    memcpy( (char*)p, (char*)str.unicode(), len*2 );  p += len*2;
    memcpy( (char*)p, (char*)fk.unicode(), fk.length()*2 ); p += fk.length()*2;
    return QString( (QChar*)buf.data(), buf.size()/2 );
}

static QBitmap *get_text_bitmap( const QString &key )
{
    return (QBitmap*)QPixmapCache::find( key );
}

static void ins_text_bitmap( const QString &key, QBitmap *bm )
{
    if ( !QPixmapCache::insert(key,bm) )	// cannot insert pixmap
	delete bm;
}


void QPainter::drawText( int x, int y, const QString &str, int len, QPainter::TextDirection dir )
{
    drawText( x, y, str, 0, len, dir );
}

void QPainter::drawText( int x, int y, const QString &str, int pos, int len, QPainter::TextDirection dir)
{
    if ( !isActive() )
	return;

#ifndef Q_OS_TEMP
    bool nat_xf = ( (qt_winver & WV_NT_based) && txop >= TxScale );
#else
    bool nat_xf = FALSE;
#endif

    if ( len < 0 )
	len = str.length();
    if ( len == 0 )				// empty string
	return;

    QString shaped = QComplexText::shapedString( str, pos, len, dir );
    len = shaped.length();

    if ( testf(DirtyFont) )
	updateFont();
    QFont *font = pfont;
    if ( !font )
	font = &cfont;

    bool force_bitmap = rop != CopyROP;
    if ( force_bitmap ) {
#ifdef Q_OS_TEMP
	    force_bitmap &= !(((TEXTMETRICW*)textMetric())->tmPitchAndFamily&(TMPF_VECTOR|TMPF_TRUETYPE));
#else
#  ifdef UNICODE
	if ( qt_winver & WV_NT_based ) {
	    force_bitmap &= !(((TEXTMETRICW*)textMetric())->tmPitchAndFamily&(TMPF_VECTOR|TMPF_TRUETYPE));
	} else
#  endif
	{
	    force_bitmap &= !(((TEXTMETRICA*)textMetric())->tmPitchAndFamily&(TMPF_VECTOR|TMPF_TRUETYPE));
	}
#endif
    }

    if ( force_bitmap || testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[3];
	    QPoint p( x, y );
	    param[0].point = &p;
	    param[1].str = &shaped;
	    param[2].ival = QFont::NoScript;
	    if ( !pdev->cmd(QPaintDevice::PdcDrawText2,this,param) || !hdc )
		return;
	}
	if ( force_bitmap || (txop >= TxScale && !nat_xf) ) {
	    // Draw rotated and sheared text on Windows 95, 98
	    const QFontMetrics & fm = fontMetrics();
	    QFontInfo	 fi = fontInfo();
	    QRect bbox = fm.boundingRect( str, len );
	    int w=bbox.width(), h=bbox.height();
	    int aw, ah;
	    int tx=-bbox.x(),  ty=-bbox.y();	// text position
	    QWMatrix mat1( m11(), m12(), m21(), m22(), dx(),  dy() );
	    QFont dfont( *font );
	    QWMatrix mat2;
	    if ( txop <= TxScale && pdev->devType() != QInternal::Printer ) {
		int newSize = qRound( m22() * (double)font->pointSize() ) - 1;
		newSize = QMAX( 6, QMIN( newSize, 72 ) ); // empirical values
		dfont.setPointSize( newSize );
		QFontMetrics fm2( dfont );
		QRect abbox = fm2.boundingRect( str, len );
		aw = abbox.width();
		ah = abbox.height();
		tx = -abbox.x();
		ty = -abbox.y();	// text position - off-by-one?
		if ( aw == 0 || ah == 0 )
		    return;
		double rx = (double)bbox.width() * mat1.m11() / (double)aw;
		double ry = (double)bbox.height() * mat1.m22() /(double)ah;
		mat2 = QWMatrix( rx, 0, 0, ry, 0, 0 );
	    } else {
		mat2 = QPixmap::trueMatrix( mat1, w, h );
		aw = w;
		ah = h;
	    }
	    bool empty = aw == 0 || ah == 0;
	    QString bm_key = gen_text_bitmap_key( mat2, dfont, str, len );
	    QBitmap *wx_bm = get_text_bitmap( bm_key );
	    bool create_new_bm = wx_bm == 0;
	    if ( create_new_bm && !empty ) {	// no such cached bitmap
		QBitmap bm( aw, ah, TRUE, QPixmap::MemoryOptim );
		QFont pmFont( dfont );
		QPainter paint;
		paint.begin( &bm );		// draw text in bitmap
		if ( pdev->devType() == QInternal::Printer ) {
		    // Adjust for the difference in lpi of pixmap vs. printer
		    int dw = pdev->metric( QPaintDeviceMetrics::PdmWidth );
		    int dh = pdev->metric( QPaintDeviceMetrics::PdmHeight );
		    bool vxfScale = testf(Qt2Compat) && testf(VxF)
			 && ( dw != ww || dw != vw || dh != wh || dh != vh );
		    float fs = dfont.pointSizeFloat();
		    int prlpy = GetDeviceCaps(hdc,LOGPIXELSY);
		    int pmlpy = GetDeviceCaps(paint.hdc, LOGPIXELSY);
		    if ( prlpy && pmlpy && !vxfScale ) {	// Sanity
			float nfs = fs * (float)prlpy / (float)pmlpy;
			pmFont.setPointSizeFloat( nfs );
		    }
		}
		paint.setFont( pmFont );
		paint.drawText( tx, ty, str, pos, len, dir );
		paint.end();
		if ( txop >= TxScale )
		    wx_bm = new QBitmap( bm.xForm(mat2) ); // transform bitmap
		else
		    wx_bm = new QBitmap( bm );
		if ( wx_bm->isNull() ) {
		    delete wx_bm;		// nothing to draw
		    return;
		}
	    }
	    if ( bg_mode == OpaqueMode ) {	// opaque fill
		int fx = x;
		int fy = y - fm.ascent();
		int fw = bbox.width();
		int fh = bbox.height();
		int m, n;
		QPointArray a(5);
		mat1.map( fx,	 fy,	&m, &n );  a.setPoint( 0, m, n );
						   a.setPoint( 4, m, n );
		mat1.map( fx+fw, fy,	&m, &n );  a.setPoint( 1, m, n );
		mat1.map( fx+fw, fy+fh, &m, &n );  a.setPoint( 2, m, n );
		mat1.map( fx,	 fy+fh, &m, &n );  a.setPoint( 3, m, n );
		QPen oldPen = cpen;
		QBrush oldBrush = cbrush;
		setPen( NoPen );
		updatePen();
		setBrush( backgroundColor() );
		updateBrush();
		Polygon( hdc, (POINT*)a.data(), a.size() );
		setPen( oldPen );
		setBrush( oldBrush );
	    }
	    if ( empty )
		return;
	    double fx=x, fy=y, nfx, nfy;
	    mat1.map( fx,fy, &nfx,&nfy );
	    double tfx=tx, tfy=ty, dx, dy;
	    mat2.map( tfx, tfy, &dx, &dy );	// compute position of bitmap
	    x = qRound(nfx-dx);
	    y = qRound(nfy-dy);
	    if ( force_bitmap || testf(ExtDev) ) {		// to printer
		uint oldf = flags;
		flags &= ~(VxF|WxF);
		drawPixmap( x, y, *wx_bm );
		flags = oldf;
	    } else {				// to screen/pixmap
		// ### some exotic codes are still missing
		DWORD ropCodes[] = {
		    0x00b8074a, // PSDPxax,  CopyROP,
		    0x00BA0B09, // DPSnao,   OrROP,
		    0x009A0709, // DPSnax,   XorROP,
		    0x008A0E06, // DSPnoa,   EraseROP=NotAndROP,
		    0x00b8074a, //           NotCopyROP,
		    0x00b8074a, //           NotOrROP,
		    0x00b8074a, //           NotXorROP,
		    0x00A803A9, // DPSoa,    NotEraseROP=AndROP,
		    0x00A90189, // DPSoxn,   NotROP,
		    0x008800C6, // DSa,      ClearROP,
		    0x00BB0226, // DSno,     SetROP,
		    0x00b8074a, //           NopROP,
		    0x00b8074a, //           AndNotROP,
		    0x00b8074a, //           OrNotROP,
		    0x00b8074a, //           NandROP,
		    0x00b8074a  //           NorROP,
		};
		HBRUSH b = CreateSolidBrush( COLOR_VALUE(cpen.data->color) );
		COLORREF tc, bc;
		b = (HBRUSH)SelectObject( hdc, b );
		tc = SetTextColor( hdc, COLOR_VALUE(black) );
		bc = SetBkColor( hdc, COLOR_VALUE(white) );
		HDC wx_dc;
		int wx_sy;
		if ( wx_bm->isMultiCellPixmap() ) {
		    wx_dc = wx_bm->multiCellHandle();
		    wx_sy = wx_bm->multiCellOffset();
		} else {
		    wx_dc = wx_bm->handle();
		    wx_sy = 0;
		}
		BitBlt( hdc, x, y, wx_bm->width(), wx_bm->height(),
			wx_dc, 0, wx_sy, ropCodes[rop] );
		SetBkColor( hdc, bc );
		SetTextColor( hdc, tc );
		DeleteObject( SelectObject(hdc, b) );
	    }
	    if ( create_new_bm )
		ins_text_bitmap( bm_key, wx_bm );
	    return;
	}
	if ( nat_xf )
	    nativeXForm( TRUE );
	else if ( txop == TxTranslate )
	    map( x, y, &x, &y );
    }


    //const TCHAR *tc = (const TCHAR *) qt_winTchar( shaped, FALSE );
    QFontPrivate::TextRun *cache = new QFontPrivate::TextRun();
    font->d->buildCache( hdc, shaped, 0, len, cache );
    if ( rop == CopyROP ) {
#ifndef Q_OS_TEMP
	font->d->drawText( hdc, x, y, cache );
	//TextOut( hdc, x, y, tc, len );
#else
	// ### Problem here is that we can't align the text to the baseline of the font
	// The value 3 is here to correct the difference between the bottom of the font
	// and the baseline of it, however this should be calulated, or somehow the alignment
	// set to the baseline
	font->d->drawText( hdc, x, y + 5, cache );
#endif
    } else {
	// Doesn't work for non-TrueType fonts, but we dealt with those
	// with the bitmap above.
#ifndef Q_OS_TEMP
	BeginPath(hdc);
	font->d->drawText( hdc, x, y, cache );
	//TextOut( hdc, x, y, tc, len );
	EndPath(hdc);
#else
	// ### See last ### comment above
	font->d->drawText( hdc, x, y + 5, cache );
#endif
	uint pix = COLOR_VALUE(cpen.data->color);
	HBRUSH tbrush = CreateSolidBrush( pix );
	SelectObject( hdc, tbrush );
#ifndef Q_OS_TEMP
	FillPath(hdc);
#endif
	SelectObject( hdc, hbrush );
	DeleteObject( tbrush );
    }

    if ( nat_xf )
	nativeXForm( FALSE );

    delete cache;
}


QPoint QPainter::pos() const
{
    QPoint p;
    if ( !isActive() )
	return p;
#ifndef Q_OS_TEMP
    POINT pt;
    if ( GetCurrentPositionEx( hdc, &pt ) ) {
	p.rx() = pt.x;
	p.ry() = pt.y;
    }
    return  p;
#else
    return internalCurrentPos;
#endif
}


#if defined(Q_WS_WIN)
void *QPainter::textMetric()
{
    if ( testf(DirtyFont) )
	updateFont();
    if ( pfont )
	return pfont->textMetric();
    return cfont.textMetric();
}
#endif
