/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter_x11.cpp#92 $
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
#include "qpmcache.h"
#include "qlist.h"
#include "qintdict.h"
#include <ctype.h>
#include <malloc.h>
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpainter_x11.cpp#92 $";
#endif


#define CHANGE_RESET  0				// QPainter::changed.. commands
#define CHANGE_ALL    1
#define CHANGE_COLOR  2

// --------------------------------------------------------------------------
// QPen member functions
//

/*!
\class QPen qpen.h
\brief The QPen class defines how the QPainter should draw lines and outlines
of shapes.

A pen has a style, a width and a color.

The pen style defines the line type. The default pen style is \c SolidPen.
Setting the style to \c NoPen tells the painter to not draw lines or outlines.

The pen width defines the line width. The default line width is 0, which
draws a 1-pixel line very fast, but not so accurate.  Setting the line
width to 1 or more draws lines that are precise, but drawing is slower.

The line color defines the color of lines and text. The default line
color is black.  The QColor documentation contains a list of standard colors.

Use the QBrush class for specifying fill styles.

Example of how to use a pen:
\code
  QPainter painter;
  QPen     pen( red );			\/ red solid line, 0 width
  painter.begin( &anyPaintDevice );	\/ paint widget, pixmap etc.
  painter.setPen( pen );		\/ sets the red pen
  painter.drawRect( 40,30, 200,100 );	\/ draw 200x100 rect at (40,30)
  painter.pen().setColor( blue );	\/ change pen color to blue
  painter.drawLine( 40,30, 240,130 );	\/ draw diagonal in rectangle
  painter.end();			\/ painting done
\endcode

The setStyle() function has a list of pen styles.
*/


void QPen::init( const QColor &color, uint width, PenStyle style )
{
    data = new QPenData;
    CHECK_PTR( data );
    data->style = style;
    data->width = width;
    data->color = color;
}

/*!
Constructs a default black solid line pen with 0 width.
*/

QPen::QPen()
{
    init( black, 0, SolidLine );		// default pen
}

/*!
Constructs a  pen black with 0 width and a specified style.

\sa setStyle().
*/

QPen::QPen( PenStyle style )
{
    init( black, 0, style );
}

/*!
Constructs a pen with a specified color, width and style.
*/

QPen::QPen( const QColor &color, uint width, PenStyle style )
{
    init( color, width, style );
}

/*!
Constructs a pen which is a shallow copy of \e p.
*/

QPen::QPen( const QPen &p )
{
    data = p.data;
    data->ref();
}

/*!
Destroys the pen.
*/

QPen::~QPen()
{
    if ( data->deref() )
	delete data;
}


/*!
Detaches from shared pen data to makes sure that this pen is the only
one referring the data.

If multiple pens share common data, this pen dereferences the
data and gets a copy of the data. Nothing will be done if there is just
a single reference.
*/

void QPen::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*!
Assigns a shallow copy of \e p to the pen and returns a reference to this
pen.
*/

QPen &QPen::operator=( const QPen &p )
{
    p.data->ref();				// beware of p = p
    if ( data->deref() )
	delete data;
    data = p.data;
    return *this;
}


/*!
Returns a deep copy of the pen.
*/

QPen QPen::copy() const
{
    QPen p( data->color, data->width, data->style );
    return p;
}


/*!
\fn PenStyle QPen::style() const
Returns the pen style.

\sa setStyle().
*/

/*!
\fn uint QPen::width() const
Returns the pen width.

\sa setWidth().
*/

/*!
\fn QColor QPen::color() const
Returns the pen color.

\sa setColor().
*/


/*!
Sets the pen style to \e s.

The pen styles are:
<dl compact>
<dt> NoPen <dd> no outline will be drawn.
<dt> SolidLine <dd> solid line (default).
<dt> DashLine <dd> - - - (dashes) line.
<dt> DotLine <dd> * * * (dots) line.
<dt> DashDotLine <dd> - * - * line.
<dt> DashDotDotLine <dd> -- * -- * line.
</dl>

\sa style().
*/

void QPen::setStyle( PenStyle s )		// set pen style
{
    if ( data->style == s )
	return;
    detach();
    data->style = s;
    QPainter::changedPen( this, CHANGE_ALL );
}

/*!
Sets the pen width to \e w.

\sa width().
*/

void QPen::setWidth( uint w )			// set pen width
{
    if ( data->width == w )
	return;
    detach();
    data->width = w;
    QPainter::changedPen( this, CHANGE_ALL );
}

/*!
Sets the pen color to \e c.

\sa color().
*/

void QPen::setColor( const QColor &c )		// set pen color
{
    ulong pixel = data->color.pixel();
    detach();
    data->color = c;
    if ( pixel != data->color.pixel() )
	QPainter::changedPen( this, CHANGE_COLOR );
}


