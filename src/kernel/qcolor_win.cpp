/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor_win.cpp#16 $
**
** Implementation of QColor class for Windows
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcolor.h"
#include "qapp.h"
#include <windows.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qcolor_win.cpp#16 $")


/*****************************************************************************
  QColor static member functions
 *****************************************************************************/

HANDLE QColor::hpal = 0;			// application global palette


int QColor::maxColors()
{
    static int maxcols = 0;
    if ( maxcols == 0 ) {
	HANDLE hdc = GetDC( 0 );
	if ( GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE )
	    maxcols = GetDeviceCaps( hdc, SIZEPALETTE );
	else
	    maxcols = GetDeviceCaps( hdc, NUMCOLORS );
	ReleaseDC( 0, hdc );
    }
    return maxcols;
}

int QColor::numBitPlanes()
{
    static int planes = 0;
    if ( planes == 0 ) {
	HANDLE hdc = GetDC( 0 );
	planes = GetDeviceCaps( hdc, BITSPIXEL );
	ReleaseDC( 0, hdc );
    }
    return planes;
}


#define TEST_WINDOWS_PALETTE

void QColor::initialize()
{
    return;
#if defined(TEST_WINDOWS_PALETTE)
    int numCols = maxColors();
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
		logPal->palPalEntry[i].peRed   = qRound(r_div * 255 * r);
		logPal->palPalEntry[i].peGreen = qRound(g_div * 255 * g);
		logPal->palPalEntry[i].peBlue  = qRound(b_div * 255 * b);
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
    if ( hpal ) {				// delete application global
	DeleteObject( hpal );			//   palette
	hpal = 0;
    }
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


/*****************************************************************************
  QColor member functions
 *****************************************************************************/

QColor::QColor( ulong rgb, ulong pixel )
{
    if ( pixel == 0xffffffff )
	setRgb( rgb );
    else {
	rgbVal = rgb;
	pix    = PALETTEINDEX( pixel );
    }
    rgbVal |= RGB_DIRECT;
}


ulong QColor::alloc()
{
    rgbVal &= RGB_MASK;
    if ( hpal )
	pix = PALETTEINDEX( GetNearestPaletteIndex(hpal,rgbVal) );
    else
	pix = rgbVal;
    return pix;
}


void QColor::setNamedColor( const char * )
{
#if defined(DEBUG)
    warning( "QColor::setNamedColor: Named colors not supported" );
#endif
    pix = rgbVal = QRGB(0,0,0);
}


void QColor::setRgb( int r, int g, int b )
{
    rgbVal = QRGB(r,g,b);
    if ( hpal ) {
#if 0
	int ri = qRound( 5.0*r/255.0 );
	int gi = qRound( 5.0*g/255.0 );
	int bi = qRound( 5.0*b/255.0 );
	pix = PALETTEINDEX( ri*6*6 + gi *6 + bi );
#else
	pix = PALETTEINDEX( GetNearestPaletteIndex(hpal,rgbVal) );
#endif
    }
    else
	pix = rgbVal;
}
