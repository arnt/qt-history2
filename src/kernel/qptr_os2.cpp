/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptr_os2.cpp#15 $
**
** Implementation of QPainter class for OS/2 PM
**
** Created : 940715
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpainter.h"
#include "qwidget.h"
#include "qpntarry.h"
#include "qpixmap.h"
#include <math.h>
#define	 INCL_PM
#include <os2.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qptr_os2.cpp#15 $");


/*****************************************************************************
  QPen member functions
 *****************************************************************************/

#define INVPEN_STYLE 0x01
#define INVPEN_WIDTH 0x02
#define INVPEN_COLOR 0x04
#define INVPEN_ALL   0x07

QPen::QPen()
{
    data = new QPenData;
    CHECK_PTR( data );
    data->style = SolidLine;			// default pen attributes
    data->width = 0;
    data->color = black;
    data->invalid = INVPEN_ALL;
}

QPen::QPen( const QColor &color, uint width, PenStyle style )
{
    data = new QPenData;
    CHECK_PTR( data );
    data->style = style;			// set pen attributes
    data->width = width;
    data->color = color;
    data->invalid = INVPEN_ALL;
}

QPen::QPen( const QPen &p )
{
    data = p.data;
    data->ref();
}

QPen::~QPen()
{
    if ( data->deref() )
	delete data;
}

QPen &QPen::operator=( const QPen &p )
{
    p.data->ref();				// beware of p = p
    if ( data->deref() )
	delete data;
    data = p.data;
    return *this;
}


void QPen::setStyle( PenStyle s )		// set pen style
{
    if ( data->style == s )
	return;
    data->style = s;
    data->invalid |= INVPEN_STYLE;
    QPainter::changedPen( this, TRUE );
}

void QPen::setWidth( uint w )			// set pen width
{
    if ( data->width == w )
	return;
    data->width = w;
    data->invalid |= INVPEN_WIDTH;
    QPainter::changedPen( this, TRUE );
}

void QPen::setColor( const QColor &c )		// set pen color
{
    data->color = c;
    data->invalid |= INVPEN_COLOR;
    QPainter::changedPen( this, TRUE );
}


bool QPen::update( HPS hps )			// update pen params
{
    if ( !data->invalid )			// pen is ok
	return FALSE;
    ulong mask = 0;
    LINEBUNDLE bundle;
    if ( data->invalid & INVPEN_STYLE ) {
	long s;
	switch ( data->style ) {
	    case NoPen:
		s = LINETYPE_INVISIBLE;
		break;
	    case SolidLine:
		s = LINETYPE_SOLID;
		break;
	    case DashLine:
		s = LINETYPE_SHORTDASH;
		break;
	    case DotLine:
		s = LINETYPE_DOT;
		break;
	    case DashDotLine:
		s = LINETYPE_DASHDOT;
		break;
	    case DashDotDotLine:
		s = LINETYPE_DASHDOUBLEDOT;
		break;
	    default:
		s = LINETYPE_INVISIBLE;
	}
	bundle.usType = s;
	mask |= LBB_TYPE;
    }
    if ( data->invalid & INVPEN_WIDTH ) {
	bundle.fxWidth = MAKEFIXED(data->width,0);
	mask |= LBB_WIDTH;
    }
    if ( data->invalid & INVPEN_COLOR ) {
	bundle.lColor = data->color.pixel();
	mask |= LBB_COLOR;
	GpiSetColor( hps, data->color.pixel() );// foreground color (text)
    }
    GpiSetAttrs( hps, PRIM_LINE, mask, 0, &bundle );
    data->invalid = 0;				// valid pen data now
    QPainter::changedPen( this, FALSE );	// update all painters
    return TRUE;
}


/*****************************************************************************
  QBrush member functions
 *****************************************************************************/

#define INVBRUSH_STYLE 0x01
#define INVBRUSH_COLOR 0x02
#define INVBRUSH_ALL   0x03