/*!
\fn bool QPen::operator!=( const QPen &p ) const
Returns TRUE if the pen is different from \e p, or FALSE if the pens are
equal.

Two pens are different if they have different styles, widths and colors.
*/

/*!
Returns TRUE if the pen is equal to \e p, or FALSE if the pens are
different.

Two pens are equal if they have equal styles, widths and colors.
*/

bool QPen::operator==( const QPen &p ) const
{
    return (p.data == data) || (p.data->style == data->style &&
	    p.data->width == data->width && p.data->color == data->color);
}


// --------------------------------------------------------------------------
// QBrush member functions
//

/*!
\class QBrush qbrush.h
\brief The QBrush class defines the fill pattern of shapes drawn using the
QPainter.

A brush has a style and a color.  One of the brush styles is a custom
pattern, which is defined by a QBitmap.

The brush style defines the fill pattern. The default brush style is \c
NoBrush (depends on how you construct a brush).  This style tells the
painter to not fill shapes. The standard style for filling is called \c
SolidPattern.

The brush color defines the color of the fill pattern.
The QColor documentation contains a list of standard colors.

Use the QPen class for specifying line/outline styles.

Example of how to use a brush:
\code
  QPainter painter;
  QBrush   brush( yellow );		\/ yellow solid pattern
  QPen     pen( red );			\/ red solid line, 0 width
  painter.begin( &anyPaintDevice );	\/ paint widget, pixmap etc.
  painter.setBrush( brush );		\/ sets the yellow brush
  painter.setPen( pen );		\/ sets the red pen
  painter.drawRect( 40,30, 200,100 );	\/ draw 200x100 rect at (40,30)
  painter.brush().setStyle( NoBrush );	\/ do not fill
  painter.drawrect( 10,10, 30,20 );	\/ draw rectangle outline
  painter.end();			\/ painting done
\endcode

The setStyle() function has a list of brush styles.
*/


void QBrush::init( const QColor &color, BrushStyle style )
{
    data = new QBrushData;
    CHECK_PTR( data );
    data->style  = style;
    data->color  = color;
    data->dpy	 = 0;
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


/*!
Constructs a default black brush with the style \c NoBrush (will not fill
shapes).
*/

QBrush::QBrush()
{
    init( black, NoBrush );
}

/*!
Constructs a black brush with the specified style.
*/

QBrush::QBrush( BrushStyle style )
{
    init( black, style );
}

/*!
Constructs a brush with a specified color and style.
*/

QBrush::QBrush( const QColor &color, BrushStyle style )
{
    init( color, style );
}

/*!
Constructs a brush with a specified color and a custom pattern.
*/

QBrush::QBrush( const QColor &color, const QBitmap &bitmap )
{
    init( color, CustomPattern );
    data->bitmap = new QBitmap( bitmap );
}

/*!
Constructs a brush which is a shallow copy of \e b.
*/

QBrush::QBrush( const QBrush &b )
{
    data = b.data;
    data->ref();
}

/*!
Destroys the brush.
*/

QBrush::~QBrush()
{
    if ( data->deref() )
	reset();
}


/*!
Detaches from shared brush data to makes sure that this brush is the only
one referring the data.

If multiple brushes share common data, this pen dereferences the
data and gets a copy of the data. Nothing will be done if there is just
a single reference.
*/

void QBrush::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*!
Assigns a shallow copy of \e b to the brush and returns a reference to
this brush.
*/

QBrush &QBrush::operator=( const QBrush &b )
{
    b.data->ref();				// beware of b = b
    if ( data->deref() )
	reset();
    data = b.data;
    return *this;
}


/*!
Returns a deep copy of the brush.
*/

QBrush QBrush::copy() const
{
    if ( data->style == CustomPattern ) {	// brush has bitmap
	QBrush b( data->color, *data->bitmap );
	return b;
    }
    else {					// brush has std pattern
	QBrush b( data->color, data->style );
	return b;
    }
}


/*!
\fn BrushStyle QBrush::style() const
Returns the brush style.

\sa setStyle().
*/

/*!
\fn QColor QBrush::color() const
Returns the brush color.

\sa setColor().
*/

/*!
\fn QBitmap *QBrush::bitmap() const
Returns a pointer to the custom brush pattern.

A null pointer is returned if no custom brush pattern has been set.

\sa setBitmap().
*/


/*!
Sets the brush style to \e s.

The brush styles are:
<dl compact>
<dt> NoBrush <dd> will not fill shapes (default).
<dt> SolidPattern <dd> solid (100%) fill pattern.
<dt> Dense1Pattern <dd> 94% fill pattern.
<dt> Dense2Pattern <dd> 88% fill pattern.
<dt> Dense3Pattern <dd> 63% fill pattern.
<dt> Dense4Pattern <dd> 50% fill pattern.
<dt> Dense5Pattern <dd> 37% fill pattern.
<dt> Dense6Pattern <dd> 12% fill pattern.
<dt> Dense7Pattern <dd> 6% fill pattern.
<dt> HorPattern <dd> horizontal lines pattern.
<dt> VerPattern <dd> vertical lines pattern.
<dt> CrossPattern <dd> crossing lines pattern.
<dt> BDiagPattern <dd> diagonal lines (directed / ) pattern.
<dt> FDiagPattern <dd> diagonal lines (directed \ ) pattern.
<dt> DiagCrossPattern <dd> diagonal crossing lines pattern.
<dt> CustomPattern <dd> internal: set when a bitmap pattern is being used.
</dl>

\sa style().
*/

void QBrush::setStyle( BrushStyle s )		// set brush style
{
    if ( data->style == s )
	return;
#if defined(CHECK_RANGE)
    if ( s == CustomPattern )
	warning( "QBrush::setStyle: CustomPattern is for internal use" );
#endif
    detach();
    data->style = s;
    QPainter::changedBrush( this, CHANGE_ALL );
}

/*!
Sets the brush color to \e c.

\sa color().
*/

void QBrush::setColor( const QColor &c )	// set brush color
{
    ulong pixel = data->color.pixel();
    detach();
    data->color = c;
    if ( pixel != data->color.pixel() )
	QPainter::changedBrush( this, CHANGE_COLOR );
}

/*!
Sets the brush bitmap.  The style is set to \c CustomPattern.

\sa bitmap().
*/

void QBrush::setBitmap( const QBitmap &bitmap )	// set brush bitmap
{
    detach();
    data->style = CustomPattern;
    if ( data->bitmap )
	delete data->bitmap;
    data->bitmap = new QBitmap( bitmap );
    QPainter::changedBrush( this, CHANGE_ALL );
}


/*!
\fn bool QBrush::operator!=( const QBrush &b ) const
Returns TRUE if the brush is different from \e b, or FALSE if the brushes are
equal.

Two brushes are different if they have different styles, colors or bitmaps.
*/

/*!
Returns TRUE if the brush is equal to \e b, or FALSE if the brushes are
different.

Two brushes are equal if they have equal styles, colors and bitmaps.
*/

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

#if defined(_OS_LINUX_) && defined(_CC_GNU_)

inline double qcos( double a ) {
    double r;
    __asm__ (
	"fcos"
	: "=t" (r) : "0" (a) );
    return(r);
}

inline double qsin( double a ) {
    double r;
    __asm__ (
	"fsin"
	: "=t" (r) : "0" (a) );
    return(r);
}

double qsincos( double a, bool calcCos=FALSE ) {
    return calcCos ? qcos(a) : qsin(a);
}

#else

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

#endif


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
    cleanup_painter_gc( qt_xdisplay() );
}


