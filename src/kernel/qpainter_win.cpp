/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter_win.cpp#12 $
**
** Implementation of QPainter class for Windows
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
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

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpainter_win.cpp#12 $";
#endif


static inline int d2i_round( double d )
{
    return d > 0 ? int(d+0.5) : int(d-0.5);
}


// --------------------------------------------------------------------------
// QPainter member functions
//

void QPainter::initialize()
{
}

void QPainter::cleanup()
{
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


void QPainter::setPen( const QPen &pen )	// set current pen
{
    if ( cpen.data != pen.data ) {
	cpen = pen;
	updatePen();
    }
}

void QPainter::setPen( PenStyle style )		// set solid pen with color
{
    QPen pen( style );
    if ( cpen != pen ) {
	cpen = pen;
	updatePen();
    }
}

void QPainter::setPen( const QColor &color )	// set solid pen with color
{
    QPen pen( color );
    if ( cpen != pen ) {
	cpen = pen;
	updatePen();
    }
}


void QPainter::setBrush( const QBrush &brush )	// set current brush
{
    if ( cbrush.data != brush.data ) {
	cbrush = brush;
	updateBrush();
    }
}

void QPainter::setBrush( BrushStyle style )	// set brush
{
    QBrush brush( style );
    if ( cbrush != brush ) {
	cbrush = brush;
	updateBrush();
    }
}

void QPainter::setBrush( const QColor &color )	// set solid brush width color
{
    QBrush brush( color );
    if ( cbrush != brush ) {
	cbrush = brush;
	updateBrush();
    }
}


void QPainter::updateFont()			// update after changed font
{
    if ( tm ) {					// delete old text metrics
	delete tm;
	tm = 0;
    }
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].font = &cfont;
	pdev->cmd( PDC_SETFONT, param );
	return;
    }
    SelectObject( hdc, cfont.handle() );
}


void QPainter::updatePen()			// update after changed pen
{
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].pen = &cpen;
	pdev->cmd( PDC_SETPEN, param );
	return;
    }
    int s;
    switch ( cpen.style() ) {
	case NoPen:
	    s = PS_NULL;
	    break;
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
	    s = PS_NULL;
    }
    HANDLE hpen_old = hpen;
    hpen = CreatePen( s, cpen.width(), cpen.color().pixel() );
    SetTextColor( hdc, cpen.color().pixel() );	// pen color is also text color
    SelectObject( hdc, hpen );
    if ( hpen_old )				// delete last pen
	DeleteObject( hpen_old );
}


void QPainter::updateBrush()			// update after changed brush
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
	pdev->cmd( PDC_SETBRUSH, param );
	return;
    }

    HANDLE hbrush_old	   = hbrush;
    HANDLE hbrushbm_old	   = hbrushbm;
    bool   stockBrush_old  = stockBrush;
    bool   pixmapBrush_old = pixmapBrush;

    int s = cbrush.style();
    stockBrush	= FALSE;
    pixmapBrush = FALSE;
    hbrushbm = 0;

    if ( s == NoBrush ) {			// no brush
	hbrush = GetStockObject( NULL_BRUSH );
	stockBrush = TRUE;
    }
    else if ( s == SolidPattern ) {		// create solid brush
	hbrush = CreateSolidBrush( cbrush.color().pixel() );
    }
    else if ( (s >= Dense1Pattern && s <= Dense7Pattern ) ||
	      (s == CustomPattern) ) {
	if ( s == CustomPattern ) {
	    hbrushbm = cbrush.pixmap()->hbm();
	    pixmapBrush = TRUE;
	}
	else {
	    short *bm = dense_patterns[ s - Dense1Pattern ];
	    hbrushbm = CreateBitmap( 8, 8, 1, 1, bm );
	}
	hbrush = CreatePatternBrush( hbrushbm );
    }
    else {					// one of the hatch brushes
	switch ( s ) {
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
	hbrush = CreateHatchBrush( s, cbrush.color().pixel() );
    }

    SelectObject( hdc, hbrush );

    if ( hbrush_old && !stockBrush_old )
	DeleteObject( hbrush_old );		// delete last brush
    if ( hbrushbm_old && !pixmapBrush_old )
	DeleteObject( hbrushbm_old );		// delete last brush pixmap
}


