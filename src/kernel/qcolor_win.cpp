/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor_win.cpp#8 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcolor_win.cpp#8 $";
#endif


// --------------------------------------------------------------------------
// QColor special member functions
//

HANDLE QColor::hpal = 0;			// application global palette


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


void QColor::initialize()			// called from startup routines
{
#if defined(TEST_WINDOWS_PALETTE)
    if ( QWinInfo::numColors() <= 16 )		// too few colors
	return;

#define PALETTESIZE 100
    LOGPALETTE *logPal;
    logPal = (LOGPALETTE*)new char[sizeof(LOGPALETTE)+
				   sizeof(PALETTEENTRY)*PALETTESIZE];
    ASSERT( logPal );
    logPal->palVersion = 0x300;			// Windows 3.0 compatible
    logPal->palNumEntries = PALETTESIZE;
    for ( int i=0; i<PALETTESIZE; i++ ) {	// fill inn palette entries
	logPal->palPalEntry[i].peRed = 0;
	logPal->palPalEntry[i].peBlue = 0;
	logPal->palPalEntry[i].peGreen = 0;
	logPal->palPalEntry[i].peFlags = PC_RESERVED;
    }

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
	pix    = pixel;
    }
}

QColor::QColor( const char *name )		// load color from database
{
    setNamedColor( name );
}


void QColor::alloc()				// allocate color
{
    rgbVal &= RGB_MASK;
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
#if defined(TEST_WINDOWS_PALETTE)
    if ( hpal )					// JUST TESTING!!!
	pix = PALETTEINDEX(r%PALETTESIZE);
    else
	pix = rgbVal;
#else
    pix = rgbVal;
#endif
}