typedef declare(QIntDictM,QPaintDevice) QPaintDeviceDict;
static QPaintDeviceDict *pdev_dict = 0;

/*!
Redirects all paint command for a paint device \e pdev to another paint
device \e replacement.

A redirected paint device is reset if \e replacement is 0.

The following example draws a widget in a pixmap:
\code
  QPixmap pm( someWidget->width(), someWidget->height() );
  pm.fill( someWidget->backgroundColor() );
  QPainter::redirect( someWidget, &pm );
  someWidget->repaint();
  QPainter::redirect( someWidget, 0 );
\endcode
*/

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


/*!
Constructs a painter.
*/

QPainter::QPainter()
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

/*!
Destroys the painter.
*/

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

/*!
Returns the current painter font.
*/

QFont &QPainter::font()
{
    return cfont;
}

/*!
Sets a new painter font.
*/

void QPainter::setFont( const QFont &font )	// set current font
{
    if ( cfont.d != font.d ) {
	setf( DirtyFont );
	cfont = font;
    }
}

/*!
Sets a new painter pen.

The pen defines how to draw lines and outlines, and it also determines
the text color.
*/

void QPainter::setPen( const QPen &pen )	// set current pen
{
    if ( cpen.data != pen.data ) {
	if ( cpen != pen )
	    setf( DirtyPen );
	cpen = pen;
    }
}

/*!
Sets a new painter pen with black color, width 0 and the specified \e style.

The pen defines how to draw lines and outlines, and it also determines
the text color.
*/

void QPainter::setPen( PenStyle style )		// set solid pen with color
{
    QPen pen( style );
    setPen( pen );
}

/*!
Sets a new painter pen with style \c SolidLine, width 0 the specified \e color.

The pen defines how to draw lines and outlines, and it also determines
the text color.
*/

void QPainter::setPen( const QColor &color )	// set solid pen with color
{
    QPen pen( color );
    cpen = pen;
    if ( isActive() && gc && cpen.style() == SolidLine && cpen.width() == 0 )
	XSetForeground( dpy, gc, cpen.color().pixel() );
    else if ( cpen != pen )
	setf( DirtyPen );
}

/*!
Sets a new painter brush.

The brush defines how to fill shapes.
*/

