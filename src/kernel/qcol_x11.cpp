/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcol_x11.cpp#1 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcol_x11.cpp#1 $";
#endif


// --------------------------------------------------------------------------
// Global colors
//

const QColor black;
const QColor white;
const QColor darkGray;
const QColor gray;
const QColor lightGray;
const QColor red;
const QColor green;
const QColor blue;
const QColor cyan;
const QColor magenta;
const QColor yellow;
const QColor darkRed;
const QColor darkGreen;
const QColor darkBlue;
const QColor darkCyan;
const QColor darkMagenta;
const QColor darkYellow;


// --------------------------------------------------------------------------
// QColor dictionary to make color lookup faster than using XAllocColor
//

#include "qintdict.h"

typedef declare(QIntDictM,QColor) QColorDict;
typedef declare(QIntDictIteratorM,QColor) QColorDictIt;

static QColorDict *colorDict = 0;		// dict of allocated colors
static bool	   colorAvail = TRUE;		// X colors available


// --------------------------------------------------------------------------
// QColor member functions
//

static Colormap cmap = 0;			// application global colormap

inline ulong _RGB( uint r, uint g, uint b )
{
    return (uchar)r | ((ushort)g << 8) | ((ulong)b << 16);
}


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
}

void QColor::cleanup()
{
    colorDict->setAutoDelete( TRUE );		// remove all entries
    colorDict->clear();
    delete colorDict;
}


QColor::QColor()				// default RGB=0,0,0
{
    rgb = RGB_INVALID;
    pix = 0;
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
#if defined(CHECK_STATE)
    else
	warning( "QColor: Not ready to set color %s", name );
#endif
}


bool QColor::setRGB( int r, int g, int b )	// set RGB value
{
    if ( !cmap ) {				// not initialized
#if defined(CHECK_STATE)
	warning( "QColor: Not ready to set color (%d,%d,%d)", r, g, b );
#endif
	rgb = RGB_INVALID;
	pix = 0;
	return FALSE;
    }
    rgb = _RGB(r,g,b);
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
