/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcol_os2.cpp#1 $
**
** Implementation of QColor class for OS/2 PM
**
** Author  : Haavard Nord
** Created : 940712
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qcolor.h"
#include "qwininfo.h"
#define	 INCL_PM
#include <os2.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcol_os2.cpp#1 $";
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
// QColor member functions
//

inline ulong _RGB( uint r, uint g, uint b )
{
    return (uchar)r | ((ushort)g << 8) | ((ulong)b << 16);
}


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
}

void QColor::cleanup()
{
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
#if defined(DEBUG)
    warning( "QColor::QColor: Named colors currently unsupported" );
#endif
    pix = rgb = _RGB(255,255,255);
}


bool QColor::setRGB( int r, int g, int b )	// set RGB value
{
    rgb = _RGB(r,g,b);
    pix = _RGB(b,g,r);				// stored in opposite order!!
    return TRUE;
}
