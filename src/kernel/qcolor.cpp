/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor.cpp#50 $
**
** Implementation of QColor class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcolor.h"
#include "qdstream.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qcolor.cpp#50 $");


/*----------------------------------------------------------------------------
  \class QColor qcolor.h
  \brief The QColor class provides colors based on RGB.
  \ingroup color

  A color is normally specified in terms of RGB (red,green and blue)
  components, but it is also possible to specify HSV (hue,saturation and
  value) or set a color name (X-Windows color database, no support yet
  on other window systems).

  In addition to the RGB value, a QColor also has a pixel value.  This
  value is used by the underlying window system to refer to a color.  It
  can be thought of as an index into the display hardware's color table.

  There are 19 predefined global QColor objects:
  \c black, \c white, \c darkGray, \c gray, \c lightGray, \c red, \c green,
  \c blue, \c cyan, \c magenta, \c yellow, \c darkRed, \c darkGreen,
  \c darkBlue, \c darkCyan, \c darkMagenta, \c darkYellow, \c color0 and
  \c color1.

  The colors \c color0 (zero pixel value) and \c color1 (non-zero pixel value)
  are special colors for drawing in \link QBitmap bitmaps\endlink.

  The QColor class has an efficient, dynamic color allocation strategy.
  A color is normally allocated the first time it is used (lazy allocation),
  that is, whenever the pixel() function is called:

  <ol>
  <li>Is the pixel value valid? If it is, just return it, otherwise,
  allocate a pixel value.
  <li>Check an internal hash table to see if we allocated an equal RGB
  value earlier. If we did, set the pixel value and return.
  <li>Try to allocate the RGB value. If we succeed, we get a pixel value
  which we save in the internal table with the RGB value.
  Return the pixel value.
  <li>The color could not be allocated. Find the closest matching
  color and save it in the internal table.
  </ol>

  \sa QPalette, QColorGroup
 ----------------------------------------------------------------------------*/

/*****************************************************************************
  Global colors
 *****************************************************************************/

#if defined(_WS_WIN_)
#define COLOR0_PIX 0
#define COLOR1_PIX 0x00ffffff
#else
#define COLOR0_PIX 0
#define COLOR1_PIX 1
#endif

const QColor color0	( 0x00ffffff, COLOR0_PIX );
const QColor color1	( 0x00000000, COLOR1_PIX );
const QColor black	(   0,	 0,   0 );
const QColor white	( 255, 255, 255 );
const QColor darkGray	( 128, 128, 128 );
const QColor gray	( 160, 160, 160 );
const QColor lightGray	( 192, 192, 192 );
const QColor red	( 255,	 0,   0 );
const QColor green	(   0, 255,   0 );
const QColor blue	(   0,	 0, 255 );
const QColor cyan	(   0, 255, 255 );
const QColor magenta	( 255,	 0, 255 );
const QColor yellow	( 255, 255,   0 );
const QColor darkRed	( 128,	 0,   0 );
const QColor darkGreen	(   0, 128,   0 );
const QColor darkBlue	(   0,	 0, 128 );
const QColor darkCyan	(   0, 128, 128 );
const QColor darkMagenta( 128,	 0, 128 );
const QColor darkYellow ( 128, 128,   0 );


/*****************************************************************************
  QColor member functions
 *****************************************************************************/

bool QColor::ginit  = FALSE;			// global color not init'ed
bool QColor::lalloc = TRUE;			// lazy color allocation

/*----------------------------------------------------------------------------
  Initializes the global colors.  This function is called if a global
  color variable is initialized before the constructors for our global
  color objects are executed.  Without this mechanism, assigning a
  color might assign an uninitialized value.

  Example:
  \code
     QColor myColor = red;			// will initialize red etc.

     int main( int argc, char **argc )
     {
     }
  \endcode
 ----------------------------------------------------------------------------*/

