/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor.cpp#11 $
**
** Implementation of QColor class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcolor.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcolor.cpp#11 $";
#endif


// --------------------------------------------------------------------------
// Global colors
//

const QColor trueColor  (   0,   0,   0 );
const QColor falseColor ( 255, 255, 255 );
const QColor FFColor    ( 255, 255, 255 );
const QColor black	(   0,   0,   0 );
const QColor white	( 255, 255, 255 );
const QColor darkGray   ( 128, 128, 128 );
const QColor gray	( 160, 160, 160 );
const QColor lightGray	( 192, 192, 192 );
const QColor red	( 255,   0,   0 );
const QColor green	(   0, 255,   0 );
const QColor blue	(   0,   0, 255 );
const QColor cyan	(   0, 255, 255 );
const QColor magenta	( 255,   0, 255 );
const QColor yellow	( 255, 255,   0 );
const QColor darkRed	( 128,   0,   0 );
const QColor darkGreen	(   0, 128,   0 );
const QColor darkBlue   (   0,   0, 128 );
const QColor darkCyan   (   0, 128, 128 );
const QColor darkMagenta( 128,   0, 128 );
const QColor darkYellow	( 128, 128,   0 );


// --------------------------------------------------------------------------
// QColor member functions
//

bool QColor::aalloc = FALSE;			// auto-alloc colors flag

void QColor::setAutoAlloc( bool onOff )		// set auto-alloc flag
{
    aalloc = onOff;
}


#undef max
#undef min

//
// Heavily optimized integer-based RGB<-->HSV conversion functions.
// These algorithms are supposed to be efficient and accurate, and
// might not be very readable!
//

void QColor::getHSV( int *h, int *s, int *v ) const
{						// get HSV value
    int r = (int)(rgb & 0xff);
    int g = (int)((rgb >> 8) & 0xff);
    int b = (int)((rgb >> 16) & 0xff);
    uint max = r;				// maximum RGB component
    int whatmax = 0;				// r=>0, g=>1, b=>2
    if ( (uint)g > max ) {
	max = g;
	whatmax = 1;
    }
    if ( (uint)b > max ) {
	max = b;
	whatmax = 2;
    }
    uint min = r;				// find minimum value
    if ( (uint)g < min ) min = g;
    if ( (uint)b < min ) min = b;
    int delta = max-min;
    *v = max;					// calc value
    *s = max ? (int)((long)(510L*delta+max)/(2L*max)) : 0;
    if ( *s == 0 )
	*h = -1;				// undefined hue
    else {
	switch ( whatmax ) {
	    case 0:				// red is max component
	        if ( g >= b )
		    *h = (120*(g-b)+delta)/(2*delta);
		else
		    *h = (120*(g-b+delta)+delta)/(2*delta) + 300;
	        break;
	    case 1:				// green is max component
	        if ( b > r )
		    *h = 120 + (120*(b-r)+delta)/(2*delta);
		else
		    *h = 60 + (120*(b-r+delta)+delta)/(2*delta);
		break;
	    case 2:				// blue is max component
	        if ( r > g )
		    *h = 240 + (120*(r-g)+delta)/(2*delta);
		else
		    *h = 180 + (120*(r-g+delta)+delta)/(2*delta);
		break;
	}
    }
}

bool QColor::setHSV( int h, int s, int v )	// set HSV value
{
    int r, g, b;
#if defined(CHECK_RANGE)
    if ( h < -1 || (uint)s > 255 || (uint)v > 255 ) {
	warning( "QColor::setHSV:  HSV parameters out of range" );
	return FALSE;
    }
#endif
    if ( s == 0 || h == -1 )	       		// achromatic case
	r = g = b = v;
    else {					// chromatic case
	if ( (uint)h >= 360 )
	    h %= 360;
	uint f = h%60;
	h /= 60;
	uint p = (uint)((ulong)(2L*v*(255L-s)+255L)/510L);
	uint q, t;
	if ( h&1 )				// do only when necessary
	    q = (uint)((ulong)(2L*v*(15300L-s*f)+15300L)/30600L);
	else
	    t = (uint)((ulong)(2L*v*(15300L-(s*(60L-f)))+15300L)/30600L);
	switch( h ) {
	    case 0: r=(int)v; g=(int)t, b=(int)p; break;
	    case 1: r=(int)q; g=(int)v, b=(int)p; break;
	    case 2: r=(int)p; g=(int)v, b=(int)t; break;
	    case 3: r=(int)p; g=(int)q, b=(int)v; break;
	    case 4: r=(int)t; g=(int)p, b=(int)v; break;
	    case 5: r=(int)v; g=(int)p, b=(int)q; break;
	}
    }
    return setRGB( r, g, b );
}


void QColor::getRGB( int *r, int *g, int *b ) const
{						// get RGB value
    *r = (int)(rgb & 0xff);
    *g = (int)((rgb >> 8) & 0xff);
    *b = (int)((rgb >> 16) & 0xff);
}

bool QColor::setRGB( ulong rgb )		// set RGB value directly
{
    int r = (int)(rgb & 0xff);
    int g = (int)((rgb >> 8) & 0xff);
    int b = (int)((rgb >> 16) & 0xff);
    return setRGB( r, g, b );
}


//
// The trick with the light() and dark() functions is to transform the RGB
// color into HSV, multiply/divide V by a factor, and then transform back.
//

QColor QColor::light( int factor ) const	// get light color
{
    if ( factor <= 0 )				// invalid lightness factor
	return *this;
    else
    if ( factor < 100 )				// makes color darker
	return dark( 100/factor );
    int h, s, v;
    getHSV( &h, &s, &v );
    v = (int)(((long)factor*v)/100L);
    if ( v > 255 ) {				// overflow
	s -= (int)(((long)factor*106L)/100L);	// adjust saturation
	if ( s < 0 )
	    s = 0;
	v = 255;
    }
    QColor c;
    c.setHSV( h, s, v );
    return c;
}

QColor QColor::dark( int factor ) const		// get dark color
{
    if ( factor <= 0 )				// invalid darkness factor
	return *this;
    else
    if ( factor < 100 )				// makes color lighter
	return light( 100/factor );
    int h, s, v;
    getHSV( &h, &s, &v );
    v = (v*100)/factor;
    QColor c;
    c.setHSV( h, s, v );
    return c;
}


// --------------------------------------------------------------------------
// QColor stream functions
//

QDataStream &operator<<( QDataStream &s, const QColor &c )
{
    return s << c.getRGB();
}

QDataStream &operator>>( QDataStream &s, QColor &c )
{
    ulong rgb;
    s >> rgb;
    c.setRGB( rgb );
    return s;
}
