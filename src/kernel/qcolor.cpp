/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor.cpp#1 $
**
** Implementation of QColor class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
** --------------------------------------------------------------------------
** This file containts the platform independent implementation of the QColor
** class.  Platform dependent functions are found in the qcol_xxx.C files.
*****************************************************************************/

#include "qcolor.h"
#include "qstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcolor.cpp#1 $";
#endif


// --------------------------------------------------------------------------
// QColor member functions
//

bool QColor::setRGB( ulong rgb )		// set RGB value directly
{
    int r = (int)(rgb & 0xff);
    int g = (int)((rgb >> 8) & 0xff);
    int b = (int)((rgb >> 16) & 0xff);
    return setRGB( r, g, b );
}


QColor QColor::light( double factor ) const	// get light color
{
    if ( factor <= 0.0 )			// invalid lightness factor
	return *this;
    else
    if ( factor < 1.0 )				// makes color darker
	return dark( 1/factor );
    int r=red(), g=green(), b=blue();
    r = (int)(r*factor);			// multiply intensities
    g = (int)(g*factor);
    b = (int)(b*factor);
    int m = r;					// find maximum value
    if ( g > m ) m = g;
    if ( b > m ) m = b;
    if ( r > 255 || g > 255 || b > 255 ) {	// handle out-of-range
	r = 255*r/m;
	r += (255 - r) >> 1;
	g = 255*g/m;
	g += (255 - g) >> 1;
	b = 255*b/m;
	b += (255 - b) >> 1;
    }
    else
    if ( r < 102 && g < 102 && b < 102 ) {	// too dark
	r = 102*r/m;
	g = 102*g/m;
	b = 102*b/m;
    }
    QColor c( r, g, b );
    return c;
}

QColor QColor::dark( double factor ) const	// get dark color
{
    if ( factor <= 0.0 )			// invalid darkness factor
	return *this;
    else
    if ( factor < 1.0 )				// makes color lighter
	return light( 1/factor );
    QColor c( (int)(red()/factor),		// divide intensities
	      (int)(green()/factor),
	      (int)(blue()/factor) );
    return c;
}


// --------------------------------------------------------------------------
// QColor stream functions
//

QStream &operator<<( QStream &s, const QColor &c )
{
    return s << c.getRGB();
}

QStream &operator>>( QStream &s, QColor &c )
{
    ulong rgb;
    s >> rgb;
    c.setRGB( rgb );
    return s;
}