void QPainter::setBrush( const QBrush &brush )	// set current brush
{
    if ( cbrush.data != brush.data ) {
	if ( cbrush != brush )
	    setf( DirtyBrush );
	cbrush = brush;
	cbrush.data->dpy = dpy;			// give brush a display pointer
    }
}

/*!
Sets a new painter brush with black color and the specified \e style.

The brush defines how to fill shapes.
*/

void QPainter::setBrush( BrushStyle style )	// set brush
{
    QBrush brush( style );
    setBrush( brush );
}

/*!
Sets a new painter brush with the style \c SolidPattern and the specified
\e color.

The brush defines how to fill shapes.
*/

void QPainter::setBrush( const QColor &color )	// set solid brush width color
{
    QBrush brush( color );
    cbrush = brush;
    cbrush.data->dpy = dpy;
    if ( isActive() && gc_brush && cbrush.style() == SolidPattern )
	XSetForeground( dpy, gc_brush, cbrush.color().pixel() );
    else if ( cbrush != brush )
	setf( DirtyBrush );
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
static uchar dense1_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
static uchar dense2_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
static uchar dense3_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
static uchar dense4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
static uchar dense5_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
static uchar dense6_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
static uchar dense7_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
static uchar hor_pat[] = {			// horizontal pattern
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uchar ver_pat[] = {			// vertical pattern
    0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
    0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
    0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
    0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
    0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
    0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
static uchar cross_pat[] = {			// cross pattern
    0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
    0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
    0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
    0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
    0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
    0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
static uchar bdiag_pat[] = {			// backward diagonal pattern
    0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01,
    0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
    0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40 };
static uchar fdiag_pat[] = {			// forward diagonal pattern
    0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
    0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
    0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01 };
static uchar dcross_pat[] = {			// diagonal cross pattern
    0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41,
    0x80, 0x80, 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
    0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41 };
static uchar *pat_tbl[] = {
    dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat,
    dense6_pat, dense7_pat,
    hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };

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
    if ( cbrush.style()>=Dense1Pattern && cbrush.style()<=DiagCrossPattern ) {
	pat = (char *)pat_tbl[ cbrush.style()-Dense1Pattern ];
	if ( cbrush.style() <= Dense7Pattern )
	    sz = 8;
	else if ( cbrush.style() <= CrossPattern )
	    sz = 24;
	else
	    sz = 16;
    }
    else
	s = FillSolid;
    if ( !gc_brush ) {				// brush not yet created
	bool mono = pdev->devType() == PDT_PIXMAP &&
	            ((QPixmap*)pdev)->depth() == 1;
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

/*!
Begins painting the paint device \e pd and returns TRUE if successful,
or FALSE if it cannot begin painting.

\warning
This function resets the pen, brush, font and all other painter settings
to default values for the painter device.  That means any pen, brush etc.
you set before calling begin() will be overwritten.

\sa end().
*/

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
    wx = wy = vx = vy = 0;			// default view origins
    if ( pdev->devType() == PDT_WIDGET ) {	// device is a widget
	QWidget *w = (QWidget*)pdev;
	gc = alloc_painter_gc( dpy, w->handle() );
	cfont = w->font();			// use widget font
	bg_col = w->backgroundColor();		// use widget bg color
	ww = vw = w->width();			// default view size
	wh = vh = w->height();
	cpen = QPen( w->foregroundColor() );	// use widget fg color
	if ( reinit ) {
	    cbrush = QBrush( NoBrush );
	}
	if ( w->testFlag(WPaintUnclipped) ) {	// paint direct on device
	    updateBrush();
	    XSetSubwindowMode( w->display(), gc, IncludeInferiors );
	    XSetSubwindowMode( w->display(), gc_brush, IncludeInferiors );
	}
    }
    else if ( pdev->devType() == PDT_PIXMAP ) {	// device is a pixmap
	QPixmap *pm = (QPixmap*)pdev;
	if ( pm->isNull() ) {
#if defined(CHECK_NULL)
	    warning( "QPainter::begin: Cannot paint null pixmap" );
#endif
	    end();
	    return FALSE;
	}
	bool mono = pm->depth() == 1;		// monochrome bitmap
	gc = alloc_painter_gc( dpy, hd, mono );	// create GC
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
	if ( mono ) {
	    bg_col = color0;
	    cpen.setColor( color1 );		// changes gc directly!
	}
    }
    else
	ww = wh = vw = vh = 1;
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

/*!
End painting.  Any resources used while painting are released.
\sa begin().
*/

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

/*!
Sets the background  color of the painter to \e c.
\sa backgroundColor().
*/

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

/*!
Sets the background mode of the painter to \e m, which must be either

The \e m parameter must be one of:
<ul>
<li> \c TransparentMode
<li> \c OpaqueMode
</ul>
\sa backgroundMode().
*/

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

/*!
Sets the raster operation to \e r.

The \e r parameter must be one of:
<ul>
<li> \c CopyROP:     dst = src.
<li> \c OrROP:       dst = dst OR src.
<li> \c XorROP:      dst = dst XOR src.
<li> \c EraseROP:    dst = (NOT src) AND dst
<li> \c NotCopyROP:  dst = NOT src
<li> \c NotOrROP:    dst = (NOT src) OR dst
<li> \c NotXorROP:   dst = (NOT src) XOR dst
<li> \c NotEraseROP: dst = src AND dst
<li> \c NotROP:      dst = NOT dst
</ul>
\sa rasterOp().
*/

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

/*!
Sets the brush origin to \e (x,y).
*/

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

/*!
Enables or disables view transformations.
*/

void QPainter::setViewXForm( bool enable )	// set xform
{
    if ( !isActive() || enable == testf(VxF) )
	return;
    setf( VxF, enable );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = enable;
	pdev->cmd( PDC_SETVXFORM, param );
    }
    if ( testf(WxF) )
	updateXForm();
}

/*!
Returns the window rectangle.
\sa setWindow().
*/

QRect QPainter::window() const			// get window
{
    return QRect( wx, wy, ww, wh );
}

/*!
Sets the window rectangle view transformation for the painter and
enables view transformation.

The window rectangle is part of the view transformation.
View transformations can be combined with world transformations.
\sa window(), setViewport() and setWorldMatrix().
*/

void QPainter::setWindow( int x, int y, int w, int h )
{						// set window
    if ( !isActive() )
	return;
    wx = x;
    wy = y;
    ww = w;
    wh = h;
    if ( testf(ExtDev) ) {
	QRect r( x, y, w, h );
	QPDevCmdParam param[1];
	param[0].rect = (QRect*)&r;
	pdev->cmd( PDC_SETWINDOW, param );
	return;
    }
    if ( !testf(VxF) )
	setViewXForm( TRUE );
}

/*!
Returns the viewport rectangle.
\sa setViewport().
*/

QRect QPainter::viewport() const		// get viewport
{
    return QRect( vx, vy, vw, vh );
}

/*!
Sets the viewport rectangle view transformation for the painter and
enables view transformation.

The viewport rectangle is part of the view transformation.
View transformations can be combined with world transformations.
\sa viewport(), setViewport() and setWorldMatrix().
*/

void QPainter::setViewport( int x, int y, int w, int h )
{						// set viewport
    vx = x;
    vy = y;
    vw = w;
    vh = h;
    if ( testf(ExtDev) ) {
	QRect r( x, y, w, h );
	QPDevCmdParam param[1];
	param[0].rect = (QRect*)&r;
	pdev->cmd( PDC_SETVIEWPORT, param );
	return;
    }
    if ( !testf(VxF) )
	setViewXForm( TRUE );
}

/*!
Enables or disables world transformations.
*/

void QPainter::setWorldXForm( bool enable )	// set world transform
{
    if ( !isActive() || enable == testf(WxF) )
	return;
    setf( WxF, enable );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = enable;
	pdev->cmd( PDC_SETWXFORM, param );
    }
    updateXForm();
}

/*!
Returns the world transformation matrix.
*/

Q2DMatrix QPainter::worldMatrix() const		// get world xform matrix
{
    return wxmat;
}

/*!
Sets the world transformation matrix to \e m.

If \e combine is TRUE, then \e m is combined with the
current transformation matrix.

\sa worldMatrix(), setWindow() and setViewport().
*/

void QPainter::setWorldMatrix( const Q2DMatrix &m, bool combine )
{						// set world xform matrix
    if ( combine )
	wxmat = m * wxmat;			// combines
    else
	wxmat = m;				// set new matrix
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	param[0].matrix = &wxmat;
	param[1].ival = combine;
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
    Q2DMatrix m;
    if ( testf(VxF) ) {
	m.translate( vx, vy );
	m.scale( 1.0*vw/ww, 1.0*vh/wh );
	m.translate( -wx, -wy );
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


/*!
Returns the point \e pv transformed from user coordinates to device
coordinates.
*/

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

/*!
Returns the rectangle \e rv transformed from user coordinates to device
coordinates.

If world transformation is enabled and rotation or shearing is used,
then the bounding rectangle will be returned.
*/

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
	w = (vw*w)/ww;
	h = (vh*h)/wh;
    }
    return QRect( x, y, w, h );
}

/*!
Returns the point array \e av transformed from user coordinates to device
coordinates.
*/

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

/*!
Returns the point \e pv transformed from device coordinates to user
coordinates.
*/

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
	p.rx() = (ww*(p.x()-vx))/vw + wx;
	p.ry() = (wh*(p.y()-vy))/vh + wy;
    }
    return p;
}

/*!
Returns the rectangle \e rv transformed from device coordinates to user
coordinates.

If world transformation is enabled and rotation or shearing is used,
then the bounding rectangle will be returned.
*/

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
	x = (ww*(x-vx))/vw + wx;
	y = (wh*(y-vy))/vh + wy;
	w = (ww*w)/vw;
	h = (wh*h)/vh;
    }
    return QRect( x, y, w, h );
}