QBrush::QBrush()
{
    data = new QBrushData;
    CHECK_PTR( data );
    data->style = NoBrush;			// default brush attributes
    data->color = white;
    data->invalid = INVBRUSH_ALL;
}

QBrush::QBrush( const QColor &color, BrushStyle style )
{
    data = new QBrushData;
    CHECK_PTR( data );
    data->style = style;			// set brush attributes
    data->color = color;
    data->invalid = INVBRUSH_ALL;
}

QBrush::QBrush( const QBrush &p )
{
    data = p.data;
    data->ref();
}

QBrush::~QBrush()
{
    if ( data->deref() )
	delete data;
}

QBrush &QBrush::operator=( const QBrush &p )
{
    p.data->ref();				// beware of p = p
    if ( data->deref() )
	delete data;
    data = p.data;
    return *this;
}


void QBrush::setStyle( BrushStyle s )		// set brush style
{
    if ( data->style == s )
	return;
    data->style = s;
    data->invalid |= INVBRUSH_STYLE;
    QPainter::changedBrush( this, TRUE );
}

void QBrush::setColor( const QColor &c )	// set brush color
{
    data->color = c;
    data->invalid |= INVBRUSH_COLOR;
    QPainter::changedBrush( this, TRUE );
}


bool QBrush::update( HPS hps )			// update brush params
{
    if ( !data->invalid )			// brush is ok
	return FALSE;
    ulong mask = 0;
    AREABUNDLE bundle;
    if ( data->invalid & INVBRUSH_STYLE ) {
	long s;
	switch ( data->style ) {
	    case NoBrush:
		s = PATSYM_NOSHADE;
		break;
	    case SolidBrush:
		s = PATSYM_SOLID;
		break;
	    case Pix1Pattern:
		s = PATSYM_DENSE8;
		break;
	    case Pix2Pattern:
		s = PATSYM_DENSE7;
		break;
	    case Pix3Pattern:
		s = PATSYM_DENSE6;
		break;
	    case Pix4Pattern:
		s = PATSYM_DENSE5;
		break;
	    case Pix5Pattern:
		s = PATSYM_DENSE4;
		break;
	    case HorPattern:
		s = PATSYM_HORIZ;
		break;
	    case VerPattern:
		s = PATSYM_VERT;
		break;
	    case CrossPattern:
		s = PATSYM_HALFTONE;	// NOTE: TODO!!!
		break;
	    case BDiagPattern:
		s = PATSYM_DIAG4;
		break;
	    case FDiagPattern:
		s = PATSYM_DIAG2;
		break;
	    case DiagCrossPattern:	// NOTE: TODO!!!
		s = PATSYM_DIAG1;
		break;
	    default:
		s = PATSYM_BLANK;
	}
	bundle.usSymbol = s;
	bundle.usBackMixMode =	BM_OVERPAINT;
	mask |= (ABB_SYMBOL | ABB_BACK_MIX_MODE);
    }
    if ( data->invalid & INVBRUSH_COLOR ) {
	bundle.lColor = data->color.pixel();
	mask |= ABB_COLOR;
    }
    GpiSetAttrs( hps, PRIM_AREA, mask, 0, &bundle );
    data->invalid = 0;				// valid brush data now
    QPainter::changedBrush( this, FALSE );	// update all painters
    return TRUE;
}


/*****************************************************************************
  QPainter member functions
 *****************************************************************************/

#include "qlist.h"

typedef Q_DECLARE(QListM,QPainter) QPnList;

QPnList *QPainter::list = 0;


QPainter::QPainter()
{
    if ( !list )				// create list
	list = new QPnList;
    isActive = 0;
    bg_col = white;				// default background color
    bg_mode = TransparentMode;			// default background mode
    rop = CopyROP;				// default ROP
    list->insert( 0, this );			// add to list of painters
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
#if 0
    if ( isActive )				// release old font
	SelectObject( hdc, GetStockObject(SYSTEM_FONT) );
    cfont = font;
    dirtyFont = TRUE;
#endif
}

