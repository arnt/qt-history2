/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcol_x11.cpp#58 $
**
** Implementation of QColor class for X11
**
** Created : 940112
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
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

RCSTAG("$Id: //depot/qt/main/src/kernel/qcol_x11.cpp#58 $");


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
static bool	colors_frozen = FALSE;		// allocating disabled
static int	current_alloc_context = 0;	// current color alloc context
static Visual  *g_vis	= 0;			// visual
static XColor  *g_carr	= 0;			// color array
static bool    *g_our_alloc = 0;		// our allocated colors
static bool	g_truecolor;
static uint	red_mask , green_mask , blue_mask;
static int	red_shift, green_shift, blue_shift;

extern int	qt_ncols_option;		// defined in qapp_x11.cpp
extern int	qt_visual_option;
extern bool	qt_cmap_option;

/*
  This function is called from the event loop. It resets the colorAvail
  flag so that the application can retry to allocate read-only colors
  that other applications may have deallocated lately.

  The g_our_alloc and g_carr are global arrays that optimize color
  approximation when there are no more colors left to allocate.
*/

void qt_reset_color_avail()
{
    colorAvail = TRUE;
    if ( g_carr ) {				// color array was allocated
	delete [] g_our_alloc;
	g_our_alloc = 0;
	delete [] g_carr;
	g_carr = 0;
    }
}


/*
  Returns a truecolor visual (if there is one). 8-bit TrueColor visuals
  are ignored, unless the user has explicitly requested -visual TrueColor.
  The SGI X server usually has an 8 bit default visual, but the application
  can also ask for a truecolor visual. This is what we do if
  QApplication::colorSpec() is QApplication::ManyColor.
*/

