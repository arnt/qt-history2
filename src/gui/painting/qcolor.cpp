/****************************************************************************
**
** Implementation of QColor class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcolor.h"
#include "qcolor_p.h"
#include "qnamespace.h"
#include "qdatastream.h"
#include "qdebug.h"

#include <stdio.h>


/*!
    \class QColor qcolor.h
    \brief The QColor class provides colors based on RGB or HSV values.

    \ingroup images
    \ingroup graphics
    \ingroup appearance

    A color is normally specified in terms of RGB (red, green and blue)
    components, but it is also possible to specify HSV (hue, saturation
    and value) or set a color name (the names are copied from from the
    X11 color database).

    In addition to the RGB value, a QColor also has a pixel value and a
    validity. The pixel value is used by the underlying window system
    to refer to a color. It can be thought of as an index into the
    display hardware's color table.

    The validity (isValid()) indicates whether the color is legal at
    all. For example, a RGB color with RGB values out of range is
    illegal. For performance reasons, QColor mostly disregards illegal
    colors. The result of using an invalid color is unspecified and
    will usually be surprising.

    There are 19 predefined QColor objects: \c white, \c black, \c
    red, \c darkRed, \c green, \c darkGreen, \c blue, \c darkBlue, \c
    cyan, \c darkCyan, \c magenta, \c darkMagenta, \c yellow, \c
    darkYellow, \c gray, \c darkGray, \c lightGray, \c color0 and \c
    color1, accessible as members of the Qt namespace (ie. \c Qt::red).

    \img qt-colors.png Qt Colors

    The colors \c color0 (zero pixel value) and \c color1 (non-zero
    pixel value) are special colors for drawing in \link QBitmap
    bitmaps\endlink. Painting with \c color0 sets the bitmap bits to 0
    (transparent, i.e. background), and painting with \c color1 sets the
    bits to 1 (opaque, i.e. foreground).

    The QColor class has an efficient, dynamic color allocation
    strategy. A color is normally allocated the first time it is used
    (lazy allocation), that is, whenever the pixel() function is called:

    \list 1
    \i Is the pixel value valid? If it is, just return it; otherwise,
    allocate a pixel value.
    \i Check an internal hash table to see if we allocated an equal RGB
    value earlier. If we did, set the pixel value and return.
    \i Try to allocate the RGB value. If we succeed, we get a pixel value
    that we save in the internal table with the RGB value.
    Return the pixel value.
    \i The color could not be allocated. Find the closest matching
    color and save it in the internal table.
    \endlist

    A color can be set by passing setNamedColor() an RGB string like
    "#112233", or a color name, e.g. "blue". The names are taken from
    X11's rgb.txt database but can also be used under Windows. To get
    a lighter or darker color use light() and dark() respectively.
    Colors can also be set using setRgb() and setHsv(). The color
    components can be accessed in one go with rgb() and hsv(), or
    individually with red(), green() and blue().

    Use maxColors() and numBitPlanes() to determine the maximum number
    of colors and the number of bit planes supported by the underlying
    window system,

    If you need to allocate many colors temporarily, for example in an
    image viewer application, enterAllocContext(), leaveAllocContext() and
    destroyAllocContext() will prove useful.

    \section1 HSV Colors

    Because many people don't know the HSV color model very well, we'll
    cover it briefly here.

    The RGB model is hardware-oriented. Its representation is close to
    what most monitors show. In contrast, HSV represents color in a way
    more suited to the human perception of color. For example, the
    relationships "stronger than", "darker than" and "the opposite of"
    are easily expressed in HSV but are much harder to express in RGB.

    HSV, like RGB, has three components:

    \list

    \i H, for hue, is either 0-359 if the color is chromatic (not
    gray), or meaningless if it is gray. It represents degrees on the
    color wheel familiar to most people. Red is 0 (degrees), green is
    120 and blue is 240.

    \i S, for saturation, is 0-255, and the bigger it is, the
    stronger the color is. Grayish colors have saturation near 0; very
    strong colors have saturation near 255.

    \i V, for value, is 0-255 and represents lightness or brightness
    of the color. 0 is black; 255 is as far from black as possible.

    \endlist

    Here are some examples: Pure red is H=0, S=255, V=255. A dark red,
    moving slightly towards the magenta, could be H=350 (equivalent to
    -10), S=255, V=180. A grayish light red could have H about 0 (say
    350-359 or 0-10), S about 50-100, and S=255.

    Qt returns a hue value of -1 for achromatic colors. If you pass a
    too-big hue value, Qt forces it into range. Hue 360 or 720 is
    treated as 0; hue 540 is treated as 180.

    \sa QPalette, QApplication::setColorSpec(),
    \link http://www.inforamp.net/~poynton/Poynton-color.html Color FAQ\endlink
*/