void QPainter::setPen( const QPen &pen )	// set current pen
{
    cpen = pen;
    dirtyPen = TRUE;
}

void QPainter::setBrush( const QBrush &brush )	// set current brush
{
    cbrush = brush;
    dirtyBrush = TRUE;
}


void QPainter::changedFont( const QFont *font, bool dirty )
{						// a font object was changed
#if 0
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
#endif
}

void QPainter::changedPen( const QPen *pen, bool dirty )
{						// a pen object was changed
    if ( !list )
	return;
    register QPainter *p = list->first();
    while ( p ) {				// notify active painters
	if ( p->isActive && p->cpen.data == pen->data )
	    p->dirtyPen = dirty;
	p = list->next();
    }
}

void QPainter::changedBrush( const QBrush *brush, bool dirty )
{						// a brush object was updated
    if ( !list )
	return;
    register QPainter *p = list->first();
    while ( p ) {				// notify active painters
	if ( p->isActive && p->cbrush.data == brush->data )
	    p->dirtyBrush = dirty;
	p = list->next();
    }
}


void QPainter::updateFont()			// update after changed font
{
#if 0 // TODO!!!
    if ( !cfont.update(hdc) )
	SelectObject( hdc, cfont.data->hfont );
#endif
}

void QPainter::updatePen()			// update after changed pen
{
    cpen.update( hps );
}

void QPainter::updateBrush()			// update after changed brush
{
    cbrush.update( hps );
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
    pdev = pdev_ov ? pdev_ov : (QPaintDevice *)pd;
    bg_col = white;				// default background color
    sx = sy = tx = ty = 0;			// default view origins
    bool setRGBTable = TRUE;
    if ( pdev->devType() == PDT_WIDGET ) {	// device is a widget
	QWidget *w = (QWidget*)pdev;
	cfont = w->font();			// use widget font
	bg_col = w->backgroundColor();		// use widget bg color
	if ( w->testWFlags(WState_Paint) ) {
	    hps = w->hps;
	    setRGBTable = FALSE;
	}
	else {
	    hps = WinGetPS( w->id() );
	    w->hps = 0;
	}
	sw = tw = w->clientSize().width();	// default view size
	sh = th = w->clientSize().height();
	dh = w->clientSize().height() - 1;	// set device height
    }
    else if ( pdev->devType() == PDT_PIXMAP ) { // device is a pixmap
	QPixMap *pm = (QPixMap*)pdev;
	hps = pm->hps;				// draw in pixmap
	sw = tw = pm->size().width();		// default view size
	sh = th = pm->size().height();
	dh = pm->sz.height() - 1;		// set device height
    }
    else
	sw = sh = tw = th = 1;
    if ( setRGBTable )				// use RGB instead of CLR_...
	GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0, 0, 0 );
    lctrl = 0;
    setBackgroundColor( bg_col );		// default background color
    setBackgroundMode( TransparentMode );	// default background mode
    setRasterOp( CopyROP );			// default raster operation
    return TRUE;
}

bool QPainter::end()				// end painting
{
    if ( !isActive ) {
#if defined(CHECK_STATE)
	warning( "QPainter::end: No begin()" );
#endif
	return FALSE;
    }
    if ( pdev->devType() == PDT_WIDGET && !pdev->hps )
	WinReleasePS( hps );
    isActive = 0;
    return TRUE;
}


void QPainter::setBackgroundColor( const QColor &c )
{						// set background color
    bg_col = c;
    if ( isActive )
	GpiSetBackColor( hps, c.pixel() );
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
	GpiSetBackMix( hps, m == TransparentMode ?
		       BM_LEAVEALONE : BM_OVERPAINT );
}

