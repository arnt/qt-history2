/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor_win.cpp#60 $
**
** Implementation of QColor class for Win32
**
** Created : 940112
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcolor.h"
#include "qcolor_p.h"
#include "qapplication.h"
#include "qt_windows.h"


/*****************************************************************************
  QColor static member functions
 *****************************************************************************/

HPALETTE QColor::hpal = 0;			// application global palette

class QColorData {
public:
    QColorData()
	: pix(0), ctx(-1) {};
    QRgb pix;					// allocated pixel value
    int ctx;					// allocation context
};

//typedef QIntDict<QColorData> QColorDict;

static QColorData* palArray = 0;
//static QColorDict* palDict = 0;
static int numPalEntries = 0;

static int current_alloc_context = 0;

inline COLORREF qrgb2colorref(QRgb rgb)
{
    return RGB(qRed(rgb),qGreen(rgb),qBlue(rgb));
}

int QColor::maxColors()
{
    static int maxcols = 0;
    if ( maxcols == 0 ) {
	HDC hdc = qt_display_dc();
	if ( GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE )
	    maxcols = GetDeviceCaps( hdc, SIZEPALETTE );
	else
	    maxcols = GetDeviceCaps( hdc, NUMCOLORS );
    }
    return maxcols;
}

int QColor::numBitPlanes()
{
    static int planes = 0;
    if ( planes == 0 )
	planes = GetDeviceCaps( qt_display_dc(), BITSPIXEL );
    return planes;
}


void QColor::initialize()
{
    if ( color_init )
	return;

    color_init = TRUE;
    if ( QApplication::colorSpec() == QApplication::NormalColor )
	return;

    int numCols = maxColors();
    if ( numCols <= 16 || numCols > 256 )	// no need to create palette
	return;

    HDC dc = qt_display_dc();			// get global DC

    if ( QApplication::colorSpec() == QApplication::ManyColor ) {
	struct {
	    WORD		palVersion;
	    WORD		palNumEntries;
	    PALETTEENTRY	entries[20+216];	// System + cube
	} pal;
	pal.palVersion = 0x300;
	pal.palNumEntries = 20+216;

	// Fill with system colors
	GetSystemPaletteEntries( dc, 0, 10, pal.entries );
	GetSystemPaletteEntries( dc, 246, 10, pal.entries+10 );
	int idx = 20;

	// Make 6x6x6 color cube
	for( int ir = 0x0; ir <= 0xff; ir+=0x33 ) {
	    for( int ig = 0x0; ig <= 0xff; ig+=0x33 ) {
		for( int ib = 0x0; ib <= 0xff; ib+=0x33 ) {
		    pal.entries[idx].peRed = ir;
		    pal.entries[idx].peGreen = ig;
		    pal.entries[idx].peBlue = ib;
		    pal.entries[idx].peFlags = 0;
		    idx++;
		}
	    }
	}
	hpal = CreatePalette( (LOGPALETTE*)&pal );
    }
    else {
	// Colorspec is Custom color; will allocate on demand
	struct {
	    WORD		palVersion;
	    WORD		palNumEntries;
	    PALETTEENTRY	entries[2];		// only black & white
	} pal;
	pal.palVersion = 0x300;
	pal.palNumEntries = 2;
	
	// Make only black & white
	pal.entries[0].peRed = 0;
	pal.entries[0].peGreen = 0;
	pal.entries[0].peBlue = 0;
	pal.entries[0].peFlags = 0;
	pal.entries[1].peRed = 0xff;
	pal.entries[1].peGreen = 0xff;
	pal.entries[1].peBlue = 0xff;
	pal.entries[1].peFlags = 0;

	// Store palette in our own array
	numPalEntries = pal.palNumEntries;
	palArray = new QColorData[256];		// Maximum palette size
	for( int i = 0; i < numPalEntries; i++ ) {
	    palArray[i].pix = qRgb( pal.entries[i].peRed,
				    pal.entries[i].peGreen,
				    pal.entries[i].peBlue ) & RGB_MASK;
	    palArray[i].ctx = 0;
	}
	hpal = CreatePalette( (LOGPALETTE*)&pal );
    }
    
    ((QColor*)(&Qt::black))->alloc();
    ((QColor*)(&Qt::white))->alloc();

    SelectPalette( dc, hpal, FALSE );
    RealizePalette( dc );
}