#if defined(Q_WS_WIN)
#define COLOR0_PIX 0x00ffffff
#define COLOR1_PIX 0
#else
#define COLOR0_PIX 0
#define COLOR1_PIX 1
#endif

/*****************************************************************************
  QColor member functions
 *****************************************************************************/

bool QColor::color_init   = false;                // color system not initialized
QColor::ColorModel QColor::colormodel = d32;

/*!
    \enum QColor::Spec

    The type of color specified, either RGB or HSV, e.g. in the
    \c{QColor::QColor(x, y, z, colorSpec)} constructor.

    \value Rgb
    \value Hsv
*/

/*!
    \fn QColor::QColor()

    Constructs an invalid color with the RGB value (0, 0, 0). An
    invalid color is a color that is not properly set up for the
    underlying window system.

    The alpha value of an invalid color is unspecified.

    \sa isValid()
*/

/*!
    \overload

    Constructs a new color with a color value of \a color.
 */
QColor::QColor(Qt::GlobalColor color)
{
#define QRGB(r, g, b) \
    ((0xff << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff))
#define QRGBA(r, g, b, a) \
    ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff)

    static const QRgb global_colors[] = {
#if defined Q_WS_X11
        // HACK: we need a way to recognize color0 and color1 uniquely, so
        // that we can use color0 and color1 with fixed pixel values on
        // all screens
        QRGBA(255, 255, 255, 1), // color0
        QRGBA( 0,   0,   0, 1), // color1
#else
        QRGB(255, 255, 255), // color0
        QRGB( 0,   0,   0), // color1
#endif
        QRGB( 0,   0,   0), // black
        QRGB(255, 255, 255), // white
        /*
         * From the "The Palette Manager: How and Why" by Ron Gery,
         * March 23, 1992, archived on MSDN:
         *
         *     The Windows system palette is broken up into two
         *     sections, one with fixed colors and one with colors
         *     that can be changed by applications. The system palette
         *     predefines 20 entries; these colors are known as the
         *     static or reserved colors and consist of the 16 colors
         *     found in the Windows version 3.0 VGA driver and 4
         *     additional colors chosen for their visual appeal.  The
         *     DEFAULT_PALETTE stock object is, as the name implies,
         *     the default palette selected into a device context (DC)
         *     and consists of these static colors. Applications can
         *     set the remaining 236 colors using the Palette Manager.
         *
         * The 20 reserved entries have indices in [0,9] and
         * [246,255]. We reuse 17 of them.
         */
        QRGB(128, 128, 128), // index 248   medium gray
        QRGB(160, 160, 164), // index 247   light gray
        QRGB(192, 192, 192), // index 7     light gray
        QRGB(255,   0,   0), // index 249   red
        QRGB( 0, 255,   0), // index 250   green
        QRGB( 0,   0, 255), // index 252   blue
        QRGB( 0, 255, 255), // index 254   cyan
        QRGB(255,   0, 255), // index 253   magenta
        QRGB(255, 255,   0), // index 251   yellow
        QRGB(128,   0,   0), // index 1     dark red
        QRGB( 0, 128,   0), // index 2     dark green
        QRGB( 0,   0, 128), // index 4     dark blue
        QRGB( 0, 128, 128), // index 6     dark cyan
        QRGB(128,   0, 128), // index 5     dark magenta
        QRGB(128, 128,   0)  // index 3     dark yellow
    };
#undef QRGB
#undef QRGBA

    setRgb(global_colors[color]);
#if !defined(Q_WS_X11)
    if (color == Qt::color0)
        setPixel(COLOR0_PIX);
    else if (color == Qt::color1)
        setPixel(COLOR1_PIX);
#endif
}


/*!
    \fn QColor::QColor(int r, int g, int b)

    Constructs a color with the RGB value \a r, \a g, \a b, in the
    same way as setRgb().

    The color is left invalid if any or the arguments are illegal.

    \sa setRgb()
*/


