/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter_win.cpp#5 $
**
** Implementation of QPainter class for Windows + NT
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
*****************************************************************************/

#include "qpainter.h"
#include "qwidget.h"
#include "qpntarry.h"
#include "qpixmap.h"
#include <math.h>
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpainter_win.cpp#5 $";
#endif


// --------------------------------------------------------------------------
// QPen member functions
//

QPen::QPen()
{
    data = new QPenData;
    CHECK_PTR( data );
    data->style = SolidLine;			// default pen attributes
    data->width = 0;
    data->color = black;
    data->hpen = 0;
    data->invalid = TRUE;
    data->stockPen = TRUE;
}

QPen::QPen( const QColor &color, uint width, PenStyle style )
{
    data = new QPenData;
    CHECK_PTR( data );
    data->style = style;			// set pen attributes
    data->width = width;
    data->color = color;
    data->hpen = 0;
    data->invalid = TRUE;
    data->stockPen = FALSE;
}

QPen::QPen( const QPen &p )
{
    data = p.data;
    data->ref();
}

QPen::~QPen()
{
    if ( data->deref() ) {
	if ( data->hpen && !data->stockPen ) {	// delete windows pen
	    QPainter::changedPen( this, TRUE );
	    DeleteObject( data->hpen );
	}
	delete data;
    }
}

QPen &QPen::operator=( const QPen &p )
{
    p.data->ref();				// beware of p = p
    if ( data->deref() ) {
	if ( data->hpen && !data->stockPen ) {	// delete windows pen
	    QPainter::changedPen( this, TRUE );
	    DeleteObject( data->hpen );
	}
	delete data;
    }
    data = p.data;
    return *this;
}


void QPen::setStyle( PenStyle s )		// set pen style
{
    if ( data->style == s )
	return;
    data->style = s;
    data->invalid = TRUE;
    QPainter::changedPen( this, TRUE );
}

void QPen::setWidth( uint w )			// set pen width
{
    if ( data->width == w )
	return;
    data->width = w;
    data->invalid = TRUE;
    QPainter::changedPen( this, TRUE );
}

void QPen::setColor( const QColor &c )		// set pen color
{
    data->color = c;
    data->invalid = TRUE;
    QPainter::changedPen( this, TRUE );
}


