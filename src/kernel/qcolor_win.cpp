/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor_win.cpp#37 $
**
** Implementation of QColor class for Win32
**
** Created : 940112
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
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

RCSTAG("$Id: //depot/qt/main/src/kernel/qcolor_win.cpp#37 $");


/*****************************************************************************
  QColor static member functions
 *****************************************************************************/


HANDLE QColor::hpal = 0;			// application global palette

static int current_alloc_context = 0;


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
    if ( (QApplication::colorSpec() & QApplication::PrivateColor) == 0 )
	return;

    int numCols = maxColors();
    if ( numCols <= 16 || numCols > 256 )	// no need to create palette
	return;

    static struct {
	WORD	     palVersion;
	WORD	     palNumEntries;
	BYTE         palPalEntries[1024];
    } rgb8palette = {
	0x300,
	256, {
	  0,  0,  0,  0,  63,  0,  0,  0, 104,  0,  0,  0, 128,  0,  0,  0,
	171,  0,  0,  0, 200,  0,  0,  0, 229,  0,  0,  0, 255,  0,  0,  0,
	  0, 63,  0,  0,  63, 63,  0,  0, 104, 63,  0,  0, 139, 63,  0,  0,
	171, 63,  0,  0, 200, 63,  0,  0, 229, 63,  0,  0, 255, 63,  0,  0,
	  0,104,  0,  0,  63,104,  0,  0, 104,104,  0,  0, 139,104,  0,  0,
	171,104,  0,  0, 200,104,  0,  0, 229,104,  0,  0, 255,104,  0,  0,
	  0,128,  0,  0,  63,139,  0,  0, 104,139,  0,  0, 128,128,  0,  0,
	171,139,  0,  0, 200,139,  0,  0, 229,139,  0,  0, 255,139,  0,  0,
	  0,171,  0,  0,  63,171,  0,  0, 104,171,  0,  0, 139,171,  0,  0,
	171,171,  0,  0, 200,171,  0,  0, 229,171,  0,  0, 255,171,  0,  0,
	  0,200,  0,  0,  63,200,  0,  0, 104,200,  0,  0, 139,200,  0,  0,
	171,200,  0,  0, 200,200,  0,  0, 229,200,  0,  0, 255,200,  0,  0,
	  0,229,  0,  0,  63,229,  0,  0, 104,229,  0,  0, 139,229,  0,  0,
	171,229,  0,  0, 200,229,  0,  0, 229,229,  0,  0, 255,229,  0,  0,
	  0,255,  0,  0,  63,255,  0,  0, 104,255,  0,  0, 139,255,  0,  0,
	171,255,  0,  0, 200,255,  0,  0, 229,255,  0,  0, 255,255,  0,  0,
	  0,  0,128,  0,  63,  0,116,  0, 104,  0,116,  0, 128,  0,128,  0,
	171,  0,116,  0, 200,  0,116,  0, 229,  0,116,  0, 255,  0,116,  0,
	  0, 63,116,  0,  63, 63,116,  0, 104, 63,116,  0, 139, 63,116,  0,
	171, 63,116,  0, 200, 63,116,  0, 229, 63,116,  0, 255, 63,116,  0,
	  0,104,116,  0,  63,104,116,  0, 104,104,116,  0, 139,104,116,  0,
	171,104,116,  0, 200,104,116,  0, 229,104,116,  0, 255,104,116,  0,
	  0,128,128,  0,  63,139,116,  0, 104,139,116,  0, 128,128,128,  0,
	171,139,116,  0, 200,139,116,  0, 229,139,116,  0, 255,139,116,  0,
	  0,171,116,  0,  63,171,116,  0, 104,171,116,  0, 139,171,116,  0,
	171,171,116,  0, 200,171,116,  0, 229,171,116,  0, 255,171,116,  0,
	  0,200,116,  0,  63,200,116,  0, 104,200,116,  0, 139,200,116,  0,
	171,200,116,  0, 200,200,116,  0, 229,200,116,  0, 255,200,116,  0,
	  0,229,116,  0,  63,229,116,  0, 104,229,116,  0, 139,229,116,  0,
	171,229,116,  0, 200,229,116,  0, 229,229,116,  0, 255,229,116,  0,
	  0,255,116,  0,  63,255,116,  0, 104,255,116,  0, 139,255,116,  0,
	171,255,116,  0, 200,255,116,  0, 229,255,116,  0, 255,255,116,  0,
	  0,  0,191,  0,  63,  0,191,  0, 104,  0,191,  0, 139,  0,191,  0,
	171,  0,191,  0, 200,  0,191,  0, 229,  0,191,  0, 255,  0,191,  0,
	  0, 63,191,  0,  63, 63,191,  0, 104, 63,191,  0, 139, 63,191,  0,
	171, 63,191,  0, 200, 63,191,  0, 229, 63,191,  0, 255, 63,191,  0,
	  0,104,191,  0,  63,104,191,  0, 104,104,191,  0, 139,104,191,  0,
	171,104,191,  0, 200,104,191,  0, 229,104,191,  0, 255,104,191,  0,
	  0,139,191,  0,  63,139,191,  0, 104,139,191,  0, 139,139,191,  0,
	171,139,191,  0, 200,139,191,  0, 229,139,191,  0, 255,139,191,  0,
	  0,171,191,  0,  63,171,191,  0, 104,171,191,  0, 139,171,191,  0,
	160,160,164,  0, 200,171,191,  0, 229,171,191,  0, 255,171,191,  0,
	  0,200,191,  0,  63,200,191,  0, 104,200,191,  0, 139,200,191,  0,
	171,200,191,  0, 192,192,192,  0, 229,200,191,  0, 255,200,191,  0,
	  0,229,191,  0,  63,229,191,  0, 104,229,191,  0, 139,229,191,  0,
	171,229,191,  0, 192,220,192,  0, 229,229,191,  0, 255,229,191,  0,
	  0,255,191,  0,  63,255,191,  0, 104,255,191,  0, 139,255,191,  0,
	171,255,191,  0, 200,255,191,  0, 229,255,191,  0, 255,255,191,  0,
	  0,  0,255,  0,  63,  0,255,  0, 104,  0,255,  0, 139,  0,255,  0,
	171,  0,255,  0, 200,  0,255,  0, 229,  0,255,  0, 255,  0,255,  0,
	  0, 63,255,  0,  63, 63,255,  0, 104, 63,255,  0, 139, 63,255,  0,
	171, 63,255,  0, 200, 63,255,  0, 229, 63,255,  0, 255, 63,255,  0,
	  0,104,255,  0,  63,104,255,  0, 104,104,255,  0, 139,104,255,  0,
	171,104,255,  0, 200,104,255,  0, 229,104,255,  0, 255,104,255,  0,
	  0,139,255,  0,  63,139,255,  0, 104,139,255,  0, 139,139,255,  0,
	171,139,255,  0, 200,139,255,  0, 229,139,255,  0, 255,139,255,  0,
	  0,171,255,  0,  63,171,255,  0, 104,171,255,  0, 139,171,255,  0,
	171,171,255,  0, 200,171,255,  0, 229,171,255,  0, 255,171,255,  0,
	  0,200,255,  0,  63,200,255,  0, 104,200,255,  0, 139,200,255,  0,
	166,202,240,  0, 200,200,255,  0, 229,200,255,  0, 255,200,255,  0,
	  0,229,255,  0,  63,229,255,  0, 104,229,255,  0, 139,229,255,  0,
	171,229,255,  0, 200,229,255,  0, 229,229,255,  0, 255,251,240,  0,
	  0,255,255,  0,  63,255,255,  0, 104,255,255,  0, 139,255,255,  0,
	171,255,255,  0, 200,255,255,  0, 229,255,255,  0, 255,255,255,  0 } };

    hpal = CreatePalette( (LOGPALETTE*)&rgb8palette );

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
    if ( pixel == 0xffffffff ) {
	setRgb( rgb );
    } else {
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


void QColor::setSystemNamedColor( const char * )
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


int QColor::enterAllocContext()
{
    static context_seq_no = 0;
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


int QColor::currentAllocContext()
{
    return current_alloc_context;
}


void QColor::destroyAllocContext( int )
{
    init_context_stack();
}
