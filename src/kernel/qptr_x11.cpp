/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptr_x11.cpp#21 $
**
** Implementation of QPainter class for X11
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
#include <ctype.h>
#include <malloc.h>
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qptr_x11.cpp#21 $";
#endif


// --------------------------------------------------------------------------
// QPen member functions
//

void QPen::init( const QColor &color, uint width, PenStyle style )
{
    data = new QPenData;
    CHECK_PTR( data );
    data->style = style;
    data->width = width;
    data->color = color;
}

QPen::QPen()
{
    init( black, 0, SolidLine );		// default pen
}

QPen::QPen( PenStyle style )
{
    init( black, 0, style );
}

QPen::QPen( const QColor &color, uint width, PenStyle style )
{
    init( color, width, style );
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


QPen QPen::copy() const
{
    QPen p( data->color, data->width, data->style );
    return p;
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


bool QPen::operator==( const QPen &p ) const
{
    return (p.data == data) || (p.data->style == data->style &&
	    p.data->width == data->width && p.data->color == data->color);
}


// --------------------------------------------------------------------------
// QBrush member functions
//

void QBrush::init( const QColor &color, BrushStyle style )
{
    data = new QBrushData;
    CHECK_PTR( data );
    data->style = style;
    data->color = color;
    data->dpy = 0;
    data->pixmap = 0;
    data->bitmap = 0;
}

void QBrush::reset()
{
    if ( data->bitmap )
	delete data->bitmap;
    else if ( data->pixmap )
	XFreePixmap( data->dpy, data->pixmap );
    delete data;
}


QBrush::QBrush()
{
    init( black, NoBrush );
}

QBrush::QBrush( BrushStyle style )
{
    init( black, style );
}

QBrush::QBrush( const QColor &color, BrushStyle style )
{
    init( color, style );
}

QBrush::QBrush( const QColor &color, QBitMap *bitmap )
{
    init( color, CustomPattern );
    data->bitmap = bitmap;
}

QBrush::QBrush( const QBrush &p )
{
    data = p.data;
    data->ref();
}

QBrush::~QBrush()
{
    if ( data->deref() )
	reset();
}

QBrush &QBrush::operator=( const QBrush &p )
{
    p.data->ref();				// beware of p = p
    if ( data->deref() )
	reset();
    data = p.data;
    return *this;
}


QBrush QBrush::copy() const
{
    QBrush b( data->color, data->style );	// NOTE: !!! Copies not bitmap
    return b;
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


bool QBrush::operator==( const QBrush &b ) const
{
    return (b.data == data) || (b.data->style == data->style &&
	    b.data->color == data->color &&
	    b.data->bitmap == data->bitmap);
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

const double Q_PI   = 3.14159265358979323846;	// pi                      
const double Q_2PI  = 6.28318530717958647693;	// 2*pi
const double Q_PI2  = 1.57079632679489661923;	// pi/2
const double Q_3PI2 = 4.71238898038468985769;	// 3*pi/2

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
    flags = IsStartingUp;
    bg_col = white;				// default background color
    bg_mode = TransparentMode;			// default background mode
    rop = CopyROP;				// default ROP
    tabstops = 0;				// default tabbing
    tabarray = 0;
    tabarraylen = 0;
    ps_stack = 0;
    list->insert( this );			// add to list of painters
}

QPainter::~QPainter()
{
    if ( isActive() ) {
#if defined(CHECK_STATE)
	warning( "QPainter: Painting wasn't properly end()'ed" );
#endif
	end();
    }
    if ( ps_stack )
	killPStack();
    list->remove( this );			// remove from painter list
#if defined(DEBUG)
    if ( list->isEmpty() ) {			// make sure we get no memory
	delete list;				//   leaks!
	list = 0;
    }
#endif
}


void QPainter::setFont( const QFont &font )	// set current font
{
    if ( cfont != font )
	setf( DirtyFont );
    cfont = font;
}

void QPainter::setPen( const QPen &pen )	// set current pen
{
    if ( cpen != pen )
	setf( DirtyPen );
    cpen = pen;
}

void QPainter::setBrush( const QBrush &brush )	// set current brush
{
    if ( cbrush != brush )
	setf( DirtyBrush );
    cbrush = brush;
    cbrush.data->dpy = dpy;			// give brush a display pointer
}


void QPainter::changedFont( const QFont *font, bool dirty )
{						// a font object was changed
    if ( !list )
	return;
    register QPainter *p = list->first();
    while ( p ) {				// notify active painters
	if ( p->isActive() && p->cfont.data == font->data )
	    dirty ? p->setf(DirtyFont) : p->clearf(DirtyFont);
	p = list->next();
    }
}

void QPainter::changedPen( const QPen *pen, bool dirty )
{						// a pen object was changed
    if ( !list )
	return;
    register QPainter *p = list->first();
    while ( p ) {				// notify active painters
	if ( p->isActive() && p->cpen.data == pen->data )
	    dirty ? p->setf(DirtyPen) : p->clearf(DirtyPen);
	p = list->next();
    }
}

void QPainter::changedBrush( const QBrush *brush, bool dirty )
{						// a brush object was updated
    if ( !list )
	return;
    register QPainter *p = list->first();
    while ( p ) {				// notify active painters
	if ( p->isActive() && p->cbrush.data == brush->data )
	    dirty ? p->setf(DirtyBrush) : p->clearf(DirtyBrush);
	p = list->next();
    }
}


void QPainter::updateFont()			// update after changed font
{
    clearf( DirtyFont );			// font becomes clean
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].font = &cfont;
	pdev->cmd( PDC_SETFONT, param );
	return;
    }
    if ( borrowWidgetGC ) {
	QWidget *w = (QWidget *)pdev;
	if ( cfont.fontId() == w->font().fontId() )
	    return;				// can still use widget's gc
	createOwnGC();
    }
    XSetFont( dpy, gc, cfont.fontId() );
}

void QPainter::updatePen()			// update after changed pen
{
    static char dash_line[]	    = { 7, 3 };
    static char dot_line[]	    = { 1, 3 };
    static char dash_dot_line[]	    = { 7, 3, 2, 3 };
    static char dash_dot_dot_line[] = { 7, 3, 2, 3, 2, 3 };

    clearf( DirtyPen );				// pen becomes clean
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].pen = &cpen;
	pdev->cmd( PDC_SETPEN, param );
	return;
    }
    if ( borrowWidgetGC ) {			// use the widget's GC
	QWidget *w = (QWidget*)pdev;
	if ( cpen.style() == SolidLine &&
	     cpen.width() == 0 &&
	     cpen.color().pixel() == w->fg_col.pixel() &&
	     bg_col.pixel() == w->bg_col.pixel() &&
	     bg_mode == TransparentMode &&
	     rop == CopyROP && !testf(ClipOn) )
	    return;				// can still use widget's gc
	else {
	    createOwnGC();			// calls updatePen()
	    return;
	}
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

    clearf(DirtyBrush);				// brush becomes clean
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].brush = &cbrush;
	pdev->cmd( PDC_SETBRUSH, param );
	return;
    }
    bool custom = cbrush.style() == CustomPattern;
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
	    setRasterOp( (RasterOp)rop );
	if ( testf(ClipOn) )
	    XSetRegion( dpy, gc_brush, crgn.handle() );
	if ( bro.x() != 0 || bro.y() != 0 )
	    XSetTSOrigin( dpy, gc_brush, bro.x(), bro.y() );
    }
    if ( cbrush.data->pixmap && !custom ) {
	XFreePixmap( cbrush.data->dpy, cbrush.data->pixmap );
	cbrush.data->pixmap = 0;
    }
    cbrush.data->dpy = dpy;			// make sure display is set
    XSetForeground( dpy, gc_brush, cbrush.color().pixel() );
    XSetBackground( dpy, gc_brush, bg_col.pixel() );
    if ( custom || pat ) {
	if ( custom )
	    cbrush.data->pixmap = cbrush.data->bitmap->hd;
	else
	    cbrush.data->pixmap = XCreateBitmapFromData( dpy, hd, pat, sz, sz);
	XSetStipple( dpy, gc_brush, cbrush.data->pixmap );
	s = bg_mode == TransparentMode ? FillStippled : FillOpaqueStippled;
    }
    XSetFillStyle( dpy, gc_brush, s );
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
    flags = DirtyFont | DirtyPen | DirtyBrush;	// default flags
    pdev = pdev_ov ? pdev_ov : (QPaintDevice *)pd;
    if ( pdev->devFlags & PDF_EXTDEV )
	setf(ExtDev);				// this is an extended device
    dpy = pdev->dpy;				// get display variable
    hd = pdev->hd;				// get handle to drawable
    borrowWidgetGC = FALSE;			// assume own gc
    if ( testf(ExtDev) ) {
	gc = 0;
	if ( !pdev->cmd( PDC_BEGIN, 0 ) ) {	// could not begin painting
	    pdev = 0;
	    return FALSE;
	}
    }
    else if ( pdev->devType() != PDT_WIDGET ) {	// i.e. pixmap device
	if ( reinit ) {
	    QFont  defaultFont;			// default drawing tools
	    QPen   defaultPen;
	    QBrush defaultBrush;
	    cfont  = defaultFont;		// set these drawing tools
	    cpen   = defaultPen;
	    cbrush = defaultBrush;
	}
	gc = XCreateGC( dpy, hd, 0, 0 );	// create GC
    }
    setf( IsActive );				// painter becomes active
    pdev->devFlags |= PDF_PAINTACTIVE;		// also tell paint device
    gc_brush = 0;
    bro = curPt = QPoint( 0, 0 );
    if ( reinit ) {
	bg_col = white;				// default background color
	wxmat.reset();				// reset world xform matrix
    }
    sx = sy = tx = ty = 0;			// default view origins
    if ( pdev->devType() == PDT_WIDGET ) {	// device is a widget
	QWidget *w = (QWidget*)pdev;
	cfont = w->font();			// use widget font
	bg_col = w->backgroundColor();		// use widget bg color
	sw = tw = w->clientSize().width();	// default view size
	sh = th = w->clientSize().height();
	cpen = QPen( w->foregroundColor() );	// use widget fg color
	flags &= ~DirtyPen;			// pen is ok
	if ( reinit ) {
	    cbrush = QBrush( NoBrush );
	}
	gc = w->getGC();
	borrowWidgetGC = TRUE;			// optimize gc
	if ( w->testFlag(WPaintUnclipped) ) {	// paint direct on device
	    createOwnGC();
	    updateBrush();
	    XSetSubwindowMode( w->display(), gc, IncludeInferiors );
	    XSetSubwindowMode( w->display(), gc_brush, IncludeInferiors );
	}
    }
    else if ( pdev->devType() == PDT_PIXMAP ) {	// device is a pixmap
	QPixMap *pm = (QPixMap*)pdev;
	sw = tw = pm->size().width();		// default view size
	sh = th = pm->size().height();
	if ( pm->planes() == 1 ) {		// monochrome bitmap
	    bg_col = falseColor;
	    cpen.setColor( trueColor );
	}
    }
    else
	sw = sh = tw = th = 1;
    if ( borrowWidgetGC ) {			// optimize
	bg_mode = TransparentMode;
	rop = CopyROP;
    }
    else {
	setBackgroundColor( bg_col );		// default background color
	setBackgroundMode( TransparentMode );	// default background mode
	setRasterOp( CopyROP );			// default raster operation
    }
    return TRUE;
}

