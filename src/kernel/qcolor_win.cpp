/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor_win.cpp#9 $
**
** Implementation of QColor class for Windows
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcolor.h"
#include "qapp.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcolor_win.cpp#9 $";
#endif


// --------------------------------------------------------------------------
// QColor special member functions
//

HANDLE QColor::hpal = 0;			// application global palette

static inline int d2i_round( double d )
{
    return d > 0 ? int(d+0.5) : int(d-0.5);
}


// --------------------------------------------------------------------------
// QColor static member functions
//

static bool did_it_yes = FALSE;

static void really_init_colors()
{
    did_it_yes = TRUE;
    *((QColor*)(&color0)) = QColor( 0x00ffffff, 0 );
    *((QColor*)(&color1)) = QColor( 0x00000000, 1 );
    ((QColor*)(&black))		->setRgb(   0,	 0,   0 );
    ((QColor*)(&white))		->setRgb( 255, 255, 255 );
    ((QColor*)(&darkGray))	->setRgb( 128, 128, 128 );
    ((QColor*)(&gray))		->setRgb( 160, 160, 160 );
    ((QColor*)(&lightGray))	->setRgb( 192, 192, 192 );
    ((QColor*)(&red))		->setRgb( 255,	 0,   0 );
    ((QColor*)(&green))		->setRgb(   0, 255,   0 );
    ((QColor*)(&blue))		->setRgb(   0,	0,  255 );
    ((QColor*)(&cyan))		->setRgb(   0, 255, 255 );
    ((QColor*)(&magenta))	->setRgb( 255,	0,  255 );
    ((QColor*)(&yellow))	->setRgb( 255, 255,   0 );
    ((QColor*)(&darkRed))	->setRgb( 128,	0,    0 );
    ((QColor*)(&darkGreen))	->setRgb(   0, 128,   0 );
    ((QColor*)(&darkBlue))	->setRgb(   0,	0,  128 );
    ((QColor*)(&darkCyan))	->setRgb(   0, 128, 128 );
    ((QColor*)(&darkMagenta))	->setRgb( 128,	0,  128 );
    ((QColor*)(&darkYellow))	->setRgb( 128, 128,   0 );
}


#define TEST_WINDOWS_PALETTE

void QColor::initialize()			// called from startup routines
{
#if defined(TEST_WINDOWS_PALETTE)
    int numCols = 256; // QWinInfo::numColors();
    if ( numCols <= 16 || numCols > 256 )	// no need to create palette
	return;

    const int max_r = 6;
    const int max_g = 6;
    const int max_b = 6;
    const double r_div = 1.0 / (max_r - 1);
    const double g_div = 1.0 / (max_g - 1);
    const double b_div = 1.0 / (max_b - 1);
    const int palsize = max_r * max_g * max_b;

    LOGPALETTE *logPal;
    logPal = (LOGPALETTE*)new char[sizeof(LOGPALETTE)+
				   sizeof(PALETTEENTRY)*palsize];
    ASSERT( logPal );
    logPal->palVersion = 0x300;			// Windows 3.0 compatible
    logPal->palNumEntries = palsize;
    int i = 0, r, g, b;
    for ( r=max_r-1; r>=0; r-- ) {
	for ( g=max_g-1; g>=0; g-- ) {
	    for ( b=max_b-1; b>=0; b-- ) {
		logPal->palPalEntry[i].peRed   = d2i_round(r_div * 255 * r);
		logPal->palPalEntry[i].peGreen = d2i_round(g_div * 255 * g);
		logPal->palPalEntry[i].peBlue  = d2i_round(b_div * 255 * b);
		logPal->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
		i++;
	    }
	}
    }
    ASSERT( i == palsize );
    hpal = CreatePalette ( logPal) ;		// create logical palette
    ASSERT( hpal );
#endif // TEST_WINDOWS_PALETTE
}

void QColor::cleanup()
{
    if ( hpal )					// delete application global
	DeleteObject( hpal );			//   palette
}


uint QColor::realizePal( QWidget *widget )	// realize palette
{
    if ( !hpal )				// not using palette
	return 0;
    HDC hdc = GetDC( widget->id() );
    HPALETTE hpalT = SelectPalette( hdc, hpal, FALSE );
    uint i = RealizePalette( hdc );
    UpdateColors( hdc );
    SelectPalette( hdc, hpalT, FALSE );
    ReleaseDC( widget->id(), hdc );
    return i;
}


// --------------------------------------------------------------------------
// QColor member functions
//

QColor::QColor()				// default RGB=0,0,0
{
    rgbVal = RGB_INVALID;
    pix = 0;
}

QColor::QColor( const QColor &c )		// copy color
{
    if ( !qApp && !did_it_yes )
	really_init_colors();
    rgbVal = c.rgbVal;
    pix = c.pix;
}

QColor::QColor( int r, int g, int b )		// specify RGB
{
    setRgb( r, g, b );
}

QColor::QColor( ulong rgb, ulong pixel )	// specify RGB and/or pixel
{
    if ( pixel == 0xffffffff )
	setRgb( rgb );
    else {
	rgbVal = rgb;
	pix    = PALETTEINDEX( pixel );
    }
}

QColor::QColor( const char *name )		// load color from database
{
    setNamedColor( name );
}


void QColor::alloc()				// allocate color
{
    rgbVal &= RGB_MASK;
    if ( hpal )
	pix = PALETTEINDEX( GetNearestPaletteIndex(hpal,rgbVal) );
    else
	pix = rgbVal;
}


void QColor::setNamedColor( const char * )	// load color from database
{
#if defined(DEBUG)
    warning( "QColor::setNamedColor: Named colors currently unsupported" );
#endif
    pix = rgbVal = QRGB(0,0,0);
}


void QColor::setRgb( int r, int g, int b )	// set RGB value
{
    rgbVal = QRGB(r,g,b);
    if ( hpal ) {
	int ri = d2i_round( 5.0*r/255.0 );
	int gi = d2i_round( 5.0*g/255.0 );
	int bi = d2i_round( 5.0*b/255.0 );
	int javelda = GetNearestPaletteIndex(hpal,rgbVal);
	pix = PALETTEINDEX( ri*6*6 + gi *6 + bi );
//	pix = PALETTEINDEX( GetNearestPaletteIndex(hpal,rgbVal) );
    }
    else
	pix = rgbVal;
}