/*!
Returns the point array \e av transformed from device coordinates to user
coordinates.
*/

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
	    x = (ww*(x-vx))/vw + wx;
	    y = (wh*(y-vy))/vh + wy;
	}
	a.setPoint( i, x, y );
    }
    return a;
}


/*!
Enables clipping if \e enable is TRUE, or disables clipping if \e enable
is FALSE.
*/

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


/*!
Synonymous to setClipRect(x,y,w,h).
*/

void QPainter::setClipRect( const QRect &rect )	// set clip rectangle
{
    QRegion rgn( rect );
    setClipRegion( rgn );
}

/*!
Clips graphics to the region \e rgn.

Clipping is automatically enabled.

\sa setClipRect().
*/

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

/*!
Draws/plots a single point at \e (x,y) using the current pen.
*/

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

/*!
Sets the current point.
\sa lineTo().
*/

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

/*!
Draws a line from the current point to \e (x,y) and sets this to the new
current point.
\sa moveTo().
*/ 

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

/*!
Draws a line from \e (x1,y2) to \e (x2,y2).
*/

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

/*!
Draws a rectangle with upper left corner at \e (x,y) and with
width \e w and height \e h.

The width and height include both lines.
*/

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

/*!
Draws a rectangle with round corners at \e (x,y), with width \e w
and height \e h.

The \e xRnd and \e yRnd arguments indicate how rounded the corners
should be.  0 is angled corners, 99 is maximum roundedness.

The width and height include both lines.
*/

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