bool QPainter::end()				// end painting
{
    if ( !isActive() )
	return FALSE;
    if ( testf(ExtDev) )
	pdev->cmd( PDC_END, 0 );
    if ( gc_brush )
	XFreeGC( dpy, gc_brush );
    if ( gc && !borrowWidgetGC )
	XFreeGC( dpy, gc );
    clearf( IsActive );
    pdev->devFlags &= ~PDF_PAINTACTIVE;
    pdev = 0;
    return TRUE;
}


void QPainter::createOwnGC()			// create our own GC
{
    borrowWidgetGC = FALSE;
    gc = XCreateGC( dpy, hd, 0, 0 );
    updatePen();
    updateFont();
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
    updatePen();				// update pen setting
    if ( gc_brush )
	updateBrush();				// update brush setting
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
    updatePen();				// update pen setting
    if ( gc_brush )
	updateBrush();				// update brush setting
}

void QPainter::setRasterOp( RasterOp r )	// set raster operation
{
    static short ropCodes[] =
	{ GXcopy, GXor, GXxor, GXandInverted,
	  GXcopyInverted, GXorInverted, GXequiv, GXand, GXinvert };
    if ( !isActive() )
	return;
    if ( !(r >= CopyROP && r <= NotROP) ) {
#if defined(CHECK_RANGE)
	warning( "QPainter::setRasterOp: Invalid ROP code" );
#endif
	return;
    }
    rop = r;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = r;
	pdev->cmd( PDC_SETROP, param );
	return;
    }
    if ( borrowWidgetGC && rop != CopyROP )
	createOwnGC();
    XSetFunction( dpy, gc, ropCodes[rop] );
    if ( gc_brush )
	XSetFunction( dpy, gc_brush, ropCodes[rop] );
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
    if ( gc_brush )
	XSetTSOrigin( dpy, gc_brush, x, y );
}


