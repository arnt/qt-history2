/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor_x11.cpp#3 $
**
** Implementation of QColor class for X11
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qcolor.h"
#include "qwininfo.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcolor_x11.cpp#3 $";
#endif


// --------------------------------------------------------------------------
// Application-wide data structures for QColor:
//   A color dictionary which speeds up color handling.
//   A color waiting list which post-initializes global custom colors.
//
// A waiting color is a color object that is declared as a global application
// object with a non-zero RGB value or a color name.
// Such colors cannot be allocated before the X display variable is set,
// therefore we need to register them in a list and perform initialization
// after the display variable is ok.
// Global QFont objects are initialized in a similar way.
//

#include "qintdict.h"
#include "qlist.h"

typedef declare(QIntDictM,QColor) QColorDict;
typedef declare(QIntDictIteratorM,QColor) QColorDictIt;
static QColorDict *colorDict = 0;		// dict of allocated colors
static bool	   colorAvail = TRUE;		// X colors available

struct QWaitingColor {				// waiting color
    QColor *cptr;
    ulong   rgb;
    char   *name;
};
typedef declare(QListM,QWaitingColor)	  QColorWaitingList;
static QColorWaitingList *colorWaitList = 0;	// list of waiting colors


// --------------------------------------------------------------------------
// QColor special member functions
//

static Colormap cmap = 0;			// application global colormap

inline ulong _RGB( uint r, uint g, uint b )
{
    return (uchar)r | ((ushort)g << 8) | ((ulong)b << 16);
}


// --------------------------------------------------------------------------
// QColor static member functions
//

void QColor::initialize()			// called from startup routines
{
    if ( cmap )					// already initialized
	return;
    Display *dpy = qXDisplay();
    int screen = qXScreen();
    cmap = DefaultColormap( dpy, screen );	// create colormap

    if ( QWinInfo::numColors() <= 256 ) {	// limited number of colors
	colorDict = new QColorDict;		// then use dictionary
	CHECK_PTR( colorDict );
    }

  // Initialize global color objects

    ((QColor*)(&black))->rgb = _RGB( 0, 0, 0 );
    ((QColor*)(&black))->pix = BlackPixel( dpy, screen );
    ((QColor*)(&white))->rgb = _RGB( 255, 255, 255 );
    ((QColor*)(&white))->pix = WhitePixel( dpy, screen );

    ((QColor*)(&darkGray))->   setRGB( 128, 128, 128 );
    ((QColor*)(&gray))->       setRGB( 160, 160, 160 );
    ((QColor*)(&lightGray))->  setRGB( 192, 192, 192 );
    ((QColor*)(&::red))->      setRGB( 255,   0,   0 );
    ((QColor*)(&::green))->    setRGB(	 0, 255,   0 );
    ((QColor*)(&::blue))->     setRGB(	 0,   0, 255 );
    ((QColor*)(&cyan))->       setRGB(	 0, 255, 255 );
    ((QColor*)(&magenta))->    setRGB( 255,   0, 255 );
    ((QColor*)(&yellow))->     setRGB( 255, 255,   0 );
    ((QColor*)(&::darkRed))->  setRGB( 128,   0,   0 );
    ((QColor*)(&::darkGreen))->setRGB(	 0, 128,   0 );
    ((QColor*)(&::darkBlue))-> setRGB(	 0,   0, 128 );
    ((QColor*)(&darkCyan))->   setRGB(	 0, 128, 128 );
    ((QColor*)(&darkMagenta))->setRGB( 128,   0, 128 );
    ((QColor*)(&darkYellow))-> setRGB( 128, 128,   0 );

  // Initialize global custom color objects

    register QWaitingColor *wc = colorWaitList ? colorWaitList->first() : 0;
    while ( wc ) {
	if ( wc->name ) {
	    XColor col, hw_col;
	    if ( XLookupColor( dpy, cmap, wc->name, &col, &hw_col ) )
		wc->cptr->setRGB( col.red>>8, col.green>>8, col.blue>>8 );
	}
	else
	    wc->cptr->setRGB( wc->rgb );
	wc = colorWaitList->next();
    }
    delete colorWaitList;
}

void QColor::cleanup()
{
    if ( !colorDict )
	return;
#if 0		/* NOTE!!! Is this required for read-only colors */
    int s = colorDict->count();
    ulong *pixels = new ulong[s];		// array of pixel values
    CHECK_PTR( pixels );
    QColorDictIt it( *colorDict );
    for ( int i=0; i<s; i++ ) {			// put all colors in array
	pixels[i] = it.current()->pixel();
	++it;
    }
    XFreeColors( qXDisplay(), cmap, pixels, s, 0 ); // free colors
    delete pixels;
#endif
    colorDict->setAutoDelete( TRUE );		// remove all entries
    colorDict->clear();
    delete colorDict;
}