void QColor::initglobals()
{
    ginit = TRUE;
    ((QColor*)(&::color0))->pix = COLOR0_PIX;
    ((QColor*)(&::color1))->pix = COLOR1_PIX;
    ((QColor*)(&::color0))->rgbVal = 0x00ffffff;
    ((QColor*)(&::color1))->rgbVal = 0;
    ((QColor*)(&::black))	->setRgb(   0,	 0,   0 );
    ((QColor*)(&::white))	->setRgb( 255, 255, 255 );
    ((QColor*)(&::darkGray))	->setRgb( 128, 128, 128 );
    ((QColor*)(&::gray))	->setRgb( 160, 160, 160 );
    ((QColor*)(&::lightGray))	->setRgb( 192, 192, 192 );
    ((QColor*)(&::red))		->setRgb( 255,	 0,   0 );
    ((QColor*)(&::green))	->setRgb(   0, 255,   0 );
    ((QColor*)(&::blue))	->setRgb(   0,	0,  255 );
    ((QColor*)(&::cyan))	->setRgb(   0, 255, 255 );
    ((QColor*)(&::magenta))	->setRgb( 255,	0,  255 );
    ((QColor*)(&::yellow))	->setRgb( 255, 255,   0 );
    ((QColor*)(&::darkRed))	->setRgb( 128,	0,    0 );
    ((QColor*)(&::darkGreen))	->setRgb(   0, 128,   0 );
    ((QColor*)(&::darkBlue))	->setRgb(   0,	0,  128 );
    ((QColor*)(&::darkCyan))	->setRgb(   0, 128, 128 );
    ((QColor*)(&::darkMagenta)) ->setRgb( 128,	0,  128 );
    ((QColor*)(&::darkYellow))	->setRgb( 128, 128,   0 );
}


/*----------------------------------------------------------------------------
  \fn QColor::QColor()

  Constructs an invalid color with the RGB value (0,0,0). An invalid color
  is a color that is not properly set up for the underlying window system.

  \sa isValid()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QColor::QColor( int r, int g, int b )

  Constructs a color with the RGB value (r,g,b).

  \e r, \e g and \e b must be in the rangle 0..255.

  \sa setRgb()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Constructs a color with the RGB \e or HSV value \e (x,y,z).

  The \e (x,y,z) triplet defines an RGB value if \e colorSpec == \c
  QColor::Rgb.	\e x (red), \e y (green) and \e z (blue) must be in the
  range 0..255.

  The \e (x,y,z) triplet defines a HSV value if \e colorSpec == \c
  QColor::Hsv.	\e x (hue) must be in the range -1..360 (-1 means
  achromatic), and \e y (saturation) and \e z (value) must be in the range
  0..255.

  \sa setRgb(), setHsv()
 ----------------------------------------------------------------------------*/

QColor::QColor( int x, int y, int z, Spec colorSpec )
{
    if ( colorSpec == Hsv )
	setHsv( x, y, z );
    else
	setRgb( x, y, z );
}

/*----------------------------------------------------------------------------
  Constructs a named color, i.e. loads the color from the color database.
  \sa setNamedColor()
 ----------------------------------------------------------------------------*/

QColor::QColor( const char *name )
{
    setNamedColor( name );
}

/*----------------------------------------------------------------------------
  Constructs a color that is a copy of \e c.
 ----------------------------------------------------------------------------*/

QColor::QColor( const QColor &c )
{
    if ( !ginit )
	initglobals();
    rgbVal = c.rgbVal;
    pix	   = c.pix;
}

/*----------------------------------------------------------------------------
  Assigns a copy of the color \c and returns a reference to this color.
 ----------------------------------------------------------------------------*/

QColor &QColor::operator=( const QColor &c )
{
    if ( !ginit )
	initglobals();
    rgbVal = c.rgbVal;
    pix	   = c.pix;
    return *this;
}


/*----------------------------------------------------------------------------
  \fn bool QColor::isValid() const
  Returns TRUE if the color is invalid, i.e. it was constructed using the
  default constructor.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QColor::isDirty() const
  Returns TRUE if the color is dirty, i.e. lazy allocation is enabled and
  an RGB/HSV value has been set but not allocated.
  \sa setLazyAlloc(), alloc(), pixel()
 ----------------------------------------------------------------------------*/


#undef max
#undef min

/*----------------------------------------------------------------------------
  Returns the current RGB value as HSV.

  \arg \e *h, hue.
  \arg \e *s, saturation.
  \arg \e *v, value.

  The hue defines the color. Its range is 0..359 if the color is chromatic
  and -1 if the color is achromatic.  The saturation and value both vary
  between 0 and 255 inclusive.

  \sa setHsv(), rgb()
 ----------------------------------------------------------------------------*/