void QPainter::setViewXForm( bool onOff )	// set xform on/off
{
    if ( !isActive() || onOff == testf(VxF) )
	return;
    setf( VxF, onOff );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = onOff;
	pdev->cmd( PDC_SETVXFORM, param );
    }
    if ( testf(WxF) )
	updateXForm();
}

QRect QPainter::sourceView() const		// get source view
{
    return QRect( sx, sy, sw, sh );
}

void QPainter::setSourceView( int x, int y, int w, int h )
{						// set source view
    if ( !isActive() )
	return;
    sx = x;
    sy = y;
    sw = w;
    sh = h;
    if ( testf(ExtDev) ) {
	QRect r( x, y, w, h );
	QPDevCmdParam param[1];
	param[0].rect = (QRect*)&r;
	pdev->cmd( PDC_SETSOURCEVIEW, param );
	return;
    }
    if ( !testf(VxF) )
	setViewXForm( TRUE );
}

QRect QPainter::targetView() const		// get target view
{
    return QRect( tx, ty, tw, th );
}

void QPainter::setTargetView( int x, int y, int w, int h )
{						// set target view
    tx = x;
    ty = y;
    tw = w;
    th = h;
    if ( testf(ExtDev) ) {
	QRect r( x, y, w, h );
	QPDevCmdParam param[1];
	param[0].rect = (QRect*)&r;
	pdev->cmd( PDC_SETTARGETVIEW, param );
	return;
    }
    if ( !testf(VxF) )
	setViewXForm( TRUE );
}


void QPainter::setWorldXForm( bool onOff )	// set world xform on/off
{
    if ( !isActive() || onOff == testf(WxF) )
	return;
    setf( WxF, onOff );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = onOff;
	pdev->cmd( PDC_SETWXFORM, param );
    }
    updateXForm();
}

QWorldMatrix QPainter::worldMatrix() const	// get world xform matrix
{
    return wxmat;
}

void QPainter::setWorldMatrix( const QWorldMatrix &m, bool concat )
{						// set world xform matrix
    if ( concat )
	wxmat = m * wxmat;			// concatenate
    else
	wxmat = m;				// set    
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	param[0].matrix = &wxmat;
	param[1].ival = concat;
	pdev->cmd( PDC_SETWMATRIX, param );
	return;
    }
    if ( !testf(WxF) )
	setWorldXForm( TRUE );
    else
	updateXForm();
}


void QPainter::updateXForm()			// update xform params
{
    QWorldMatrix m;
    if ( testf(VxF) ) {
	m.translate( tx, ty );
	m.scale( 1.0*tw/sw, 1.0*th/sh );
	m.translate( -sx, -sy );
	m = wxmat * m;
    }
    else
	m = wxmat;
    wm11 = (int)(m.m11()*65536.0);
    wm12 = (int)(m.m12()*65536.0);
    wm21 = (int)(m.m21()*65536.0);
    wm22 = (int)(m.m22()*65536.0);
    wdx  = (int)(m.dx() *65536.0);
    wdy  = (int)(m.dy() *65536.0);
    bool invertible;
    m = m.invert( &invertible );		// invert matrix
#if defined(CHECK_RANGE)
    if ( !invertible )
	warning( "QPainter::updateXForm: World xform matrix not invertible" );
#endif
    im11 = (int)(m.m11()*65536.0);
    im12 = (int)(m.m12()*65536.0);
    im21 = (int)(m.m21()*65536.0);
    im22 = (int)(m.m22()*65536.0);
    idx  = (int)(m.dx() *65536.0);
    idy  = (int)(m.dy() *65536.0);
}


// xForm macros, use with care...

#define VXFORM_P(x,y)						\
    { x = (tw*(x-sx))/sw + tx; y = (th*(y-sy))/sh + ty; }

#define VXFORM_R(x,y,w,h)					\
    { x = (tw*(x-sx))/sw + tx; y = (th*(y-sy))/sh + ty;		\
      w = (tw*w)/sw; h = (th*h)/sh;				\
      if ( w < 0 ) { w = -w; x -= w; }				\
      if ( h < 0 ) { h = -h; y -= h; } }

#define WXFORM_P(x,y)						\
    { int xx = wm11*x+wm21*y+wdx;				\
      xx += xx>0 ? 32768 : -32768;				\
      y = wm12*x+wm22*y+wdy;					\
      y += y>0 ? 32768 : -32768;				\
      x = xx/65536;  y /= 65536; }

#define WXFORM_R(x,y,w,h)					\
    { int x1 = wm11*x+wm21*y+wdx;				\
      int y1 = wm12*x+wm22*y+wdy; 				\
      int x2 = wm11*(x+w-1)+wm21*(y+h-1)+wdx;			\
      int y2 = wm12*(x+w-1)+wm22*(y+h-1)+wdy;			\
      x1 += x1>0 ? 32768 : -32768;				\
      y1 += y1>0 ? 32768 : -32768;				\
      x2 += x2>0 ? 32768 : -32768;				\
      y2 += y2>0 ? 32768 : -32768;				\
      x=x1/65536; y=y1/65536; w=(x2-x1)/65536+1; h=(y2-y1)/65536+1; }


QPoint QPainter::xForm( const QPoint &pv ) const
{						// map point, virtual -> device
    QPoint p = pv;
    if ( testf(WxF) ) {				// world xform
	VXFORM_P( p.rx(), p.ry() );
    }
    else if ( testf(VxF) ) {			// view xform
	VXFORM_P( p.rx(), p.ry() );
    }
    return p;
}

QRect QPainter::xForm( const QRect &rv ) const
{						// map rect, virtual -> device
    if ( !testf(VxF|WxF) )
	return rv;
    int x, y, w, h;
    rv.rect( &x, &y, &w, &h );
    if ( testf(WxF) ) {				// world xform (no rot./shear)
	WXFORM_R(x,y,w,h)
    }
    else if ( testf(VxF) ) {			// view xform
	VXFORM_P(x,y);
	w = (tw*w)/sw;
	h = (th*h)/sh;
    }
    return QRect( x, y, w, h );
}

QPointArray QPainter::xForm( const QPointArray &av ) const
{						// map point array, v -> d
    if ( !testf(VxF|WxF) )
	return av;
    QPointArray a = av.copy();
    int x, y;
    for ( int i=0; i<a.size(); i++ ) {
	a.point( i, &x, &y );
	if ( testf(WxF) )
	    WXFORM_P( x, y )
	else if ( testf(VxF) )
	    VXFORM_P( x, y )
	a.setPoint( i, x, y );
    }
    return a;
}

