/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptr_x11.cpp#2 $
**
** Implementation of QPainter class for X11
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qpainter.h"
#include "qwidget.h"
#include "qpntarry.h"
#include "qpixmap.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qptr_x11.cpp#2 $";
#endif


// --------------------------------------------------------------------------
// QPen member functions
//

QPen::QPen()
{
    data = new QPenData;
    CHECK_PTR( data );
    data->style = SolidLine;			// default pen settings
    data->width = 0;
    data->color = black;
}

QPen::QPen( const QColor &color, uint width, PenStyle style )
{
    data = new QPenData;
    CHECK_PTR( data );
    data->style = style;			// set pen attributes
    data->width = width;
    data->color = color;
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
    QPainter::changedPen( this, TRUE );
}

void QPen::setWidth( uint w )			// set pen width
{
    if ( data->width == w )
	return;
    data->width = w;
    QPainter::changedPen( this, TRUE );
}

void QPen::setColor( const QColor & c )		// set pen color
{
    data->color = c;
    QPainter::changedPen( this, TRUE );
}


// --------------------------------------------------------------------------
// QBrush member functions
//

QBrush::QBrush()
{
    data = new QBrushData;
    CHECK_PTR( data );
    data->style = NoBrush;			// default brush settings
    data->color = white;
    data->dpy = 0;
    data->pixmap = 0;
}

QBrush::QBrush( const QColor &color, BrushStyle style )
{
    data = new QBrushData;
    CHECK_PTR( data );
    data->style = style;			// set brush attributes
    data->color = color;
    data->dpy = 0;
    data->pixmap = 0;
}

QBrush::QBrush( const QBrush &p )
{
    data = p.data;
    data->ref();
}

QBrush::~QBrush()
{
    if ( data->deref() ) {			// delete X11 brush
	if ( data->pixmap )
	    XFreePixmap( data->dpy, data->pixmap );
	delete data;
    }
}

QBrush &QBrush::operator=( const QBrush &p )
{
    p.data->ref();				// beware of p = p
    if ( data->deref() ) {			// delete X11 brush
	if ( data->pixmap )
	    XFreePixmap( data->dpy, data->pixmap );
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
    QPainter::changedBrush( this, TRUE );
}

void QBrush::setColor( const QColor &c )	// set brush color
{
    data->color = c;
    QPainter::changedBrush( this, TRUE );
}


// --------------------------------------------------------------------------
// Trigonometric function for QPainter
//
// We have implemented simple sine and cosine function that are called from
// QPainter::drawPie() and QPainter::drawChord() when drawing the outline of
// pies and chords.
// These functions are slower and less accurate than math.h sin() and cos(),
// but with still around 1/70000th sec. execution time (on a 486DX2-66) and
// 8 digits accuracy, it should not be the bottleneck in drawing these shapes.
// The advantage is that you don't have to link in the math library.
//

const double Q_PI   = 3.14159265358979323;	// pi
const double Q_2PI  = 6.28318530717958620;	// 2*pi
const double Q_PI2  = 1.57079632679489660;	// pi/2
const double Q_3PI2 = 4.71238898038468984;	// 3*pi/2

double qsincos( double a, bool calcCos=FALSE )
{
    if ( calcCos )				// calculate cosine
	a -= Q_PI2;
    if ( a >= Q_2PI || a <= -Q_2PI ) {		// fix range: -2*pi < a < 2*pi
	int m = (int)(a/Q_2PI);
	a -= Q_2PI*m;
    }
    if ( a < 0.0 )				// 0 <= a < 2*pi
	a += Q_2PI;
    int sign = a > Q_PI ? -1 : 1;
    if ( a >= Q_PI )
	a = Q_2PI - a;
    if ( a >= Q_PI2 )
	a = Q_PI - a;
    if ( calcCos )
	sign = -sign;
    double a2  = a*a;				// here: 0 <= a < pi/4
    double a3  = a2*a;				// make taylor sin sum
    double a5  = a3*a2;
    double a7  = a5*a2;
    double a9  = a7*a2;
    double a11 = a9*a2;
    return (a-a3/6+a5/120-a7/5040+a9/362880-a11/39916800)*sign;
}

inline double qsin( double a ) { return qsincos(a,FALSE); }
inline double qcos( double a ) { return qsincos(a,TRUE); }


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
    cfont = font;
    dirtyFont = TRUE;
}