// --------------------------------------------------------------------------
// QColor member functions
//

QColor::QColor()				// default RGB=0,0,0
{
    rgb = RGB_INVALID;
    pix = 0;
}

QColor::QColor( const QColor &c )		// copy color
{
    if ( cmap ) {				// initialized
	rgb = c.rgb;
	pix = c.pix;
    }
    else {					// not initialized
	if ( !colorWaitList )
	    return;
	QWaitingColor *wc = colorWaitList->first();
	QWaitingColor *new_wc;
	QColor *pc = (QColor *)&c;
	while ( wc && wc->cptr != pc )		// find other color in list
	    wc = colorWaitList->next();
	if ( wc ) {				// found color in list
	    rgb = pc->rgb;
	    new_wc = new QWaitingColor;
	    new_wc->cptr = (QColor *)this;
	    new_wc->rgb = rgb;
	    new_wc->name = wc->name;
	    colorWaitList->append( new_wc );	// add similar entry to list
	}
    }
}

QColor::QColor( int r, int g, int b )		// specify RGB
{
    setRGB( r, g, b );
}

QColor::QColor( const char *name )		// load color from database
{
    rgb = RGB_INVALID;
    pix = 0;
    if ( cmap  ) {				// initialized
	XColor col, hw_col;
	if ( !XLookupColor( qXDisplay(), cmap, name, &col, &hw_col ) ) {
#if defined(CHECK_NULL)
	    warning( "QColor: Color name %s not found", name );
#endif
	    return;
	}
	setRGB( col.red>>8, col.green>>8, col.blue>>8 );
    }
    else {					// not initialized
	if ( !colorWaitList ) {
	    colorWaitList = new QColorWaitingList;
	    CHECK_PTR( colorWaitList );
	    colorWaitList->setAutoDelete( TRUE );
	}
	QWaitingColor *wc = new QWaitingColor;
	CHECK_PTR( wc );
	wc->cptr = (QColor *)this;
	wc->rgb = 0;
	wc->name = (char *)name;
	colorWaitList->append( wc );		// add entry to list
    }
}

QColor::~QColor()
{
    if ( !cmap ) {				// not initialized
	QWaitingColor *wc = colorWaitList ? colorWaitList->first() : 0;
	while ( wc ) {				// remove entry from list
	    if ( wc->cptr == this ) {
		colorWaitList->remove();
		break;
	    }
	    wc = colorWaitList->next();
	}
    }
}


bool QColor::setRGB( int r, int g, int b )	// set RGB value
{
    rgb = _RGB(r,g,b);
    if ( !cmap ) {				// not initialized
	if ( !colorWaitList ) {
	    colorWaitList = new QColorWaitingList;
	    CHECK_PTR( colorWaitList );
	    colorWaitList->setAutoDelete( TRUE );
	}
	QWaitingColor *wc = new QWaitingColor;
	CHECK_PTR( wc );
	wc->cptr = (QColor *)this;
	wc->rgb = rgb;
	wc->name = 0;
	colorWaitList->append( wc );		// add entry to list
	return FALSE;
    }
    if ( colorDict ) {				// lookup color dictionary
	QColor *c = colorDict->find( (long)rgb );
	if ( c ) {				// found color
	    pix = c->pix;			// use same pixel value
	    return TRUE;
	}
    }
    XColor col;
    col.red = r << 8;
    col.green = g << 8;
    col.blue = b << 8;
    if ( colorAvail && XAllocColor( qXDisplay(), cmap, &col ) ) {
	pix = col.pixel;			// allocated X11 color
	if ( colorDict ) {			// insert into color dict
	    QColor *c = new QColor;
	    CHECK_PTR( c );
	    c->rgb = rgb;			// copy values
	    c->pix = pix;
	    colorDict->insert( (long)rgb, c );	// store color in dict
	}
    }
    else {					// compute closest color
	QColorDictIt it( *colorDict );
	register QColor *c;
	QColor *mincol = 0;			// differs at a minimum
	int minsum = 30000;			// minimum is max 25500
	int rx, gx, bx, wsum;
	colorAvail = FALSE;			// no more avail colors
	while ( (c=it.current()) ) {
	    rx = r - c->red();
	    if ( rx < 0 )
		rx = -rx;
	    gx = g - c->green();
	    if ( gx < 0 )
		gx = -gx;
	    bx = b - c->blue();
	    if ( bx < 0 )
		bx = -bx;
	    wsum = rx*58 + gx*30 + bx*11;	// calc weighted diff sum
	    if ( wsum < minsum ) {		// minimal?
		minsum = wsum;
		mincol = c;
	    }
	    ++it;
	}
	if ( !mincol ) {
	    rgb |= RGB_INVALID;
	    pix = BlackPixel( qXDisplay(), qXScreen() );
	    return FALSE;
	}
	pix = mincol->pix;
    }
    return TRUE;
}