void QColor::cleanup()
{
    if ( hpal ) {				// delete application global
	DeleteObject( hpal );			// palette
	hpal = 0;
    }
    if ( palArray ) {
	delete palArray;
	palArray = 0;
    }
    color_init = FALSE;
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

uint QColor::alloc()
{
    if ( (rgbVal & RGB_INVALID) || !color_init ) {
	rgbVal = 0;				// invalid color or state
	pix = 0;
    } else {
	rgbVal &= RGB_MASK;
	pix = qrgb2colorref( rgbVal );
	if ( hpal ) {
	    int idx = GetNearestPaletteIndex( hpal, pix );
	    pix = PALETTEINDEX( idx );
	    if ( QApplication::colorSpec() == QApplication::CustomColor ) {
		// # Should speed up this using a dict into palArray
		PALETTEENTRY fe;
		GetPaletteEntries( hpal, idx, 1, &fe );
		QRgb fc = qRgb( fe.peRed, fe.peGreen, fe.peBlue ) & RGB_MASK;
		if ( fc != rgbVal ) {	// Color not found in palette
		    // Find a free palette entry
		    bool found = FALSE;
		    for ( int i = 0; i < numPalEntries; i++ ) {
			if ( palArray[i].ctx < 0 ) {
			    found = TRUE;
			    idx = i;
			    break;
			}
		    }
		    if ( !found && numPalEntries < 256 ) {
			idx = numPalEntries;
			numPalEntries++;
			ResizePalette( hpal, numPalEntries );
			found = TRUE;
		    }
		    if ( found ) {
			// Change unused palette entry into the new color
			PALETTEENTRY ne;
			ne.peRed = qRed( rgbVal );
			ne.peGreen = qGreen( rgbVal );
			ne.peBlue = qBlue( rgbVal );
			ne.peFlags = 0;
			SetPaletteEntries( hpal, idx, 1, &ne );
			pix = PALETTEINDEX( idx );
			palArray[idx].pix = rgbVal;
			palArray[idx].ctx = current_alloc_context;
			HDC dc = qt_display_dc();
			UnrealizeObject( hpal );
			SelectPalette( dc, hpal, FALSE );
			RealizePalette( dc );
		    }
		}
		if ( idx < numPalEntries ) { 	// Sanity check
		    if ( palArray[idx].ctx < 0 )
			palArray[idx].ctx = current_alloc_context; // mark it
		    else if ( palArray[idx].ctx != current_alloc_context )
			palArray[idx].ctx = 0;	// Set it to default ctx
		}
	    }
	}
    }

    return pix;
}

void QColor::setSystemNamedColor( const QString& name )
{
    if ( !color_init ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QColor::setSystemNamedColor: Cannot perform this operation "
		 "because QApplication does not exist" );
#endif
	alloc();				// makes the color black
	return;
    }
    rgbVal = qt_get_rgb_val( name.latin1() );
    if ( lazy_alloc ) {
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
    static int context_seq_no = 0;
    init_context_stack();
    if ( context_ptr+1 == MAX_CONTEXTS ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QColor::enterAllocContext: Context stack overflow" );
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
#if defined(QT_CHECK_STATE)
	qWarning( "QColor::leaveAllocContext: Context stack underflow" );
#endif
	return;
    }
    
    current_alloc_context = context_stack[--context_ptr];
}


int QColor::currentAllocContext()
{
    return current_alloc_context;
}


void QColor::destroyAllocContext( int context )
{
    if ( !hpal || QApplication::colorSpec() != QApplication::CustomColor )
	return;

    init_context_stack();

    for ( int i = 2; i < numPalEntries; i++ ) {	  // 2: keep black & white
	switch ( context ) {
	case -2:
	    if ( palArray[i].ctx > 0 )
		palArray[i].ctx = -1;
	    break;
	case -1:
	    palArray[i].ctx = -1;
	    break;
	default:
	    if ( palArray[i].ctx == context )
		palArray[i].ctx = -1;
	break;
	}
    }
    
    //# Should reset unused entries in hpal to 0, to minimize the app's demand
    
}
