/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor_win.cpp#6 $
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
#include "qwidget.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcolor_win.cpp#6 $";
#endif


// --------------------------------------------------------------------------
// QColor special member functions
//

HANDLE QColor::hpal = 0;			// application global palette


// --------------------------------------------------------------------------
// QColor static member functions
//

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

QColor::QColor( ulong r_g_b, ulong p_i_x )	// specify RGB and/or pixel
{
    if ( p_i_x == 0xffffffff )
	setRGB( r_g_b );
    else {
	rgb = r_g_b;
	pix = p_i_x;
    }
}

QColor::QColor( const char *name )		// load color from database
{
    setNamedColor( name );
}

QColor::QColor( const QColor &c )		// copy color
{
    rgb = c.rgb;
    pix = c.pix;
}

QColor::~QColor()
{
}


bool QColor::alloc()				// allocate color
{
    rgb &= RGB_MASK;
    pix = rgb;
    return TRUE;
}


bool QColor::setNamedColor( const char * )	// load color from database
{
#if defined(DEBUG)
    warning( "QColor::setNamedColor: Named colors currently unsupported" );
#endif
    pix = rgb = QRGB(0,0,0);
    return FALSE;
}


bool QColor::setRGB( int r, int g, int b )	// set RGB value
{
    rgb = QRGB(r,g,b);
#if defined(TEST_WINDOWS_PALETTE)
    if ( hpal )					// JUST TESTING!!!
	pix = PALETTEINDEX(r%PALETTESIZE);
    else
	pix = rgb;
#else
    pix = rgb;
#endif
    return TRUE;
}
