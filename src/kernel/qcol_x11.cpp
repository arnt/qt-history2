/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcol_x11.cpp#52 $
**
** Implementation of QColor class for X11
**
** Created : 940112
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcolor.h"
#include "string.h"
#include "qpaintd.h"
#include "qapp.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qcol_x11.cpp#52 $");


/*****************************************************************************
  The color dictionary speeds up color allocation significantly for X11.
  When there are no more colors, QColor::alloc() will set the colorAvail
  flag to FALSE and try to find/approximate a close color.
  NOTE: From deep within the event loop, the colorAvail flag is reset to
  TRUE (calls the function qResetColorAvailFlag()), because some other
  application might free its colors, thereby making them available for
  this Qt application.
 *****************************************************************************/

#include "qintdict.h"

struct QColorData {
    uint pix;					// allocated pixel value
    int	 context;				// allocation context
};

typedef Q_DECLARE(QIntDictM,QColorData)		QColorDict;
typedef Q_DECLARE(QIntDictIteratorM,QColorData) QColorDictIt;
static QColorDict *colorDict  = 0;		// dict of allocated colors
static bool	   colorAvail = TRUE;		// X colors available

static bool	color_init = FALSE;		// module was initialized
static int	current_alloc_context = 0;	// current color alloc context
static Visual  *g_vis	= 0;			// visual
static XColor  *g_carr	= 0;			// color array
static bool	g_truecolor;
static uint	red_mask , green_mask , blue_mask;
static int	red_shift, green_shift, blue_shift;


/*
  This function is called from the event loop. It resets the colorAvail
  flag so that the application can retry to allocate read-only colors
  that other applications may have deallocated lately.
*/

void qt_reset_color_avail()
{
    colorAvail = TRUE;
    if ( g_carr ) {				// color array was allocated
	delete [] g_carr;
	g_carr = 0;				// reset
    }
}


/*
  Returns a truecolor visual (if there is one). The SGI X server usually
  has an 8 bit default visual, but the application can also ask for a
  truecolor visual. This is what we do if QApplication::colorSpec() includes
  QApplication::TrueColor.
*/

static Visual *find_truecolor_visual( Display *dpy, int *depth, int *ncols )
{
    XVisualInfo *vi, rvi;
    int best=-1, n, i;
    int scr = DefaultScreen(dpy);
    rvi.c_class = TrueColor;
    rvi.screen  = scr;
    vi = XGetVisualInfo( dpy, VisualClassMask | VisualScreenMask,
			 &rvi, &n );
    if ( vi ) {
	for ( i=0; i<n; i++ ) {
	    if ( (vi[i].depth == 24) || (vi[i].depth > 24 && best<0) )
		best = i;
	}
    }
    Visual *v = DefaultVisual(dpy,scr);
    if ( best < 0 || (vi[best].visualid == XVisualIDFromVisual(v)) ) {
	*depth = DefaultDepth(dpy,scr);
	*ncols = DisplayCells(dpy,scr);	
    } else {
	v = vi[best].visual;
	*depth = vi[best].depth;
	*ncols = vi[best].colormap_size;
    }
    if ( vi )
	XFree( (char *)vi );
    return v;
}


/*****************************************************************************
  QColor misc internal functions
 *****************************************************************************/

static int highest_bit( uint v )
{
    int i;
    uint b = (uint)1 << 31;			// get pos of highest bit in v
    for ( i=31; ((b & v) == 0) && i>=0;	 i-- )
	b >>= 1;
    return i;
}


/*****************************************************************************
  QColor static member functions
 *****************************************************************************/

/*!
  Returns the maximum number of colors supported by the underlying window
  system.
*/

int QColor::maxColors()
{
    return QPaintDevice::x11Cells();
}

/*!
  Returns the number of color bit planes for the underlying window system.

  The returned values is equal to the default pixmap depth;

  \sa QPixmap::defaultDepth()
*/

int QColor::numBitPlanes()
{
    return QPaintDevice::x11Depth();
}


/*!
  Internal initialization required for QColor.
  This function is called from the QApplication constructor.
  \sa cleanup()
*/