QPoint QPainter::xFormDev( const QPoint &pd ) const
{						// map point, device -> virtual
    QPoint p = pd;
    if ( testf(WxF) ) {
	int xx = im11*p.x()+im21*p.y()+idx;
	xx += xx > 0 ? 32768 : -32768;
	int yy = im12*p.x()+im22*p.y()+idy;
	yy += yy > 0 ? 32768 : -32768;
	p.rx() = xx/65536;
	p.ry() = yy/65536;
    }
    else if ( testf(VxF) ) {
	p.rx() = (sw*(p.x()-tx))/tw + sx;
	p.ry() = (sh*(p.y()-ty))/th + sy;
    }
    return p;
}

QRect QPainter::xFormDev( const QRect &rd ) const
{						// map rect, device -> virtual
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
	x = (sw*(x-tx))/tw + sx;
	y = (sh*(y-ty))/th + sy;
	w = (sw*w)/tw;
	h = (sh*h)/th;
    }
    return QRect( x, y, w, h );
}

QPointArray QPainter::xFormDev( const QPointArray &ad ) const
{						// map point array, d -> v
    if ( !testf(VxF|WxF) )
	return ad;
    QPointArray a = ad.copy();
    int x, y;
    for ( int i=0; i<a.size(); i++ ) {
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
	    x = (sw*(x-tx))/tw + sx;
	    y = (sh*(y-ty))/th + sy;
	}
	a.setPoint( i, x, y );
    }
    return a;
}


void QPainter::setClipping( bool onOff )	// set clipping on/off
{
    if ( !isActive() || onOff == testf(ClipOn) )
	return;
    setf( ClipOn, onOff );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = onOff;
	pdev->cmd( PDC_SETCLIP, param );
	return;
    }
    if ( borrowWidgetGC )
	createOwnGC();
    if ( testf(ClipOn) ) {
	XSetRegion( dpy, gc, crgn.handle() );
	if ( gc_brush )
	    XSetRegion( dpy, gc_brush, crgn.handle() );
    }
    else {
	XSetClipMask( dpy, gc, None );
	if ( gc_brush )
	    XSetClipMask( dpy, gc_brush, None );
    }
}


void QPainter::setClipRect( const QRect &rect )	// set clip rectangle
{
    QRegion rgn( rect );
    setClipRegion( rgn );
}

void QPainter::setClipRegion( const QRegion &rgn ) // set clip region
{
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
    if ( !isActive() )
	return;
    if ( testf(DirtyPen|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QPoint p( x, y );
	    param[0].point = &p;
	    pdev->cmd( PDC_DRAWPOINT, param );
	    return;
	}
	if ( testf(WxF) ) {
	    WXFORM_P( x, y );
	}
	else if ( testf(VxF) ) {
	    VXFORM_P( x, y );
	}
    }
    XDrawPoint( dpy, hd, gc, x, y );
}


void QPainter::moveTo( int x, int y )		// set current point for lineTo
{
    if ( !isActive() )
	return;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QPoint p( x, y );
	    param[0].point = &p;
	    pdev->cmd( PDC_MOVETO, param );
	    return;
	}
	if ( testf(WxF) ) {
	    WXFORM_P( x, y );
	}
	else if ( testf(VxF) ) {
	    VXFORM_P( x, y );
	}
    }
    curPt = QPoint( x, y );
}


void QPainter::lineTo( int x, int y )		// draw line from current point
{
    if ( !isActive() )
	return;
    if ( testf(DirtyPen|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QPoint p( x, y );
	    param[0].point = &p;
	    pdev->cmd( PDC_LINETO, param );
	    return;
	}
	if ( testf(WxF) ) {
	    WXFORM_P( x, y );
	}
	else if ( testf(VxF) ) {
	    VXFORM_P( x, y );
	}
    }
    if ( cpen.style() != NoPen )
	XDrawLine( dpy, hd, gc, curPt.x(), curPt.y(), x, y );
    curPt = QPoint( x, y );
}


void QPainter::drawLine( int x1, int y1, int x2, int y2 )
{						// draw line
    if ( !isActive() )
	return;
    if ( testf(DirtyPen|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[2];
	    QPoint p1( x1, y1 ),
	    	   p2( x2, y2 );
	    param[0].point = &p1;
	    param[1].point = &p2;
	    pdev->cmd( PDC_DRAWLINE, param );
	    return;
	}
	if ( testf(WxF) ) {
	    WXFORM_P( x1, y1 );
	    WXFORM_P( x2, y2 );
	}
	else if ( testf(VxF) ) {
	    VXFORM_P( x1, y1 );
	    VXFORM_P( x2, y2 );
	}
    }
    if ( cpen.style() != NoPen )
	XDrawLine( dpy, hd, gc, x1, y1, x2, y2 );
    curPt = QPoint( x2, y2 );
}


static void fix_neg_rect( int *x, int *y, int *w, int *h )
{
    if ( *w < 0 ) {
	*w = -*w;
	*x -= *w;
    }
    if ( *h < 0 ) {
	*h = -*h;
	*y -= *h;
    }
}

void QPainter::drawRect( int x, int y, int w, int h )
{						// draw rectangle
    if ( !isActive() )
	return;
    if ( testf(DirtyPen|DirtyBrush|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(DirtyBrush) )
	    updateBrush();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    pdev->cmd( PDC_DRAWRECT, param );
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
		flags = IsActive | SafePolygon;	// fake flags to speed up
		drawPolygon( a );
		flags = tmpf;
		return;
	    }
	}
	else if ( testf(VxF) )
	    VXFORM_R( x, y, w, h );
    }
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cbrush.style() != NoBrush ) {
	int dp = cpen.style() == NoPen ? 0 : 1;
	XFillRectangle( dpy, hd, gc_brush, x+dp, y+dp, w-dp, h-dp );
    }
    if ( cpen.style() != NoPen )
	XDrawRectangle( dpy, hd, gc, x, y, w-1, h-1 );
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
    if ( testf(DirtyPen|DirtyBrush|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(DirtyBrush) )
	     updateBrush();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[3];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    param[1].ival = xRnd;
	    param[2].ival = yRnd;
	    pdev->cmd( PDC_DRAWROUNDRECT, param );
	    return;
	}
	if ( testf(WxF) ) {			// world transform
	    if ( wm12 == 99 && wm21 == 0 ) {	// scaling+translation only
		WXFORM_R(x,y,w,h);
	    }
	    else {
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
		a = xForm( a );			// xform polygon
		uint tmpf = flags;
		flags = IsActive | SafePolygon;	// fake flags to speed up
		drawPolygon( a );
		flags = tmpf;
		return;
	    }
	}
	else if ( testf(VxF) )
	    VXFORM_R( x, y, w, h );
    }
    if ( w <= 0 || h <= 0 )
	fix_neg_rect( &x, &y, &w, &h );
    else {
	w--;
	h--;
    }
    if ( w == 0 || h == 0 )
	return;
    int rx = (w*xRnd)/200;
    int ry = (h*yRnd)/200;
    int rx2 = 2*rx;
    int ry2 = 2*ry;
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
#define SET_RCT(px,py,w,h) \
    r->x=px; r->y=py; r->width=w; r->height=h; r++
	XRectangle rects[3];
	XRectangle *r = rects;
	SET_RCT( x+rx, y+dp, w-rx2, ry );
	SET_RCT( x+dp, y+ry, w+ds, h-ry2 );
	SET_RCT( x+rx, y+h-ry, w-rx2, ry+ds );
	XFillRectangles( dpy, hd, gc_brush, rects, 3 );