/*!
    Constructs a color with the RGB value \a rgb and a custom pixel
    value \a pixel.

    If \a pixel == 0xffffffff (the default), then the color uses the
    RGB value in a standard way. If \a pixel is something else, then
    the pixel value is set directly to \a pixel, skipping the normal
    allocation procedure.
*/

QColor::QColor(QRgb rgb, uint pixel)
{
    if (pixel == 0xffffffff) {
        setRgb(rgb);
    } else {
        d.argb = rgb;
        setPixel(pixel);
    }
}

void QColor::setPixel(uint pixel)
{
    switch (colormodel) {
    case d8:
        d.d8.direct = true;
        d.d8.invalid = false;
        d.d8.dirty = false;
        d.d8.pix = pixel;
        break;
    case d32:
        d.d32.pix = pixel;
        break;
    }
}


/*!
    Constructs a color with the RGB or HSV value \a x, \a y, \a z.

    The arguments are an RGB value if \a colorSpec is QColor::Rgb. \a
    x (red), \a y (green), and \a z (blue). All of them must be in the
    range 0-255.

    The arguments are an HSV value if \a colorSpec is QColor::Hsv. \a
    x (hue) must be -1 for achromatic colors and 0-359 for chromatic
    colors; \a y (saturation) and \a z (value) must both be in the
    range 0-255.

    \sa setRgb(), setHsv()
*/

QColor::QColor(int x, int y, int z, Spec colorSpec)
{
    d.d32.argb = Invalid;
    d.d32.pix = Dirt;
    if (colorSpec == Hsv)
        setHsv(x, y, z);
    else
        setRgb(x, y, z);
}


/*! \fn QColor::QColor(const QString& name)

    Constructs a named color in the same way as setNamedColor() using
    name \a name.

    The color is left invalid if \a name cannot be parsed.

    \sa setNamedColor()
*/


/*! \fn QColor::QColor(const char *name)

    Constructs a named color in the same way as setNamedColor() using
    name \a name.

    The color is left invalid if \a name cannot be parsed.

    \sa setNamedColor()
*/




/*!
  \fn QColor::QColor(const QColor &c)

    Constructs a color that is a copy of \a c.
*/


/*! \fn QColor &QColor::operator=(const QColor &c)

    Assigns a copy of the color \a c and returns a reference to this
    color.
*/


/*!
    \overload

    Assigns a copy of the color \a color and returns a reference to
    this color.
 */
QColor &QColor::operator=(Qt::GlobalColor color)
{
    return operator=(QColor(color));
}


/*!
    \fn bool QColor::isValid() const

    Returns false if the color is invalid, i.e. it was constructed using the
    default constructor; otherwise returns true.
*/

/*!
    \internal
*/
bool QColor::isDirty() const
{
    if (colormodel == d8) {
        return d.d8.dirty;
    } else {
        return d.d32.probablyDirty();
    }
}

/*!
    Returns the name of the color in the format "#RRGGBB", i.e. a "#"
    character followed by three two-digit hexadecimal numbers.

    \sa setNamedColor()
*/

QString QColor::name() const
{
#ifndef QT_NO_SPRINTF
    QString s;
    s.sprintf("#%02x%02x%02x", red(), green(), blue());
    return s;
#else
    char s[20];
    sprintf(s, "#%02x%02x%02x", red(), green(), blue());
    return QString(s);
#endif
}

/*!
    Sets the RGB value to \a name, which may be in one of these
    formats:
    \list
    \i #RGB (each of R, G and B is a single hex digit)
    \i #RRGGBB
    \i #RRRGGGBBB
    \i #RRRRGGGGBBBB
    \i A name from the X color database (rgb.txt) (e.g.
    "steelblue" or "gainsboro"). These color names also work
    under Windows.
    \endlist

    The color is invalid if \a name cannot be parsed.
*/

void QColor::setNamedColor(const QString &name)
{
    if (name.isEmpty()) {
        d.argb = 0;
        if (colormodel == d8) {
            d.d8.invalid = true;
        } else {
            d.d32.argb = Invalid;
        }
    } else if (name[0] == '#') {
        QRgb rgb;
        if (!qt_get_hex_rgb(name.latin1(), &rgb)) {
            qWarning("QColor::setNamedColor: could not parse color '%s'", name.latin1());
            invalidate();
        } else {
            setRgb(rgb);
        }
    } else {
        setSystemNamedColor(name);
    }
}


