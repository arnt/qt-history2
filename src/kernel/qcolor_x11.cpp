/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor_x11.cpp#4 $
**
** Implementation of QColor class for X11
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcolor.h"
#include "qwininfo.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcolor_x11.cpp#4 $";
#endif


// --------------------------------------------------------------------------
// The color dictionary speeds up color allocation significantly for X11.
// When there are no more colors, QColor::alloc() will set the colorAvail
// flag to FALSE and try to find/approximate a close color.
// WATCH OUT: From deep within the event loop, the colorAvail flag is
// reset to TRUE (calls the function qResetColorAvailFlag()), because some
// other application might free its colors, thereby making them available
// for this Qt application.
//

#include "qintdict.h"
#include "qlist.h"

typedef declare(QIntDictM,QColor) QColorDict;
typedef declare(QIntDictIteratorM,QColor) QColorDictIt;
static QColorDict *colorDict = 0;		// dict of allocated colors
static bool	   colorAvail = TRUE;		// X colors available

void qResetColorAvailFlag()			// OOPS: called from event loop
{
    colorAvail = TRUE;
}


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
    int nc = QWinInfo::numColors();		// number of colors
    int dictsize = 211;				// standard dict size
    if ( nc > 256 )
	dictsize = 2113;
    colorDict = new QColorDict(dictsize);	// create dictionary
    CHECK_PTR( colorDict );

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
}

void QColor::cleanup()
{
    if ( !colorDict )
	return;
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
    rgb = c.rgb;
    pix = c.pix;
}

QColor::QColor( int r, int g, int b )		// specify RGB
{
    setRGB( r, g, b );
}

QColor::QColor( const char *name )		// load color from database
{
    setNamedColor( name );
}

QColor::~QColor()
{
}


bool QColor::alloc()				// allocate color
{
    if ( (rgb & RGB_INVALID) || !colorDict ) {	// invalid color or state
	rgb = _RGB( 0, 0, 0 );
	pix = 0;
	return TRUE;
    }
    register QColor *c = colorDict->find( (long)(rgb&RGB_MASK) );
    if ( c ) {					// found color in dictionary
	rgb &= RGB_MASK;			// color ok
	pix = c->pix;				// use same pixel value
	return TRUE;
    }
    int r = (int)(rgb & 0xff);
    int g = (int)((rgb >> 8) & 0xff);
    int b = (int)((rgb >> 16) & 0xff);
    XColor col;
    col.red = r << 8;
    col.green = g << 8;
    col.blue = b << 8;
    if ( colorAvail && XAllocColor( qXDisplay(), cmap, &col ) ) {
	pix = col.pixel;			// allocated X11 color
	c = new QColor;				// insert into color dict
	CHECK_PTR( c );
	rgb &= RGB_MASK;
	c->rgb = rgb;				// copy values
	c->pix = pix;
	colorDict->insert( (long)rgb, c );	// store color in dict
    }
    else {					// compute closest color
	QColorDictIt it( *colorDict );
	QColor *mincol = 0;			// differs at a minimum
	int mindist = 200000;			// minimum is max 195075
	int rx, gx, bx, dist;
	colorAvail = FALSE;			// no more avail colors
	while ( (c=it.current()) ) {		// examine all colors in dict
	    rx = r - c->red();
	    gx = g - c->green();
	    bx = b - c->blue();
	    dist = rx*rx + gx*gx + bx*bx;	// calculate color distance
	    if ( dist < mindist ) {		// minimal?
		mindist = dist;
		mincol = c;
	    }
	    ++it;
	}
	if ( !mincol ) {			// there are no colors
	    rgb |= RGB_INVALID;
	    pix = BlackPixel( qXDisplay(), qXScreen() );
	    return FALSE;
	}

	pix = mincol->pix;
    }
    return TRUE;
}


bool QColor::setNamedColor( const char *name )	// load color from database
{
    bool ok = FALSE;
    if ( cmap  ) {				// initialized
	XColor col, hw_col;
	if ( XLookupColor( qXDisplay(), cmap, name, &col, &hw_col ) )
	    ok = setRGB( col.red>>8, col.green>>8, col.blue>>8 );
    }
    else {
	rgb = RGB_INVALID;
	pix = 0;
    }
    return ok;
}


bool QColor::setRGB( int r, int g, int b )	// set RGB value
{
    rgb = _RGB(r,g,b);
    if ( !autoAlloc() || !cmap ) {
	rgb |= RGB_DIRTY;			// alloc later
	return TRUE;
    }
    return alloc();
}