#undef SET_RCT
    }
    if ( cpen.style() != NoPen ) {		// draw outline
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
    }
}


void QPainter::drawEllipse( int x, int y, int w, int h )
{						// draw ellipse
    if ( !isActive() )
	return;
    if ( testf(DirtyPen|DirtyBrush|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(DirtyBrush) )
	    updateBrush();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    pdev->cmd( PDC_DRAWELLIPSE, param );
	    return;
	}
	if ( testf(WxF) ) {			// world transform
	    if ( wm12 == 0 && wm21 == 0 ) {	// scaling+translation only
		WXFORM_R(x,y,w,h);
	    }
	    else {
    		QPointArray a;
		a.makeEllipse( x, y, w, h );
		a = xForm( a );
		uint tmpf = flags;
		flags = IsActive | SafePolygon;	// fake flags to avoid overhead
		drawPolygon( a );
		flags = tmpf;
		return;
	    }
	}
	else if ( testf(VxF) )
	    VXFORM_R( x, y, w, h );
    }
    w--;
    h--;
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cbrush.style() != NoBrush )		// draw filled ellipse
	XFillArc( dpy, hd, gc_brush, x, y, w, h, 0, 360*64 );
    if ( cpen.style() != NoPen )		// draw outline
	XDrawArc( dpy, hd, gc, x, y, w, h, 0, 360*64 );
}


void QPainter::drawArc( int x, int y, int w, int h, int a1, int a2 )
{						// draw arc
    if ( !isActive() )
	return;
    if ( testf(DirtyPen|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[3];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    param[1].ival = a1;
	    param[2].ival = a2;
	    pdev->cmd( PDC_DRAWARC, param );
	    return;
	}
	if ( testf(WxF) ) {			// world transform
	    if ( wm12 == 0 && wm21 == 0 ) {	// scaling+translation only
		WXFORM_R(x,y,w,h);
	    }
	    else {
		QPointArray a;
		a.makeArc( x, y, w, h, a1, a2 );// arc polygon
		a = xForm( a );			// xform polygon
		uint tmpf = flags;
		flags = IsActive | SafePolygon;	// fake flags to speed up
		drawPolygon( a );
		flags = tmpf;
		return;
	    }
	}
	else if ( testf(VxF) )
	    VXFORM_R( x, y, w, h );
    }
    w--;
    h--;
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() != NoPen )
	XDrawArc( dpy, hd, gc, x, y, w, h, a1*4, a2*4 );
}


void QPainter::drawPie( int x, int y, int w, int h, int a1, int a2 )
{						// draw arc
    if ( !isActive() )
	return;
    if ( testf(DirtyPen|DirtyBrush|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(DirtyBrush) )
	    updateBrush();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[3];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    param[1].ival = a1;
	    param[2].ival = a2;
	    pdev->cmd( PDC_DRAWPIE, param );
	    return;
	}
	if ( testf(WxF) ) {			// world transform
	    if ( wm12 == 0 && wm21 == 0 ) {	// scaling+translation only
		WXFORM_R(x,y,w,h);
	    }
	    else {
		QPointArray a;
		a.makeArc( x, y, w, h, a1, a2 );// arc polygon
		int n = a.size();
		a.resize( n+2 );
		a.setPoint( n, x+w/2, y+h/2 );	// add legs
		a.setPoint( n+1, a.at(0) );
		a = xForm( a );			// xform polygon
		uint tmpf = flags;
		flags = IsActive | SafePolygon;	// fake flags to speed up
		drawPolygon( a );
		flags = tmpf;
		return;
	    }
	}
	else if ( testf(VxF) )
	    VXFORM_R( x, y, w, h );
    }
    XSetArcMode( dpy, gc_brush, ArcPieSlice );
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
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
    if ( !isActive() )
	return;
    if ( testf(DirtyPen|DirtyBrush|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(DirtyBrush) )
	    updateBrush();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[3];
	    QRect r( x, y, w, h );
	    param[0].rect = &r;
	    param[1].ival = a1;
	    param[2].ival = a2;
	    pdev->cmd( PDC_DRAWCHORD, param );
	    return;
	}
	if ( testf(WxF) ) {			// world transform
	    if ( wm12 == 0 && wm21 == 0 ) {	// scaling+translation only
		WXFORM_R(x,y,w,h);
	    }
	    else {
		QPointArray a;
		a.makeArc( x, y, w-1, h-1, a1, a2 ); // arc polygon
		int n = a.size();
		a.resize( n+1 );
		a.setPoint( n, a.at(0) );	// connect endpoints
		a = xForm( a );			// xform polygon
		uint tmpf = flags;
		flags = IsActive | SafePolygon;	// fake flags to speed up
		drawPolygon( a );
		flags = tmpf;
		return;
	    }
	}
	else if ( testf(VxF) )
	    VXFORM_R( x, y, w, h );
    }
    XSetArcMode( dpy, gc_brush, ArcChord );
    w--;
    h--;
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }
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