void QColor::initialize()
{
    if ( color_init )				// already initialized
	return;
    color_init = TRUE;

    Display *dpy  = qt_xdisplay();
    int      scr  = DefaultScreen(dpy);
    int	     spec = QApplication::colorSpec();
    int	     depth, ncols;
    Colormap cmap;    
    const int tc = TrueColor;
#undef  TrueColor				// defined in X.h

    if ( spec & (int)QApplication::TrueColor ) {
	g_vis = find_truecolor_visual( dpy, &depth, &ncols );
    } else {
	g_vis = DefaultVisual(dpy,scr);
	depth = DefaultDepth(dpy,scr);
	ncols = DisplayCells(dpy,scr);
    }
    g_truecolor = g_vis->c_class == tc;
    bool defVis = g_vis == DefaultVisual(dpy,scr);
    bool defCmap;

    if ( g_truecolor )
	defCmap = !defVis;
    else
	defCmap = (spec & QApplication::PrivateColor) == 0;

    if ( defCmap )
	cmap = DefaultColormap(dpy,scr);
    else
	cmap = XCreateColormap(dpy, RootWindow(dpy,scr), g_vis, AllocNone );

    QPaintDevice::x_display   = dpy;
    QPaintDevice::x_screen    = scr;
    QPaintDevice::x_depth     = depth;
    QPaintDevice::x_cells     = ncols;
    QPaintDevice::x_colormap  = cmap;
    QPaintDevice::x_defcmap   = defCmap;
    QPaintDevice::x_visual    = g_vis;
    QPaintDevice::x_defvisual = defVis;

    int dictsize = 419;				// standard dict size
    if ( ncols > 256 || depth > 8 )
	dictsize = 2113;

    if ( g_truecolor ) {			// truecolor
	dictsize    = 1;			// will not need color dict
	red_mask    = (uint)g_vis->red_mask;
	green_mask  = (uint)g_vis->green_mask;
	blue_mask   = (uint)g_vis->blue_mask;
	red_shift   = highest_bit( red_mask )	- 7;
	green_shift = highest_bit( green_mask ) - 7;
	blue_shift  = highest_bit( blue_mask )	- 7;
    }
    colorDict = new QColorDict(dictsize);	// create dictionary
    CHECK_PTR( colorDict );

  // Initialize global color objects

    ((QColor*)(&black))->rgbVal = qRgb( 0, 0, 0 );
    ((QColor*)(&white))->rgbVal = qRgb( 255, 255, 255 );
    if ( defVis ) {
	((QColor*)(&black))->pix = BlackPixel( dpy, scr );
	((QColor*)(&white))->pix = WhitePixel( dpy, scr );
    } else {
	((QColor*)(&black))->alloc();
	((QColor*)(&white))->alloc();
    }

#if 0 /* 0 == allocate colors on demand */
    setLazyAlloc( FALSE );			// allocate global colors
    ((QColor*)(&darkGray))->	alloc();
    ((QColor*)(&gray))->	alloc();
    ((QColor*)(&lightGray))->	alloc();
    ((QColor*)(&::red))->	alloc();
    ((QColor*)(&::green))->	alloc();
    ((QColor*)(&::blue))->	alloc();
    ((QColor*)(&cyan))->	alloc();
    ((QColor*)(&magenta))->	alloc();
    ((QColor*)(&yellow))->	alloc();
    ((QColor*)(&darkRed))->	alloc();
    ((QColor*)(&darkGreen))->	alloc();
    ((QColor*)(&darkBlue))->	alloc();
    ((QColor*)(&darkCyan))->	alloc();
    ((QColor*)(&darkMagenta))-> alloc();
    ((QColor*)(&darkYellow))->	alloc();
    setLazyAlloc( TRUE );
#endif
}

/*!
  Internal clean up required for QColor.
  This function is called from the QApplication destructor.
  \sa initialize()
*/

void QColor::cleanup()
{
    if ( !color_init )
	return;
    color_init = FALSE;
    if ( colorDict ) {
	colorDict->setAutoDelete( TRUE );
	colorDict->clear();
	delete colorDict;
	colorDict = 0;
    }
}


/*****************************************************************************
  QColor member functions
 *****************************************************************************/

/*!
  Constructs a color with a RGB value and a custom pixel value.

  If the \e pix = 0xffffffff, then the color uses the RGB value in a
  standard way.	 If \e pix is something else, then the pixel value will
  be set directly to \e pix (skips the standard allocation procedure).
*/

QColor::QColor( QRgb rgb, uint pixel )
{
    if ( pixel == 0xffffffff ) {
	setRgb( rgb );
    } else {
	rgbVal = rgb;
	pix    = pixel;
    }
    rgbVal |= RGB_DIRECT;
}


/*!
  Allocates the RGB color and returns the pixel value.

  Allocating a color means to obtain a pixel value from the RGB specification.
  The pixel value is an index into the global color table.

  Calling the pixel() function will allocate automatically if
  the color was not already allocated.

  \sa setLazyAlloc(), enterAllocContext()
*/