bool QPainter::begin( const QPaintDevice *pd )	// begin painting in device
{
    if ( isActive() ) {				// already active painting
#if defined(CHECK_STATE)
	warning( "QPainter::begin: Painter is already active" );
#endif
	end();
	return begin( pd );
    }
    else if ( pd == 0 ) {
#if defined(CHECK_NULL)
	warning( "QPainter::begin: Invalid NULL argument" );
#endif
	return FALSE;
    }

    bool reinit = flags != IsStartingUp;	// 2nd or 3rd etc. time called
    flags = 0;					// default flags
    if ( pdev_dict ) {				// redirected paint device?
	pdev = pdev_dict->find( (long)pd );
	if ( !pdev )				// no
	    pdev = (QPaintDevice *)pd;
    }
    else
	pdev = (QPaintDevice *)pd;
    if ( (pdev->devFlags & PDF_EXTDEV) != 0 )	// this is an extended device
	setf(ExtDev);
    else if ( pdev->devType() == PDT_PIXMAP )	// device is a pixmap
	((QPixmap*)pdev)->detach();		// will modify pixmap

    hpen = hbrush = hbrushbm = 0;
    stockBrush = pixmapBrush = tmpHandle = 0;

    hdc = 0;
    if ( testf(ExtDev) ) {			// external device
	if ( !pdev->cmd( PDC_BEGIN, 0 ) ) {	// could not begin painting
	    pdev = 0;
	    return FALSE;
	}
	if ( tabstops )				// update tabstops for device
	    setTabStops( tabstops );
	if ( tabarray )				// update tabarray for device
	    setTabArray( tabarray );
    }

    setf( IsActive );				// painter becomes active
    pdev->devFlags |= PDF_PAINTACTIVE;		// also tell paint device
    bro = QPoint( 0, 0 );
    if ( reinit ) {
	bg_col = white;				// default background color
	wxmat.reset();				// reset world xform matrix
    }
    wx = wy = vx = vy = 0;			// default view origins

    if ( pdev->devType() == PDT_WIDGET ) {	// device is a widget
	QWidget *w = (QWidget*)pdev;
	cfont = w->font();			// use widget font
	bg_col = w->backgroundColor();		// use widget bg color
	ww = vw = w->width();			// default view size
	wh = vh = w->height();
	cpen = QPen( w->foregroundColor() );	// use widget fg color
	if ( reinit ) {
	    cbrush = QBrush( NoBrush );
	}
	if ( w->testWFlags(WState_Paint) ) {	// during paint event
	    hdc = w->hdc;
	}
	else {
	    hdc = GetDC( w->id() );
	    tmpHandle = TRUE;
	}
	if ( w->testWFlags(WPaintUnclipped) ) { // paint direct on device
	    // !!!hanord todo
	}
    }
    else if ( pdev->devType() == PDT_PIXMAP ) { // device is a pixmap
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
	if ( reinit ) {
	    QFont  defaultFont;			// default drawing tools
	    QPen   defaultPen;
	    QBrush defaultBrush;
	    cfont  = defaultFont;		// set these drawing tools
	    cpen   = defaultPen;
	    cbrush = defaultBrush;
	}
	if ( pm->depth() == 1 ) {		// monochrome pixmap
	    bg_col = color0;
	    cpen.setColor( color1 );
	}
    }
    else
	ww = wh = vw = vh = 1;
    updatePen();
    updateBrush();
    updateFont();
    if ( QColor::hPal() ) {			// realize global palette
	SelectPalette( hdc, QColor::hPal(), FALSE );
	RealizePalette( hdc );
    }
    if ( hdc ) {				// not ext device
	SetTextAlign( hdc, TA_BASELINE );
	SetStretchBltMode( hdc, COLORONCOLOR );
    }
    setBackgroundColor( bg_col );		// default background color
    setBackgroundMode( TransparentMode );	// default background mode
    setRasterOp( CopyROP );			// default raster operation
    return TRUE;
}