void QPainter::drawLineSegments( const QPointArray &a, int index, int nlines )
{						// draw line segments
    if ( nlines < 0 )
	nlines = a.size()/2 - index/2;
    if ( index + nlines*2 > a.size() )
	nlines = (a.size() - index)/2;
    if ( !isActive() || nlines < 1 || index < 0 )
	return;
    if ( testf(DirtyPen|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(ExtDev) ) {
	    QPointArray tmp;
	    if ( nlines == a.size()/2 )
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
	if ( testf(VxF|WxF) ) {
	    if ( cpen.style() != NoPen ) {
		QPointArray axf = xForm( a );
		XDrawSegments( dpy, hd, gc, (XSegment*)(axf.data()+index),
			       nlines );
	    }
	    return;
	}
    }
    if ( cpen.style() != NoPen )
	XDrawSegments( dpy, hd, gc, (XSegment*)(a.data()+index), nlines );
}


void QPainter::drawPolyline( const QPointArray &a, int index, int npoints )
{						// draw connected lines
    if ( npoints < 0 )
	npoints = a.size() - index;
    if ( index + npoints > a.size() )
	npoints = a.size() - index;
    if ( !isActive() || npoints < 1 || index < 0 )
	return;
    if ( testf(DirtyPen|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(ExtDev) ) {
	    QPointArray tmp;
	    if ( npoints == a.size() )
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
	if ( testf(VxF|WxF) ) {
	    if ( cpen.style() != NoPen ) {
		QPointArray axf = xForm( a );
		XDrawLines( dpy, hd, gc, (XPoint*)(axf.data()+index), npoints,
			    CoordModeOrigin );
	    }
	    return;
	}
    }
    if ( cpen.style() != NoPen )
	XDrawLines( dpy, hd, gc, (XPoint*)(a.data()+index), npoints,
		    CoordModeOrigin );
}


void QPainter::drawPolygon( const QPointArray &a, bool winding,
			    int index, int npoints )
{						// draw polygon
    if ( npoints < 0 )
	npoints = a.size() - index;
    if ( index + npoints > a.size() )
	npoints = a.size() - index;
    if ( !isActive() || npoints < 1 || index < 0 )
	return;
    QPointArray axf;
    QPointArray *pa = (QPointArray*)&a;
    if ( testf(DirtyPen|DirtyBrush|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(DirtyBrush) )
	    updateBrush();
	if ( testf(ExtDev) ) {
	    QPointArray tmp;
	    if ( npoints == a.size() )
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
	if ( testf(VxF|WxF) ) {
	    axf = xForm( a );
	    pa = &axf;
	}
    }
    bool quickwxf = testf(SafePolygon);
    
    if ( winding && !quickwxf )			// set to winding fill rule
	XSetFillRule( dpy, gc_brush, WindingRule );
    if ( cbrush.style() != NoBrush ) {		// draw filled polygon
	int shape = quickwxf ? Nonconvex : Complex;
	XFillPolygon( dpy, hd, gc_brush, (XPoint*)(pa->data()+index),
		      npoints, shape, CoordModeOrigin );
    }
    if ( cpen.style() != NoPen ) {		// draw outline
	XDrawLines( dpy, hd, gc, (XPoint*)(pa->data()+index), npoints,
		    CoordModeOrigin );
	if ( pa->point(0) != a.point(index+npoints-1) ) {
	    int x1, y1, x2, y2;			// connect last to first point
	    pa->point( index+npoints-1, &x1, &y1 );
	    pa->point( index, &x2, &y2 );
	    XDrawLine( dpy, hd, gc, x1, y1, x2, y2 );
	}
    }
    if ( winding && !quickwxf )			// set to normal fill rule
	XSetFillRule( dpy, gc_brush, EvenOddRule );
}


void QPainter::drawPixMap( int x, int y, const QPixMap &pixmap )
{						// draw pixmap
    if ( !isActive() )
	return;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) )			// NOTE: bitmaps should work
	    return;
	if ( testf(WxF) )			// no scaling or rotation ;-(
	    WXFORM_P( x, y );
	if ( testf(VxF) )
	    VXFORM_P( x, y );	    
    }
    if ( pixmap.bitPlanes == 1 ) {		// bitmap
	if ( testf(DirtyPen) )			// bitmap gets pen color
	    updatePen();
	XCopyPlane( dpy, pixmap.hd, hd, gc, 0, 0,
		    pixmap.sz.width(), pixmap.sz.height(), x, y, 1 );
    }
    else
	XCopyArea( dpy, pixmap.hd, hd, gc, 0, 0,
		   pixmap.sz.width(), pixmap.sz.height(), x, y );
}


QString get_tbm_key(  const QWorldMatrix &m, const char *str, int len )
{
    QString s = str;
    s.resize( len );
    QString k;
    if ( len > 150 )
	k.resize( len + 100 );
    k.sprintf( "%s%g%g%g%g%g%g", (char *)s, m.m11(), m.m12(), m.m21(),m.m22(),
	       m.dx(), m.dy() );
    return k;
}

#include "qdict.h"
declare(QDictM,QBitMap);
static QDictM(QBitMap) *bmDict = 0;

static long bmSize = 0;

QBitMap *get_text_bitmap( const QWorldMatrix &m, const char *str, int len )
{
    QString k = get_tbm_key(m,str,len);
    if ( !bmDict )
	bmDict = new QDictM(QBitMap);
    QBitMap *bm = bmDict->find( k );
    return bm;
}
// NOTE.... Husk aa legge inn kall fra global destrukt|r!!!
void ins_text_bitmap( const QWorldMatrix &m, const char *str, int len,
		      QBitMap *bm )
{
    int sz = bm->size().width()*bm->size().height();
    if ( bmSize + sz < 240000*8 ) {
	bmSize += sz;
	QString k = get_tbm_key(m,str,len);
	bmDict->insert(k,bm);
    }
    else {
	delete bm;
//	debug( "bm refused, count = %d, size=%d", bmDict->count(),bmSize );
    }
}