void QPainter::setRasterOp( RasterOp r )	// set raster operation
{
    static long ropCodes[] =
	{ FM_OVERPAINT, FM_OR, FM_XOR, FM_SUBTRACT,
	  FM_NOTCOPYSRC, FM_MERGENOTSRC, FM_NOTXORSRC,
	  FM_AND, FM_INVERT };
    if ( r < CopyROP || r > NotROP ) {
#if defined(CHECK_RANGE)
	warning( "QPainter::setRasterOp: Invalid ROP code" );
#endif
	return;
    }
    rop = r;
    if ( isActive )
	GpiSetMix( hps, ropCodes[rop] );
}


void QPainter::setXForm( bool onOff )		// set xform on/off
{
    if ( !isActive || onOff == doXForm )
	return;
    doXForm = onOff;
}

QRect QPainter::sourceView() const		// get source view
{
    return QRect( sx, sy, sw, sh );
}

void QPainter::setSourceView( const QRect &r )	// set source view
{
    r.rect( &sx, &sy, &sw, &sh );
}

QRect QPainter::targetView() const		// get target view
{
    return QRect( tx, ty, tw, th );
}

void QPainter::setTargetView( const QRect &r )	// set target view
{
    r.rect( &tx, &ty, &tw, &th );
}

// xForm macros, use with care...

#define XFORM_P(x,y) \
    { x = (tw*(x-sx))/sw + tx; y = (th*(y-sy))/sh + ty; }

#define XFORM_R(x,y,w,h) \
    { x = (tw*(x-sx))/sw + tx; y = (th*(y-sy))/sh + ty; \
      w = (tw*w)/sw; h = (th*h)/sh; \
      if ( w < 0 ) { w = -w; x -= w; } \
      if ( h < 0 ) { h = -h; y -= h; } }

QPoint QPainter::xForm( const QPoint &pv ) const
{						// map point, virtual -> device
    if ( !doXForm )
	return pv;
    return QPoint( (tw*(pv.x()-sx))/sw + tx,
		   (th*(pv.y()-sy))/sh + ty );
}

QRect QPainter::xForm( const QRect &rv ) const
{						// map rect, virtual -> device
    if ( !doXForm )
	return rv;
    int x = (tw*(rv.left()-sx))/sw + tx;
    int y = (th*(rv.top()-sy))/sh + ty;
    int w = (tw*rv.width())/sw;
    int h = (th*rv.height())/sh;
    if ( w < 0 ) {
	w = -w;
	x -= w;
    }
    if ( h < 0 ) {
	h = -h;
	y -= h;
    }
    return QRect( x, y, w, h );
}

QPointArray QPainter::xForm( const QPointArray &av ) const
{
    QPointArray a = av.copy();
    if ( !doXForm )
	return a;
    int x, y;
    for ( int i=0; i<a.size(); i++ ) {
	a.point( i, &x, &y );
	XFORM_P( x, y );
	a.setPoint( i, x, y );
    }
    return a;
}

QPoint QPainter::xFormDev( const QPoint &pd ) const
{						// map point, device -> virtual
    if ( !doXForm )
	return pd;
    return QPoint( (sw*(pd.x()-tx))/tw + sx,
		   (sh*(pd.y()-ty))/th + sy );
}

QRect QPainter::xFormDev( const QRect &rd ) const
{						// map rect, device -> virtual
    if ( !doXForm )
	return rd;
    int x = (sw*(rd.left()-tx))/tw + sx;
    int y = (sh*(rd.right()-ty))/th + sy;
    int w = (sw*rd.width())/tw;
    int h = (sh*rd.height())/th;
    if ( w < 0 ) {
	w = -w;
	x -= w;
    }
    if ( h < 0 ) {
	h = -h;
	y -= h;
    }
    return QRect( x, y, w, h );
}

QPointArray QPainter::xFormDev( const QPointArray &ad ) const
{
    QPointArray a = ad.copy();
    if ( !doXForm )
	return a;
    int x, y;
    for ( int i=0; i<a.size(); i++ ) {
	a.point( i, &x, &y );
	x = (sw*(x-tx))/tw + sx;
	y = (sh*(y-ty))/th + sy;
	a.setPoint( i, x, y );
    }
    return a;
}