bool QPen::update( HDC hdc )			// update pen handle
{
    if ( !data->invalid )			// pen handle is ok
	return FALSE;
    int s;
    switch ( data->style ) {
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
    if ( data->hpen && !data->stockPen )
	DeleteObject( data->hpen );		// delete last pen
    data->stockPen = TRUE;			// assume stock pen
    ulong pixel = data->color.pixel();
    if ( data->width == 0 && s == PS_SOLID && pixel == black.pixel() )
	data->hpen = GetStockObject( BLACK_PEN );
    else
    if ( data->width == 0 && s == PS_SOLID && pixel == white.pixel() )
	data->hpen = GetStockObject( WHITE_PEN );
    else {					// must create new pen
	data->hpen = CreatePen( s, data->width, pixel );
	data->stockPen = FALSE;
    }
    SetTextColor( hdc, pixel );			// pen color is also text color
    data->invalid = FALSE;			// valid pen data now
    QPainter::changedPen( this, FALSE );	// update all painters
    return TRUE;
}


// --------------------------------------------------------------------------
// QBrush member functions
//

QBrush::QBrush()
{
    data = new QBrushData;
    CHECK_PTR( data );
    data->style = NoBrush;			// default brush attributes
    data->color = white;
    data->hbrush = 0;
    data->hbmp = 0;
    data->invalid = TRUE;
    data->stockBrush = TRUE;
}

QBrush::QBrush( const QColor &color, BrushStyle style )
{
    data = new QBrushData;
    CHECK_PTR( data );
    data->style = style;			// set brush attributes
    data->color = color;
    data->hbrush = 0;
    data->hbmp = 0;
    data->invalid = TRUE;
    data->stockBrush = FALSE;
}

QBrush::QBrush( const QBrush &p )
{
    data = p.data;
    data->ref();
}

QBrush::~QBrush()
{
    if ( data->deref() ) {			// delete windows brush
	if ( data->hbrush && !data->stockBrush ) {
	    QPainter::changedBrush( this, TRUE );
	    DeleteObject( data->hbrush );
	}
	if ( data->hbmp )
	    DeleteObject( data->hbmp );
	delete data;
    }
}

QBrush &QBrush::operator=( const QBrush &p )
{
    p.data->ref();				// beware of p = p
    if ( data->deref() ) {			// delete windows brush
	if ( data->hbrush && !data->stockBrush ) {
	    QPainter::changedBrush( this, TRUE );
	    DeleteObject( data->hbrush );
	}
	if ( data->hbmp )
	    DeleteObject( data->hbmp );
	delete data;
    }
    data = p.data;
    return *this;
}


void QBrush::setStyle( BrushStyle s )		// set brush style
{
    if ( data->style == s )
	return;
    data->style = s;
    data->invalid = TRUE;
    QPainter::changedBrush( this, TRUE );
}

void QBrush::setColor( const QColor &c )	// set brush color
{
    data->color = c;
    data->invalid = TRUE;
    QPainter::changedBrush( this, TRUE );
}


bool QBrush::update( HDC hdc )			// update brush handle
{
    static short pix1_bmp[] = {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55};
    static short pix2_bmp[] = {0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee};
    static short pix3_bmp[] = {0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff};
    static short pix4_bmp[] = {0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff};
    static short pix5_bmp[] = {0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff};

    if ( !data->invalid )			// brush handle is ok
	return FALSE;
    int s;
    ulong pixel = data->color.pixel();
    if ( data->hbrush && !data->stockBrush )
	DeleteObject( data->hbrush );		// delete last brush
    data->hbrush = 0;
    if ( data->hbmp ) {
	DeleteObject( data->hbmp );		// delete last bitmap
	data->hbmp = 0;
    }
    if ( data->style == SolidBrush ) {		// create solid brush
	data->stockBrush = TRUE;		// assume stock brush
	if ( pixel == black.pixel() )
	    s = BLACK_BRUSH;
	else
	if ( pixel == white.pixel() )
	    s = WHITE_BRUSH;
	else
	if ( pixel == gray.pixel() )
	    s = GRAY_BRUSH;
	else
	if ( pixel == darkGray.pixel() )
	    s = DKGRAY_BRUSH;
	else
	if ( pixel == lightGray.pixel() )
	    s = LTGRAY_BRUSH;
	else
	    data->stockBrush = FALSE;
	if ( data->stockBrush )
	    data->hbrush = GetStockObject( s );
	else
	    data->hbrush = CreateSolidBrush( pixel );
    }
    else
    if ( data->style == NoBrush ) {		// no brush
	data->hbrush = GetStockObject( NULL_BRUSH );
	data->stockBrush = TRUE;
    }
    else
    if ( data->style >= Pix1Pattern && data->style <= Pix5Pattern ) {
	short *bmp;
	switch ( data->style ) {		// custom pattern brush
	    case Pix1Pattern:
		bmp = pix1_bmp;
		break;
	    case Pix2Pattern:
		bmp = pix2_bmp;
		break;
	    case Pix3Pattern:
		bmp = pix3_bmp;
		break;
	    case Pix4Pattern:
		bmp = pix4_bmp;
		break;
	    case Pix5Pattern:
		bmp = pix5_bmp;
		break;
	    default:
		bmp = pix1_bmp;
	}
	data->hbmp = CreateBitmap( 8, 8, 1, 1, bmp );
	data->hbrush = CreatePatternBrush( data->hbmp );
	data->stockBrush = FALSE;
    }
    else
    switch ( data->style ) {			// hatch brush
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
    if ( !data->hbrush ) {			// one of the hatch brushes
	data->hbrush = CreateHatchBrush( s, pixel );
	data->stockBrush = FALSE;
    }
    data->invalid = FALSE;			// valid brush data now
    QPainter::changedBrush( this, FALSE );	// update all painters
    return TRUE;
}


// --------------------------------------------------------------------------
// QPainter member functions
//

#include "qlist.h"

typedef declare(QListM,QPainter) QPnList;

QPnList *QPainter::list = 0;


QPainter::QPainter()
{
    if ( !list )				// create list
	list = new QPnList;
    isActive = 0;
    bg_col = white;				// default background color
    bg_mode = TransparentMode;			// default background mode
    rop = CopyROP;				// default ROP
    list->insert( this );			// add to list of painters
}

QPainter::~QPainter()
{
#if defined(CHECK_STATE)
    if ( isActive )
	warning( "QPainter::~QPainter: No end()" );
#endif
    list->remove( this );			// remove from painter list
}


void QPainter::setFont( const QFont &font )	// set current font
{
    if ( isActive )				// release old font
	SelectObject( hdc, GetStockObject(SYSTEM_FONT) );
    cfont = font;
    dirtyFont = TRUE;
}

void QPainter::setPen( const QPen &pen )	// set current pen
{
    if ( isActive )				// release old pen
	SelectObject( hdc, GetStockObject(BLACK_PEN) );
    cpen = pen;
    dirtyPen = TRUE;
}

void QPainter::setBrush( const QBrush &brush )	// set current brush
{
    if ( isActive )				// release old brush
	SelectObject( hdc, GetStockObject(NULL_BRUSH) );
    cbrush = brush;
    dirtyBrush = TRUE;
}


void QPainter::changedFont( const QFont *font, bool dirty )
{						// a font object was changed
    if ( !list )
	return;
    register QPainter *p = list->first();
    HANDLE dummyFont = GetStockObject(SYSTEM_FONT);
    while ( p ) {				// notify active painters
	if ( p->isActive && p->cfont.data == font->data ) {
	    p->dirtyFont = dirty;
	    SelectObject( p->hdc, dirty ?
			  dummyFont :		// release current font, or
			  font->data->hfont );	// set new font handle
	}
	p = list->next();
    }
}

void QPainter::changedPen( const QPen *pen, bool dirty )
{						// a pen object was changed
    if ( !list )
	return;
    register QPainter *p = list->first();
    HANDLE dummyPen = GetStockObject(BLACK_PEN);
    while ( p ) {				// notify active painters
	if ( p->isActive && p->cpen.data == pen->data ) {
	    p->dirtyPen = dirty;
	    SelectObject( p->hdc, dirty ?
			  dummyPen :		// release current pen, or
			  pen->data->hpen );	// set new pen handle
	}
	p = list->next();
    }
}

void QPainter::changedBrush( const QBrush *brush, bool dirty )
{						// a brush object was updated
    if ( !list )
	return;
    register QPainter *p = list->first();
    HANDLE dummyBrush = GetStockObject(NULL_BRUSH);
    while ( p ) {				// notify active painters
	if ( p->isActive && p->cbrush.data == brush->data ) {
	    p->dirtyBrush = dirty;
	    SelectObject( p->hdc, dirty ?
			  dummyBrush :		// release current brush, or
			  brush->data->hbrush );// set new brush handle
	}
	p = list->next();
    }
}


void QPainter::updateFont()			// update after changed font
{
//    if ( !cfont.update(hdc) )
	SelectObject( hdc, cfont.data->hfont );
}

void QPainter::updatePen()			// update after changed pen
{
    if ( !cpen.update(hdc) )			// pen is ok
	SelectObject( hdc, cpen.data->hpen );
}

void QPainter::updateBrush()			// update after changed brush
{
    if ( !cbrush.update(hdc) )
	SelectObject( hdc, cbrush.data->hbrush );
}


bool QPainter::begin( const QPaintDevice *pd )	// begin painting in device
{
    if ( isActive ) {				// already active painting
#if defined(CHECK_STATE)
	warning( "QPainter::begin: Already active" );
#endif
	return FALSE;
    }
    isActive = TRUE;				// set active
    dirtyFont = TRUE;				// set all dirty flags
    dirtyPen = TRUE;
    dirtyBrush = TRUE;
    doXForm = FALSE;				// no transformation
    doClip = FALSE;				// no clipping
    extPDev = FALSE;				// never extern for Windows
    pdev = pdev_ov ? pdev_ov : (QPaintDevice *)pd;
    bg_col = white;				// default background color
    sx = sy = tx = ty = 0;			// default view origins
    if ( pdev->devType() == PDT_WIDGET ) {	// device is a widget
	QWidget *w = (QWidget*)pdev;
	cfont = w->font();			// use widget font
	bg_col = w->backgroundColor();		// use widget bg color
	if ( w->testFlag(WState_Paint) )
	    hdc = w->hdc;
	else {
	    hdc = GetDC( w->id() );
	    w->hdc = 0;
	}
	sw = tw = w->clientSize().width();	// default view size
	sh = th = w->clientSize().height();
    }
    else if ( pdev->devType() == PDT_PIXMAP ) { // device is a pixmap
	QPixMap *pm = (QPixMap*)pdev;
	hdc = pm->allocMemDC();
	SelectObject( hdc, pm->hbm );
	sw = tw = pm->size().width();		// default view size
	sh = th = pm->size().height();
    }
    else
	sw = sh = tw = th = 1;
    if ( QColor::hPal() ) {			// realize global palette
	SelectPalette( hdc, QColor::hPal(), FALSE );
	RealizePalette( hdc );
    }
    SetTextAlign( hdc, TA_BASELINE );
    setBackgroundColor( bg_col );		// default background color
    setBackgroundMode( TransparentMode );	// default background mode
    setRasterOp( CopyROP );			// default raster operation
    return TRUE;
}

bool QPainter::end()				// end painting
{
#if defined(CHECK_STATE)
    if ( !isActive ) {
	warning( "QPainter::end: No begin()" );
	return FALSE;
    }
#endif
    SelectObject( hdc, GetStockObject(BLACK_PEN) );
    SelectObject( hdc, GetStockObject(WHITE_BRUSH) );
    if ( pdev->devType() == PDT_WIDGET && !pdev->hdc )
	ReleaseDC( ((QWidget*)pdev)->id(), hdc );
    else if ( pdev->devType() == PDT_PIXMAP )
	((QPixMap*)pdev)->freeMemDC();
    isActive = 0;
    return TRUE;
}


void QPainter::setBackgroundColor( const QColor &c )
{						// set background color
    bg_col = c;
    if ( isActive )
	SetBkColor( hdc, c.pixel() );
}

void QPainter::setBackgroundMode( BGMode m )	// set background mode
{
    if ( m != TransparentMode && m != OpaqueMode ) {
#if defined(CHECK_RANGE)
	warning( "QPainter::setBackgroundMode: Invalid mode" );
#endif
	return;
    }
    bg_mode = m;
    if ( isActive )
	SetBkMode( hdc, m == TransparentMode ? TRANSPARENT : OPAQUE );
}

void QPainter::setRasterOp( RasterOp r )	// set raster operation
{
    static short ropCodes[] =
	{ R2_COPYPEN, R2_MERGEPEN, R2_XORPEN, R2_MASKNOTPEN,
	  R2_NOTCOPYPEN, R2_MERGENOTPEN, R2_NOTXORPEN, R2_MASKPEN,
	  R2_NOT };
    if ( r < CopyROP || r > NotROP ) {
#if defined(CHECK_RANGE)
	warning( "QPainter::setRasterOp: Invalid ROP code" );
#endif
	return;
    }
    rop = r;
    if ( isActive )
	SetROP2( hdc, ropCodes[rop] );
}


void QPainter::setXForm( bool onOff )		// set xform on/off
{
    if ( !isActive || onOff == doXForm )
	return;
    doXForm = onOff;
    if ( doXForm ) {
	SetMapMode( hdc, MM_ANISOTROPIC );
#if defined(_WS_WIN32_)
	SetWindowExtEx( hdc, sw, sh, 0 );
	SetWindowOrgEx( hdc, sx, sy, 0 );
	SetViewportExtEx( hdc, tw, th, 0 );
	SetViewportOrgEx( hdc, tx, ty, 0 );
#else
	SetWindowExt( hdc, sw, sh );
	SetWindowOrg( hdc, sx, sy );
	SetViewportExt( hdc, tw, th );
	SetViewportOrg( hdc, tx, ty );
#endif
    }
    else {
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
}

QRect QPainter::sourceView() const		// get source view
{
    return QRect( sx, sy, sw, sh );
}

void QPainter::setSourceView( const QRect &r )	// set source view
{
    r.rect( &sx, &sy, &sw, &sh );
    if ( doXForm ) {				// update Windows xform
#if defined(_WS_WIN32_)
	SetWindowExtEx( hdc, sw, sh, 0 );
	SetWindowOrgEx( hdc, sx, sy, 0 );
#else
	SetWindowExt( hdc, sw, sh );
	SetWindowOrg( hdc, sx, sy );
#endif
    }
}

QRect QPainter::targetView() const		// get target view
{
    return QRect( tx, ty, tw, th );
}

void QPainter::setTargetView( const QRect &r )	// set target view
{
    r.rect( &tx, &ty, &tw, &th );
    if ( doXForm ) {				// update windows xform
#if defined(_WS_WIN32_)
	SetViewportExtEx( hdc, tw, th, 0 );
	SetViewportOrgEx( hdc, tx, ty, 0 );
#else
	SetViewportExt( hdc, tw, th );
	SetViewportOrg( hdc, tx, ty );
#endif
    }
}

QPoint QPainter::xForm( const QPoint &pv ) const
{						// map point, virtual -> device
    POINT p;
    p.x = pv.x();
    p.y = pv.y();
    LPtoDP( hdc, &p, 1 );
    return QPoint( (QCOOT)p.x, (QCOOT)p.y );
}

QRect QPainter::xForm( const QRect &rv ) const
{						// map rect, virtual -> device
    RECT r;
    SetRect( &r, rv.left(), rv.top(), rv.right(), rv.bottom() );
    LPtoDP( hdc, (POINT*)&r, 2 );
    if ( r.right < r.left ) {			// normalize
	int t = r.right;
	r.right = r.left;
	r.left = t;
    }
    if ( r.bottom < r.top ) {
	int t = r.bottom;
	r.bottom = r.top;
	r.top = t;
    }
    return QRect( QPoint((QCOOT)r.left,(QCOOT)r.top),
		  QPoint((QCOOT)r.right,(QCOOT)r.bottom) );
}

QPointArray QPainter::xForm( const QPointArray &av ) const
{
    QPointArray a = av.copy();
    LPtoDP( hdc, (POINT*)a.data(), a.size() );
    return a;
}

QPoint QPainter::xFormDev( const QPoint &pd ) const
{						// map point, device -> virtual
    POINT p;
    p.x = pd.x();
    p.y = pd.y();
    DPtoLP( hdc, &p, 1 );
    return QPoint( (QCOOT)p.x, (QCOOT)p.y );
}

QRect QPainter::xFormDev( const QRect &rd ) const
{						// map rect, device -> virtual
    RECT r;
    SetRect( &r, rd.left(), rd.top(), rd.right(), rd.bottom() );
    DPtoLP( hdc, (POINT*)&r, 2 );
    if ( r.right < r.left ) {			// normalize
	int t = r.right;
	r.right = r.left;
	r.left = t;
    }
    if ( r.bottom < r.top ) {
	int t = r.bottom;
	r.bottom = r.top;
	r.top = t;
    }
    return QRect( QPoint((QCOOT)r.left,(QCOOT)r.top),
		  QPoint((QCOOT)r.right,(QCOOT)r.bottom) );
}

QPointArray QPainter::xFormDev( const QPointArray &ad ) const
{
    QPointArray a = ad.copy();
    DPtoLP( hdc, (POINT*)a.data(), a.size() );
    return a;
}


void QPainter::setClipping( bool onOff )	// set clipping on/off
{
    if ( !isActive || onOff == doClip )
	return;
    doClip = onOff;
    if ( doClip )
	SelectClipRgn( hdc, crgn.data->rgn );
    else
	SelectClipRgn( hdc, 0 );
}

void QPainter::setRegion( const QRegion &rgn )	// set clip region
{
    crgn = rgn;
    setClipping( TRUE );
}


void QPainter::drawPoint( int x, int y )	// draw a single point
{
    if ( !isActive || cpen.style() == NoPen )
	return;
    if ( dirtyPen )
	updatePen();
    SetPixel( hdc, x, y, cpen.color().pixel() );
}


void QPainter::moveTo( int x, int y )		// set current point for lineTo
{
    if ( !isActive )
	return;
#if defined(_WS_WIN32_)
    MoveToEx( hdc, x, y, 0 );
#else
    MoveTo( hdc, x, y );
#endif
}


void QPainter::lineTo( int x, int y )		// draw line from current point
{
    if ( !isActive )
	return;
    if ( dirtyPen )
	updatePen();
    LineTo( hdc, x, y );
    SetPixel( hdc, x, y, cpen.color().pixel() );
}


void QPainter::drawLine( int x1, int y1, int x2, int y2 )
{						// draw line
    if ( !isActive )
	return;
    if ( dirtyPen )
	updatePen();
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
    else
    if ( cpen.style() == SolidLine )		// draw last pixel
	SetPixel( hdc, x2, y2, cpen.color().pixel() );
    LineTo( hdc, x2, y2 );
}


void QPainter::drawRect( int x, int y, int w, int h )
{						// draw rectangle
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    if ( cpen.style() == NoPen ) {
	w++;
	h++;
    }
    Rectangle( hdc, x, y, x+w, y+h );
}


void QPainter::drawRoundRect( int x, int y, int w, int h, int xRnd, int yRnd )
{						// draw round rectangle
    if ( xRnd > 100 || xRnd <= 0 || yRnd > 100 || yRnd <= 0 ) {
	drawRect( x, y, w, h );			// draw normal rectangle
	return;
    }
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( cpen.style() == NoPen ) {
	w++;
	h++;
    }
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    RoundRect( hdc, x, y, x+w, y+h,
	       (int)(((long)w)*xRnd/100L),
	       (int)(((long)h)*yRnd/100L) );
}


void QPainter::drawEllipse( int x, int y, int w, int h )
{						// draw ellipse
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    Ellipse( hdc, x, y, x+w, y+h );
}


void QPainter::drawArc( int x, int y, int w, int h, int a1, int a2 )
{						// draw arc
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    int xa1, ya1, xa2, ya2;
    double ra1 = 1.090830e-3*a1;
    double ra2 = 1.090830e-3*a2 + ra1;
    if ( a2 < 0.0 ) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    int w2 = w/2;
    int h2 = h/2;
    float r = (float)(w2+h2);
    xa1 = x+w2+(int)(cos(ra1)*r);
    ya1 = y+h2-(int)(sin(ra1)*r);
    xa2 = x+w2+(int)(cos(ra2)*r);
    ya2 = y+h2-(int)(sin(ra2)*r);
    Arc( hdc, x, y, x+w, y+h, xa1, ya1, xa2, ya2 );
}


void QPainter::drawPie( int x, int y, int w, int h, int a1, int a2 )
{						// draw pie
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    int xa1, ya1, xa2, ya2;
    double ra1 = 1.090830e-3*a1;
    double ra2 = 1.090830e-3*a2 + ra1;
    if ( a2 < 0.0 ) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    int w2 = w/2;
    int h2 = h/2;
    float r = (float)(w2+h2);
    xa1 = x+w2+(int)(cos(ra1)*r);
    ya1 = y+h2-(int)(sin(ra1)*r);
    xa2 = x+w2+(int)(cos(ra2)*r);
    ya2 = y+h2-(int)(sin(ra2)*r);
    Pie( hdc, x, y, x+w, y+h, xa1, ya1, xa2, ya2 );
}


void QPainter::drawChord( int x, int y, int w, int h, int a1, int a2 )
{						// draw chord
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    int xa1, ya1, xa2, ya2;
    double ra1 = 1.090830e-3*a1;
    double ra2 = 1.090830e-3*a2 + ra1;
    if ( a2 < 0.0 ) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    int w2 = w/2;
    int h2 = h/2;
    float r = (float)(w2+h2);
    xa1 = x+w2+(int)(cos(ra1)*r);
    ya1 = y+h2-(int)(sin(ra1)*r);
    xa2 = x+w2+(int)(cos(ra2)*r);
    ya2 = y+h2-(int)(sin(ra2)*r);
    Chord( hdc, x, y, x+w, y+h, xa1, ya1, xa2, ya2 );
}


void QPainter::drawLineSegments( const QPointArray &a )
{						// draw line segments
    if ( !isActive || a.size() < 2 || (a.size() & 1) )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    int x1, y1, x2, y2;
    uint i = 0;
    while ( i<a.size() ) {
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
	else
	if ( cpen.style() == SolidLine )	// draw last pixel
	    SetPixel( hdc, x2, y2, cpen.color().pixel() );
#if defined(_WS_WIN32_)
	MoveToEx( hdc, x1, y1, 0 );
#else
	MoveTo( hdc, x1, y1 );
#endif
	LineTo( hdc, x2, y2 );
    }
}


void QPainter::drawPolyline( const QPointArray &a )
{						// draw connected lines
    if ( !isActive || a.size() < 2 )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    Polyline( hdc, (POINT*)a.data(), a.size() );
    if ( cpen.style() == SolidLine ) {
	QPoint p = a.point( a.size()-1 );	// plot last point
	SetPixel( hdc, p.x(), p.y(), cpen.color().pixel() );
    }
}


void QPainter::drawPolygon( const QPointArray &a, bool winding )
{						// draw polygon
    if ( !isActive || a.size() < 2 )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    if ( winding )				// set to winding fill mode
	SetPolyFillMode( hdc, WINDING );
    Polygon( hdc, (POINT*)a.data(), a.size() );
    if ( winding )				// set to normal fill mode
	SetPolyFillMode( hdc, ALTERNATE );
}


void QPainter::drawPixMap( int x, int y, const QPixMap &pm )
{						// draw pixmap
    if ( !isActive )
	return;
    HDC hdcmem = CreateCompatibleDC( hdc );
    SelectObject( hdcmem, pm.hbm );
    BitBlt( hdc, x, y, pm.sz.width(), pm.sz.height(),
	    hdcmem, 0, 0, SRCCOPY );
    DeleteDC( hdcmem );
}


void QPainter::drawText( int x, int y, const char *s, int len )
{						// draw text
    if ( !isActive )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    if ( dirtyFont )
	updateFont();
    if ( len < 0 )
	len = lstrlen( s );
    TextOut( hdc, x, y, s, len );
}


void QPainter::drawText( int x, int y, int w, int h, TextAlignment ta,
			 const char *s, int len)// draw aligned text
{
    if ( !isActive )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    if ( dirtyFont )
	updateFont();
    if ( len < 0 )
	len = lstrlen( s );
    TextOut( hdc, x, y, s, len );
}