uint QColor::alloc()
{
    Display *dpy = QPaintDevice::x__Display();
    int      scr = QPaintDevice::x11Screen();
    if ( (rgbVal & RGB_INVALID) || !color_init ) {
	rgbVal = qRgb( 0, 0, 0 );		// invalid color or state
	if ( dpy )
	    pix = BlackPixel(dpy, scr);
	else
	    pix = 0;
	return pix;
    }
    int r, g, b;
    if ( g_truecolor ) {			// truecolor: map to pixel
	r = (int)(rgbVal & 0xff);
	g = (int)((rgbVal >> 8) & 0xff);
	b = (int)((rgbVal >> 16) & 0xff);
	r = red_shift	> 0 ? r << red_shift   : r >> -red_shift;
	g = green_shift > 0 ? g << green_shift : g >> -green_shift;
	b = blue_shift	> 0 ? b << blue_shift  : b >> -blue_shift;
	pix = (b & blue_mask) | (g & green_mask) | (r & red_mask);
	rgbVal &= RGB_MASK;
	return pix;
    }
    QColorData *c = colorDict->find( (long)(rgbVal&RGB_MASK) );
    if ( c ) {					// found color in dictionary
	rgbVal &= RGB_MASK;			// color ok
	pix = c->pix;				// use same pixel value
	if ( c->context != current_alloc_context )
	    c->context = 0;			// convert to standard context
	return pix;
    }
    XColor col;
    r = (int)(rgbVal & 0xff);
    g = (int)((rgbVal >> 8) & 0xff);
    b = (int)((rgbVal >> 16) & 0xff);
    col.red   = r << 8;
    col.green = g << 8;
    col.blue  = b << 8;
    if ( colorAvail && XAllocColor(dpy, QPaintDevice::x11Colormap(), &col) ) {
	pix = (uint)col.pixel;			// allocated X11 color
	rgbVal &= RGB_MASK;
    } else {					// get closest color
	int mincol = -1;
	int mindist = 200000;
	int rx, gx, bx, dist;
	int i, maxi = QMIN(QPaintDevice::x11Cells(),256);
	register XColor *xc;
	colorAvail = FALSE;			// no more avail colors
	if ( !g_carr ) {			// get colors in colormap
	    g_carr = new XColor[maxi];
	    CHECK_PTR( g_carr );
	    xc = &g_carr[0];
	    for ( i=0; i<maxi; i++ ) {
		xc->pixel = i;			// carr[i] = color i
		xc++;
	    }
	    XQueryColors( dpy, QPaintDevice::x11Colormap(), g_carr, maxi );
	}
	xc = &g_carr[0];
	for ( i=0; i<maxi; i++ ) {		// find closest color
	    rx = r - (xc->red >> 8);
	    gx = g - (xc->green >> 8);
	    bx = b - (xc->blue>> 8);
	    dist = rx*rx + gx*gx + bx*bx;	// calculate distance
	    if ( dist < mindist ) {		// minimal?
		mindist = dist;
		mincol = i;
	    }
	    xc++;
	}
	if ( mincol == -1 ) {			// there are no colors, yuck
	    rgbVal |= RGB_INVALID;
	    pix = BlackPixel( dpy, scr );
	    return pix;
	}
	XAllocColor( dpy, QPaintDevice::x11Colormap(), &g_carr[mincol] );
	pix = g_carr[mincol].pixel;		// allocated X11 color
	rgbVal &= RGB_MASK;
    }
    if ( colorDict->count() < colorDict->size() * 8 ) {
	c = new QColorData;			// insert into color dict
	CHECK_PTR( c );
	c->pix	   = pix;
	c->context = current_alloc_context;
	colorDict->insert( (long)rgbVal, c );	// store color in dict
    }
    return pix;
}


/*!
  Sets the RGB value to that of the named color.

  This function searches the X color database for the color and sets the
  RGB value.  The color will be set to invalid if such a color does not
  exist.
*/

void QColor::setNamedColor( const char *name )
{
    bool ok = FALSE;
    if ( color_init	 ) {			// initialized
	XColor col, hw_col;
	if ( XLookupColor(QPaintDevice::x__Display(),
			  QPaintDevice::x11Colormap(), name, &col, &hw_col) ) {
	    ok = TRUE;
	    setRgb( col.red>>8, col.green>>8, col.blue>>8 );
	}
    }
    if ( !ok ) {
	rgbVal = RGB_INVALID;
	pix = BlackPixel( QPaintDevice::x__Display(),
			  QPaintDevice::x11Screen() );
    }
}


/*!
  Sets the RGB value to (\e r, \e g, \e b).
  \sa rgb(), setHsv()
*/