void QPainter::setClipping( bool onOff )	// set clipping on/off
{
    if ( !isActive || onOff == doClip )
	return;
    doClip = onOff;
    HRGN old;
    if ( doClip )
	GpiSetClipRegion( hps, crgn.data->rgn, &old );
    else
	GpiSetClipRegion( hps, 0, &old );
}

void QPainter::setRegion( const QRegion &rgn )	// set clip region
{
    crgn = rgn;
    setClipping( TRUE );
}


void QPainter::updateCtrl()			// update lctrl variable
{
    lctrl = 0;
    if ( cpen.style() != NoPen )
	lctrl |= DRO_OUTLINE;
    if ( cbrush.style() != NoBrush )
	lctrl |= DRO_FILL;
}

void QPainter::drawPoint( int x, int y )	// draw a single point
{
    if ( !isActive || cpen.style() == NoPen )
	return;
    if ( dirtyPen )
	updatePen();
    if ( doXForm )
	XFORM_P( x, y );
    POINTL p;
    p.x = x;
    p.y = dh - y;
    GpiSetPel( hps, &p );
}


void QPainter::moveTo( int x, int y )		// set current point for lineTo
{
    if ( !isActive )
	return;
    if ( doXForm )
	XFORM_P( x, y );
    POINTL p;
    p.x = x;
    p.y = dh - y;
    GpiMove( hps, &p );
}


void QPainter::lineTo( int x, int y )		// draw line from current point
{
    if ( !isActive )
	return;
    if ( dirtyPen )
	updatePen();
    if ( doXForm )
	XFORM_P( x, y );
    POINTL p;
    p.x = x;
    p.y = dh - y;
    GpiLine( hps, &p );
}


void QPainter::drawLine( int x1, int y1, int x2, int y2 )
{						// draw line
    if ( !isActive )
	return;
    if ( dirtyPen )
	updatePen();
    if ( doXForm ) {
	XFORM_P( x1, y1 );
	XFORM_P( x2, y2 );
    }
    POINTL p;
    p.x = x1;
    p.y = dh - y1;
    GpiMove( hps, &p );
    p.x = x2;
    p.y = dh - y2;
    GpiLine( hps, &p );
}


void QPainter::drawRect( int x, int y, int w, int h )
{						// draw rectangle
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    updateCtrl();
    w--; h--;
    if ( doXForm )
	XFORM_R( x, y, w, h );
    POINTL p;
    p.x = x;
    p.y = dh - y;
    GpiMove( hps, &p );
    p.x += w;
    p.y -= h;
    GpiBox( hps, lctrl, &p, 0, 0 );
}


void QPainter::drawRoundRect( int x, int y, int w, int h, int xRnd, int yRnd )
{						// draw round rectangle
    if ( xRnd > 100 || xRnd <= 0 || yRnd > 100 || yRnd <= 0 ) {
	drawRect( x, y, w, h );			// draw normal rectangle
	return;
    }
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    updateCtrl();
    w--; h--;
    if ( doXForm )
	XFORM_R( x, y, w, h );
    POINTL p;
    p.x = x;
    p.y = dh - y;
    GpiMove( hps, &p );
    p.x += w;
    p.y -= h;
    GpiBox( hps, lctrl, &p, xRnd*w/100, yRnd*h/100 );
}


void QPainter::drawEllipse( int x, int y, int w, int h )
{						// draw ellipse
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    updateCtrl();
    w--; h--;
    if ( doXForm )
	XFORM_R( x, y, w, h );
    w /= 2;
    h /= 2;
    ARCPARAMS ap = { w, h, 0, 0 };
    GpiSetArcParams( hps, &ap );
    POINTL p;
    p.x = x+w;
    p.y = dh - y-h;
    GpiMove( hps, &p );
    GpiFullArc( hps, lctrl, MAKEFIXED(1,0) );
}