void QPainter::setPen( const QPen &pen )	// set current pen
{
    cpen = pen;
    dirtyPen = TRUE;
}

void QPainter::setBrush( const QBrush &brush )	// set current brush
{
    cbrush = brush;
    cbrush.data->dpy = dpy;			// give brush a display pointer
    dirtyBrush = TRUE;
}


void QPainter::changedFont( const QFont *font, bool dirty )
{						// a font object was changed
    if ( !list )
	return;
    register QPainter *p = list->first();
    while ( p ) {				// notify active painters
	if ( p->isActive && p->cfont.data == font->data )
	    p->dirtyFont = dirty;
	p = list->next();
    }
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
    if ( extPDev ) {
	return;
    }
    XSetFont( dpy, gc, cfont.fontId() );
}

void QPainter::updatePen()			// update after changed pen
{
    static char dash_line[]	    = { 7, 3 };
    static char dot_line[]	    = { 1, 3 };
    static char dash_dot_line[]	    = { 7, 3, 2, 3 };
    static char dash_dot_dot_line[] = { 7, 3, 2, 3, 2, 3 };

    dirtyPen = FALSE;				// pen becomes clean
    if ( extPDev ) {
	QPDevCmdParam param[3];
	param[0].i = cpen.style();
	param[1].i = cpen.width();
	param[2].ul = cpen.color().getRGB();
	pdev->cmd( PDC_SETPEN, param );
	return;
    }
    char *dashes;				// custom pen dashes
    int dash_len = 0;				// length of dash list
    int s = LineSolid;
    switch( cpen.style() ) {
	case NoPen:
	case SolidLine:
	    s = LineSolid;
	    break;
	case DashLine:
	    dashes = dash_line;
	    dash_len = sizeof(dash_line);
	    break;
	case DotLine:
	    dashes = dot_line;
	    dash_len = sizeof(dot_line);
	    break;
	case DashDotLine:
	    dashes = dash_dot_line;
	    dash_len = sizeof(dash_dot_line);
	    break;
	case DashDotDotLine:
	    dashes = dash_dot_dot_line;
	    dash_len = sizeof(dash_dot_dot_line);
	    break;
    }
    XSetForeground( dpy, gc, cpen.color().pixel() );
    XSetBackground( dpy, gc, bg_col.pixel() );
    if ( dash_len ) {				// make dash list
	XSetDashes( dpy, gc, 0, dashes, dash_len );
	s = bg_mode == TransparentMode ? LineOnOffDash : LineDoubleDash;
    }
    XSetLineAttributes( dpy, gc, cpen.width(), s, CapButt, JoinMiter );
}