bool QPainter::end()				// end painting
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
	warning( "QPainter::end: No begin()" );
#endif
	return FALSE;
    }
    if ( testf(ExtDev) )
	pdev->cmd( PDC_END, 0 );
    if ( testf(FontMet) )			// remove references to this
	QFontMetrics::reset( this );
    if ( testf(FontInf) )			// remove references to this
	QFontInfo::reset( this );	    
    flags = 0;
    pdev->devFlags &= ~PDF_PAINTACTIVE;

    if ( tm ) {					// delete old text metrics
	delete tm;
	tm = 0;
    }
    if ( !hdc ) {
	pdev = 0;
	return TRUE;
    }
    if ( hpen )
	DeleteObject( SelectObject(hdc, GetStockObject(BLACK_PEN)) );
    if ( hbrush && !stockBrush ) {
	DeleteObject( SelectObject(hdc, GetStockObject(WHITE_BRUSH)) );
	if ( hbrushbm && !pixmapBrush )
	    DeleteObject( hbrushbm );
    }
    if ( pdev->devType() == PDT_WIDGET ) {
	if ( tmpHandle )
	    ReleaseDC( ((QWidget*)pdev)->id(), hdc );
    }
    else if ( pdev->devType() == PDT_PIXMAP ) {
	QPixmap *pm = (QPixmap*)pdev;
	pm->freeMemDC();
	if ( pm->optimized() )
	    pm->allocMemDC();
    }
    hdc	 = 0;
    pdev = 0;
    return TRUE;
}


void QPainter::setBackgroundColor( const QColor &c )
{						// set background color
    bg_col = c;
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].color = &bg_col;
	pdev->cmd( PDC_SETBKCOLOR, param );
	return;
    }
    SetBkColor( hdc, c.pixel() );
}

void QPainter::setBackgroundMode( BGMode m )	// set background mode
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
	pdev->cmd( PDC_SETBKMODE, param );
	return;
    }
    SetBkMode( hdc, m == TransparentMode ? TRANSPARENT : OPAQUE );
}

void QPainter::setRasterOp( RasterOp r )	// set raster operation
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
    if ( rop == r )				// no need to change
	return;
    rop = r;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = r;
	pdev->cmd( PDC_SETROP, param );
	return;
    }
    SetROP2( hdc, ropCodes[rop] );
}

void QPainter::setBrushOrigin( int x, int y )	// set brush origin
{
    if ( !isActive() )
	return;
    bro = QPoint(x,y);
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].point = &bro;
	pdev->cmd( PDC_SETBRUSHORIGIN, param );
	return;
    }
#if defined(_WS_WIN32_)
    POINT dummy;
    SetBrushOrgEx( hdc, x, y, &dummy );
#else
    SetBrushOrg( hdc, x, y );
#endif
}


void QPainter::updateXForm()
{
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
	SetGraphicsMode( hdc, GM_ADVANCED );
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
}


QPoint QPainter::xForm( const QPoint &pv ) const
{						// map point, virtual -> device
    if ( !hdc ) {
	return pv;	// !!!hanord what about ext devs???
    }
    POINT p;
    p.x = pv.x();
    p.y = pv.y();
    LPtoDP( hdc, &p, 1 );
    return QPoint( p.x, p.y );
}

QRect QPainter::xForm( const QRect &rv ) const
{						// map rect, virtual -> device
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
}

QPointArray QPainter::xForm( const QPointArray &av ) const
{						// map point array, v -> d
    if ( !hdc ) {
	return av.copy(); // !!!hanord what about ext devs???
    }
    QPointArray a = av.copy();
    LPtoDP( hdc, (POINT*)a.data(), a.size() );
    return a;
}

QPoint QPainter::xFormDev( const QPoint &pd ) const
{						// map point, device -> virtual
    if ( !hdc ) {
	return pd;	// !!!hanord what about ext devs???
    }
    POINT p;
    p.x = pd.x();
    p.y = pd.y();
    DPtoLP( hdc, &p, 1 );
    return QPoint( p.x, p.y );
}

