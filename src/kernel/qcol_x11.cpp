/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcol_x11.cpp#48 $
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
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qcol_x11.cpp#48 $");


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

static int	current_alloc_context = 0;	// current color alloc context
static int	g_depth = 0;			// display depth
static int	g_ncols = 0;			// number of colors
static Colormap g_cmap	= 0;			// application global colormap
static XColor  *g_carr	= 0;			// color array
static Visual  *g_vis	= 0;
static bool	g_truecolor;
static uint	red_mask , green_mask , blue_mask;
static int	red_shift, green_shift, blue_shift;


void qt_reset_color_avail()			// OOPS: called from event loop
{
    colorAvail = TRUE;
    if ( g_carr ) {				// color array was allocated
	delete [] g_carr;
	g_carr = 0;				// reset
    }
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
    return g_ncols;
}

/*!
  Returns the number of color bit planes for the underlying window system.

  The returned values is equal to the default pixmap depth;

  \sa QPixmap::defaultDepth()
*/

int QColor::numBitPlanes()
{
    return g_depth;
}


/*!
  Internal initialization required for QColor.
  This function is called from the QApplication constructor.
  \sa cleanup()
*/

void QColor::initialize()
{
    if ( g_cmap )				// already initialized
	return;
    ginit = TRUE;

    Display *dpy    = qt_xdisplay();
    int	     screen = qt_xscreen();
    g_depth = DefaultDepth( dpy, screen );	// default depth of display
    g_cmap  = DefaultColormap( dpy, screen );	// create colormap
    g_ncols = DisplayCells( dpy, screen );	// number of colors
    g_vis   = DefaultVisual( dpy, screen );
    g_truecolor = g_vis->c_class == TrueColor;

    int dictsize = 419;				// standard dict size
    if ( g_ncols > 256 || g_depth > 8 )
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
    ((QColor*)(&black))->pix	= BlackPixel( dpy, screen );
    ((QColor*)(&white))->rgbVal = qRgb( 255, 255, 255 );
    ((QColor*)(&white))->pix	= WhitePixel( dpy, screen );

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
    if ( !colorDict )
	return;
    colorDict->setAutoDelete( TRUE );		// remove all entries
    colorDict->clear();
    delete colorDict;
    colorDict = 0;
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
    Display *dpy = qt_xdisplay();
    int      scr = qt_xscreen();
    if ( (rgbVal & RGB_INVALID) || !g_cmap ) { // invalid color or state
	rgbVal = qRgb( 0, 0, 0 );
	if ( dpy )
	    pix = BlackPixel( dpy, scr );
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
    if ( colorAvail && XAllocColor(dpy, g_cmap, &col) ) {
	pix = (uint)col.pixel;			// allocated X11 color
	rgbVal &= RGB_MASK;
    } else {					// get closest color
	int mincol = -1;
	int mindist = 200000;
	int rx, gx, bx, dist;
	int i, maxi = g_ncols > 256 ? 256 : g_ncols;
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
	    XQueryColors( dpy, g_cmap, g_carr, maxi );
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
	XAllocColor( dpy, g_cmap, &g_carr[mincol] );
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
    if ( g_cmap	 ) {				// initialized
	XColor col, hw_col;
	if ( XLookupColor( qt_xdisplay(), g_cmap, name, &col, &hw_col ) ) {
	    ok = TRUE;
	    setRgb( col.red>>8, col.green>>8, col.blue>>8 );
	}
    }
    if ( !ok ) {
	rgbVal = RGB_INVALID;
	pix = BlackPixel( qt_xdisplay(), qt_xscreen() );
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
    if ( lalloc || g_cmap == 0 ) {
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
  
  \sa leaveAllocContext(), currentAllocContext(), destroyAllocContext()
*/

int QColor::enterAllocContext()
{
    static context_seq_no = 0;
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
	return 0;
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
    if ( !g_cmap || g_truecolor )
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
	XFreeColors( qt_xdisplay(), g_cmap, pixels, i, 0 );
    delete [] pixels;
}