#undef max
#undef min

/*!
    Returns the current RGB value as HSV. The contents of the \a h, \a
    s and \a v pointers are set to the HSV values. If any of the three
    pointers are null, the function does nothing.

    The hue (which \a h points to) is set to -1 if the color is
    achromatic.

    \warning Colors are stored internally as RGB values, so getHSv()
    may return slightly different values to those set by setHsv().

    \sa setHsv(), rgb()
*/
void QColor::getHsv(int *h, int *s, int *v) const
{
    if (!h || !s || !v)
        return;
    int r = qRed(d.argb);
    int g = qGreen(d.argb);
    int b = qBlue(d.argb);
    uint max = r;                                // maximum RGB component
    int whatmax = 0;                                // r=>0, g=>1, b=>2
    if ((uint)g > max) {
        max = g;
        whatmax = 1;
    }
    if ((uint)b > max) {
        max = b;
        whatmax = 2;
    }
    uint min = r;                                // find minimum value
    if ((uint)g < min) min = g;
    if ((uint)b < min) min = b;
    int delta = max-min;
    *v = max;                                        // calc value
    *s = max ? (510*delta+max)/(2*max) : 0;
    if (*s == 0) {
        *h = -1;                                // undefined hue
    } else {
        switch (whatmax) {
            case 0:                                // red is max component
                if (g >= b)
                    *h = (120*(g-b)+delta)/(2*delta);
                else
                    *h = (120*(g-b+delta)+delta)/(2*delta) + 300;
                break;
            case 1:                                // green is max component
                if (b > r)
                    *h = 120 + (120*(b-r)+delta)/(2*delta);
                else
                    *h = 60 + (120*(b-r+delta)+delta)/(2*delta);
                break;
            case 2:                                // blue is max component
                if (r > g)
                    *h = 240 + (120*(r-g)+delta)/(2*delta);
                else
                    *h = 180 + (120*(r-g+delta)+delta)/(2*delta);
                break;
        }
    }
}


/*!
    Sets a HSV color value. \a h is the hue, \a s is the saturation
    and \a v is the value of the HSV color.

    If \a s or \a v are not in the range 0-255, or \a h is < -1, the
    color is not changed.

    \warning Colors are stored internally as RGB values, so getHSv()
    may return slightly different values to those set by setHsv().

    \sa getHsv(), setRgb()
*/

void QColor::setHsv(int h, int s, int v)
{
    if (h < -1 || (uint)s > 255 || (uint)v > 255) {
        qWarning("QColor::setHsv: HSV parameters out of range");
        invalidate();
        return;
    }
    int r=v, g=v, b=v;
    if (s == 0 || h == -1) {                        // achromatic case
        // Ignore
    } else {                                        // chromatic case
        if ((uint)h >= 360)
            h %= 360;
        uint f = h%60;
        h /= 60;
        uint p = (uint)(2*v*(255-s)+255)/510;
        uint q, t;
        if (h&1) {
            q = (uint)(2*v*(15300-s*f)+15300)/30600;
            switch(h) {
                case 1: r=(int)q; g=(int)v, b=(int)p; break;
                case 3: r=(int)p; g=(int)q, b=(int)v; break;
                case 5: r=(int)v; g=(int)p, b=(int)q; break;
            }
        } else {
            t = (uint)(2*v*(15300-(s*(60-f)))+15300)/30600;
            switch(h) {
                case 0: r=(int)v; g=(int)t, b=(int)p; break;
                case 2: r=(int)p; g=(int)v, b=(int)t; break;
                case 4: r=(int)t; g=(int)p, b=(int)v; break;
            }
        }
    }
    setRgb(r, g, b);
}


/*!
    \fn QRgb QColor::rgb() const

    Returns the RGB value.

    The return type \e QRgb is equivalent to \c unsigned \c int.

    For an invalid color, the alpha value of the returned color is
    unspecified.

    \sa setRgb(), getHsv(), qRed(), qBlue(), qGreen(), isValid()
*/

