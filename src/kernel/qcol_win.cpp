/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcol_win.cpp#22 $
**
** Implementation of QColor class for Win32
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcolor.h"
#include "qapp.h"

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qcol_win.cpp#22 $")


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


void QColor::initialize()
{
    return;

    int numCols = maxColors();
    if ( numCols <= 16 || numCols > 256 )	// no need to create palette
	return;

    LOGPALETTE *logPal;
    logPal = (LOGPALETTE*)new char[sizeof(LOGPALETTE)+
				   sizeof(PALETTEENTRY)*256];
    ASSERT( logPal );
    logPal->palVersion = 0x300;			// Windows 3.0 compatible
    logPal->palNumEntries = 256;
    HDC hdcScreen = CreateCompatibleDC( 0 );
    GetSystemPaletteEntries( hdcScreen, 0, 10, &logPal->palPalEntry[0] );
    GetSystemPaletteEntries( hdcScreen, 246, 10, &logPal->palPalEntry[246] );
    DeleteDC( hdcScreen );
    int r, g, b;
    PALETTEENTRY *pe = &logPal->palPalEntry[10];
    for ( r=0; r<6; r++ ) {			// create color cube
	for ( g=0; g<6; g++ ) {
	    for ( b=0; b<6; b++ ) {
		pe->peRed   = r * 255 / 6;
		pe->peGreen = g * 255 / 6;
		pe->peBlue  = b * 255 / 6;
		pe->peFlags = PC_NOCOLLAPSE;
		pe++;
	    }
	}
    }
    for ( r=0; r<20; r++ ) {			// grey scale palette
	pe->peRed   = r * 255 / 20;
	pe->peGreen = r * 255 / 20;
	pe->peBlue  = r * 255 / 20;
	pe->peFlags = PC_NOCOLLAPSE;
	pe++;
    }
    hpal = CreatePalette( logPal );		// create logical palette
    delete [] logPal;

    ((QColor*)(&black))->   alloc();
    ((QColor*)(&white))->   alloc();
    ((QColor*)(&::red))->   alloc();
    ((QColor*)(&::green))-> alloc();
    ((QColor*)(&::blue))->  alloc();
}


void QColor::cleanup()
{
    if ( hpal ) {				// delete application global
	DeleteObject( hpal );			//   palette
	hpal = 0;
    }
}


uint QColor::realizePal( QWidget *widget )
{
    if ( !hpal )				// not using palette
	return 0;
    HDC hdc = GetDC( widget->winId() );
    HPALETTE hpalT = SelectPalette( hdc, hpal, FALSE );
    uint i = RealizePalette( hdc );
    UpdateColors( hdc );
    SelectPalette( hdc, hpalT, FALSE );
    ReleaseDC( widget->winId(), hdc );
    return i;
}


/*****************************************************************************
  QColor member functions
 *****************************************************************************/

QColor::QColor( QRgb rgb, uint pixel )
{
    if ( pixel == 0xffffffff )
	setRgb( rgb );
    else {
	rgbVal = rgb;
	pix    = pixel;
    }
    rgbVal |= RGB_DIRECT;
}


uint QColor::alloc()
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
    pix = rgbVal = qRgb(0,0,0);
}


void QColor::setRgb( int r, int g, int b )
{
#if defined(CHECK_RANGE)
    if ( (uint)r > 255 || (uint)g > 255 || (uint)b > 255 )
	warning( "QColor::setRgb: RGB parameter(s) out of range" );
#endif
    rgbVal = qRgb(r,g,b);
    if ( lalloc ) {
	rgbVal |= RGB_DIRTY;			// alloc later
	pix = 0;
    } else {
	alloc();				// alloc now
    }
}