void QPainter::drawText( int x, int y, const char *str, int len )
{
    if ( !isActive() )
	return;
    if ( len < 0 )
	len = strlen( str );
#if defined(CHECK_RANGE)
    else if ( len > strlen(str) )
	warning( "QPainter::drawText: Length arg exceeds real string length" );
#endif
    if ( len == 0 )				// empty string
	return;
    if ( testf(DirtyPen|DirtyFont|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(DirtyFont) )	    
	    updateFont();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[2];
	    QPoint p( x, y );
	    QString newstr = str;
	    newstr.resize( len+1 );
	    param[0].point = &p;
	    param[1].str = newstr.data();
	    pdev->cmd( PDC_DRAWTEXT, param );
	    return;
	}
	if ( testf(WxF) ) {			// draw transformed text
	    QFontMetrics fm(cfont);
	    int w = fm.width( str, len );
	    int h = fm.height();
	    int asc = fm.ascent();
	    QWorldMatrix eff_mat( wm11/65536.0, wm12/65536.0,
				  wm21/65536.0, wm22/65536.0,
				  wdx/65536.0,  wdy/65536.0 );
	    QBitMap *wx_bm = get_text_bitmap( eff_mat, str, len );
	    bool create_new_bm = wx_bm == 0;
	    QWorldMatrix mat( 1, 0, 0, 1, -eff_mat.dx(), -eff_mat.dy() );
	    mat = eff_mat * mat;
	    if ( borrowWidgetGC )
		createOwnGC();
	    if ( create_new_bm ) {		// no such cached bitmap
		QBitMap bm( w, h );		// create bitmap
		QPainter paint;
		paint.begin( &bm );		// draw text in bitmap
		paint.setFont( cfont );
		paint.eraseRect( bm.rect() );
		paint.drawText( 0, asc, str, len );
		paint.end();
		wx_bm = bm.xForm( mat );	// transform bitmap
	    }
	    WXFORM_P( x, y );
	    mat = QBitMap::trueMatrix( mat, w, h );
	    int dx, dy;
	    mat.map( 0, asc, &dx, &dy );	// compute position of bitmap
	    x -= dx;  y -= dy;
	    if ( bg_mode == OpaqueMode ) {	// opaque fill
		QPointArray a(4);
		int m, n;
		mat.map( 0, 0, &m, &n );  a.setPoint( 0, m, n );
		mat.map( w, 0, &m, &n );  a.setPoint( 1, m, n );
		mat.map( w, h, &m, &n );  a.setPoint( 2, m, n );
		mat.map( 0, h, &m, &n );  a.setPoint( 3, m, n );
		a.move( x, y );
		QBrush oldBrush = cbrush;
		setBrush( backgroundColor() );
		updateBrush();
		XFillPolygon( dpy, hd, gc_brush, (XPoint*)a.data(),
			      a.size(), Convex, CoordModeOrigin );
		setBrush( oldBrush );
	    }
	    bool do_clip = hasClipping();
	    uint tmpf = flags;
	    flags = IsActive;
	    QBitMap *draw_bm;
	    if ( do_clip ) {			// clipping enabled
		int ww = wx_bm->size().width();
		int hh = wx_bm->size().height();
		draw_bm = new QBitMap( ww, hh );
		QPainter paint;
		paint.begin( draw_bm );
		paint.eraseRect( 0, 0, ww, hh );
		QRegion rgn = crgn.copy();
		rgn.move( -x, -y );
		paint.setClipRegion( rgn );
		paint.drawPixMap( 0, 0, *wx_bm );
		paint.end();
	    }
	    else
		draw_bm = wx_bm;
	    XSetClipMask( dpy, gc, draw_bm->handle() );
	    XSetClipOrigin( dpy, gc, x, y );
	    drawPixMap( x, y, *draw_bm );	// draw bitmap!
	    flags = tmpf;
	    XSetClipMask( dpy, gc, 0 );
	    if ( do_clip ) {
		delete draw_bm;			// delete temporary bitmap
		XSetClipOrigin( dpy, gc, 0, 0 );
		XSetRegion( dpy, gc, crgn.handle() );
	    }
	    if ( create_new_bm )
		ins_text_bitmap( eff_mat, str, len, wx_bm );
	    return;
	}
	if ( testf(VxF) )
	    VXFORM_P( x, y );
    }
    if ( bg_mode == TransparentMode )
	XDrawString( dpy, hd, gc, x, y, str, len );
    else
	XDrawImageString( dpy, hd, gc, x, y, str, len );
}


class QIntPainter : public QPainter {		// internal painter functions
public:
    void addClipRect( int, int, int, int );
    void restoreClipping();
};

void QIntPainter::addClipRect( int x, int y, int w, int h )
{
    if ( borrowWidgetGC )
	createOwnGC();
    QRect r( x, y, w, h );
    if ( testf(WxF) ) {				// world xform active
	QPointArray a( r );			// complex region
	a = xForm( a );
	QRegion new_rgn( a );
	if ( testf(ClipOn) )			// add to existing region
	    new_rgn = new_rgn.intersect( crgn );
	setClipRegion( new_rgn );
    }
    else {					// simple region
	Region rgn;
	r = xForm( r );
	XRectangle xr;
	xr.x = r.x();
	xr.y = r.y();
	xr.width = r.width();
	xr.height = r.height();
	rgn = XCreateRegion();			// create X region directly
	XUnionRectWithRegion( &xr, rgn, rgn );
	if ( testf(ClipOn) )			// add to existing region
	    XIntersectRegion( rgn, crgn.handle(), rgn );
	XSetRegion( dpy, gc, rgn );
	XDestroyRegion( rgn );			// no longer needed
    }
}

void QIntPainter::restoreClipping()
{
    if ( testf(ClipOn) )			// set original region
	XSetRegion( dpy, gc, crgn.handle() );
    else
	XSetClipMask( dpy, gc, None );
}


void QPainter::drawText( int x, int y, int w, int h, int tf,
			 const char *str, int len )
{
    if ( !isActive() )
	return;
    if ( len < 0 )
	len = strlen( str );
#if defined(CHECK_RANGE)
    else if ( len > strlen(str) )
	warning( "QPainter::drawText: Length arg exceeds real string length" );
#endif
    if ( len == 0 )				// empty string
	return;
    if ( testf(DirtyPen|DirtyFont|ExtDev) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(DirtyFont) )	    
	    updateFont();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[3];
	    QRect r( x, y, w, h );
	    QString newstr = str;
	    if ( len >= 0 )
		newstr.resize( len );
	    param[0].rect = &r;
	    param[1].str = newstr.data();
	    param[2].ival = flags;
	    pdev->cmd( PDC_DRAWTEXTFRMT, param );
	    return;
	}
    }
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }

    QFontMetrics fm( cfont );

    int codelen = len + len/2 + 4;;
    ushort *codes = (ushort *)malloc( sizeof(ushort)*codelen );
    ushort cc = 0;				// character code

    const BEGLINE  = 0x8000;			// encoding 0x8zzz, zzz=width
    const TABSTOP  = 0x4000;			// encoding 0x4zzz, zzz=tab pos
    const PREFIX   = 0x2000;			// encoding 0x20zz, zz=char
    const WDBITS   = 0x1fff;			// bits for width encoding

    register char *p = (char *)str;
    int nlines     = 0;				// number of lines
    int index      = 0;				// index for codes
    int begline    = 0;				// index at beginning of line
    int breakindex = 0;				// index where to break
    int breakwidth = 0;				// width of text at breakindex
    int bcwidth	   = 0;				// width of break char
    int tabindex   = 0;				// tab array index
    int cw;					// character width
    int k = 0;					// index for p
    int tw = 0;					// text width
    short charwidth[255];			// TO BE REMOVED LATER!!!
    memset( charwidth, -1, 255*sizeof(short) );

    bool wordbreak  = (tf & WordBreak)  == WordBreak;
    bool expandtabs = (tf & ExpandTabs) == ExpandTabs;
    bool singleline = (tf & SingleLine) == SingleLine;
    bool showprefix = (tf & ShowPrefix) == ShowPrefix;