/*! \fn void QColor::getRgb(int *r, int *g, int *b) const

    Sets the contents pointed to by \a r, \a g and \a b to the red,
    green and blue components of the RGB value respectively. The value
    range for a component is 0..255.

    \sa rgb(), setRgb(), getHsv()
*/

/*!
    Sets the RGB value to \a r, \a g, \a b. The arguments, \a r, \a g
    and \a b must all be in the range 0..255. If any of them are
    outside the legal range, the color is not changed.

    \sa rgb(), setHsv()
*/

void QColor::setRgb(int r, int g, int b)
{
    if ((uint)r > 255 || (uint)g > 255 || (uint)b > 255) {
        qWarning("QColor::setRgb: RGB parameter(s) out of range");
        invalidate();
        return;
    }
    d.argb = qRgb(r, g, b);
    if (colormodel == d8) {
        d.d8.invalid = false;
        d.d8.direct = false;
        d.d8.dirty = true;
    } else {
        d.d32.pix = Dirt;
    }
}

/*!
    \fn void QColor::getRgba(int *r, int *g, int *b, int *a) const

    Populates \a r, \a g, and \a b with the color's RGB values,
    and, \a a with the color's alpha channel (opacity) value. All the
    values are in the range 0..255.

    \sa setRgba() setRgb() setHsv()
*/

/*!
    Sets the RGBA value to \a r, \a g, \a b, \a a. The arguments, \a
    r, \a g, \a b and \a a must all be in the range 0..255. If any of
    them are outside the legal range, the color is not changed.

    \sa getRgba(), setRgb(), setHsv()
*/

void QColor::setRgba(int r, int g, int b, int a)
{
    if ((uint)r > 255 || (uint)g > 255 || (uint)b > 255 || (uint)a > 255) {
        qWarning("QColor::setRgba: RGBA parameter(s) out of range");
        invalidate();
        return;
    }
    d.argb = qRgba(r, g, b, a);
    if (colormodel == d8) {
        d.d8.invalid = false;
        d.d8.direct = false;
        d.d8.dirty = true;
    } else {
        d.d32.pix = Dirt;
    }
}


/*!
    \overload
    Sets the RGB value to \a rgb.

    The type \e QRgb is equivalent to \c unsigned \c int.

    \sa rgb(), setHsv()
*/

void QColor::setRgb(QRgb rgb)
{
    d.argb = rgb;
    if (colormodel == d8) {
        d.d8.invalid = false;
        d.d8.direct = false;
        d.d8.dirty = true;
    } else {
        d.d32.pix = Dirt;
    }
}

/*!
    \fn int QColor::red() const

    Returns the R (red) component of the RGB value.
*/


/*!
    \fn int QColor::green() const

    Returns the G (green) component of the RGB value.
*/

/*!
    \fn int QColor::blue() const

    Returns the B (blue) component of the RGB value.
*/


/*!
    \fn int QColor::alpha() const

    Returns the alpha channel component of the RGBA value.
*/


/*!
    Returns a lighter (or darker) color, but does not change this
    object.

    Returns a lighter color if \a factor is greater than 100. Setting
    \a factor to 150 returns a color that is 50% brighter.

    Returns a darker color if \a factor is less than 100. We recommend
    using dark() for this purpose. If \a factor is 0 or negative, the
    return value is unspecified.

    (This function converts the current RGB color to HSV, multiplies V
    by \a factor, and converts the result back to RGB.)

    \sa dark()
*/

QColor QColor::light(int factor) const
{
    if (factor <= 0)                                // invalid lightness factor
        return *this;
    else if (factor < 100)                        // makes color darker
        return dark(10000/factor);

    int h, s, v;
    getHsv(&h, &s, &v);
    v = (factor*v)/100;
    if (v > 255) {                                // overflow
        s -= v-255;                                // adjust saturation
        if (s < 0)
            s = 0;
        v = 255;
    }
    QColor c;
    c.setHsv(h, s, v);
    return c;
}


/*!
    Returns a darker (or lighter) color, but does not change this
    object.

    Returns a darker color if \a factor is greater than 100. Setting
    \a factor to 300 returns a color that has one-third the
    brightness.

    Returns a lighter color if \a factor is less than 100. We
    recommend using lighter() for this purpose. If \a factor is 0 or
    negative, the return value is unspecified.

    (This function converts the current RGB color to HSV, divides V by
    \a factor and converts back to RGB.)

    \sa light()
*/