void QColor::hsv( int *h, int *s, int *v ) const
{
    int r = (int)(rgbVal & 0xff);
    int g = (int)((rgbVal >> 8) & 0xff);
    int b = (int)((rgbVal >> 16) & 0xff);
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
    *s = max ? (510*delta+max)/(2*max) : 0;
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

/*----------------------------------------------------------------------------
  Sets a HSV color value.

  \arg \e h, hue (-1,0..360).  -1 means achromatic.
  \arg \e s, saturation (0..255).
  \arg \e v, value (0..255).

  \sa hsv(), setRgb()
 ----------------------------------------------------------------------------*/

void QColor::setHsv( int h, int s, int v )
{
#if defined(CHECK_RANGE)
    if ( h < -1 || (uint)s > 255 || (uint)v > 255 ) {
	warning( "QColor::setHsv: HSV parameters out of range" );
	return;
    }
#endif
    int r=v, g=v, b=v;
    if ( s == 0 || h == -1 )			// achromatic case
	;
    else {					// chromatic case
	if ( (uint)h >= 360 )
	    h %= 360;
	uint f = h%60;
	h /= 60;
	uint p = (uint)(2*v*(255-s)+255)/510;
	uint q, t;
	if ( h&1 ) {
	    q = (uint)(2*v*(15300-s*f)+15300)/30600;
	    switch( h ) {
		case 1: r=(int)q; g=(int)v, b=(int)p; break;
		case 3: r=(int)p; g=(int)q, b=(int)v; break;
		case 5: r=(int)v; g=(int)p, b=(int)q; break;
	    }
	} else {
	    t = (uint)(2*v*(15300-(s*(60-f)))+15300)/30600;
	    switch( h ) {
		case 0: r=(int)v; g=(int)t, b=(int)p; break;
		case 2: r=(int)p; g=(int)v, b=(int)t; break;
		case 4: r=(int)t; g=(int)p, b=(int)v; break;
	    }
	}
    }
    setRgb( r, g, b );
}


/*----------------------------------------------------------------------------
  \fn QRgb QColor::rgb() const
  Returns the RGB value.

  Bits 0-7 = red, bits 8-15 = green, bits 16-23 = blue.

  The return type \e QRgb is equivalent to \c unsigned \c int.

  \sa setRgb(), hsv()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Returns the red, green and blue components of the RGB value in
  \e *r, \e *g and \e *b.
  \sa setRgb(), hsv()
 ----------------------------------------------------------------------------*/

void QColor::rgb( int *r, int *g, int *b ) const
{
    *r = (int)(rgbVal & 0xff);
    *g = (int)((rgbVal >> 8) & 0xff);
    *b = (int)((rgbVal >> 16) & 0xff);
}

/*----------------------------------------------------------------------------
  Sets the RGB value to \e rgb.

  Bits 0-7 = red, bits 8-15 = green, bits 16-23 = blue.

  The type \e QRgb is equivalent to \c unsigned \c int.

  \sa rgb(), setHsv()
 ----------------------------------------------------------------------------*/

void QColor::setRgb( QRgb rgb )
{
    int r = (int)(rgb & 0xff);
    int g = (int)((rgb >> 8) & 0xff);
    int b = (int)((rgb >> 16) & 0xff);
    setRgb( r, g, b );
}


/*----------------------------------------------------------------------------
  \fn int QColor::red() const
  Returns the red component of the RGB value.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QColor::green() const
  Returns the green component of the RGB value.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QColor::blue() const
  Returns the blue component of the RGB value.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Returns a lighter (or darker) color.

  Returns a lighter color if \e factor is greater than 100.
  Setting \e factor to 150 returns a color that is 50% brighter.

  Returns a darker color if \e factor is less than 100, equal to
  dark(10000 / \e factor).

  This function converts the current RGB color to HSV, multiplies V with
  \e factor and converts back to RGB.

  \sa dark()
 ----------------------------------------------------------------------------*/

QColor QColor::light( int factor ) const
{
    if ( factor <= 0 )				// invalid lightness factor
	return *this;
    else if ( factor < 100 )			// makes color darker
	return dark( 10000/factor );
    int h, s, v;
    hsv( &h, &s, &v );
    v = (factor*v)/100;
    if ( v > 255 ) {				// overflow
	s -= (factor*106)/100;			// adjust saturation
	if ( s < 0 )
	    s = 0;
	v = 255;
    }
    QColor c;
    c.setHsv( h, s, v );
    return c;
}

/*----------------------------------------------------------------------------
  Returns a darker (or lighter) color.

  Returns a darker color if \e factor is greater than 100.
  Setting \e factor to 300 returns a color that has
  one third the brightness.

  Returns a lighter color if \e factor is less than 100, equal to
  light(10000 / \e factor).

  This function converts the current RGB color to HSV, divides V by
  \e factor and converts back to RGB.

  \sa light()
 ----------------------------------------------------------------------------*/

QColor QColor::dark( int factor ) const
{
    if ( factor <= 0 )				// invalid darkness factor
	return *this;
    else if ( factor < 100 )			// makes color lighter
	return light( 10000/factor );
    int h, s, v;
    hsv( &h, &s, &v );
    v = (v*100)/factor;
    QColor c;
    c.setHsv( h, s, v );
    return c;
}


/*----------------------------------------------------------------------------
  \fn bool QColor::operator==( const QColor &c ) const
  Returns TRUE if this color has the same RGB value as \e c,
  or FALSE if they have different RGB values.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QColor::operator!=( const QColor &c ) const
  Returns TRUE if this color has different RGB value from \e c,
  or FALSE if they have equal RGB values.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn bool QColor::lazyAlloc()
  Returns TRUE if lazy color allocation is enabled (on-demand allocation),
  or FALSE if it is disabled (immediate allocation).
  \sa setLazyAlloc()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Enables or disables lazy color allocation.

  If lazy allocation is enabled, colors are allocated the first time they
  are used (upon calling the pixel() function).	 If lazy allocation is
  disabled, colors are allocated when they are constructed or when either
  setRgb() or setHsv() is called.

  Lazy color allocation is enabled by default.

  \sa lazyAlloc()
 ----------------------------------------------------------------------------*/

void QColor::setLazyAlloc( bool enable )
{
    lalloc = enable;
}


/*----------------------------------------------------------------------------
  \fn uint QColor::pixel() const
  Returns the pixel value.

  This value is used by the underlying window system to refer to a color.
  It can be thought of as an index into the display hardware's color table.
 ----------------------------------------------------------------------------*/


/*****************************************************************************
  QColor stream functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \relates QColor
  Writes a color object to the stream.

  Serialization format: RGB value serialized as an UINT32.
 ----------------------------------------------------------------------------*/

QDataStream &operator<<( QDataStream &s, const QColor &c )
{
    return s << (UINT32)c.rgb();
}

/*----------------------------------------------------------------------------
  \relates QColor
  Reads a color object from the stream.
 ----------------------------------------------------------------------------*/

QDataStream &operator>>( QDataStream &s, QColor &c )
{
    UINT32 rgb;
    s >> rgb;
    c.setRgb( rgb );
    return s;
}


/*****************************************************************************
  QColor global functions (documentation only)
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \fn int qRed( QRgb rgb )
  \relates QColor
  Returns the red component of the RGB triplet \e rgb.
  \sa qRgb(), QColor::red()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int qGreen( QRgb rgb )
  \relates QColor
  Returns the green component of the RGB triplet \e rgb.
  \sa qRgb(), QColor::green()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int qBlue( QRgb rgb )
  \relates QColor
  Returns the blue component of the RGB triplet \e rgb.
  \sa qRgb(), QColor::blue()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QRgb qRgb( int r, int g, int b )
  \relates QColor
  Returns the RGB triplet \e (r,g,b).

  Bits 0-7 = \e r (red), bits 8-15 = \e g (green), bits 16-23 = \e b (blue).

  The return type \e QRgb is equivalent to \c unsigned \c int.

  \sa qRed(), qGreen(), qBlue()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int qGray( int r, int g, int b )
  \relates QColor
  Returns a gray value 0..255 from the \e (r,g,b) triplet.

  The gray value is calculated using the formula:
  <code>(r*11 + g*16 + b*5)/32</code>.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \overload int qGray( qRgb rgb )
  \relates QColor
 ----------------------------------------------------------------------------*/