static Visual *find_truecolor_visual( Display *dpy, int *depth, int *ncols )
{
    XVisualInfo *vi, rvi;
    int best=0, n, i;
    int scr = DefaultScreen(dpy);
    rvi.c_class = TrueColor;
    rvi.screen  = scr;
    vi = XGetVisualInfo( dpy, VisualClassMask | VisualScreenMask,
			 &rvi, &n );
    if ( vi ) {
	for ( i=0; i<n; i++ ) {
	    if ( vi[i].depth > vi[best].depth )
		best = i;
	}
    }
    Visual *v = DefaultVisual(dpy,scr);
    if ( !vi || (vi[best].visualid == XVisualIDFromVisual(v))
       || (vi[best].depth <= 8 && qt_visual_option != TrueColor) )
    {
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

#if QT_VERSION == 200
#error "Remove old colorspecs"
    if ( spec == (int)QApplication::ManyColor ) {
#else /* } bracket match */
    if ( spec & ((int)QApplication::ManyColor | 2) ) {
#endif
	g_vis = find_truecolor_visual( dpy, &depth, &ncols );
    } else {
	g_vis = DefaultVisual(dpy,scr);
	depth = DefaultDepth(dpy,scr);
	ncols = DisplayCells(dpy,scr);
    }
    g_truecolor = g_vis->c_class == tc;
    bool defVis = (XVisualIDFromVisual(g_vis) == 
		   XVisualIDFromVisual(DefaultVisual(dpy,scr)));
    bool defCmap;

    if ( g_truecolor )
	defCmap = defVis;
    else
	defCmap = !qt_cmap_option;

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
    if ( defVis && defCmap ) {
	((QColor*)(&black))->pix = BlackPixel( dpy, scr );
	((QColor*)(&white))->pix = WhitePixel( dpy, scr );
    } else {
	((QColor*)(&black))->alloc();
	((QColor*)(&white))->alloc();
    }

#if QT_VERSION == 200
#error "Remove old colorspecs"
    if ( spec == (int)QApplication::ManyColor && !g_truecolor ) {
#else /* } bracket match */
    if ( (spec & ((int)QApplication::ManyColor | 2)) && !g_truecolor ) {
#endif
	// Allocate a color cube, starting from the most common and extreme
	// colours in the cube, then each most-distant color.
	static QRgb cmap[216] = {
	    0x000000, 0xffffff, 0x0000ff, 0x00ff00,
	    0xff0000, 0x00ffff, 0xffff00, 0xff00ff,
	    0x00ff99, 0xff0099, 0xcccc00, 0x6699ff,
	    0x666666, 0x33ff00, 0xff3300, 0xff66ff,
	    0x9900ff, 0x99ff99, 0xff9966, 0x009933,
	    0x990033, 0x33ffff, 0x0099cc, 0x003399,
	    0x660099, 0x669900, 0x99ffff, 0xff00ff,
	    0xffff99, 0xcccccc, 0x9966cc, 0x66cc66,
	    0xcc3366, 0xffff33, 0x99ff33, 0x333333,
	    0x00ccff, 0xcc99ff, 0x3366ff, 0xcc33ff,
	    0x6633ff, 0x66ffcc, 0x33cccc, 0xff99cc,
	    0xff33cc, 0x3333cc, 0xcc00cc, 0xcc9999,
	    0x669999, 0xff6699, 0x336699, 0x993399,
	    0xccff66, 0x33ff66, 0x00cc66, 0x999966,
	    0x339966, 0x006666, 0x330066, 0x00ff33,
	    0x33cc33, 0xcc9933, 0xff6633, 0x996633,
	    0xff0033, 0x00cc00, 0xff9900, 0xcc6600,
	    0x336600, 0x993300, 0xcc0000, 0x660000,
	    0xccffff, 0x66ffff, 0x00ffff, 0xffccff,
	    0xccccff, 0x99ccff, 0x66ccff, 0x33ccff,
	    0xff99ff, 0x9999ff, 0x3399ff, 0x0099ff,
	    0xcc66ff, 0x9966ff, 0x6666ff, 0x0066ff,
	    0xff33ff, 0x9933ff, 0x3333ff, 0x0033ff,
	    0xcc00ff, 0x6600ff, 0x3300ff, 0xffffcc,
	    0xccffcc, 0x99ffcc, 0x33ffcc, 0x00ffcc,
	    0xffcccc, 0x99cccc, 0x66cccc, 0x00cccc,
	    0xcc99cc, 0x9999cc, 0x6699cc, 0x3399cc,
	    0xff66cc, 0xcc66cc, 0x6666cc, 0x3366cc,
	    0x0066cc, 0xcc33cc, 0x9933cc, 0x6633cc,
	    0x0033cc, 0xff00cc, 0x9900cc, 0x6600cc,
	    0x3300cc, 0x0000cc, 0xccff99, 0x66ff99,
	    0x33ff99, 0xffcc99, 0xcccc99, 0x99cc99,
	    0x66cc99, 0x33cc99, 0x00cc99, 0xff9999,
	    0x999999, 0x339999, 0x009999, 0xcc6699,
	    0x996699, 0x666699, 0x006699, 0xff3399,
	    0xcc3399, 0x663399, 0x333399, 0xcc0099,
	    0x990099, 0x330099, 0x000099, 0xffff66,
	    0x99ff66, 0x66ff66, 0x00ff66, 0xffcc66,
	    0xcccc66, 0x99cc66, 0x33cc66, 0xcc9966,
	    0x669966, 0x009966, 0xff6666, 0xcc6666,
	    0x996666, 0x336666, 0xff3366, 0x993366,
	    0x663366, 0x333366, 0x003366, 0xff0066,
	    0xcc0066, 0x990066, 0x660066, 0x000066,
	    0xccff33, 0x66ff33, 0x33ff33, 0xffcc33,
	    0xcccc33, 0x99cc33, 0x66cc33, 0x00cc33,
	    0xff9933, 0x999933, 0x669933, 0x339933,
	    0xcc6633, 0x666633, 0x336633, 0x006633,
	    0xff3333, 0xcc3333, 0x993333, 0x663333,
	    0x003333, 0xcc0033, 0x660033, 0x330033,
	    0x000033, 0xffff00, 0xccff00, 0x99ff00,
	    0x66ff00, 0x00ff00, 0xffcc00, 0x99cc00,
	    0x66cc00, 0x33cc00, 0xcc9900, 0x999900,
	    0x339900, 0x009900, 0xff6600, 0x996600,
	    0x666600, 0x006600, 0xcc3300, 0x663300,
	};

	if (qt_ncols_option>216) qt_ncols_option=216;

	for ( int i = 0; i < qt_ncols_option; i++ ) {
            QColor *c=new QColor(cmap[i]);
	    c->alloc();
        }

	// No more custom allocations: user has set limit
	colors_frozen = TRUE;
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
    if ( !QPaintDevice::x_defcmap )
	XFreeColormap( QPaintDevice::x__Display(), QPaintDevice::x_colormap );
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

  If the \e pixel = 0xffffffff, then the color uses the RGB value in a
  standard way.	 If \e pixel is something else, then the pixel value will
  be set directly to \e pixel (skips the standard allocation procedure).
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
    if ( !colors_frozen
    && colorAvail && XAllocColor(dpy, QPaintDevice::x11Colormap(), &col) ) {
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
	    g_our_alloc = new bool[maxi];
	    CHECK_PTR( g_our_alloc );
	    memset( g_our_alloc, FALSE, maxi*sizeof(*g_our_alloc) );
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
	if ( !g_our_alloc[mincol] ) {		// we haven't allocated it
	    XAllocColor( dpy, QPaintDevice::x11Colormap(), &g_carr[mincol] );
	    g_our_alloc[mincol] = TRUE;
	}
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


void QColor::setSystemNamedColor( const char *name )
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