QColor QColor::dark(int factor) const
{
    if (factor <= 0)                                // invalid darkness factor
        return *this;
    else if (factor < 100)                        // makes color lighter
        return light(10000/factor);
    int h, s, v;
    getHsv(&h, &s, &v);
    v = (v*100)/factor;
    QColor c;
    c.setHsv(h, s, v);
    return c;
}


/*!
    \fn bool QColor::operator==(const QColor &c) const

    Returns true if this color has the same RGB value as \a c;
    otherwise returns false.
*/

/*!
    \fn bool QColor::operator!=(const QColor &c) const
    Returns true if this color has a different RGB value from \a c;
    otherwise returns false.
*/

/*!
    Returns the pixel value.

    This value is used by the underlying window system to refer to a
    color. It can be thought of as an index into the display
    hardware's color table, but the value is an arbitrary 32-bit
    value.

    \sa alloc()
*/
uint QColor::pixel() const
{
    if (isDirty())
        return ((QColor*)this)->alloc();
    else if (colormodel == d8)
#ifdef Q_WS_WIN
        // since d.d8.pix is uchar we have to use the PALETTEINDEX
        // macro to get the respective palette entry index.
        return (0x01000000 | (int)(short)(d.d8.pix));
#else
        return d.d8.pix;
#endif
    else
        return d.d32.pix;
}

/*!
    Returns a QStringList containing the color names Qt knows about.
*/
QStringList QColor::colorNames()
{
    return qt_get_colornames();
}

#if !defined(Q_OS_MAC) || QT_MACOSX_VERSION >= 0x1030
#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QColor &c)
{
    dbg.nospace() << "QColor(" << c.name() << ')';
    return dbg.space();
}
#endif
#endif

/*****************************************************************************
  QColor stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QColor
    Writes a color object, \a c to the stream, \a s.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QColor &c)
{
    Q_UINT32 p = (Q_UINT32)c.rgb();
    if (s.version() == 1)                        // Swap red and blue
        p = ((p << 16) & 0xff0000) | ((p >> 16) & 0xff) | (p & 0xff00ff00);
    return s << p;
}

/*!
    \relates QColor
    Reads a color object, \a c, from the stream, \a s.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QColor &c)
{
    Q_UINT32 p;
    s >> p;
    if (s.version() == 1)                        // Swap red and blue
        p = ((p << 16) & 0xff0000) | ((p >> 16) & 0xff) | (p & 0xff00ff00);
    c.setRgb(p);
    return s;
}
#endif

/*****************************************************************************
  QColor global functions (documentation only)
 *****************************************************************************/

/*!
    \fn int qRed(QRgb rgb)
    \relates QColor

    Returns the red component of the RGB triplet \a rgb.
    \sa qRgb(), QColor::red()
*/

/*!
    \fn int qGreen(QRgb rgb)
    \relates QColor

    Returns the green component of the RGB triplet \a rgb.
    \sa qRgb(), QColor::green()
*/

/*!
    \fn int qBlue(QRgb rgb)
    \relates QColor

    Returns the blue component of the RGB triplet \a rgb.
    \sa qRgb(), QColor::blue()
*/

/*!
    \fn int qAlpha(QRgb rgba)
    \relates QColor

    Returns the alpha component of the RGBA quadruplet \a rgba.
    */

/*!
    \fn QRgb qRgb(int r, int g, int b)
    \relates QColor

    Returns the RGB triplet \a (r,g,b).

    The return type QRgb is equivalent to \c unsigned \c int.

    \sa qRgba(), qRed(), qGreen(), qBlue()
*/

/*!
    \fn QRgb qRgba(int r, int g, int b, int a)
    \relates QColor

    Returns the RGBA quadruplet \a (r,g,b,a).

    The return type QRgba is equivalent to \c unsigned \c int.

    \sa qRgb(), qRed(), qGreen(), qBlue()
*/

/*!
    \fn int qGray(int r, int g, int b)
    \relates QColor

    Returns a gray value 0..255 from the (\a r, \a g, \a b) triplet.

    The gray value is calculated using the formula (r*11 + g*16 +
    b*5)/32.
*/

/*!
    \fn int qGray(qRgb rgb)
    \overload
    \relates QColor

    Returns a gray value 0..255 from the given \a rgb colour.
*/

