/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor_win.cpp#4 $
**
** Implementation of QColor class for Windows + NT
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcolor.h"
#include "qwidget.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcolor_win.cpp#4 $";
#endif


// --------------------------------------------------------------------------
// QColor special member functions
//

HANDLE QColor::hpal = 0;			// application global palette

inline ulong _RGB( uint r, uint g, uint b )
{
    return (uchar)r | ((ushort)g << 8) | ((ulong)b << 16);
}


// --------------------------------------------------------------------------
// QColor static member functions
//

void QColor::initialize()			// called from startup routines
{
  // Initialize global color objects

    ((QColor*)(&black))->      setRGB(	 0,   0,   0 );
    ((QColor*)(&white))->      setRGB( 255, 255, 255 );
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

QColor::QColor( const char *name )		// load color from database
{
    setNamedColor( name );
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
    pix = rgb = _RGB(0,0,0);
    return FALSE;
}


bool QColor::setRGB( int r, int g, int b )	// set RGB value
{
    rgb = _RGB(r,g,b);
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