/*!
Draws an ellipse with center at \e (x+w/2,y+h/2) and size \e
(w,h).
*/

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


/*!
Draws an arc defined by the rectangle \e (x,y,w,h) and the start
angle \e a1 and the end angle \e a2.
*/

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


/*!
Draws a pie defined by the rectangle \e (x,y,w,h) and the start
angle \e a1 and the end angle \e a2.
*/

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


/*!
Draws a chord defined by the rectangle \e (x,y,w,h) and the start
angle \e a1 and the end angle \e a2.
*/

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


/*!
Draws \e nlines separate lines from points defined in \e a, starting at
a[\e index].

Draws the 1st line from \e a[index] to \e a[index+1].
Draws the 2nd line from \e a[index+2] to \e a[index+3] etc.
*/

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


/*!
Draws the polyline defined by the \e npoints points in \e a starting at
\e a[index].
*/

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


/*!
Draws the polygon defined by the \e npoints points in \e a starting at
\e a[index].

The first point is connected to the last point.
*/

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


/*!
Draws a Bezier curve defined by the \e npoints control points in \e a, starting
at \e a[index].
*/

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


/*!
Draws a pixmap at \e (x,y).
*/

void QPainter::drawPixmap( int x, int y, const QPixmap &pixmap,
			   int sx, int sy, int sw, int sh )
{						// draw pixmap
    if ( !isActive() || pixmap.isNull() )
	return;
    if ( sw < 0 )
	sw = pixmap.width();
    if ( sh < 0 )
	sh = pixmap.height();
    if ( testf(ExtDev|VxF|WxF) ) {
	if ( testf(ExtDev|WxF) ) {
	    if ( sx != 0 || sy != 0 ||
		 sw != pixmap.width() || sh != pixmap.height() ) {
		QPixmap tmp( sw, sh, pixmap.depth() );
		bitBlt( &tmp, 0, 0, &pixmap, sx, sy, sw, sh );
		drawPixmap( x, y, tmp );
		return;
	    }
	    if ( testf(ExtDev) ) {
		QPDevCmdParam param[3];
		QRect  r(0,0,sw,sh);
		QPoint p(x,y);
		param[0].rect   = &r;
		param[1].point  = &p;
		param[2].pixmap = &pixmap;
		pdev->cmd( PDC_DRAWPIXMAP, param );
		return;
	    }
							// world transform
	    Q2DMatrix mat( wm11/65536.0, wm12/65536.0,
			   wm21/65536.0, wm22/65536.0,
			   wdx/65536.0,  wdy/65536.0 );
	    mat = QPixmap::trueMatrix( mat, sw, sh );
	    QPixmap bm_clip( sw, sh, 1 );
	    bm_clip.fill( color1 );
	    QPixmap pm = pixmap.xForm( mat );
	    QPixmap bm = bm_clip.xForm( mat );
	    WXFORM_P( x, y );
	    int dx, dy;
	    mat.map( 0, 0, &dx, &dy );		// compute position of pixmap
	    x -= dx;  y -= dy;
	    bool do_clip = hasClipping();
	    QPixmap *draw_pm;
	    if ( do_clip ) {
		draw_pm = new QPixmap( sw, sh, pm.depth() );
		QPainter paint;
		paint.begin( draw_pm );
		QRegion rgn = crgn.copy();
		rgn.move( -x, -y );
		paint.setClipRegion( rgn );
		paint.drawPixmap( 0, 0, pm );
		paint.end();
	    }
	    else
		draw_pm = &pm;
	    XSetClipMask( dpy, gc, bm.handle() );
	    XSetClipOrigin( dpy, gc, x, y );
	    ushort save_flags = flags;
	    flags = IsActive;
	    drawPixmap( x, y, pm );
	    flags = save_flags;
	    XSetClipOrigin( dpy, gc, 0, 0 );
	    if ( do_clip ) {
		delete draw_pm;
		XSetRegion( dpy, gc, crgn.handle() );
	    }
	    else
		XSetClipMask( dpy, gc, None );
	    return;
	}
	if ( testf(VxF) )
	    VXFORM_P( x, y );
    }
    if ( pixmap.depth() == 1 ) {		// bitmap
	if ( testf(DirtyPen) )			// bitmap gets pen color
	    updatePen();
	bool do_clip = hasClipping();
	if ( bg_mode == TransparentMode ) {	// set up transparency clipping
	    XSetClipMask( dpy, gc, pixmap.handle() );
	    XSetClipOrigin( dpy, gc, x-sx, y-sy );
	}
	XCopyPlane( dpy, pixmap.handle(), hd, gc, sx, sy,
		    sw, sh, x, y, 1 );
	if ( bg_mode == TransparentMode ) {	// restore clipping
	    XSetClipOrigin( dpy, gc, 0, 0 );
	    if ( do_clip )
		XSetRegion( dpy, gc, crgn.handle() );
	    else
		XSetClipMask( dpy, gc, None );
	}
    }
    else
	XCopyArea( dpy, pixmap.handle(), hd, gc, sx, sy,
		   sw, sh, x, y );
}


