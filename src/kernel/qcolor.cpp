/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor.cpp#20 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcolor.cpp#20 $";
#endif


/*!
\class QColor qcolor.h
\brief The QColor class provides colors based on RGB.

A color is normally specified in terms of RGB (red,green and blue) components,
but it is also possible to convert from HSV (hue,saturation and value) or
set a color name (X-Windows color database).

There are 19 predefined global QColor objects:
\c black, \c white, \c darkGray, \c gray, \c lightGray, \c red, \c green,
\c blue, \c cyan, \c magenta, \c yellow, \c darkRed, \c darkGreen,
\c darkBlue, \c darkCyan, \c darkMagenta, \c darkYellow, \c color0 and
\c color1.

The colors \c color0 (pixel value = 0) and \c color1 (pixel value = 1) are
special colors for drawing in bitmaps.

The QColor class has an efficient, dynamic color allocation strategy.
A color is normally allocated the first time it is used (lazy allocation),
that is, whenever the pixel() function is called:

<ol>
<li> Is the pixel value valid?  If it is, just return it, otherwise,
allocate a pixel value (2).
<li> Check an internal hash table to see if we allocated an equal RGB
value earlier.  If we did, set the pixel value and return.
<li> Try to allocate the RGB value. If we succeed, we get a pixel value
which we save in the internal table with the RGB value.
Return the pixel value.
<li> The color could not be allocated.  Find the closest matching
color and save it in the internal table.
</ol>

This method seems to work well even with thousands of allocated colors.

\sa QPalette.
*/

// --------------------------------------------------------------------------
// Global colors
//

const QColor color0	( 0x00ffffff, 0 );
const QColor color1	( 0x00000000, 1 );
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

/*!
\fn QColor::QColor()
Constructs an invalid color with the RGB value (0,0,0). An invalid color
is a color that is not properly set up for the underlying window system.
*/

/*!
\fn QColor::QColor( int r, int g, int b )
Constructs a color with the RGB value (r,g,b).
*/

/*!
\fn QColor::QColor( ulong rgb, ulong pix=0xffffffff )
Constructs a color with a RGB value and a custom pixel value.

If the \e pix = 0xffffffff, then the color uses this RGB value
in a standard way.  If \e pix is something else, then the pixel value
will be set directly to \e pix (skips the standard allocation procedure).
*/

/*!
\fn QColor::QColor( const char *name )
Constructs a named color.

\sa setNamedColor().
*/

/*!
\fn QColor::QColor( const QColor &c )
Constructs a color that is a copy of \e c.
*/


bool QColor::lalloc = TRUE;			// lazy color allocation

/*!
\fn bool QColor::lazyAlloc()
Returns TRUE if colors are allocated on-demand.
The default setting is TRUE.

\sa setLazyAlloc().
*/

/*!
Enables or disables lazy color allocation.

\sa lazyAlloc().
*/

void QColor::setLazyAlloc( bool enable )	// enable/disable lazy alloc
{
    lalloc = enable;
}


/*!
\fn void QColor::alloc()
Allocates a pixel value for the current RGB value.
*/


#undef max
#undef min

/*!
Returns the current RGB value as HSV.

\arg \e *h, hue.
\arg \e *s, saturation.
\arg \e *v, value.

The hue defines the color. It is between 0 and 360 inclusive in the chromatic
case, and -1 if the color is achromatic.
The saturation and value vary between 0 and 255 inclusive.

\sa setHSV().
*/

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

/*!
Sets a HSV color value.

\arg \e h, hue (-1,0..360).
\arg \e s, saturation (0..255).
\arg \e v, value (0..255).

\sa getHSV().
*/

void QColor::setHSV( int h, int s, int v )	// set HSV value
{
    int r, g, b;
#if defined(CHECK_RANGE)
    if ( h < -1 || (uint)s > 255 || (uint)v > 255 ) {
	warning( "QColor::setHSV:  HSV parameters out of range" );
	return;
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
    setRGB( r, g, b );
}


/*!
\fn ulong QColor::getRGB() const
Returns the RGB value.
*/

/*!
Gets the red, green and blue components of the RGB value.

\sa setRGB().
*/

void QColor::getRGB( int *r, int *g, int *b ) const
{						// get RGB value
    *r = (int)(rgb & 0xff);
    *g = (int)((rgb >> 8) & 0xff);
    *b = (int)((rgb >> 16) & 0xff);
}

/*!
\fn void QColor::setRGB( int r, int g, int b )
Sets the RGB value to (\e r, \e g, \e b).

\sa getRGB().
*/

/*!
Sets the RGB value to \e rgb.

Bits 0-7 = red, bits 8-15 = green, bits 16-23 = blue.

\sa getRGB().
*/

void QColor::setRGB( ulong rgb )		// set RGB value directly
{
    int r = (int)(rgb & 0xff);
    int g = (int)((rgb >> 8) & 0xff);
    int b = (int)((rgb >> 16) & 0xff);
    setRGB( r, g, b );
}


/*!
\fn void QColor::setNamedColor( const char *name )
Sets the RGB value to that of the named color.

This function searches the X color database for the color and sets the
RGB value.  The color will be set to invalid if such a color does not
exist.
*/


/*!
\fn int QColor::red() const
Returns the red component of the RGB value.
*/

/*!
\fn int QColor::green() const
Returns the green component of the RGB value.
*/

/*!
\fn int QColor::blue() const
Returns the blue component of the RGB value.
*/


/*!
Returns a lighter (or darker) color.

Returns a lighter color if \e factor is greater than 100.
Setting \e factor to 150 returns a color that is 50% brighter.

Returns a darker color if \e factor is less than 100, equal to
dark(10000 / \e factor).

This function converts the current RGB color to HSV, multiplies V with
\e factor and converts back to RGB.
*/

QColor QColor::light( int factor ) const	// get light color
{
    if ( factor <= 0 )				// invalid lightness factor
	return *this;
    else if ( factor < 100 )			// makes color darker
	return dark( 10000/factor );
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

/*!
Returns a darker (or lighter) color.

Returns a darker color if \e factor is greater than 100.
Setting \e factor to 300 returns a color that has 
one third the brightness.

Returns a lighter color if \e factor is less than 100, equal to
light(10000 / \e factor).

This function converts the current RGB color to HSV, divides V by
\e factor and converts back to RGB.
*/

QColor QColor::dark( int factor ) const		// get dark color
{
    if ( factor <= 0 )				// invalid darkness factor
	return *this;
    else if ( factor < 100 )			// makes color lighter
	return light( 10000/factor );
    int h, s, v;
    getHSV( &h, &s, &v );
    v = (v*100)/factor;
    QColor c;
    c.setHSV( h, s, v );
    return c;
}


/*!
\fn ulong QColor::pixel() const
Returns the pixel value.
*/

/*!
\fn bool QColor::operator==( const QColor &c ) const
Returns TRUE if this color has the same RGB value as \e c,
or FALSE if they have different RGB values.
*/

/*!
\fn bool QColor::operator!=( const QColor &c ) const
Returns TRUE if this color has different RGB value from \e c,
or FALSE if they have equal RGB values.
*/


// --------------------------------------------------------------------------
// QColor stream functions
//

/*! Writes a color object to the stream.

  Format: RGB value serialized as UINT32.

  \relates QColor */

QDataStream &operator<<( QDataStream &s, const QColor &c )
{
    return s << c.getRGB();
}

/*! Reads a color object from the stream. \related QColor */

QDataStream &operator>>( QDataStream &s, QColor &c )
{
    ulong rgb;
    s >> rgb;
    c.setRGB( rgb );
    return s;
}
