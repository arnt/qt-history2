/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter_x11.cpp#59 $
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
#include "qlist.h"
#include <ctype.h>
#include <malloc.h>
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpainter_x11.cpp#59 $";
#endif


#define CHANGE_RESET  0				// QPainter::changed.. commands
#define CHANGE_ALL    1
#define CHANGE_COLOR  2

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
    QPainter::changedPen( this, CHANGE_ALL );
}

void QPen::setWidth( uint w )			// set pen width
{
    if ( data->width == w )
	return;
    data->width = w;
    QPainter::changedPen( this, CHANGE_ALL );
}

void QPen::setColor( const QColor & c )		// set pen color
{
    ulong pixel = data->color.pixel();
    data->color = c;
    if ( pixel != c.pixel() )
	QPainter::changedPen( this, CHANGE_COLOR );
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

QBrush::QBrush( const QColor &color, const QBitMap &bitmap )
{
    init( color, CustomPattern );
    data->bitmap = new QBitMap;
    *(data->bitmap) = bitmap;
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
    QPainter::changedBrush( this, CHANGE_ALL );
}

void QBrush::setColor( const QColor &c )	// set brush color
{
    ulong pixel = data->color.pixel();
    data->color = c;
    if ( pixel != c.pixel() )
	QPainter::changedBrush( this, CHANGE_COLOR );
}

void QBrush::setBitMap( const QBitMap &bitmap )	// set brush bitmap
{
    data->style = CustomPattern;
    if ( data->bitmap )
	delete data->bitmap;
    data->bitmap = new QBitMap;
    *(data->bitmap) = bitmap;
    QPainter::changedBrush( this, CHANGE_ALL );
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
// QPainter internal GC cache
//

struct QGC {
    GC	 gc;
    char in_use;
    char mono;
};

const 	    gc_array_size = 32;
static QGC  gc_array[gc_array_size];
static bool gc_array_init = FALSE;

// #define SLOW_GC_ALLOC

static GC alloc_painter_gc( Display *dpy, Drawable hd, bool monochrome=FALSE )
{
#if defined(SLOW_GC_ALLOC)
    return XCreateGC( dpy, hd, 0, 0 );
#endif
    register QGC *p = gc_array;
    int i = gc_array_size;
    if ( !gc_array_init ) {			// initialize GC array
	while ( i-- ) {
	    p->gc     = 0;
	    p->in_use = FALSE;
	    p++;
	}
	gc_array_init = TRUE;
	p = gc_array;
	i = gc_array_size;
    }
    while ( i-- ) {
	if ( !p->gc ) {				// create GC (once)
	    p->gc = XCreateGC( dpy, hd, 0, 0 );
	    p->in_use = FALSE;
	    p->mono   = monochrome;
	}
	if ( !p->in_use && p->mono == monochrome ) {
	    p->in_use = TRUE;			// available/compatible GC
	    return p->gc;
	}
	p++;
    }
#if defined(CHECK_NULL)
    warning( "QPainter: Internal error; no available GC" );
#endif
    return XCreateGC( dpy, hd, 0, 0 );
}

static void free_painter_gc( Display *dpy, GC gc )
{
#if defined(SLOW_GC_ALLOC)
    XFreeGC( dpy, gc );
    return;
#endif
    register QGC *p = gc_array;
    int i = gc_array_size;
    if ( gc_array_init ) {
	while ( i-- ) {
	    if ( p->gc == gc ) {
		p->in_use = FALSE;		// set available
		return;
	    }
	    p++;
	}
    }
}

static void cleanup_painter_gc( Display *dpy )
{
    register QGC *p = gc_array;
    int i = gc_array_size;
    if ( gc_array_init ) {
	while ( i-- ) {
	    if ( p->gc )			// destroy GC
		XFreeGC( dpy, p->gc );
	    p++;
	}
	gc_array_init = FALSE;
    }
}


// --------------------------------------------------------------------------
// QPainter member functions
//

typedef declare(QListM,QPainter) QPnList;

QPnList *QPainter::list = 0;			// list of painters


void QPainter::initialize()
{
}

void QPainter::cleanup()
{
    delete list;
    list = 0;
    cleanup_painter_gc( qXDisplay() );
}


QPainter::QPainter()
         : cfont( TRUE )                        // create a default font
{
    if ( !list ) {				// create list
	list = new QPnList;
	CHECK_PTR( list );
    }
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
    if ( tabarray )				// delete tab array
	delete tabarray;
    if ( ps_stack )
	killPStack();
    if ( list->findRef(this) >= 0 )
	list->remove();				// remove from painter list
}

QFont &QPainter::font()
{
    if ( cfont.isDefaultFont() )
        cfont = cfont.copy();
    return cfont;
}

void QPainter::setFont( const QFont &font )	// set current font
{
    if ( cfont.d != font.d ) {
	setf( DirtyFont );
	cfont = font;
    }
}

void QPainter::setPen( const QPen &pen )	// set current pen
{
    if ( cpen.data != pen.data ) {
	if ( cpen != pen )
	    setf( DirtyPen );
	cpen = pen;
    }
}

void QPainter::setPen( PenStyle style )		// set solid pen with color
{
    QPen pen( style );
    setPen( pen );
}

void QPainter::setPen( const QColor &color )	// set solid pen with color
{
    QPen pen( color );
    if ( isActive() && gc && cpen.style() == SolidLine && cpen.width() == 0 )
	XSetForeground( dpy, gc, color.pixel() );
    else if ( cpen != pen )
	setf( DirtyPen );
    cpen = pen;
}

void QPainter::setBrush( const QBrush &brush )	// set current brush
{
    if ( cbrush.data != brush.data ) {
	if ( cbrush != brush )
	    setf( DirtyBrush );
	cbrush = brush;
	cbrush.data->dpy = dpy;			// give brush a display pointer
    }
}

void QPainter::setBrush( BrushStyle style )	// set brush
{
    QBrush brush( style );
    setBrush( brush );
}

void QPainter::setBrush( const QColor &color )	// set solid brush width color
{
    QBrush brush( color );
    if ( isActive() && gc_brush && cbrush.style() == SolidBrush )
	XSetForeground( dpy, gc_brush, color.pixel() );
    else if ( cbrush != brush )
	setf( DirtyBrush );
    cbrush = brush;
    cbrush.data->dpy = dpy;
}


void QPainter::changedFont( const QFont *font )	// a font object was changed
{
    if ( !list )
	return;
    register QPainter *p = list->first();
    while ( p ) {				// notify active painters
	if ( p->isActive() && p->cfont.d == font->d )
	    p->setf(DirtyFont);
	p = list->next();
    }
}

void QPainter::changedPen( const QPen *pen, int cmd )
{						// a pen object was changed
    if ( !list )
	return;
    register QPainter *p = list->first();
    while ( p ) {				// notify active painters
	if ( p->isActive() && p->cpen.data == pen->data ) {
	    switch ( cmd ) {
	        case CHANGE_COLOR:
		    if ( p->gc ) {
			XSetForeground( p->dpy, p->gc, pen->color().pixel() );
			break;
		    }
	        case CHANGE_ALL:
		    p->setf(DirtyPen);
		    break;
		case CHANGE_RESET:
		    p->clearf(DirtyPen);
		    break;
	    }
	}
	p = list->next();
    }
}

void QPainter::changedBrush( const QBrush *brush, int cmd )
{						// a brush object was updated
    if ( !list )
	return;
    register QPainter *p = list->first();
    while ( p ) {				// notify active painters
	if ( p->isActive() && p->cbrush.data == brush->data ) {
	    switch ( cmd ) {
	        case CHANGE_COLOR:
		    if ( p->gc_brush ) {
			XSetForeground( p->dpy, p->gc_brush,
					brush->color().pixel() );
			break;
		    }
	        case CHANGE_ALL:
		    p->setf(DirtyBrush);
		    break;
		case CHANGE_RESET:
		    p->clearf(DirtyBrush);
		    break;
	    }
	}
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
    XSetFont( dpy, gc, cfont.handle() );
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
    char *dashes = 0;				// custom pen dashes
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
    int sz = 0;					// defalt pattern size: sz*sz
    int s  = FillSolid;
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
	bool mono = pdev->devType() == PDT_PIXMAP &&
	            ((QPixMap*)pdev)->depth() == 1;
	gc_brush = alloc_painter_gc( dpy, hd, mono );
	XSetLineAttributes( dpy, gc_brush, 0, LineSolid, CapButt, JoinMiter );
	if ( rop != CopyROP ) {			// update raster op for brush
	    RasterOp r = (RasterOp)rop;
	    rop = CopyROP;			// force a change
	    setRasterOp( r );
	}
	if ( testf(ClipOn) )
	    XSetRegion( dpy, gc_brush, crgn.handle() );
	if ( bro.x() != 0 || bro.y() != 0 )
	    XSetTSOrigin( dpy, gc_brush, bro.x(), bro.y() );
    }
    if ( cbrush.data->pixmap ) {		// delete old pixmap
	XFreePixmap( cbrush.data->dpy, cbrush.data->pixmap );
	cbrush.data->pixmap = 0;
    }
    cbrush.data->dpy = dpy;			// make sure display is set
    XSetForeground( dpy, gc_brush, cbrush.color().pixel() );
    XSetBackground( dpy, gc_brush, bg_col.pixel() );
    if ( custom || pat ) {
	if ( custom )
	    cbrush.data->pixmap = cbrush.data->bitmap->handle();
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
    if ( testf(ExtDev) ) {
	gc = 0;
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
    gc_brush = 0;
    bro = curPt = QPoint( 0, 0 );
    if ( reinit ) {
	bg_col = white;				// default background color
	wxmat.reset();				// reset world xform matrix
    }
    sx = sy = tx = ty = 0;			// default view origins
    if ( pdev->devType() == PDT_WIDGET ) {	// device is a widget
	QWidget *w = (QWidget*)pdev;
	cfont = w->fontRef();			// use widget font
	bg_col = w->backgroundColor();		// use widget bg color
	sw = tw = w->clientSize().width();	// default view size
	sh = th = w->clientSize().height();
	cpen = QPen( w->foregroundColor() );	// use widget fg color
	if ( reinit ) {
	    cbrush = QBrush( NoBrush );
	}
	gc = alloc_painter_gc( dpy, w->id() );
	if ( w->testFlag(WPaintUnclipped) ) {	// paint direct on device
	    updateBrush();
	    XSetSubwindowMode( w->display(), gc, IncludeInferiors );
	    XSetSubwindowMode( w->display(), gc_brush, IncludeInferiors );
	}
    }
    else if ( pdev->devType() == PDT_PIXMAP ) {	// device is a pixmap
	QPixMap *pm = (QPixMap*)pdev;
	sw = tw = pm->size().width();		// default view size
	sh = th = pm->size().height();
	bool mono = pm->depth() == 1;		// monochrome bitmap
	if ( mono ) {
	    bg_col = color0;
	    cpen.setColor( color1 );
	}
	gc = alloc_painter_gc( dpy, hd, mono );	// create GC
	if ( reinit ) {
	    QFont  defaultFont(TRUE);		// default drawing tools
	    QPen   defaultPen;
	    QBrush defaultBrush;
	    cfont  = defaultFont;		// set these drawing tools
	    cpen   = defaultPen;
	    cbrush = defaultBrush;
	}
    }
    else
	sw = sh = tw = th = 1;
    if ( gc ) {					// widget or pixmap
	XSetBackground( dpy, gc, bg_col.pixel() );
	bg_mode = TransparentMode;
	rop = CopyROP;
    }
    else {					// external devices
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
    if ( gc_brush ) {				// restore brush gc
	if ( testf(ClipOn) )
	    XSetClipMask( dpy, gc_brush, None );
	if ( rop != CopyROP )
	    XSetFunction( dpy, gc_brush, GXcopy );
	free_painter_gc( dpy, gc_brush );
    }
    if ( gc ) {					// restore pen gc
	if ( testf(ClipOn) )
	    XSetClipMask( dpy, gc, None );
	if ( rop != CopyROP )
	    XSetFunction( dpy, gc, GXcopy );
	free_painter_gc( dpy, gc );
    }
    clearf( IsActive );
    pdev->devFlags &= ~PDF_PAINTACTIVE;
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
	wxmat = m;				// set new matrix
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


static inline int d2i_round( double d )
{
    return d > 0 ? int(d+0.5) : int(d-0.5);
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
    wm11 = d2i_round((double)m.m11()*65536.0);
    wm12 = d2i_round((double)m.m12()*65536.0);
    wm21 = d2i_round((double)m.m21()*65536.0);
    wm22 = d2i_round((double)m.m22()*65536.0);
    wdx  = d2i_round((double)m.dx() *65536.0);
    wdy  = d2i_round((double)m.dy() *65536.0);
    bool invertible;
    m = m.invert( &invertible );		// invert matrix
    im11 = d2i_round((double)m.m11()*65536.0);
    im12 = d2i_round((double)m.m12()*65536.0);
    im21 = d2i_round((double)m.m21()*65536.0);
    im22 = d2i_round((double)m.m22()*65536.0);
    idx  = d2i_round((double)m.dx() *65536.0);
    idy  = d2i_round((double)m.dy() *65536.0);
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
    { x = wm11*x+wdx;						\
      y = wm22*y+wdy;						\
      w = wm11*w;						\
      h = wm22*h;						\
      x += x>0 ? 32768 : -32768;				\
      y += y>0 ? 32768 : -32768;				\
      w += w>0 ? 32768 : -32768;				\
      h += h>0 ? 32768 : -32768;				\
      x/=65536; y/=65536; w/=65536; h/=65536; }


QPoint QPainter::xForm( const QPoint &pv ) const
{						// map point, virtual -> device
    QPoint p = pv;
    if ( testf(WxF) ) {				// world xform
	WXFORM_P( p.rx(), p.ry() );
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
	    if ( wm12 == 0 && wm21 == 0 ) {	// scaling+translation only
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
    if ( !isActive() || npoints < 2 || index < 0 )
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
    if ( !isActive() || npoints < 2 || index < 0 )
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
	if ( cpen.style() == NoPen ) {		// draw fake outline
	    XDrawLines( dpy, hd, gc_brush, (XPoint*)(pa->data()+index),
			npoints, CoordModeOrigin );
	    int x1, y1, x2, y2;			// connect last to first point
	    pa->point( index+npoints-1, &x1, &y1 );
	    pa->point( index, &x2, &y2 );
	    XDrawLine( dpy, hd, gc_brush, x1, y1, x2, y2 );
	}
    }
    if ( cpen.style() != NoPen ) {		// draw outline
	XDrawLines( dpy, hd, gc, (XPoint*)(pa->data()+index), npoints,
		    CoordModeOrigin );
	if ( pa->point(index) != a.point(index+npoints-1) ) {
	    int x1, y1, x2, y2;			// connect last to first point
	    pa->point( index+npoints-1, &x1, &y1 );
	    pa->point( index, &x2, &y2 );
	    XDrawLine( dpy, hd, gc, x1, y1, x2, y2 );
	}
    }
    if ( winding && !quickwxf )			// set to normal fill rule
	XSetFillRule( dpy, gc_brush, EvenOddRule );
}


#define BEZIER_CACHE

#if defined(BEZIER_CACHE)
struct QBezData {				// used for Bezier cache
    QPointArray controls;
    QPointArray points;
};

typedef declare(QListM,QBezData) QBezList;	// list of Bezier curves
#endif


void QPainter::drawBezier( const QPointArray &a, int index, int npoints )
{						// draw Bezier curve
#if defined(BEZIER_CACHE)
    static QBezList *bezlist = 0;
    if ( !bezlist ) {				// create Bezier cache
	bezlist = new QBezList;
	CHECK_PTR( bezlist );
	bezlist->setAutoDelete( TRUE );
    }
#endif
    if ( npoints < 0 )
	npoints = a.size() - index;
    if ( index + npoints > a.size() )
	npoints = a.size() - index;
    if ( !isActive() || npoints < 2 || index < 0 )
	return;
    QPointArray a2;
    if ( npoints == a.size() )
	a2 = a;
    else {
	if ( npoints != a.size() ) {
	    a2.resize( npoints );
	    for ( int i=0; i<npoints; i++ )
		a2.setPoint( i, a.point(index+i) );
	}
    }
    if ( testf(DirtyPen|ExtDev|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[1];
	    param[0].ptarr = (QPointArray*)&a2;
	    pdev->cmd( PDC_DRAWBEZIER, param );
	    return;
	}
	if ( testf(VxF|WxF) )
	    a2 = xForm( a2 );
    }
    if ( cpen.style() != NoPen ) {
#if defined(BEZIER_CACHE)
	int i;
	for ( i=0; i<bezlist->count(); i++ ) {
	    if ( bezlist->at(i)->controls == a2 ) {
		a2 = bezlist->at(i)->points;
		i = -1;
		break;
	    }
	}
	if ( i >= 0 ) {				// not found in bezlist
	    QBezData *bez = new QBezData;
	    CHECK_PTR( bez );
	    bez->controls = a2.copy();
	    a2 = a2.bezier();
	    bez->points   = a2;
	    if ( bezlist->count() > 13 )
		bezlist->removeLast();
	    bezlist->insert( bez );
	}
#else
	a2 = a2.bezier();
#endif
	XDrawLines( dpy, hd, gc, (XPoint*)a2.data(), a2.size(),
		    CoordModeOrigin);
    }
}


void QPainter::drawPixMap( int x, int y, const QPixMap &pixmap )
{						// draw pixmap
    if ( !isActive() )
	return;
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev) ) {
	    QPDevCmdParam param[3];
	    QRect  r(0,0,pixmap.width(),pixmap.height());
	    QPoint p(x,y);
	    param[0].rect   = &r;
	    param[1].point  = &p;
	    param[2].pixmap = &pixmap;
	    pdev->cmd( PDC_DRAWPIXMAP, param );
	    return;
	}
	if ( testf(WxF) )			// no scaling or rotation ;-(
	    WXFORM_P( x, y );
	if ( testf(VxF) )
	    VXFORM_P( x, y );
    }
    if ( pixmap.depth() == 1 ) {		// bitmap
	if ( testf(DirtyPen) )			// bitmap gets pen color
	    updatePen();
	bool do_clip = hasClipping();
	if ( bg_mode == TransparentMode ) {	// set up transparency clipping
	    XSetClipMask( dpy, gc, pixmap.handle() );
	    XSetClipOrigin( dpy, gc, x, y );
	}
	XCopyPlane( dpy, pixmap.handle(), hd, gc, 0, 0,
		    pixmap.width(), pixmap.height(), x, y, 1 );
	if ( bg_mode == TransparentMode ) {	// restore clipping
	    XSetClipOrigin( dpy, gc, 0, 0 );
	    if ( do_clip )
		XSetRegion( dpy, gc, crgn.handle() );
	    else
		XSetClipMask( dpy, gc, None );
	}
    }
    else
	XCopyArea( dpy, pixmap.handle(), hd, gc, 0, 0,
		   pixmap.width(), pixmap.height(), x, y );
}


//
// Generate a string that describes a transformed bitmap. This string is used
// to insert and find bitmaps in the global pixmap cache.
//
static QString gen_xbm_key(  const QWorldMatrix &m, const QFont &f,
			     const char *str, int len )
{
    QString s = str;
    s.truncate( len );
    QString k;
    QFontInfo fi( f );
    QString fd;
    if ( fi.rawMode() )
	fd.sprintf( "&%s", fi.family() );
    else
	fd.sprintf( "x%s_%i_%i_%i_%i_%i_%i_%i",
		    fi.family(), fi.pointSize(), fi.italic(), fi.weight(),
		    fi.underline(), fi.strikeOut(), fi.fixedPitch(),
		    fi.charSet() );
    k.resize( len + 100 + fd.length() );
    k.sprintf( "$qt$%s,%g,%g,%g,%g,%g,%g,%s", (char *)s,
	       m.m11(), m.m12(), m.m21(),m.m22(), m.dx(), m.dy(), (char *)fd );
    return k;
}


static QPixMap *get_text_bitmap( const QWorldMatrix &m, const QFont &f,
				 const char *str, int len )
{
    QString k = gen_xbm_key( m, f, str, len );
    return QPixMap::find( k );
}


static void ins_text_bitmap( const QWorldMatrix &m, const QFont &f,
			     const char *str, int len, QPixMap *pm )
{
    QString k = gen_xbm_key( m, f, str, len );
    if ( !QPixMap::insert(k,pm) )		// cannot insert pixmap
	delete pm;
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

    if ( cfont.dirty() || testf(DirtyFont) )
	updateFont();
    if ( testf(DirtyPen|ExtDev|VxF|WxF) ) {
	if ( testf(DirtyPen) )
	    updatePen();
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
	if ( testf(WxF) ) {			// draw transformed text
	    QFontMetrics fm(cfont);
	    QRect bbox = fm.boundingRect( str, len );
	    int w=bbox.width(), h=bbox.height();
	    int tx=-bbox.x(),  ty=-bbox.y();	// text position
	    QWorldMatrix eff_mat( wm11/65536.0, wm12/65536.0,
				  wm21/65536.0, wm22/65536.0,
				  wdx/65536.0,  wdy/65536.0 );
	    QPixMap *wx_bm = get_text_bitmap( eff_mat, cfont, str, len );
	    bool create_new_bm = wx_bm == 0;
	    QWorldMatrix mat( 1, 0, 0, 1, -eff_mat.dx(), -eff_mat.dy() );
	    mat = eff_mat * mat;
	    if ( create_new_bm ) {		// no such cached bitmap
		QPixMap bm( w, h, 1 );		// create bitmap
		bm.fill( color0 );
		QPainter paint;
		paint.begin( &bm );		// draw text in bitmap
		paint.setFont( cfont );
		paint.drawText( tx, ty, str, len );
		paint.end();
		wx_bm = bm.xForm( mat );	// transform bitmap
	    }
	    mat = QPixMap::trueMatrix( mat, w, h );
#if 1
	    WXFORM_P( x, y );
	    int dx, dy;
	    mat.map( tx, ty, &dx, &dy );	// compute position of bitmap
	    x -= dx;  y -= dy;
#else
            QWorldMatrix wm = mat * eff_mat;
	    wm.map( x, y, &x, &y );
#endif
	    if ( bg_mode == OpaqueMode ) {	// opaque fill
		QPointArray a(5);
		int m, n;
		mat.map(   0,   0, &m, &n );  a.setPoint( 0, m, n );
					      a.setPoint( 4, m, n );
		mat.map( w-1,   0, &m, &n );  a.setPoint( 1, m, n );
		mat.map( w-1, h-1, &m, &n );  a.setPoint( 2, m, n );
		mat.map(   0, h-1, &m, &n );  a.setPoint( 3, m, n );
		a.move( x, y );
		QBrush oldBrush = cbrush;
		setBrush( backgroundColor() );
		updateBrush();
		XFillPolygon( dpy, hd, gc_brush, (XPoint*)a.data(), 4,
			      Convex, CoordModeOrigin );
		XDrawLines( dpy, hd, gc_brush, (XPoint*)a.data(), 5,
			    CoordModeOrigin );
		setBrush( oldBrush );
	    }
	    bool do_clip = hasClipping();
	    QPixMap *draw_bm;
	    if ( do_clip ) {			// clipping enabled
		int ww = wx_bm->size().width();
		int hh = wx_bm->size().height();
		draw_bm = new QPixMap( ww, hh, 1 );
		draw_bm->fill( color0 );
		QPainter paint;
		paint.begin( draw_bm );
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
	    XCopyPlane( dpy, draw_bm->handle(), hd, gc, 0, 0,
			draw_bm->width(), draw_bm->height(), x, y, 1 );
	    XSetClipOrigin( dpy, gc, 0, 0 );
	    if ( do_clip ) {
		delete draw_bm;			// delete temporary bitmap
		XSetRegion( dpy, gc, crgn.handle() );
	    }
	    else				// restore clip mask
		XSetClipMask( dpy, gc, None );
	    if ( create_new_bm )
		ins_text_bitmap( eff_mat, cfont, str, len, wx_bm );
	    return;
	}
	if ( testf(VxF) )
	    VXFORM_P( x, y );
    }

    if ( cfont.underline() || cfont.strikeOut() ) {
	QFontMetrics fm( cfont );
	int lw = fm.lineWidth();
	int tw = fm.width( str, len );
	if ( cfont.underline() )	       	// draw underline effect
	    XFillRectangle( dpy, hd, gc, x, y+fm.underlinePos(),
			    tw, lw );
	if ( cfont.strikeOut() )       		// draw strikeout effect
	    XFillRectangle( dpy, hd, gc, x, y-fm.strikeOutPos(),
			    tw, lw );
    }
    if ( bg_mode == TransparentMode )
	XDrawString( dpy, hd, gc, x, y, str, len );
    else
	XDrawImageString( dpy, hd, gc, x, y, str, len );
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

    if ( cfont.dirty() || testf(DirtyFont) )
	updateFont();
    if ( testf(DirtyPen|ExtDev) ) {
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(ExtDev) ) {
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
    }
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }

    QFontMetrics fm( cfont );			// get font metrics

    ushort codearray[200];
    int	   codelen    = 200;
    bool   code_alloc = FALSE;
    ushort *codes     = codearray;
    ushort cc 	      = 0;			// character code

    if ( len > 150 ) {				// need to alloc code array
	codelen = len + len/2;
	codes = (ushort *)malloc( sizeof(ushort)*codelen );
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
#undef  MIN
#undef  MAX
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

    bool wordbreak  = (tf & WordBreak)  == WordBreak;
    bool expandtabs = (tf & ExpandTabs) == ExpandTabs;
    bool singleline = (tf & SingleLine) == SingleLine;
    bool showprefix = (tf & ShowPrefix) == ShowPrefix;

    int  spacewidth = CWIDTH( ' ' );		// width of space char

    nlines = 0;
    index  = 1;					// first index contains BEGLINE
    begline = breakindex = breakwidth = maxwidth = bcwidth = tabindex = 0;
    k = tw = 0;

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

	    else if ( *p == '\t' ) { 		// TAB character
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
		    cc = TABSTOP | MIN(tw+cw,MAXWIDTH);
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
		codes[begline] = BEGLINE | MIN(breakwidth,MAXWIDTH);
		maxwidth = MAX(maxwidth,breakwidth);
		begline = breakindex;
		nlines++;
		tw -= breakwidth + bcwidth;
		breakindex = tabindex = 0;
	    }
	}

	tw += cw;				// increment text width

	if ( cc == BEGLINE ) {
	    codes[begline] = BEGLINE | MIN(tw,MAXWIDTH);
	    maxwidth = MAX(maxwidth,tw);
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

    codes[begline] = BEGLINE | MIN(tw,MAXWIDTH);
    maxwidth = MAX(maxwidth,tw);
    nlines++;
    codes[index++] = 0;
    codelen = index;

    int     fascent  = fm.ascent();		// get font measurements
    int     fheight  = fm.height();
    QRegion save_rgn = crgn;			// save the current region
    int     xp, yp;
    char    p_array[200];
    bool    p_alloc;

    if ( len > 200 ) {
	p = new char[len];			// buffer for printable string
	CHECK_PTR( p );
	p_alloc = TRUE;
    }
    else {
	p = p_array;
	p_alloc = FALSE;
    }

    if ( nlines == 1 && maxwidth < w && h > fheight )
	tf |= DontClip;				// no need to clip

    if ( (tf & DontClip) == 0 )	{		// clip text
	QRect r( x, y, w, h );
	if ( testf(WxF) ) {			// world xform active
	    QPointArray a( r );			// complex region
	    a = xForm( a );
	    QRegion new_rgn( a );
	    if ( testf(ClipOn) )		// add to existing region
		new_rgn = new_rgn.intersect( crgn );
	    setClipRegion( new_rgn );
	}
	else {					// simple region
	    r = xForm( r );
	    XRectangle xr;
	    xr.x = r.x();
	    xr.y = r.y();
	    xr.width = r.width();
	    xr.height = r.height();
	    if ( testf(ClipOn) ) {		// clipping active
		Region rgn = XCreateRegion();
		XUnionRectWithRegion( &xr, rgn, rgn );
		XIntersectRegion( rgn, crgn.handle(), rgn );
		XSetRegion( dpy, gc, rgn );
		XDestroyRegion( rgn );		// no longer needed
	    }
	    else				// slightly faster
		XSetClipRectangles( dpy, gc, 0, 0, &xr, 1, YXBanded );
	}
    }

    QPixMap  *pm;
    QPainter *pp;

    if ( (tf & GrayText) == GrayText ) {	// prepare to draw gray text
	pm = new QPixMap( w, fheight );
	CHECK_PTR( pm );
	pp = new QPainter;
	CHECK_PTR( pp );
	pp->begin( pm );
	pp->setBackgroundColor( bg_col );
	pp->setFont( cfont );
	pp->setPen( cpen.color() );
	pp->updatePen();
	pp->setBrush( QBrush(bg_col, Pix1Pattern) );
	pp->updateBrush();
    }
    else {
	pm = 0;
	pp = 0;
    }

    if ( (tf & AlignVCenter) == AlignVCenter )	// vertically centered text
	yp = h/2 - nlines*fheight/2;
    else if ( (tf & AlignBottom) == AlignBottom)// bottom aligned
	yp = h - nlines*fheight;
    else					// top aligned
	yp = 0;
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
	else if ( (tf & AlignCenter) == AlignCenter )
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
				  cpen.color());
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
	    drawPixMap( x, y+yp-fascent, *pm );
	}

	yp += fheight;
    }

    if ( pp ) {					// gray text
	pp->end();
	delete pp;
	delete pm;
    }

    if ( (tf & DontClip) == 0 ) {		// restore clipping
	if ( testf(ClipOn) )			// set original region
	    XSetRegion( dpy, gc, crgn.handle() );
	else
	    XSetClipMask( dpy, gc, None );
	if ( save_rgn.handle() != crgn.handle() )
	    setClipRegion( save_rgn );
    }

    if ( p_alloc )
	delete p;
    if ( code_alloc )
	free( (void *)codes );
}


QRect QPainter::calcRect( int x, int y, int w, int h, int tf,
			  const char *str, int len )
{
    if ( len < 0 )
	len = strlen( str );
    QFontMetrics fm( cfont );
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