//
// Generate a string that describes a transformed bitmap. This string is used
// to insert and find bitmaps in the global pixmap cache.
//
static QString gen_xbm_key(  const Q2DMatrix &m, const QFont &f,
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


static QPixmap *get_text_bitmap( const Q2DMatrix &m, const QFont &f,
				 const char *str, int len )
{
    QString k = gen_xbm_key( m, f, str, len );
    return QPixmapCache::find( k );
}


static void ins_text_bitmap( const Q2DMatrix &m, const QFont &f,
			     const char *str, int len, QPixmap *pm )
{
    QString k = gen_xbm_key( m, f, str, len );
    if ( !QPixmapCache::insert(k,pm) )		// cannot insert pixmap
	delete pm;
}

/*!
Draws at most \e len characters from \e str at position \e (x,y).

\e (x,y) is the base line position.
*/

void QPainter::drawText( int x, int y, const char *str, int len )
{
    if ( !isActive() )
	return;
    if ( len < 0 )
	len = strlen( str );
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
	    Q2DMatrix mat1( wm11/65536.0, wm12/65536.0,
			   wm21/65536.0, wm22/65536.0,
			   wdx/65536.0,  wdy/65536.0 );
	    Q2DMatrix mat = QPixmap::trueMatrix( mat1, w, h );
	    QPixmap *wx_bm = get_text_bitmap( mat, cfont, str, len );
	    bool create_new_bm = wx_bm == 0;
	    if ( create_new_bm ) {		// no such cached bitmap
		QPixmap bm( w, h, 1 );		// create bitmap
		bm.fill( color0 );
		QPainter paint;
		paint.begin( &bm );		// draw text in bitmap
		paint.setFont( cfont );
		paint.drawText( tx, ty, str, len );
		paint.end();
		wx_bm = new QPixmap( bm.xForm( mat ) );	// transform bitmap
		if ( wx_bm->isNull() ) {
		    delete wx_bm;		// nothing to draw
		    return;
		}
	    }
	    float fx=x, fy=y, nfx, nfy;
	    mat1.map( fx,fy, &nfx,&nfy );
	    float tfx=tx, tfy=ty, dx, dy;
	    mat.map( tfx, tfy, &dx, &dy );	// compute position of bitmap
	    x = int(nfx-dx);  y = int(nfy-dy);
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
	    QPixmap *draw_bm;
	    if ( do_clip ) {			// clipping enabled
		int ww = wx_bm->size().width();
		int hh = wx_bm->size().height();
		draw_bm = new QPixmap( ww, hh, 1 );
		draw_bm->fill( color0 );
		QPainter paint;
		paint.begin( draw_bm );
		QRegion rgn = crgn.copy();
		rgn.move( -x, -y );
		paint.setClipRegion( rgn );
		paint.drawPixmap( 0, 0, *wx_bm );
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
		ins_text_bitmap( mat, cfont, str, len, wx_bm );
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


//
// The drawText function takes two special parameters; 'internal' and 'brect'.
//
// The 'internal' parameter contains a pointer to an array of encoded
// information that keeps internal geometry data.
// If the drawText function is called repeatedly to display the same text,
// it makes sense to calculate text width and linebreaks the first time,
// and use these parameters later to print the text because we save a lot of
// CPU time.
// The 'internal' parameter will not be used if it is a null pointer.
// The 'internal' parameter will be generated if it is not null, but points
// to a null pointer, i.e. internal != 0 && *internal == 0.
// The 'internal' parameter will be used if it contains a non-null pointer.
//
// If the 'brect parameter is a non-null pointer, then the bounding rectangle
// of the text will be returned in 'brect'.
//

/*!
Draws at most \e len characters from \e str in the rectangle \e (x,y,w,h).

This function draws formatted text.

The \e tf text formatting is the bitwise OR of the following flags:
<ul>
<li> \c AlignLeft aligns to the left border.
<li> \c AlignRight aligns to the right border.
<li> \c AlignHCenter aligns horizontally centered.
<li> \c AlignTop aligns to the top border.
<li> \c AlignBottom aligns to the bottom border.
<li> \c AlignVCenter aligns vertically centered
<li> \c AlignCenter (= \c AlignHCenter | AlignVCenter)
<li> \c SingleLine ignores newline characters in the text.
<li> \c DontClip never clips the text to the rectangle.
<li> \c ExpandTabs expands tabulators.
<li> \c ShowPrefix displays "&x" as "x" underlined.
<li> \c WordBreak breaks the text to fit the rectangle.
<li> \c GrayText grays out the text.
</ul>
*/

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

    if ( cfont.dirty() || testf(DirtyFont) )
	updateFont();
    if ( testf(DirtyPen|ExtDev) ) {
	if ( testf(DirtyPen) )
	    updatePen();
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
    }
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 )
	    return;
	fix_neg_rect( &x, &y, &w, &h );
    }

    QFontMetrics fm( cfont );			// get font metrics

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
    ushort cc 	      = 0;			// character code
    bool   decode     = internal && *internal;	// decode from internal data
    bool   encode     = internal && !*internal; // build internal data

    if ( len > 150 && !decode ) {		// need to alloc code array
	codelen = len + len/2;
	codes   = new ushort[codelen];
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

    bool wordbreak  = (tf & WordBreak)  == WordBreak;
    bool expandtabs = (tf & ExpandTabs) == ExpandTabs;
    bool singleline = (tf & SingleLine) == SingleLine;
    bool showprefix = (tf & ShowPrefix) == ShowPrefix;

    int  spacewidth = CWIDTH( ' ' );		// width of space char

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
	char      *data = *internal;
	text_info *ti   = (text_info*)data;
	if ( strncmp(ti->tag,"qptr",4)!=0 || ti->w != w || ti->h != h ||
	     ti->tf != tf || ti->len != len ) {
#if defined(CHECK_STATE)
	    warning( "QPainter::drawText: Internal text info is invalid" );
#endif
	    return;
	}
	maxwidth = ti->maxwidth;		// get internal values
	nlines   = ti->nlines;
	codelen  = ti->codelen;
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
	char      *data = new char[sizeof(text_info)+codelen*sizeof(ushort)];
	text_info *ti   = (text_info*)data;
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

    int     fascent  = fm.ascent();		// get font measurements
    int     fheight  = fm.height();
    QRegion save_rgn = crgn;			// save the current region
    bool    clip_on  = testf(ClipOn);
    int     xp, yp;
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

    if ( (tf & DontClip) == 0 )	{		// clip text
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
	    XSetClipMask( dpy, gc, None );
	    if ( gc_brush )
		XSetClipMask( dpy, gc_brush, None );
	}
    }

    if ( p_alloc )
	delete p;
    if ( code_alloc )
	delete codes;
}


/*!
Returns the bounding rectangle of the aligned text that would be
printed with the corresponding drawText() function.

The \e tf text formatting is the bitwise OR of the following flags:
<ul>
<li> \c AlignLeft aligns to the left border.
<li> \c AlignRight aligns to the right border.
<li> \c AlignHCenter aligns horizontally centered.
<li> \c AlignTop aligns to the top border.
<li> \c AlignBottom aligns to the bottom border.
<li> \c AlignVCenter aligns vertically centered
<li> \c AlignCenter (= \c AlignHCenter | AlignVCenter)
<li> \c SingleLine ignores newline characters in the text.
<li> \c DontClip never clips the text to the rectangle.
<li> \c ExpandTabs expands tabulators.
<li> \c ShowPrefix displays "&x" as "x" underlined.
<li> \c WordBreak breaks the text to fit the rectangle.
<li> \c GrayText grays out the text.
</ul>
*/

QRect QPainter::boundingRect( int x, int y, int w, int h, int tf,
			      const char *str, int len, char **internal )
{
    QRect brect;
    drawText( x, y, w, h, tf, str, len, &brect, internal );
    return brect;
}