QRect QPainter::xFormDev( const QRect &rd ) const
{						// map rect, device -> virtual
    if ( !hdc ) {
	return rd;	// !!!hanord what about ext devs???
    }
    RECT r;
    SetRect( &r, rd.left(), rd.top(), rd.right(), rd.bottom() );
    DPtoLP( hdc, (POINT*)&r, 2 );
    return QRect( QPoint(r.left, r.top),
		  QPoint(r.right,r.bottom) );
}

QPointArray QPainter::xFormDev( const QPointArray &ad ) const
{
    if ( !hdc ) {
	return ad.copy(); // !!!hanord what about ext devs???
    }
    QPointArray a = ad.copy();
    DPtoLP( hdc, (POINT*)a.data(), a.size() );
    return a;
}


void QPainter::setClipping( bool enable )	// set clipping
{
    if ( !isActive() || enable == testf(ClipOn) )
	return;
    setf( ClipOn, enable );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = enable;
	pdev->cmd( PDC_SETCLIP, param );
	return;
    }
    if ( testf(ClipOn) )
	SelectClipRgn( hdc, crgn.handle() );
    else
	SelectClipRgn( hdc, 0 );
}


void QPainter::setClipRect( const QRect &r )	// set clip rectangle
{
    QRegion rgn( r );
    setClipRegion( rgn );
}

void QPainter::setClipRegion( const QRegion &rgn )
{						// set clip region
    crgn = rgn;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].rgn = &crgn;
	pdev->cmd( PDC_SETCLIPRGN, param );
	return;
    }
    clearf( ClipOn );				// be sure to update clip rgn
    setClipping( TRUE );
}


void QPainter::drawPoint( int x, int y )	// draw a single point
{
    if ( !isActive() || cpen.style() == NoPen )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	QPoint p( x, y );
	param[0].point = &p;
	pdev->cmd( PDC_DRAWPOINT, param );
	return;
    }
    SetPixel( hdc, x, y, cpen.color().pixel() );
}


void QPainter::moveTo( int x, int y )		// set current point for lineTo
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	QPoint p( x, y );
	param[0].point = &p;
	pdev->cmd( PDC_MOVETO, param );
	return;
    }
#if defined(_WS_WIN32_)
    MoveToEx( hdc, x, y, 0 );
#else
    MoveTo( hdc, x, y );
#endif
}


void QPainter::lineTo( int x, int y )		// draw line from current point
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	QPoint p( x, y );
	param[0].point = &p;
	pdev->cmd( PDC_LINETO, param );
	return;
    }
    LineTo( hdc, x, y );
    SetPixel( hdc, x, y, cpen.color().pixel() );
}


void QPainter::drawLine( int x1, int y1, int x2, int y2 )
{						// draw line
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	QPoint p1( x1, y1 ), p2( x2, y2 );
	param[0].point = &p1;
	param[1].point = &p2;
	pdev->cmd( PDC_DRAWLINE, param );
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
    else
    if ( y1 == y2 ) {				// horizontal
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
{						// draw rectangle
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	QRect r( x, y, w, h );
	param[0].rect = &r;
	pdev->cmd( PDC_DRAWRECT, param );
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
    Rectangle( hdc, x, y, x+w, y+h );
}


void QPainter::drawRoundRect( int x, int y, int w, int h, int xRnd, int yRnd )
{						// draw round rectangle
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
	pdev->cmd( PDC_DRAWROUNDRECT, param );
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
    RoundRect( hdc, x, y, x+w, y+h,
	       (int)(((long)w)*xRnd/100L),
	       (int)(((long)h)*yRnd/100L) );

}


void QPainter::drawEllipse( int x, int y, int w, int h )
{						// draw ellipse
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	QRect r( x, y, w, h );
	param[0].rect = &r;
	pdev->cmd( PDC_DRAWELLIPSE, param );
	return;
    }
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
    Ellipse( hdc, x, y, x+w, y+h );
}


void QPainter::drawArc( int x, int y, int w, int h, int a, int alen )
{						// draw arc
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	param[0].rect = &r;
	param[1].ival = a;
	param[2].ival = alen;
	pdev->cmd( PDC_DRAWARC, param );
	return;
    }
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
    double ra1 = 1.09083078249645598-3*a;
    double ra2 = 1.09083078249645598-3*alen + ra1;
    if ( ra2 < 0.0 ) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    double w2 = 0.5*w;
    double h2 = 0.5*h;
    float r = (float)(w2+h2);
    Arc( hdc, x, y, x+w, y+h,
	 d2i_round(w2 + (cos(ra1)*r) + x),
	 d2i_round(h2 - (sin(ra1)*r) + y),
	 d2i_round(w2 + (cos(ra2)*r) + x),
	 d2i_round(h2 - (sin(ra2)*r) + y) );
}