void QPainter::drawArc( int x, int y, int w, int h, int a1, int a2 )
{						// draw arc
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( doXForm )
	XFORM_R( x, y, w, h );
    if ( a2 < 0 ) {				// adjust angles
	a1 -= a2;
	a2 = -a2;
    }
    if ( a1 < 0 )
	a1 += 16*360;
    w /= 2;
    h /= 2;
    ARCPARAMS ap = { w, h, 0, 0 };		// arc box
    GpiSetArcParams( hps, &ap );
    POINTL p;
    p.x = x+w;
    p.y = dh - y-h;
    LONG old = GpiQueryLineType( hps );
    GpiSetLineType( hps, LINETYPE_INVISIBLE );	// move to arc start
    GpiPartialArc( hps, &p, MAKEFIXED(1,0), MAKEFIXED(0,0),
		   MAKEFIXED(a1/16,0) );
    GpiSetLineType( hps, old );			// draw arc
    GpiPartialArc( hps, &p, MAKEFIXED(1,0), MAKEFIXED(a1/16,0),
		   MAKEFIXED(a2/16,0) );
}


void QPainter::drawPie( int x, int y, int w, int h, int a1, int a2 )
{						// draw pie
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    updateCtrl();
    if ( doXForm )
	XFORM_R( x, y, w, h );
    if ( a2 < 0 ) {				// adjust angles
	a1 -= a2;
	a2 = -a2;
    }
    if ( a1 < 0 )
	a1 += 16*360;
    w /= 2;
    h /= 2;
    ARCPARAMS ap = { w, h, 0, 0 };
    GpiSetArcParams( hps, &ap );
    POINTL p;
    p.x = x+w;
    p.y = dh - y-h;
    GpiMove( hps, &p );
    if ( lctrl & DRO_FILL )
	GpiBeginArea( hps,
		      (lctrl & DRO_OUTLINE ? BA_BOUNDARY : BA_NOBOUNDARY) |
		      BA_ALTERNATE );
    GpiPartialArc( hps, &p, MAKEFIXED(1,0), MAKEFIXED(a1/16,0),
		   MAKEFIXED(a2/16,0) );
    GpiLine( hps, &p );
    if ( lctrl & DRO_FILL )
	GpiEndArea( hps );
}


void QPainter::drawChord( int x, int y, int w, int h, int a1, int a2 )
{						// draw chord
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    updateCtrl();
    if ( doXForm )
	XFORM_R( x, y, w, h );
    if ( a2 < 0 ) {				// adjust angles
	a1 -= a2;
	a2 = -a2;
    }
    if ( a1 < 0 )
	a1 += 16*360;
    w /= 2;
    h /= 2;
    ARCPARAMS ap = { w, h, 0, 0 };		// arc box
    GpiSetArcParams( hps, &ap );
    POINTL p;
    p.x = x+w;
    p.y = dh - y-h;
    LONG old = GpiQueryLineType( hps );
    GpiSetLineType( hps, LINETYPE_INVISIBLE );	// move to arc start
    int a3 = a1 + a2;
    if ( a3 > 16*360 )
	a3 -= 16*360;
    GpiPartialArc( hps, &p, MAKEFIXED(1,0), MAKEFIXED(0,0),
		   MAKEFIXED(a3/16,0) );
    GpiSetLineType( hps, old );
    if ( lctrl & DRO_FILL )
	GpiBeginArea( hps,
		      (lctrl & DRO_OUTLINE ? BA_BOUNDARY : BA_NOBOUNDARY) |
		      BA_ALTERNATE );
    GpiPartialArc( hps, &p, MAKEFIXED(1,0), MAKEFIXED(a1/16,0),
		   MAKEFIXED(a2/16,0) );
    if ( lctrl & DRO_FILL )
	GpiEndArea( hps );
}