#define CWIDTH(x) (charwidth[x]>=0 ? charwidth[x] : (charwidth[x]=fm.width(x)))

    index++;					// first index contains BEGLINE

    while ( k < len ) {				// convert string to codes

	if ( wordbreak && isspace(*p) ) {
	    breakindex = index;			// good position for word break
	    breakwidth = tw;
	    bcwidth = CWIDTH( *p );
	}

	if ( *p == '\n' ) {			// newline
	    if ( singleline ) {
		cc = ' ';			// convert newline to space
		cw = CWIDTH( ' ' );
	    }
	    else {
		cc = BEGLINE;
		cw = 0;
	    }
	}

	else if ( *p == '\t' ) { 		// TAB character
	    if ( expandtabs ) {
		int tstop = 0;
		if ( tabarray ) {
		    debug( "TAB ARRAY NOT IMPLEMENTED" );
		    while ( tabindex < tabarraylen ) {
			/* NOTE!!! Tab array not implemented */
			tabindex++;
		    }
		}
		else if ( tabstops )
		    tstop = tabstops - tw%tabstops;
		cw = tstop;
		cc = TABSTOP | (tw + cw);
	    }
	    else {				// convert TAB to space
		cc = ' ';
		cw = CWIDTH( ' ' );
	    }
	}

	else if ( *p == '&' && showprefix ) {
	    cc = '&';				// assume ampersand
	    if ( k < len-1 ) {
		k++;
		p++;
		if ( *p != '&' && isprint(*p) )	// use prefix char
		    cc = PREFIX | *p;
	    }
	    cw = CWIDTH( (char)(cc & 0xff) );
	}

	else {					// normal character
	    cc = *p;
	    cw = CWIDTH( *p );
	}

	if ( wordbreak ) {			// break line
	    if ( tw+cw > w && breakindex > 0 ) {
		if ( index == breakindex ) {	// break at current index
		    cc = BEGLINE;
		    cw = 0;
		}
		else {
		    codes[begline] = BEGLINE | (breakwidth & WDBITS);
		    begline = breakindex;
		    nlines++;
		    tw -= breakwidth + bcwidth;
		    breakindex = tabindex = 0;
		}
	    }
	}

	tw += cw;
	if ( (cc & BEGLINE) == BEGLINE ) {
	    codes[begline] = BEGLINE | (tw & WDBITS);
	    begline = index;
	    nlines++;
	    tw = 0;
	    breakindex = tabindex = 0;
	}
	codes[index++] = cc;
	if ( index >= codelen ) {		// grow code array
	    codelen += len;
	    codes = (ushort *)realloc( codes, sizeof(ushort)*codelen );
	}
	k++;
	p++;
    }

    if ( (cc & BEGLINE) != BEGLINE ) {		// !!!hmm, er denne sikker???
	codes[begline] = BEGLINE | tw;
	nlines++;
    }
    else
	debug( "QPainter::drawText: UNEXPECTED" );
    codes[index++] = 0;
    codelen = index;

#if 0
    QString s;
    QString n;
    for ( index=0; index<codelen; index++ ) {
	cc = codes[index];
	if ( (cc & BEGLINE) == BEGLINE ) {
	    if ( index > 0 )
		s += '\n';
	    n.sprintf( "[%d]", cc & WDBITS );
	    s += n;
	}
	else if ( (cc & TABSTOP) == TABSTOP ) {
	    n.setNum( cc & WDBITS );
	    s += "<\\t:";
	    s += n;
	    s += ">";
	}
	else if ( (cc & PREFIX) == PREFIX ) {
	    s += "<";
	    s += (char)(cc & 0xff);
	    s += ">";
	}
	else
	    s += (char)(cc & 0xff);
    }
    debug( s );
    debug( "NLINES = %d", nlines );
#endif

    int fascent  = fm.ascent();			// get font measurements
    int fdescent = fm.descent();
    int fheight  = fm.height();
    QRegion save_rgn = crgn;			// save the current region
    int xp, yp;
    p = new char[codelen];			// buffer for printable string

    if ( nlines == 1 && (codes[0] & WDBITS) < w && h > fheight )
	tf |= DontClip;				// no need to clip

    if ( (tf & DontClip) == 0 )			// clip text
	((QIntPainter*)this)->addClipRect( x, y, w, h );

    if ( (tf & AlignVCenter) == AlignVCenter )	// vertically centered text
	yp = h/2 - nlines*fheight/2;
    else if ( (tf & AlignBottom) == AlignBottom)// bottom aligned
	yp = h - nlines*fheight;
    else					// top aligned
	yp = 0;
    yp += fascent;

    index = 0;
    while ( index < codelen ) {			// finally, draw the text

	if ( codes[index] == 0 )		// end of text
	    break;

	tw = codes[index++] & WDBITS;		// text width

	if ( tw == 0 ) {			// ignore empty line
	    while ( codes[index] && (codes[index] & BEGLINE) != BEGLINE )
		index++;
	    yp += fheight;
	    continue;
	}

	if ( (tf & AlignRight) == AlignRight )
	    xp = w - tw;			// right aligned
	else if ( (tf & AlignCenter) == AlignCenter )
	    xp = w/2 - tw/2;			// centered text
	else
	    xp = 0;				// left aligned

	int bx = xp;				// base x position
	while ( TRUE ) {
	    ushort *ci = &codes[index];
	    k = 0;
	    while ( *ci && (*ci & (BEGLINE|TABSTOP)) == 0 ) {
		if ( (*ci & PREFIX) == PREFIX ) {
		    int xcpos = fm.width( p, k );
		    drawLine( x+xp+xcpos, y+yp,
			      x+xp+xcpos+CWIDTH( *ci&0xff ), y+yp );
		}
		p[k++] = (char)*ci++;
		index++;
	    }
	    p[k] = 0;
	    drawText( x+xp, y+yp, p, k );
	    if ( (*ci & BEGLINE) == BEGLINE || *ci == 0 )
		break;
	    else if ( (*ci & TABSTOP) == TABSTOP ) {
		xp = bx + (*ci & WDBITS);
		index++;
	    }
	}
	yp += fheight;
    }

    if ( (tf & DontClip) == 0 ) {		// restore clipping
	((QIntPainter*)this)->restoreClipping();
	if ( save_rgn.handle() != crgn.handle() )
	    setClipRegion( save_rgn );
    }

    delete p;
    free( (void *)codes );
}


QRect QPainter::calcRect( int x, int y, int w, int h, int tf,
			  const char *str, int len )
{
    if ( len < 0 )
	len = strlen( str );
    QFontMetrics fm( font() );
    int fheight = fm.height();
    if ( tf & SingleLine ) {
	w = fm.width( str );
	h = fheight;
    }
    else {
	register char *p;
	int n = len;
	w = 0;
	h = 0;
	do {
	    p = (char *)str;
	    while ( n-- && *p && *p != '\n' )
		p++;
	    int tw = fm.width( str, (int)p-(int)str );
	    if ( tw > w )
		w = tw;
	    h += fheight;
	    str = p+1;
	} while ( n && *p );
    }
    return QRect( x, y, w, h );
}