void QPainter::drawPie( int x, int y, int w, int h, int a, int alen )
{						// draw pie
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	param[0].rect = &r;
	param[1].ival = a;
	param[2].ival = alen;
	pdev->cmd( PDC_DRAWPIE, param );
	return;
    }
    double ra1 = 1.09083078249645598-3*a;
    double ra2 = 1.09083078249645598-3*alen + ra1;
    if ( ra2 < 0.0 ) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    double w2 = 0.5*w;
    double h2 = 0.5*h;
    float r = (float)(w2+h2);
    Pie( hdc, x, y, x+w, y+h,
	 d2i_round(w2 + (cos(ra1)*r) + x),
	 d2i_round(h2 - (sin(ra1)*r) + y),
	 d2i_round(w2 + (cos(ra2)*r) + x),
	 d2i_round(h2 - (sin(ra2)*r) + y) );
}


void QPainter::drawChord( int x, int y, int w, int h, int a, int alen )
{						// draw chord
    if ( !isActive() )
	return;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	param[0].rect = &r;
	param[1].ival = a;
	param[2].ival = alen;
	pdev->cmd( PDC_DRAWPIE, param );
	return;
    }
    double ra1 = 1.09083078249645598-3*a;
    double ra2 = 1.09083078249645598-3*alen + ra1;
    if ( ra2 < 0.0 ) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    double w2 = 0.5*w;
    double h2 = 0.5*h;
    float r = (float)(w2+h2);
    Chord( hdc, x, y, x+w, y+h,
	   d2i_round(w2 + (cos(ra1)*r) + x),
	   d2i_round(h2 - (sin(ra1)*r) + y),
	   d2i_round(w2 + (cos(ra2)*r) + x),
	   d2i_round(h2 - (sin(ra2)*r) + y) );
}


void QPainter::drawLineSegments( const QPointArray &a, int index, int nlines )
{						// draw line segments
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
	pdev->cmd( PDC_DRAWLINESEGS, param );
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
{						// draw connected lines
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
	pdev->cmd( PDC_DRAWPOLYLINE, param );
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
{						// draw polygon
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
	pdev->cmd( PDC_DRAWPOLYGON, param );
	return;
    }
    if ( winding )				// set to winding fill mode
	SetPolyFillMode( hdc, WINDING );
    Polygon( hdc, (POINT*)(a.data()+index), npoints );
    if ( winding )				// set to normal fill mode
	SetPolyFillMode( hdc, ALTERNATE );
}


void QPainter::drawBezier(  const QPointArray &a, int index, int npoints )
{						// draw Bezier curve
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
	pdev->cmd( PDC_DRAWBEZIER, param );
	return;
    }
    PolyBezier( hdc, (POINT*)(a.data()+index), npoints );
}