void QPainter::updateBrush()			// update after changed brush
{
static char pix1_pat[] = {0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
static char pix2_pat[] = {0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
static char pix3_pat[] = {0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
static char pix4_pat[] = {0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
static char pix5_pat[] = {0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00 };
static char hor_pat[] = {			// horizontal pattern (15x15)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0x7f };
static char ver_pat[] = {			// vertical pattern (15x15)
    0x10, 0x42, 0x10, 0x42, 0x10, 0x42, 0x10, 0x42, 0x10, 0x42, 0x10, 0x42,
    0x10, 0x42, 0x10, 0x42, 0x10, 0x42, 0x10, 0x42, 0x10, 0x42, 0x10, 0x42,
    0x10, 0x42, 0x10, 0x42, 0x10, 0x42 };
static char cross_pat[] = {			// cross pattern (15x15)
    0x10, 0x42, 0x10, 0x42, 0x10, 0x42, 0x10, 0x42, 0xff, 0x7f, 0x10, 0x42,
    0x10, 0x42, 0x10, 0x42, 0x10, 0x42, 0xff, 0x7f, 0x10, 0x42, 0x10, 0x42,
    0x10, 0x42, 0x10, 0x42, 0xff, 0x7f };
static char bdiag_pat[] = {			// backward diagonal pattern
    0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01,
    0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
    0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40 };
static char fdiag_pat[] = {			// forward diagonal pattern
    0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
    0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
    0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01 };
static char dcross_pat[] = {			// diagonal cross pattern
    0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41,
    0x80, 0x80, 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
    0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41 };
static char *pat_tbl[] = {
    pix1_pat, pix2_pat, pix3_pat, pix4_pat, pix5_pat,
    hor_pat, ver_pat, cross_pat,
    bdiag_pat, fdiag_pat, dcross_pat };

    dirtyBrush = FALSE;				// brush becomes clean
    if ( extPDev ) {
	QPDevCmdParam param[2];
	param[0].i = cbrush.style();
	param[1].ul = cbrush.color().getRGB();
	pdev->cmd( PDC_SETBRUSH, param );
	return;
    }
    char *pat = 0;				// pattern
    int sz;					// defalt pattern size: sz*sz
    int s = FillSolid;
    if ( cbrush.style() >= Pix1Pattern && cbrush.style() <= DiagCrossPattern ){
	pat = pat_tbl[ cbrush.style()-Pix1Pattern ];
	if ( cbrush.style() <= Pix5Pattern )
	    sz = 8;
	else
	if ( cbrush.style() <= CrossPattern )
	    sz = 15;
	else
	    sz = 16;
    }
    else
    switch( cbrush.style() ) {			// not pattern brush
	case NoBrush:
	case SolidBrush:
	    s = FillSolid;
	    break;
	default:
	    s = FillSolid;
    }
    if ( !gc_brush ) {				// brush not yet created
	gc_brush = XCreateGC( dpy, hd, 0, 0 );
	if ( rop != CopyROP )			// update raster op for brush
	    setRasterOp( rop );
	if ( doClip )
	    XSetRegion( dpy, gc_brush, crgn.data->rgn );
    }
    if ( cbrush.data->pixmap ) {		// delete old pixmap
	XFreePixmap( cbrush.data->dpy, cbrush.data->pixmap );
	cbrush.data->pixmap = 0;
    }
    cbrush.data->dpy = dpy;			// make sure display is set
    XSetForeground( dpy, gc_brush, cbrush.color().pixel() );
    XSetBackground( dpy, gc_brush, bg_col.pixel() );
    if ( pat ) {
	cbrush.data->pixmap = XCreateBitmapFromData( dpy, hd, pat, sz, sz);
	XSetStipple( dpy, gc_brush, cbrush.data->pixmap );
	s = bg_mode == TransparentMode ? FillStippled : FillOpaqueStippled;
    }
    XSetFillStyle( dpy, gc_brush, s );
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
    extPDev = (pdev->devFlags & PDF_EXTDEV);
    dpy = pdev->dpy;
    hd = pdev->hd;				// get handle to drawable
    pdev->devFlags |= PDF_PAINTACTIVE;
    if ( extPDev ) {
	gc = 0;
	pdev->cmd( PDC_BEGIN, 0 );
    }
    else
	gc = XCreateGC( dpy, hd, 0, 0 );
    gc_brush = 0;
    bg_col = white;				// default background color
    curPt = QPoint( 0, 0 );
    sx = sy = tx = ty = 0;			// default view origins
    if ( pdev->devType() == PDT_WIDGET ) {	// device is a widget
	QWidget *w = (QWidget*)pdev;
	cfont = w->font();			// use widget font
	bg_col = w->backgroundColor();		// use widget bg color
	sw = tw = w->clientSize().width();	// default view size
	sh = th = w->clientSize().height();
    }
    else if ( pdev->devType() == PDT_PIXMAP ) {	// device is a pixmap
	QPixMap *pm = (QPixMap*)pdev;
	sw = tw = pm->size().width();		// default view size
	sh = th = pm->size().height();
    }
    else
	sw = sh = tw = th = 1;
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
    if ( extPDev )
	pdev->cmd( PDC_END, 0 );
    if ( gc_brush )
	XFreeGC( dpy, gc_brush );
    if ( gc )
	XFreeGC( dpy, gc );
    isActive = FALSE;
    pdev->devFlags &= ~PDF_PAINTACTIVE;
    pdev = 0;
    return TRUE;
}


void QPainter::setBackgroundColor( const QColor &c )
{						// set background color
    bg_col = c;
    if ( !isActive )
	return;
    if ( extPDev ) {
	QPDevCmdParam param[1];
	param[0].ul = bg_col.getRGB();
	pdev->cmd( PDC_SETBKCOLOR, param );
	return;
    }
    updatePen();				// update pen setting
    if ( gc_brush )
	updateBrush();				// update brush setting
}

void QPainter::setBackgroundMode( BGMode m )	// set background mode
{
    if ( !isActive )
	return;
    if ( m != TransparentMode && m != OpaqueMode ) {
#if defined(CHECK_RANGE)
	warning( "QPainter::setBackgroundMode: Invalid mode" );
#endif
	return;
    }
    bg_mode = m;
    if ( extPDev ) {
	QPDevCmdParam param[1];
	param[0].i = m;
	pdev->cmd( PDC_SETBKMODE, param );
	return;
    }
    updatePen();				// update pen setting
    if ( gc_brush )
	updateBrush();				// update brush setting
}

void QPainter::setRasterOp( RasterOp r )	// set raster operation
{
    static short ropCodes[] =
	{ GXcopy, GXor, GXxor, GXandInverted,
	  GXcopyInverted, GXorInverted, GXequiv, GXand };
    if ( !isActive )
	return;
    if ( !(r >= CopyROP && r <= NotEraseROP) ) {
#if defined(CHECK_RANGE)
	warning( "QPainter::setRasterOp: Invalid ROP code" );
#endif
	return;
    }
    rop = r;
    if ( extPDev ) {
	QPDevCmdParam param[1];
	param[0].i = r;
	pdev->cmd( PDC_SETROP, param );
	return;
    }
    XSetFunction( dpy, gc, ropCodes[rop] );
    if ( gc_brush )
	XSetFunction( dpy, gc_brush, ropCodes[rop] );
}


void QPainter::setXForm( bool onOff )		// set xform on/off
{
    if ( !isActive || onOff == doXForm )
	return;
    doXForm = onOff;
    if ( extPDev ) {
	QPDevCmdParam param[1];
	param[0].i = onOff;
	pdev->cmd( PDC_SETXFORM, param );
    }
}

QRect QPainter::sourceView() const		// get source view
{
    return QRect( sx, sy, sw, sh );
}

void QPainter::setSourceView( const QRect &r )	// set source view
{
    r.rect( &sx, &sy, &sw, &sh );
    if ( extPDev ) {
	QPDevCmdParam param[1];
	param[0].r = (QRect*)&r;
	pdev->cmd( PDC_SETSOURCEVIEW, param );
	return;
    }
}

QRect QPainter::targetView() const		// get target view
{
    return QRect( tx, ty, tw, th );
}

void QPainter::setTargetView( const QRect &r )	// set target view
{
    r.rect( &tx, &ty, &tw, &th );
    if ( extPDev ) {
	QPDevCmdParam param[1];
	param[0].r = (QRect*)&r;
	pdev->cmd( PDC_SETTARGETVIEW, param );
	return;
    }
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
    return QPoint( (tw*(pv.getX()-sx))/sw + tx,
		   (th*(pv.getY()-sy))/sh + ty );
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
{						// map point array, v -> d
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
    return QPoint( (sw*(pd.getX()-tx))/tw + sx,
		   (sh*(pd.getY()-ty))/th + sy );
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
{						// map point array, d -> v
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
    if ( extPDev )
	return;
    if ( doClip ) {
	XSetRegion( dpy, gc, crgn.data->rgn );
	if ( gc_brush )
	    XSetRegion( dpy, gc_brush, crgn.data->rgn );
    }
    else {
	XSetClipMask( dpy, gc, None );
	if ( gc_brush )
	    XSetClipMask( dpy, gc_brush, None );
    }
}

void QPainter::setRegion( const QRegion &rgn )	// set clip region
{
    crgn = rgn;
    setClipping( TRUE );
}


void QPainter::drawPoint( int x, int y )	// draw a single point
{
    if ( !isActive )
	return;
    if ( dirtyPen )
	updatePen();
    if ( extPDev ) {
	QPDevCmdParam param[1];
	QPoint p( x, y );
	param[0].p = &p;
	pdev->cmd( PDC_DRAWPOINT, param );
	return;
    }
    if ( doXForm )
	XFORM_P( x, y );
    XDrawPoint( dpy, hd, gc, x, y );
}


void QPainter::moveTo( int x, int y )		// set current point for lineTo
{
    if ( !isActive )
	return;
    if ( extPDev ) {
	QPDevCmdParam param[1];
	QPoint p( x, y );
	param[0].p = &p;
	pdev->cmd( PDC_MOVETO, param );
	return;
    }
    if ( doXForm )
	XFORM_P( x, y );
    curPt = QPoint( x, y );
}


void QPainter::lineTo( int x, int y )		// draw line from current point
{
    if ( !isActive )
	return;
    if ( dirtyPen )
	updatePen();
    if ( extPDev ) {
	QPDevCmdParam param[1];
	QPoint p( x, y );
	param[0].p = &p;
	pdev->cmd( PDC_LINETO, param );
	return;
    }
    if ( doXForm )
	XFORM_P( x, y );
    if ( cpen.style() != NoPen )
	XDrawLine( dpy, hd, gc, curPt.x(), curPt.y(), x, y );
    curPt = QPoint( x, y );
}


void QPainter::drawLine( int x1, int y1, int x2, int y2 )
{						// draw line
    if ( !isActive )
	return;
    if ( dirtyPen )
	updatePen();
    if ( extPDev ) {
	QPDevCmdParam param[2];
	QPoint p1( x1, y1 ),
	       p2( x2, y2 );
	param[0].p = &p1;
	param[1].p = &p2;
	pdev->cmd( PDC_DRAWLINE, param );
	return;
    }
    if ( doXForm ) {
	XFORM_P( x1, y1 );
	XFORM_P( x2, y2 );
    }
    if ( cpen.style() != NoPen )
	XDrawLine( dpy, hd, gc, x1, y1, x2, y2 );
    curPt = QPoint( x2, y2 );
}


void QPainter::drawRect( int x, int y, int w, int h )
{						// draw rectangle
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    if ( extPDev ) {
	QPDevCmdParam param[1];
	QRect r( x, y, w, h );
	param[0].r = &r;
	pdev->cmd( PDC_DRAWRECT, param );
	return;
    }
    if ( doXForm )
	XFORM_R( x, y, w, h );
    if ( cbrush.style() != NoBrush ) {
	int dp = cpen.style() == NoPen ? 0 : 1;
	XFillRectangle( dpy, hd, gc_brush, x+dp, y+dp, w-dp, h-dp );
    }
    if ( cpen.style() != NoPen )
	XDrawRectangle( dpy, hd, gc, x, y, w-1, h-1 );
}


void QPainter::drawRoundRect( int x, int y, int w, int h, int xRnd, int yRnd )
{						// draw round rectangle
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( xRnd <= 0 || yRnd <= 0 ) {
	drawRect( x, y, w, h );			// draw normal rectangle
	return;
    }
    if ( xRnd >= 100 )				// fix ranges
	xRnd = 99;
    if ( yRnd >= 100 )
	yRnd = 99;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    if ( extPDev ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	param[0].r = &r;
	param[1].i = xRnd;
	param[2].i = yRnd;
	pdev->cmd( PDC_DRAWROUNDRECT, param );
	return;
    }
    if ( doXForm )
	XFORM_R( x, y, w, h );
    int rx = (w*xRnd)/200;
    int ry = (h*yRnd)/200;
    int rx2 = 2*rx;
    int ry2 = 2*ry;
    w--;
    h--;
    if ( cbrush.style() != NoBrush ) {		// draw filled round rect
	int dp, ds;
	if ( cpen.style() == NoPen ) {
	    dp = 0;
	    ds = 1;
	}
	else {
	    dp = 1;
	    ds = 0;
	}
#define SET_RCT(px,py,w,h) \
    r->x=px; r->y=py; r->width=w; r->height=h; r++
	XRectangle rects[3];
	XRectangle *r = rects;
	SET_RCT( x+rx, y+dp, w-rx2, ry );
	SET_RCT( x+dp, y+ry, w+ds, h-ry2 );
	SET_RCT( x+rx, y+h-ry, w-rx2, ry+ds );
	XFillRectangles( dpy, hd, gc_brush, rects, 3 );
#undef SET_RCT
#define SET_ARC(px,py,w,h,a1,a2) \
    a->x=px; a->y=py; a->width=w; a->height=h; a->angle1=a1; a->angle2=a2; a++
	XArc arcs[4];
	XArc *a = arcs;
	SET_ARC( x+w-rx2, y, rx2, ry2, 0, 90*64 );
	SET_ARC( x, y, rx2, ry2, 90*64, 90*64 );
	SET_ARC( x, y+h-ry2, rx2, ry2, 180*64, 90*64 );
	SET_ARC( x+w-rx2, y+h-ry2, rx2, ry2, 270*64, 90*64 );
	XFillArcs( dpy, hd, gc_brush, arcs, 4 );
#undef SET_ARC
    }
    if ( cpen.style() != NoPen ) {		// draw outline
#define SET_SEG(xp1,yp1,xp2,yp2) \
    s->x1=xp1; s->y1=yp1; s->x2=xp2; s->y2=yp2; s++
	XSegment segs[4];
	XSegment *s = segs;
	SET_SEG( x+rx, y, x+w-rx, y );
	SET_SEG( x+rx, y+h, x+w-rx, y+h );
	SET_SEG( x, y+ry, x, y+h-ry );
	SET_SEG( x+w, y+ry, x+w, y+h-ry );
	XDrawSegments( dpy, hd, gc, segs, 4 );
#undef SET_SET
#define SET_ARC(px,py,w,h,a1,a2) \
    a->x=px; a->y=py; a->width=w; a->height=h; a->angle1=a1; a->angle2=a2; a++
	XArc arcs[4];
	XArc *a = arcs;
	SET_ARC( x+w-rx2, y, rx2, ry2, 0, 90*64 );
	SET_ARC( x, y, rx2, ry2, 90*64, 90*64 );
	SET_ARC( x, y+h-ry2, rx2, ry2, 180*64, 90*64 );
	SET_ARC( x+w-rx2, y+h-ry2, rx2, ry2, 270*64, 90*64 );
	XDrawArcs( dpy, hd, gc, arcs, 4 );
#undef SET_ARC
    }
}


void QPainter::drawEllipse( int x, int y, int w, int h )
{						// draw ellipse
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    if ( extPDev ) {
	QPDevCmdParam param[1];
	QRect r( x, y, w, h );
	param[0].r = &r;
	pdev->cmd( PDC_DRAWELLIPSE, param );
	return;
    }
    if ( doXForm )
	XFORM_R( x, y, w, h );
    w--;
    h--;
    if ( cbrush.style() != NoBrush )		// draw filled ellipse
	XFillArc( dpy, hd, gc_brush, x, y, w, h, 0, 360*64 );
    if ( cpen.style() != NoPen )		// draw outline
	XDrawArc( dpy, hd, gc, x, y, w, h, 0, 360*64 );
}


void QPainter::drawArc( int x, int y, int w, int h, int a1, int a2 )
{						// draw arc
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( extPDev ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	param[0].r = &r;
	param[1].i = a1;
	param[2].i = a2;
	pdev->cmd( PDC_DRAWARC, param );
	return;
    }
    if ( doXForm )
	XFORM_R( x, y, w, h );
    if ( cpen.style() != NoPen )
	XDrawArc( dpy, hd, gc, x, y, w, h, a1*4, a2*4 );
}


void QPainter::drawPie( int x, int y, int w, int h, int a1, int a2 )
{						// draw arc
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    if ( extPDev ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	param[0].r = &r;
	param[1].i = a1;
	param[2].i = a2;
	pdev->cmd( PDC_DRAWPIE, param );
	return;
    }
    if ( doXForm )
	XFORM_R( x, y, w, h );
    XSetArcMode( dpy, gc_brush, ArcPieSlice );
    if ( cbrush.style() != NoBrush )		// draw filled pie
	XFillArc( dpy, hd, gc_brush, x, y, w, h, a1*4, a2*4 );
    if ( cpen.style() != NoPen ) {		// draw pie outline
	int w2 = w/2;				// with, height in ellipsis
	int h2 = h/2;
	int xc = x+w2;
	int yc = y+h2;
	int xa1, ya1, xa2, ya2;
	double ra1, ra2;
	ra1 = Q_PI/2880.0*a1;			// convert a1,a2 to radians
	ra2 = ra1 + Q_PI/2880.0*a2;
	xa1 = xc + (int)(qcos(ra1)*w2);
	ya1 = yc - (int)(qsin(ra1)*h2);
	xa2 = xc + (int)(qcos(ra2)*w2);
	ya2 = yc - (int)(qsin(ra2)*h2);
	XDrawLine( dpy, hd, gc, xc, yc, xa1, ya1 );
	XDrawLine( dpy, hd, gc, xc, yc, xa2, ya2 );
	XDrawArc( dpy, hd, gc, x, y, w, h, a1*4, a2*4 );
    }
}


void QPainter::drawChord( int x, int y, int w, int h, int a1, int a2 )
{						// draw chord
    if ( !isActive || w <= 0 || h <= 0 )
	return;
    if ( dirtyPen )
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    if ( extPDev ) {
	QPDevCmdParam param[3];
	QRect r( x, y, w, h );
	param[0].r = &r;
	param[1].i = a1;
	param[2].i = a2;
	pdev->cmd( PDC_DRAWCHORD, param );
	return;
    }
    if ( doXForm )
	XFORM_R( x, y, w, h );
    XSetArcMode( dpy, gc_brush, ArcChord );
    if ( cbrush.style() != NoBrush )		// draw filled chord
	XFillArc( dpy, hd, gc_brush, x, y, w, h, a1*4, a2*4 );
    if ( cpen.style() != NoPen ) {		// draw chord outline
	int w2 = w/2;				// with, height in ellipsis
	int h2 = h/2;
	int xc = x+w2;
	int yc = y+h2;
	int xa1, ya1, xa2, ya2;
	double ra1, ra2;
	ra1 = Q_PI/2880.0*a1;			// convert a1,a2 to radians
	ra2 = ra1 + Q_PI/2880.0*a2;
	xa1 = xc + (int)(qcos(ra1)*w2);
	ya1 = yc - (int)(qsin(ra1)*h2);
	xa2 = xc + (int)(qcos(ra2)*w2);
	ya2 = yc - (int)(qsin(ra2)*h2);
	XDrawLine( dpy, hd, gc, xa1, ya1, xa2, ya2 );
	XDrawArc( dpy, hd, gc, x, y, w, h, a1*4, a2*4 );
    }
    XSetArcMode( dpy, gc_brush, ArcPieSlice );
}


void QPainter::drawLineSegments( const QPointArray &a )
{						// draw line segments
    if ( !isActive || a.size() < 2 || (a.size() & 1) )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    if ( extPDev ) {
	QPDevCmdParam param[1];
	param[0].a = (QPointArray*)&a;
	pdev->cmd( PDC_DRAWLINESEGS, param );
	return;
    }
    if ( doXForm ) {				// xform point array
	QPointArray axf = xForm( a );
	if ( cpen.style() != NoPen )
	    XDrawSegments( dpy, hd, gc, (XSegment*)axf.data(), axf.size()/2 );
    }
    else if ( cpen.style() != NoPen )
	XDrawSegments( dpy, hd, gc, (XSegment*)a.data(), a.size()/2 );
}


void QPainter::drawPolyline( const QPointArray &a )
{						// draw connected lines
    if ( !isActive || a.size() < 2 )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    if ( extPDev ) {
	QPDevCmdParam param[1];
	param[0].a = (QPointArray*)&a;
	pdev->cmd( PDC_DRAWPOLYLINE, param );
	return;
    }
    if ( doXForm ) {
	QPointArray axf = xForm( a );
	if ( cpen.style() != NoPen )
	    XDrawLines( dpy, hd, gc, (XPoint*)axf.data(), axf.size(),
			CoordModeOrigin );
    }
    else if ( cpen.style() != NoPen )
	XDrawLines( dpy, hd, gc, (XPoint*)a.data(), a.size(),
		    CoordModeOrigin );
}


void QPainter::drawPolygon( const QPointArray &a, bool winding )
{						// draw polygon
    if ( !isActive || a.size() < 2 )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    if ( dirtyBrush )
	updateBrush();
    if ( extPDev ) {
	QPDevCmdParam param[2];
	param[0].a = (QPointArray*)&a;
	param[1].i = winding;
	pdev->cmd( PDC_DRAWPOLYGON, param );
	return;
    }
    QPointArray axf;
    QPointArray *pa;
    if ( doXForm ) {
	axf = xForm( a );
	pa = &axf;
    }
    else
	pa = (QPointArray*)&a;
    if ( winding )				// set to winding fill rule
	XSetFillRule( dpy, gc_brush, WindingRule );
    if ( cbrush.style() != NoBrush )		// draw filled polygon
	XFillPolygon( dpy, hd, gc_brush, (XPoint*)pa->data(), pa->size(),
		      Complex, CoordModeOrigin );
    if ( cpen.style() != NoPen ) {		// draw outline
	XDrawLines( dpy, hd, gc, (XPoint*)pa->data(), pa->size(),
		    CoordModeOrigin );
	if ( pa->point(0) != a.point(pa->size()-1) ) {
	    int x1, y1, x2, y2;			// connect last to first point
	    pa->point( a.size()-1, &x1, &y1 );
	    pa->point( 0, &x2, &y2 );
	    XDrawLine( dpy, hd, gc, x1, y1, x2, y2 );
	}
    }
    if ( winding )				// set to normal fill rule
	XSetFillRule( dpy, gc_brush, EvenOddRule );
}


void QPainter::drawPixmap( int x, int y, const QPixMap &pixmap )
{						// draw pixmap
    if ( !isActive )
	return;
    if ( extPDev )
	return;
    if ( doXForm )
	XFORM_P( x, y );
    if ( pixmap.bitPlanes == 1 )		// bitmap
	XCopyPlane( dpy, pixmap.hd, hd, gc, 0, 0,
		   pixmap.sz.getWidth(), pixmap.sz.getHeight(), x, y, 1 );
    else
	XCopyArea( dpy, pixmap.hd, hd, gc, 0, 0,
		   pixmap.sz.getWidth(), pixmap.sz.getHeight(), x, y );
}


void QPainter::drawText( int x, int y, const char *str, int len )
{
    if ( !isActive )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    if ( dirtyFont )
	updateFont();
    if ( extPDev ) {
	QPDevCmdParam param[2];
	QPoint p( x, y );
	QString newstr = str;
	if ( len >= 0 )
	    newstr.resize( len );
	param[0].p = &p;
	param[1].str = newstr.data();
	pdev->cmd( PDC_DRAWTEXT, param );
	return;
    }
    if ( doXForm )
	XFORM_P( x, y );
    if ( len < 0 )
	len = strlen( str );
    if ( bg_mode == TransparentMode )
	XDrawString( dpy, hd, gc, x, y, str, len );
    else
	XDrawImageString( dpy, hd, gc, x, y, str, len );
}


void QPainter::drawText( int x, int y, int w, int h, TextAlignment ta,
			 const char *str, int len )
{
    if ( !isActive )
	return;
    if ( dirtyPen )				// uses same colors as pen
	updatePen();
    if ( dirtyFont )
	updateFont();
    if ( extPDev ) {
	return;
    }
    if ( doXForm ) {
	XFORM_R( x, y, w, h );
    }
    else if ( w <= 0 || h <= 0 )
	return;
    if ( len < 0 )
	len = strlen( str );
    if ( bg_mode == TransparentMode )
	XDrawString( dpy, hd, gc, x, y, str, len );
    else
	XDrawImageString( dpy, hd, gc, x, y, str, len );
}