void QPainter::drawLineSegments( const QPointArray &a )
{						// draw line segments
    if ( !isActive || a.size() < 2 || (a.size() & 1) )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    int x1, y1, x2, y2;
    int i = 0;
    POINTL p;
    while ( i<a.size() ) {
	a.point( i++, &x1, &y1 );
	a.point( i++, &x2, &y2 );
	if ( doXForm ) {
	    XFORM_P( x1, y1 );
	    XFORM_P( x2, y2 );
	}
	p.x = x1;
	p.y = dh - y1;
	GpiMove( hps, &p );
	p.x = x2;
	p.y = dh - y2;
	GpiLine( hps, &p );
    }
}


void QPainter::drawPolyline( const QPointArray &a )
{						// draw connected lines
    if ( !isActive || a.size() < 2 )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    QPointArray axf = a.copy();
    register int i;
    if ( doXForm ) {
	int x, y;
	for ( i=0; i<axf.size(); i++ ) {
	    axf.point( i, &x, &y );
	    XFORM_P( x, y );
	    y = dh - y;
	    axf.setPoint( i, x, y );
	}
    }
    else {
	int x, y;
	for ( i=0; i<axf.size(); i++ ) {
	    axf.point( i, &x, &y );
	    XFORM_P( x, y );
	    y = dh - y;
	    axf.setPoint( i, x, y );
	}
    }
    GpiMove( hps, (POINTL*)axf.data() );
    GpiPolyLine( hps, axf.size()-1, (POINTL*)axf.data()+1 );
}


void QPainter::drawPolygon( const QPointArray &a, bool winding )
{						// draw polygon
    if ( !isActive || a.size() < 2 )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    updateCtrl();
    QPointArray axf = a.copy();
    register int i;
    if ( doXForm ) {
	int x, y;
	for ( i=0; i<axf.size(); i++ ) {
	    axf.point( i, &x, &y );
	    XFORM_P( x, y );
	    y = dh - y;
	    axf.setPoint( i, x, y );
	}
    }
    else {
	int x, y;
	for ( i=0; i<axf.size(); i++ ) {
	    axf.point( i, &x, &y );
	    XFORM_P( x, y );
	    y = dh - y;
	    axf.setPoint( i, x, y );
	}
    }
    if ( lctrl & DRO_FILL )
	GpiBeginArea( hps,
		      (lctrl & DRO_OUTLINE ? BA_BOUNDARY : BA_NOBOUNDARY) |
		      (winding ? BA_WINDING : BA_ALTERNATE) );
    GpiMove( hps, (POINTL*)axf.data() );
    GpiPolyLine( hps, axf.size()-1, (POINTL*)axf.data()+1 );
    if ( lctrl & DRO_OUTLINE )
	GpiLine( hps, (POINTL*)axf.data() );
    if ( lctrl & DRO_FILL )
	GpiEndArea( hps );
}


void QPainter::drawPixMap( int x, int y, const QPixMap &pm )
{						// draw pixmap
    if ( !isActive )
	return;
    if ( doXForm )
	XFORM_P( x, y );
    POINTL pts[3];
    pts[0].x = x;				// target, lower left
    pts[0].y = dh - y;
    pts[1].x = x + pm.sz.width();		// target, upper right
    pts[1].y = dh - y - pm.sz.height();
    pts[2].x = 0;				// source, lower left
    pts[2].y = 0;
    GpiBitBlt( hps, pm.hps, 3, pts, ROP_SRCCOPY, BBO_AND );
}


void QPainter::drawText( int x, int y, const char *s, int len )
{						// draw text
    if ( !isActive )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    if ( dirtyFont )
	updateFont();
    if ( doXForm )
	XFORM_P( x, y );
    if ( len < 0 )
	len = strlen( s );
    POINTL p;
    p.x = x;
    p.y = dh - y;
    GpiCharStringAt( hps, &p, len, (PSZ)s );
}


void QPainter::drawText( int x, int y, int w, int h, TextAlignment ta,
			 const char *s, int len)// draw aligned text
{
    drawText( x, y, s, len );
}