void QColor::setRgb( int r, int g, int b )
{
#if defined(CHECK_RANGE)
    if ( (uint)r > 255 || (uint)g > 255 || (uint)b > 255 )
	warning( "QColor::setRgb: RGB parameter(s) out of range" );
#endif
    rgbVal = qRgb(r,g,b);
    if ( lalloc || !color_init ) {
	rgbVal |= RGB_DIRTY;			// alloc later
	pix = 0;
    } else {
	alloc();				// alloc now
    }
}


#define MAX_CONTEXTS 16
static int  context_stack[MAX_CONTEXTS];
static int  context_ptr = 0;

static void init_context_stack()
{
    static bool did_init = FALSE;
    if ( !did_init ) {
	did_init = TRUE;
	context_stack[0] = current_alloc_context = 0;
    }
}


/*!
  Enters a color allocation context and returns a nonzero unique identifier.

  Color allocation contexts are useful for programs that need to
  allocate many colors and throw them away later, like image viewers.
  The allocation context functions work for true color displays as
  well as colormap display, except that QColor::destroyAllocContext()
  does nothing for true color.

  Example:
  \code
    QPixmap loadPixmap( const char *fileName )
    {
        static int alloc_context = 0;
	if ( alloc_context )
	    QColor::destroyAllocContext( alloc_context );
	alloc_context = QColor::enterAllocContext();
	QPixmap pm( fileName );
	QColor::leaveAllocContext();
	return pm;
    }
  \endcode

  The example code loads a pixmap from file. It frees up all colors
  that were allocated the last time loadPixmap() was called.

  The initial/default context is 0. Qt keeps a list of colors
  associated with their allocation contexts. You can call
  destroyAllocContext() to get rid of all colors that were allocated
  in a specific context.

  Calling enterAllocContext() enters an allocation context. The
  allocation context lasts until you call leaveAllocContext(). QColor
  has an internal stack of allocation contexts. Each call to
  enterAllocContex() must have a corresponding leaveAllocContext().

  \code
      // context 0 active
    int c1 = QColor::enterAllocContext();	// enter context c1
      // context c1 active
    int c2 = QColor::enterAllocContext();	// enter context c2
      // context c2 active
    QColor::leaveAllocContext();		// leave context c2
      // context c1 active
    QColor::leaveAllocContext();		// leave context c1
      // context 0 active
      // Now, free all colors that were allocated in context c2
    QColor::destroyAllocContext( c2 );
  \endcode

  You may also want to set the application's color specification.
  See QApplication::setColorSpec() for more information.

  \sa leaveAllocContext(), currentAllocContext(), destroyAllocContext(),
  QApplication::setColorSpec()
*/

int QColor::enterAllocContext()
{
    static int context_seq_no = 0;
    init_context_stack();
    if ( context_ptr+1 == MAX_CONTEXTS ) {
#if defined(CHECK_STATE)
	warning( "QColor::enterAllocContext: Context stack overflow" );
#endif
	return 0;
    }
    current_alloc_context = context_stack[++context_ptr] = ++context_seq_no;
    return current_alloc_context;
}


/*!
  Leaves a color allocation context.

  See enterAllocContext() for a detailed explanation.

  \sa enterAllocContext(), currentAllocContext()
*/

void QColor::leaveAllocContext()
{
    init_context_stack();
    if ( context_ptr == 0 ) {
#if defined(CHECK_STATE)
	warning( "QColor::leaveAllocContext: Context stack underflow" );
#endif
	return;
    }
    current_alloc_context = context_stack[--context_ptr];
}


/*!
  Returns the current color allocation context.

  The default context is 0.

  \sa enterAllocContext(), leaveAllocContext()
*/

int QColor::currentAllocContext()
{
    return current_alloc_context;
}


/*!
  Destroys a color allocation context, \e context.

  This function deallocates all colors that were allocated in the
  specified \a context. If \a context == -1, it frees up all colors
  that the application has allocated.

  The function does nothing for true color displays.

  \sa enterAllocContext(), alloc()
*/

void QColor::destroyAllocContext( int context )
{
    init_context_stack();
    if ( !color_init || g_truecolor )
	return; 
    ulong *pixels = new ulong[colorDict->count()];
    QColorData   *d;
    QColorDictIt it( *colorDict );
    int i = 0;
    uint rgbVal;
    while ( (d=it.current()) ) {
	rgbVal = (uint)it.currentKey();
	++it;
	if ( d->context == context || context == -1 ) {
	    pixels[i++] = d->pix;		// delete this color
	    colorDict->remove( (long)rgbVal );	// remove from dict
	}
    }
    if ( i )
	XFreeColors( QPaintDevice::x__Display(), QPaintDevice::x11Colormap(),
		     pixels, i, 0 );
    delete [] pixels;
}