void QPainter::drawPixmap( int x, int y, const QPixmap &pixmap,
			   int sx, int sy, int sw, int sh )
{						// draw pixmap
    if ( !isActive() || pixmap.isNull() )
	return;
    if ( sw < 0 )
	sw = pixmap.width();
    if ( sh < 0 )
	sh = pixmap.height();
    if ( testf(ExtDev) ) {
	if ( sx != 0 || sy != 0 ||
	     sw != pixmap.width() || sh != pixmap.height() ) {
	    QPixmap tmp( sw, sh, pixmap.depth() );
	    bitBlt( &tmp, 0, 0, &pixmap, sx, sy, sw, sh );
	    drawPixmap( x, y, tmp );
	    return;
	}
	QPDevCmdParam param[3];
	QRect  r(0,0,sw,sh);
	QPoint p(x,y);
	param[0].rect	= &r;
	param[1].point	= &p;
	param[2].pixmap = &pixmap;
	pdev->cmd( PDC_DRAWPIXMAP, param );
	return;
    }
    QPixmap *pm = (QPixmap*)&pixmap;
    bool tmp_dc = pm->handle() == 0;
    if ( tmp_dc )
	pm->allocMemDC();
    BitBlt( hdc, x, y, sw, sh, pm->handle(), sx, sy, SRCCOPY );
    if ( tmp_dc )
	pm->freeMemDC();
}


void QPainter::drawText( int x, int y, const char *str, int len )
{						// draw text
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
	pdev->cmd( PDC_DRAWTEXT, param );
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
	if ( len >= 0 )
	    newstr.truncate( len );
	param[0].rect = &r;
	param[1].ival = tf;
	param[2].str = newstr.data();
	pdev->cmd( PDC_DRAWTEXTFRMT, param );
	return;
    }

    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }

    QFontMetrics fm = fontMetrics();		// get font metrics

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
	codes	= new ushort[codelen];
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

    bool wordbreak  = (tf & WordBreak)	== WordBreak;
    bool expandtabs = (tf & ExpandTabs) == ExpandTabs;
    bool singleline = (tf & SingleLine) == SingleLine;
    bool showprefix = (tf & ShowPrefix) == ShowPrefix;

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

    if ( (tf & AlignVCenter) == AlignVCenter )	// vertically centered text
	yp = h/2 - nlines*fheight/2;
    else if ( (tf & AlignBottom) == AlignBottom)// bottom aligned
	yp = h - nlines*fheight;
    else					// top aligned
	yp = 0;
    if ( (tf & AlignRight) == AlignRight )
	xp = w - maxwidth;			// right aligned
    else if ( (tf & AlignHCenter) == AlignHCenter )
	xp = w/2 - maxwidth/2;			// centered text
    else
	xp = 0;					// left aligned

    QRect br( x+xp, y+yp, maxwidth, nlines*fheight );
    if ( brect )				// set bounding rect
	*brect = br;

    if ( (tf & DontPrint) != 0 ) {		// don't print any text
	if ( code_alloc )
	    delete codes;
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
	    while ( *cp && (*cp & BEGLINE) != BEGLINE )
		cp++;
	    yp += fheight;
	    continue;
	}

	if ( (tf & AlignRight) == AlignRight )
	    xp = w - tw;			// right aligned
	else if ( (tf & AlignHCenter) == AlignHCenter )
	    xp = w/2 - tw/2;			// centered text
	else
	    xp = 0;				// left aligned

	if ( pp )				// erase pixmap if gray text
	    pp->eraseRect( 0, 0, w, fheight );

	int bx = xp;				// base x position
	while ( TRUE ) {
	    k = 0;
	    while ( *cp && (*cp & (BEGLINE|TABSTOP)) == 0 ) {
		if ( (*cp & PREFIX) == PREFIX ) {
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
	    if ( (*cp & TABSTOP) == TABSTOP )
		xp = bx + (*cp++ & WIDTHBITS);
	    else				// *cp == 0 || *cp == BEGLINE
		break;
	}
	if ( pp ) {				// gray text
	    pp->cpen.setStyle( NoPen );
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
	    setClipRegion( crgn );
	else {					// clipping was off
	    crgn = save_rgn;
	    SelectClipRgn( hdc, 0 );
	}
    }

    if ( p_alloc )
	delete p;
    if ( code_alloc )
	delete codes;
}


QRect QPainter::boundingRect( int x, int y, int w, int h, int tf,
			      const char *str, int len, char **internal )
{
    QRect brect;
    drawText( x, y, w, h, tf, str, len, &brect, internal );
    return brect;
}
