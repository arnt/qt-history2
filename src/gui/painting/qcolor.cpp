/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcolor.h"
#include "qcolor_p.h"
#include "qnamespace.h"
#include "qcolormap.h"
#include "qdatastream.h"
#include "qvariant.h"
#include "qdebug.h"

#include <math.h>
#include <stdio.h>
#include <limits.h>


/*!
    \class QColor qcolor.h
    \brief The QColor class provides colors based on RGB or HSV values.

    \ingroup multimedia
    \ingroup appearance
    \mainclass

    A color is normally specified in terms of RGB (red, green, and blue)
    components, but it is also possible to specify HSV (hue, saturation,
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
    darkYellow, \c gray, \c darkGray, \c lightGray, \c Qt::color0, and
    \c Qt::color1, accessible as members of the Qt namespace
    (i.e. \c Qt::red).

    \img qt-colors.png Qt Colors

    The colors \c Qt::color0 (zero pixel value) and \c Qt::color1 (non-zero
    pixel value) are special colors for drawing in \link QBitmap
    bitmaps\endlink. Painting with \c Qt::color0 sets the bitmap bits to 0
    (transparent, i.e. background), and painting with \c Qt::color1 sets the
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
    color, and save it in the internal table.
    \endlist

    A color can be set by passing an RGB string to setNamedColor() (such
    as "#112233"), or a color name (such as "blue"). The names are taken
    from X11's rgb.txt database but can also be used under Windows. To get
    a lighter or darker color use light() and dark() respectively.
    Colors can also be set using setRgb() and setHsv(). The color
    components can be accessed in one go with rgb() and hsv(), or
    individually with red(), green(), and blue().

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
    relationships "stronger than", "darker than", and "the opposite of"
    are easily expressed in HSV but are much harder to express in RGB.

    HSV, like RGB, has three components:

    \list

    \i H, for hue, is in the range 0 to 359 if the color is chromatic (not
    gray), or meaningless if it is gray. It represents degrees on the
    color wheel familiar to most people. Red is 0 (degrees), green is
    120, and blue is 240.

    \i S, for saturation, is in the range 0 to 255, and the bigger it is,
    the stronger the color is. Grayish colors have saturation near 0; very
    strong colors have saturation near 255.

    \i V, for value, is in the range 0 to 255 and represents lightness or
    brightness of the color. 0 is black; 255 is as far from black as
    possible.

    \endlist

    Here are some examples: pure red is H=0, S=255, V=255; a dark red,
    moving slightly towards the magenta, could be H=350 (equivalent to
    -10), S=255, V=180; a grayish light red could have H about 0 (say
    350-359 or 0-10), S about 50-100, and S=255.

    Qt returns a hue value of -1 for achromatic colors. If you pass a
    hue value that is too large, Qt forces it into range. Hue 360 or 720 is
    treated as 0; hue 540 is treated as 180.

    \sa QPalette, QApplication::setColorSpec(),
    \link http://www.inforamp.net/~poynton/Poynton-color.html Color FAQ\endlink
*/


/*****************************************************************************
  QColor member functions
 *****************************************************************************/

/*!
    \enum QColor::Spec

    The type of color specified, either RGB, HSV or CMYK.

    \value Rgb
    \value Hsv
    \value Cmyk
    \value Invalid
*/

/*!
    \fn Spec QColor::spec() const

    Returns how the color was specified.
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
#if defined(Q_WS_QWS)
        QRGBA(  0,   0,   0, 255), // Qt::color0
        QRGBA(255, 255, 255, 255), // Qt::color1
#elif defined( Q_WS_X11 )
        // HACK: we need a way to recognize Qt::color0 and Qt::color1 uniquely, so
        // that we can use Qt::color0 and Qt::color1 with fixed pixel values on
        // all screens
        QRGBA(255, 255, 255, 1), // Qt::color0
        QRGBA(  0,   0,   0, 1), // Qt::color1
#else
        QRGB(255, 255, 255), // Qt::color0
        QRGB(  0,   0,   0), // Qt::color1
#endif
        QRGB(  0,   0,   0), // black
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
        QRGB(  0, 255,   0), // index 250   green
        QRGB(  0,   0, 255), // index 252   blue
        QRGB(  0, 255, 255), // index 254   cyan
        QRGB(255,   0, 255), // index 253   magenta
        QRGB(255, 255,   0), // index 251   yellow
        QRGB(128,   0,   0), // index 1     dark red
        QRGB(  0, 128,   0), // index 2     dark green
        QRGB(  0,   0, 128), // index 4     dark blue
        QRGB(  0, 128, 128), // index 6     dark cyan
        QRGB(128,   0, 128), // index 5     dark magenta
        QRGB(128, 128,   0)  // index 3     dark yellow
    };
#undef QRGB
#undef QRGBA

    setRgb(qRed(global_colors[color]),
           qGreen(global_colors[color]),
           qBlue(global_colors[color]),
           qAlpha(global_colors[color]));
}

/*!
    \fn QColor::QColor(int r, int g, int b, int a = 255)

    Constructs a color with the RGB value \a r, \a g, \a b, and the
    alpha-channel (transparency) value of \a a, in the same way as setRgb().

    The color is left invalid if any of the arguments are invalid.

    \sa setRgba()
*/

/*!
    Constructs a color with the value \a color. The alpha component is
    ignored and set to solid.
*/

QColor::QColor(QRgb color)
{
    cspec = Rgb;
    ct.argb.alpha = 0xffff;
    ct.argb.red   = qRed(color)   * 0x101;
    ct.argb.green = qGreen(color) * 0x101;
    ct.argb.blue  = qBlue(color)  * 0x101;
    ct.argb.pad   = 0;
}


/*!
    \fn QColor::QColor(const QString &name)

    Constructs a named color in the same way as setNamedColor() using
    the \a name given.

    The color is left invalid if the \a name cannot be parsed.

    \sa setNamedColor()
*/

/*!
    \fn QColor::QColor(const char *name)

    Constructs a named color in the same way as setNamedColor() using
    the \a name given.

    The color is left invalid if the \a name cannot be parsed.

    \sa setNamedColor()
*/

/*!
    \fn QColor::QColor(const QColor &color)

    Constructs a color that is a copy of \a color.
*/

/*!
    \fn bool QColor::isValid() const

    Returns true if the color is valid; otherwise returns false.

    If the color was constructed using the default constructor, false is
    returned.
*/

/*!
    Returns the name of the color in the format "#AARRGGBB"; i.e. a "#"
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
    \i #RGB (each of R, G, and B is a single hex digit)
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
        invalidate();
        return;
    }

    QByteArray n = name.toLatin1();
    if (name[0] == '#') {
        QRgb rgb;
        if (qt_get_hex_rgb(n, &rgb)) {
            setRgb(rgb);
        } else {
            qWarning("QColor::setNamedColor: could not parse color '%s'", n.constData());
            invalidate();
        }
        return;
    }

    QRgb rgb;
    if (qt_get_named_rgb(n, &rgb)) {
        setRgb(rgb);
    } else {
        qWarning("QColor::setNamedColor: unknown color name '%s'", n.constData());
        invalidate();
    }
}

/*!
    Returns a QStringList containing the color names Qt knows about.
*/
QStringList QColor::colorNames()
{
    return qt_get_colornames();
}

/*!
    \overload

    Returns the current RGB value as HSV. The contents of the \a h, \a
    s, and \a v pointers are set to the HSV values, and the contents
    of \a a is set to the alpha-channel (transparency) value. If any
    of the pointers are null, the function does nothing.

    \sa setHsv(), rgb()
*/
void QColor::getHsvF(qreal *h, qreal *s, qreal *v, qreal *a) const
{
        if (!h || !s || !v)
        return;

    if (cspec != Invalid && cspec != Hsv) {
        toHsv().getHsvF(h, s, v, a);
        return;
    }

    *h = ct.ahsv.hue == USHRT_MAX ? -1.0 : ct.ahsv.hue / 100.0;
    *s = ct.ahsv.saturation / qreal(USHRT_MAX);
    *v = ct.ahsv.value / qreal(USHRT_MAX);

    if (a)
        *a = ct.ahsv.alpha / qreal(USHRT_MAX);
}

/*!
    Returns the current RGB value as HSV. The contents of the \a h, \a
    s, and \a v pointers are set to the HSV values, and the contents
    of \a a is set to the alpha-channel (transparency) value. If any
    of the pointers are null, the function does nothing.

    The hue (which \a h points to) is set to -1 if the color is
    achromatic.

    \warning Colors are stored internally as RGB values, so getHSv()
    may return slightly different values to those set by setHsv().

    \sa setHsv(), rgb()
*/
void QColor::getHsv(int *h, int *s, int *v, int *a) const
{
    if (!h || !s || !v)
        return;

    if (cspec != Invalid && cspec != Hsv) {
        toHsv().getHsv(h, s, v, a);
        return;
    }

    *h = ct.ahsv.hue == USHRT_MAX ? -1 : ct.ahsv.hue / 100;
    *s = ct.ahsv.saturation >> 8;
    *v = ct.ahsv.value      >> 8;

    if (a)
        *a = ct.ahsv.alpha >> 8;
}

/*!
    \overload

    The value of \a s, \a v, and \a a must all be in the range
    0.0-1.0; the value of \a h must be in the range 0.0-360.0.
*/
void QColor::setHsvF(qreal h, qreal s, qreal v, qreal a)
{
    if (((h < 0.0 || h >= 360.0) && h != -1.0)
        || (s < 0.0 || s > 1.0)
        || (v < 0.0 || v > 1.0)
        || (a < 0.0 || a > 1.0)) {
        qWarning("QColor::setHsv: HSV parameters out of range");
        return;
    }

    cspec = Hsv;
    ct.ahsv.alpha      = qRound(a * USHRT_MAX);
    ct.ahsv.hue        = h == -1.0 ? USHRT_MAX : qRound(h * 100);
    ct.ahsv.saturation = qRound(s * USHRT_MAX);
    ct.ahsv.value      = qRound(v * USHRT_MAX);
    ct.ahsv.pad        = 0;
}

/*!
    Sets a HSV color value; \a h is the hue, \a s is the saturation,
    \a v is the value and \a a is the alpha component of the HSV
    color.

    If \a s, \a v or \a a are not in the range 0 to 255, or \a h is <
    -1, the color is not changed.

    \sa getHsv(), setRgb()
*/
void QColor::setHsv(int h, int s, int v, int a)
{
    if (h < -1 || (uint)s > 255 || (uint)v > 255 || (uint)a > 255) {
        qWarning("QColor::setHsv: HSV parameters out of range");
        invalidate();
        return;
    }

    cspec = Hsv;
    ct.ahsv.alpha      = a * 0x101;
    ct.ahsv.hue        = h == -1 ? USHRT_MAX : (h % 360) * 100;
    ct.ahsv.saturation = s * 0x101;
    ct.ahsv.value      = v * 0x101;
    ct.ahsv.pad        = 0;
}

/*!
    \overload

    Sets the contents pointed to by \a r, \a g, \a b, and \a a, to the
    red, green, blue, and alpha-channel (transparency) components of
    the RGB value.

    \sa rgb(), setRgb(), getHsv()
*/
void QColor::getRgbF(qreal *r, qreal *g, qreal *b, qreal *a) const
{
    if (!r || !g || !b)
        return;

    if (cspec != Invalid && cspec != Rgb) {
        toRgb().getRgbF(r, g, b, a);
        return;
    }

    *r = ct.argb.red   / qreal(USHRT_MAX);
    *g = ct.argb.green / qreal(USHRT_MAX);
    *b = ct.argb.blue  / qreal(USHRT_MAX);

    if (a)
        *a = ct.argb.alpha / qreal(USHRT_MAX);

}

/*!
    Sets the contents pointed to by \a r, \a g, \a b, and \a a, to the
    red, green, blue, and alpha-channel (transparency) components of
    the RGB value.

    \sa rgb(), setRgb(), getHsv()
*/
void QColor::getRgb(int *r, int *g, int *b, int *a) const
{
    if (!r || !g || !b)
        return;

    if (cspec != Invalid && cspec != Rgb) {
        toRgb().getRgb(r, g, b, a);
        return;
    }

    *r = ct.argb.red   >> 8;
    *g = ct.argb.green >> 8;
    *b = ct.argb.blue  >> 8;

    if (a)
        *a = ct.argb.alpha >> 8;
}

/*! \obsolete
    \fn void QColor::getRgba(int *r, int *g, int *b, int *a) const

    Populates \a r, \a g, and \a b with the color's RGB values.
    \a a is populated with the color's alpha channel (opacity) value.
    All the values are in the range 0 to 255.

    \sa setRgba() setRgb() setHsv()
*/

/*!
    \fn void QColor::setRgbF(qreal r, qreal g, qreal b, qreal a)

    Sets the color channels of this color to \a r (red), \a g (green),
    \a b (blue) and \a a (alpha, transparency).

    \overload

    All values must be in the range 0.0-1.0.
*/
void QColor::setRgbF(qreal r, qreal g, qreal b, qreal a)
{
    if (r < 0.0 || r > 1.0
        || g < 0.0 || g > 1.0
        || b < 0.0 || b > 1.0
        || a < 0.0 || a > 1.0) {
        qWarning("QColor::setRgb: RGB parameter(s) out of range");
        invalidate();
        return;
    }

    cspec = Rgb;
    ct.argb.alpha = qRound(a * USHRT_MAX);
    ct.argb.red   = qRound(r * USHRT_MAX);
    ct.argb.green = qRound(g * USHRT_MAX);
    ct.argb.blue  = qRound(b * USHRT_MAX);
    ct.argb.pad   = 0;
}

/*!
    Sets the RGB value to \a r, \a g, \a b and the alpha value to \a
    a. The arguments, \a r, \a g, \a b and \a a must all be in the
    range 0 to 255. The color becomes invalid if any of them are
    outside the legal range.

    \sa rgb(), setHsv()
*/
void QColor::setRgb(int r, int g, int b, int a)
{
    if ((uint)r > 255 || (uint)g > 255 || (uint)b > 255 || (uint)a > 255) {
        qWarning("QColor::setRgb: RGB parameter(s) out of range");
        invalidate();
        return;
    }

    cspec = Rgb;
    ct.argb.alpha = a * 0x101;
    ct.argb.red   = r * 0x101;
    ct.argb.green = g * 0x101;
    ct.argb.blue  = b * 0x101;
    ct.argb.pad   = 0;
}

/*! \obsolete
    \fn void QColor::setRgba(int r, int g, int b, int a)

    Sets the RGBA value to \a r, \a g, \a b, \a a. The arguments, \a
    r, \a g, \a b, and \a a must all be in the range 0 to 255. If any of
    them are outside the legal range, the color is not changed.

    \sa getRgba(), setRgb(), setHsv()
*/

/*!
    \fn QRgb QColor::rgba() const

    The return type \e QRgb is equivalent to \c unsigned \c int.

    For an invalid color, the alpha value of the returned color is
    unspecified.

    \sa setRgb(), getHsv(), qRed(), qBlue(), qGreen(), isValid()
*/

QRgb QColor::rgba() const
{
    if (cspec != Invalid && cspec != Rgb)
        return toRgb().rgba();
    return qRgba(ct.argb.red >> 8, ct.argb.green >> 8, ct.argb.blue >> 8, ct.argb.alpha >> 8);
}

/*!
    \fn QRgb QColor::rgb() const

    Returns the RGB value of the color. The alpha is stripped for compatibility.

    The return type \e QRgb is equivalent to \c unsigned \c int.

    For an invalid color, the alpha value of the returned color is
    unspecified.

    \sa setRgb(), getHsv(), qRed(), qBlue(), qGreen(), isValid()
*/
QRgb QColor::rgb() const
{
    if (cspec != Invalid && cspec != Rgb)
        return toRgb().rgb();
    return qRgb(ct.argb.red >> 8, ct.argb.green >> 8, ct.argb.blue >> 8);
}

/*!
    \overload
    Sets the RGB value to \a rgb, ignoring the alpha.

    The type \e QRgb is equivalent to \c unsigned \c int.

    \sa rgb(), setHsv()
*/
void QColor::setRgb(QRgb rgb)
{
    cspec = Rgb;
    ct.argb.alpha = 0xffff;
    ct.argb.red   = qRed(rgb)   * 0x101;
    ct.argb.green = qGreen(rgb) * 0x101;
    ct.argb.blue  = qBlue(rgb)  * 0x101;
    ct.argb.pad   = 0;
}

/*!
    Returns the alpha color component of this color.

    \sa alphaF() red() green() blue()
*/
int QColor::alpha() const
{ return ct.argb.alpha >> 8; }


/*!
    Sets the alpha of this color to \a alpha. Integer alpha is
    specified in the range 0-255.

*/

void QColor::setAlpha(int alpha)
{
    ct.argb.alpha = alpha * 0x101;
}

/*!
    Returns the alpha color component of this color.

    \sa alpha() redF() greenF() blueF()
*/
qreal QColor::alphaF() const
{ return ct.argb.alpha / qreal(USHRT_MAX); }

/*!
    Sets the alpha of this color to \a alpha. Qreal alpha is
    specified in the range 0-1.
*/
void QColor::setAlphaF(qreal alpha)
{
    qreal tmp = alpha * USHRT_MAX;
    ct.argb.alpha = qRound(tmp);
}


/*!
    Returns the red color component of this color.

    \sa redF() green() blue() alpha()
*/
int QColor::red() const
{
    if (cspec != Invalid && cspec != Rgb)
        return toRgb().red();
    return ct.argb.red >> 8;
}

/*!
    Sets the red color component of this color to \a red. Int
    components are specified in the range 0-255.
*/
void QColor::setRed(int red)
{
    ct.argb.red = red * 0x101;
}

/*!
    Returns the green color component of this color.

    \sa greenF() red() blue() alpha()
*/
int QColor::green() const
{
    if (cspec != Invalid && cspec != Rgb)
        return toRgb().green();
    return ct.argb.green >> 8;
}

/*!
    Sets the green color component of this color to \a green. Int
    components are specified in the range 0-255.
*/
void QColor::setGreen(int green)
{
    ct.argb.green = green * 0x101;
}


/*!
    Returns the blue color component of this color.

    \sa blueF() red() green() alpha()
*/
int QColor::blue() const
{
    if (cspec != Invalid && cspec != Rgb)
        return toRgb().blue();
    return ct.argb.blue >> 8;
}


/*!
    Sets the blue color component of this color to \a blue. Int
    components are specified in the range 0-255.
*/
void QColor::setBlue(int blue)
{
    ct.argb.blue = blue * 0x101;
}

/*!
    Returns the red color component of this color.

    \sa red() greenF() blueF() alphaF()
*/
qreal QColor::redF() const
{
    if (cspec != Invalid && cspec != Rgb)
        return toRgb().redF();
    return ct.argb.red / qreal(USHRT_MAX);
}


/*!
    Sets the red color component of this color to \a red. Float
    components are specified in the range 0-1.
*/
void QColor::setRedF(qreal red)
{
    ct.argb.red = qRound(red * USHRT_MAX);
}

/*!
    Returns the green color component of this color.

    \sa green() redF() blueF() alphaF()
*/
qreal QColor::greenF() const
{
    if (cspec != Invalid && cspec != Rgb)
        return toRgb().greenF();
    return ct.argb.green / qreal(USHRT_MAX);
}


/*!
    Sets the green color component of this color to \a green. Float
    components are specified in the range 0-1.
*/
void QColor::setGreenF(qreal green)
{
    ct.argb.green = qRound(green * USHRT_MAX);
}

/*!
    Returns the blue color component of this color.

    \sa blue() redF() greenF() alphaF()
*/
qreal QColor::blueF() const
{
    if (cspec != Invalid && cspec != Rgb)
        return toRgb().blueF();
    return ct.argb.blue / qreal(USHRT_MAX);
}

/*!
    Sets the blue color component of this color to \a blue. Float
    components are specified in the range 0-1.
*/
void QColor::setBlueF(qreal blue)
{
    ct.argb.blue = qRound(blue * USHRT_MAX);
}

/*!
    Returns the hue color component of this color.

    \sa hueF() saturation() value() alpha()
*/
int QColor::hue() const
{
    if (cspec != Invalid && cspec != Hsv)
        return toHsv().hue();
    return ct.ahsv.hue == USHRT_MAX ? -1 : ct.ahsv.hue / 100;
}

/*!
    Returns the saturation color component of this color.

    \sa saturationF() hue() value() alpha()
*/
int QColor::saturation() const
{
    if (cspec != Invalid && cspec != Hsv)
        return toHsv().saturation();
    return ct.ahsv.saturation >> 8;
}

/*!
    Returns the value color component of this color.

    \sa valueF() hue() saturation() alpha()
*/
int QColor::value() const
{
    if (cspec != Invalid && cspec != Hsv)
        return toHsv().value();
    return ct.ahsv.value >> 8;
}

/*!
    Returns the hue color component of this color.

    \sa hue() saturationF() valueF() alphaF()
*/
qreal QColor::hueF() const
{
    if (cspec != Invalid && cspec != Hsv)
        return toHsv().hueF();
    return ct.ahsv.hue == USHRT_MAX ? -1.0 : ct.ahsv.hue / 100.0;
}

/*!
    Returns the saturation color component of this color.

    \sa saturation() hueF() valueF() alphaF()
*/
qreal QColor::saturationF() const
{
    if (cspec != Invalid && cspec != Hsv)
        return toHsv().saturationF();
    return ct.ahsv.saturation / qreal(USHRT_MAX);
}

/*!
    Returns the value color component of this color.

    \sa value() hueF() saturationF() alphaF()
*/
qreal QColor::valueF() const
{
    if (cspec != Invalid && cspec != Hsv)
        return toHsv().valueF();
    return ct.ahsv.value / qreal(USHRT_MAX);
}

/*!
    Returns the cyan color component of this color.

    \sa cyanF() black() magenta() yellow() alpha()
*/
int QColor::cyan() const
{
    if (cspec != Invalid && cspec != Cmyk)
        return toCmyk().cyan();
    return ct.acmyk.cyan >> 8;
}

/*!
    Returns the magenta color component of this color.

    \sa magentaF() cyan() black() yellow() alpha()
*/
int QColor::magenta() const
{
    if (cspec != Invalid && cspec != Cmyk)
        return toCmyk().magenta();
    return ct.acmyk.magenta >> 8;
}

/*!
    Returns the yellow color component of this color.

    \sa yellowF() cyan() magenta() black() alpha()
*/
int QColor::yellow() const
{
    if (cspec != Invalid && cspec != Cmyk)
        return toCmyk().yellow();
    return ct.acmyk.yellow >> 8;
}

/*!
    Returns the black color component of this color.

    \sa blackF() cyan() magenta() yellow() alpha()
*/
int QColor::black() const
{
    if (cspec != Invalid && cspec != Cmyk)
        return toCmyk().black();
    return ct.acmyk.black >> 8;
}

/*!
    Returns the cyan color component of this color.

    \sa cyan() blackF() magentaF() yellowF() alphaF()
*/
qreal QColor::cyanF() const
{
    if (cspec != Invalid && cspec != Cmyk)
        return toCmyk().cyanF();
    return ct.acmyk.cyan / qreal(USHRT_MAX);
}

/*!
    Returns the magenta color component of this color.

    \sa magenta() cyanF() blackF() yellowF() alphaF()
*/
qreal QColor::magentaF() const
{
    if (cspec != Invalid && cspec != Cmyk)
        return toCmyk().magentaF();
    return ct.acmyk.magenta / qreal(USHRT_MAX);
}

/*!
    Returns the yellow color component of this color.

    \sa yellow() cyanF() magentaF() blackF() alphaF()
*/
qreal QColor::yellowF() const
{
    if (cspec != Invalid && cspec != Cmyk)
        return toCmyk().yellowF();
    return ct.acmyk.yellow / qreal(USHRT_MAX);
}

/*!
    Returns the black color component of this color.

    \sa black() cyanF() magentaF() yellowF() alphaF()
*/
qreal QColor::blackF() const
{
    if (cspec != Invalid && cspec != Cmyk)
        return toCmyk().blackF();
    return ct.acmyk.black / qreal(USHRT_MAX);
}

/*!
    Returns an RGB QColor based on this color.

    \sa fromRgb() toCmyk() toHsv()
*/
QColor QColor::toRgb() const
{
    if (!isValid() || cspec == Rgb)
        return *this;

    QColor color;
    color.cspec = Rgb;
    color.ct.argb.alpha = ct.argb.alpha;
    color.ct.argb.pad = 0;

    switch (cspec) {
    case Hsv:
        {
            if (ct.ahsv.saturation == 0 || ct.ahsv.hue == USHRT_MAX) {
                // achromatic case
                color.ct.argb.red = color.ct.argb.green = color.ct.argb.blue = ct.ahsv.value;
                break;
            }

            // chromatic case
            const qreal h = ct.ahsv.hue / 6000.;
            const qreal s = ct.ahsv.saturation / qreal(USHRT_MAX);
            const qreal v = ct.ahsv.value / qreal(USHRT_MAX);
            const int i = int(h);
            const qreal f = h - i;
            const qreal p = v * (1.0 - s);

            if (i & 1) {
                const qreal q = v * (1.0 - (s * f));

                switch (i) {
                case 1:
                    color.ct.argb.red   = qRound(q * USHRT_MAX);
                    color.ct.argb.green = qRound(v * USHRT_MAX);
                    color.ct.argb.blue  = qRound(p * USHRT_MAX);
                    break;
                case 3:
                    color.ct.argb.red   = qRound(p * USHRT_MAX);
                    color.ct.argb.green = qRound(q * USHRT_MAX);
                    color.ct.argb.blue  = qRound(v * USHRT_MAX);
                    break;
                case 5:
                    color.ct.argb.red   = qRound(v * USHRT_MAX);
                    color.ct.argb.green = qRound(p * USHRT_MAX);
                    color.ct.argb.blue  = qRound(q * USHRT_MAX);
                    break;
                }
            } else {
                const qreal t = v * (1.0 - (s * (1.0 - f)));

                switch (i) {
                case 0:
                    color.ct.argb.red   = qRound(v * USHRT_MAX);
                    color.ct.argb.green = qRound(t * USHRT_MAX);
                    color.ct.argb.blue  = qRound(p * USHRT_MAX);
                    break;
                case 2:
                    color.ct.argb.red   = qRound(p * USHRT_MAX);
                    color.ct.argb.green = qRound(v * USHRT_MAX);
                    color.ct.argb.blue  = qRound(t * USHRT_MAX);
                    break;
                case 4:
                    color.ct.argb.red   = qRound(t * USHRT_MAX);
                    color.ct.argb.green = qRound(p * USHRT_MAX);
                    color.ct.argb.blue  = qRound(v * USHRT_MAX);
                    break;
                }
            }
            break;
        }
    case Cmyk:
        {
            const qreal c = ct.acmyk.cyan / qreal(USHRT_MAX);
            const qreal m = ct.acmyk.magenta / qreal(USHRT_MAX);
            const qreal y = ct.acmyk.yellow / qreal(USHRT_MAX);
            const qreal k = ct.acmyk.black / qreal(USHRT_MAX);

            color.ct.argb.red   = qRound((1.0 - (c * (1.0 - k) + k)) * USHRT_MAX);
            color.ct.argb.green = qRound((1.0 - (m * (1.0 - k) + k)) * USHRT_MAX);
            color.ct.argb.blue  = qRound((1.0 - (y * (1.0 - k) + k)) * USHRT_MAX);
            break;
        }
    default:
        break;
    }

    return color;
};

/*!
    Returns an HSV QColor based on this color.

    \sa fromHsv() toCmyk() toRgb()
*/
QColor QColor::toHsv() const
{
    if (!isValid())
        return *this;

    if (cspec != Invalid && cspec != Rgb)
        return toRgb().toHsv();

    QColor color;
    color.cspec = Hsv;
    color.ct.ahsv.alpha = ct.argb.alpha;
    color.ct.ahsv.pad = 0;

    const qreal r = ct.argb.red   / qreal(USHRT_MAX);
    const qreal g = ct.argb.green / qreal(USHRT_MAX);
    const qreal b = ct.argb.blue  / qreal(USHRT_MAX);
    const qreal max = qMax(r, qMax(g, b));
    const qreal min = qMin(r, qMin(g, b));
    const qreal delta = max - min;
    color.ct.ahsv.value = qRound(max * USHRT_MAX);
    if (delta == 0.0) {
        // achromatic case, hue is undefined
        color.ct.ahsv.hue = USHRT_MAX;
        color.ct.ahsv.saturation = 0;
    } else {
        // chromatic case
        color.ct.ahsv.saturation = qRound((delta / max) * USHRT_MAX);
        if (r == max) {
            color.ct.ahsv.hue = qRound(((g - b) /delta) * 6000);
        } else if (g == max) {
            color.ct.ahsv.hue = qRound((2.0 + (b - r) / delta) * 6000);
        } else if (b == max) {
            color.ct.ahsv.hue = qRound((4.0 + (r - g) / delta) * 6000);
        } else {
            Q_ASSERT_X(false, "QColor::toHsv", "internal error");
        }
        color.ct.ahsv.hue %= 36000;
    }

    return color;
}

/*!
    Returns a CMYK QColor based on this color.

    \sa fromCmyk() toHsv() toRgb()
*/
QColor QColor::toCmyk() const
{
    QColor rgb = toRgb();
    if (!rgb.isValid())
        return rgb;

    QColor color;
    color.cspec = Cmyk;
    color.ct.acmyk.alpha = ct.argb.alpha;

    // rgb -> cmy
    const qreal r = ct.argb.red   / qreal(USHRT_MAX);
    const qreal g = ct.argb.green / qreal(USHRT_MAX);
    const qreal b = ct.argb.blue  / qreal(USHRT_MAX);
    qreal c = 1.0 - r;
    qreal m = 1.0 - g;
    qreal y = 1.0 - b;

    // cmy -> cmyk
    const qreal k = qMin(c, qMin(m, y));
    c = (c - k) / (1.0 - k);
    m = (m - k) / (1.0 - k);
    y = (y - k) / (1.0 - k);

    color.ct.acmyk.cyan    = qRound(c * USHRT_MAX);
    color.ct.acmyk.magenta = qRound(m * USHRT_MAX);
    color.ct.acmyk.yellow  = qRound(y * USHRT_MAX);
    color.ct.acmyk.black   = qRound(k * USHRT_MAX);

    return color;
}

/*!
    Returns a new QColor of the given color specification based on
    this color.
*/
QColor QColor::convertTo(QColor::Spec colorSpec) const
{
    if (colorSpec == cspec)
        return *this;
    switch (colorSpec) {
    case Rgb:
        return toRgb();
    case Hsv:
        return toHsv();
    case Cmyk:
        return toCmyk();
    case Invalid:
        break;
    }
    return QColor(); // must be invalid
}


/*!
    Creates a color from the argb value \a rgb.

    The alpha component of \a rgb is ignored. For conversion from an RGBA value
    use fromRgba().

    \sa fromRgba()
*/

QColor QColor::fromRgb(QRgb rgb)
{
    return fromRgb(qRed(rgb), qGreen(rgb), qBlue(rgb));
}


/*!
    Creates a color from the rgba value \a rgba.

    \sa fromRgb()
*/

QColor QColor::fromRgba(QRgb rgba)
{
    return fromRgb(qRed(rgba), qGreen(rgba), qBlue(rgba), qAlpha(rgba));
}

/*!
    Static convenience function that returns a QColor constructed from
    the RGB color values, \a r (red), \a g (green), \a b (blue),
    and \a a (alpha-channel, i.e. transparency).

    All the values must be in the range 0-255.

    \sa toRgb() fromCmyk() fromHsv()
*/
QColor QColor::fromRgb(int r, int g, int b, int a)
{
    if (r < 0 || r > 255
        || g < 0 || g > 255
        || b < 0 || b > 255
        || a < 0 || a > 255) {
        qWarning("QColor::fromRgb: RGB paramaters out of range");
        return QColor();
    }

    QColor color;
    color.cspec = Rgb;
    color.ct.argb.alpha = a * 0x101;
    color.ct.argb.red   = r * 0x101;
    color.ct.argb.green = g * 0x101;
    color.ct.argb.blue  = b * 0x101;
    color.ct.argb.pad   = 0;
    return color;
}

/*!
    \overload

    Static convenience function that returns a QColor constructed from
    the RGB color values, \a r (red), \a g (green), \a b (blue),
    and \a a (alpha-channel, i.e. transparency).

    All the values must be in the range 0.0-1.0.

    \sa toRgb() fromCmyk() fromHsv()
*/
QColor QColor::fromRgbF(qreal r, qreal g, qreal b, qreal a)
{
    if (r < 0.0 || r > 1.0
        || g < 0.0 || g > 1.0
        || b < 0.0 || b > 1.0
        || a < 0.0 || a > 1.0) {
        qWarning("QColor::fromRgb: RGB paramaters out of range");
        return QColor();
    }

    QColor color;
    color.cspec = Rgb;
    color.ct.argb.alpha = qRound(a * USHRT_MAX);
    color.ct.argb.red   = qRound(r * USHRT_MAX);
    color.ct.argb.green = qRound(g * USHRT_MAX);
    color.ct.argb.blue  = qRound(b * USHRT_MAX);
    color.ct.argb.pad   = 0;
    return color;
}

/*!
    Static convenience function that returns a QColor constructed from
    the HSV color values, \a h (hue), \a s (saturation), \a v (value),
    and \a a (alpha-channel, i.e. transparency).

    The value of \a s, \a v, and \a a must all be in the range
    0-255; the value of \a h must be in the range 0-360.

    \sa toHsv() fromCmyk() fromRgb()
*/
QColor QColor::fromHsv(int h, int s, int v, int a)
{
    if (((h < 0 || h >= 360) && h != -1)
        || s < 0 || s > 255
        || v < 0 || v > 255
        || a < 0 || a > 255) {
        qWarning("QColor::fromHsv: HSV parameters out of range");
        return QColor();
    }

    QColor color;
    color.cspec = Hsv;
    color.ct.ahsv.alpha      = a * 0x101;
    color.ct.ahsv.hue        = h == -1 ? USHRT_MAX : (h % 360) * 100;
    color.ct.ahsv.saturation = s * 0x101;
    color.ct.ahsv.value      = v * 0x101;
    color.ct.ahsv.pad        = 0;
    return color;
}

/*!
    \overload

    Static convenience function that returns a QColor constructed from
    the HSV color values, \a h (hue), \a s (saturation), \a v (value),
    and \a a (alpha-channel, i.e. transparency).

    The value of \a s, \a v, and \a a must all be in the range
    0.0-1.0; the value of \a h must be in the range 0.0-360.0.

    \sa toHsv() fromCmyk() fromRgb()
*/
QColor QColor::fromHsvF(qreal h, qreal s, qreal v, qreal a)
{
    if (((h < 0.0 || h >= 360.0) && h != -1.0)
        || (s < 0.0 || s > 1.0)
        || (v < 0.0 || v > 1.0)
        || (a < 0.0 || a > 1.0)) {
        qWarning("QColor::fromHsv: HSV parameters out of range");
        return QColor();
    }

    QColor color;
    color.cspec = Hsv;
    color.ct.ahsv.alpha      = qRound(a * USHRT_MAX);
    color.ct.ahsv.hue        = h == -1.0 ? USHRT_MAX : qRound(h * 100);
    color.ct.ahsv.saturation = qRound(s * USHRT_MAX);
    color.ct.ahsv.value      = qRound(v * USHRT_MAX);
    color.ct.ahsv.pad        = 0;
    return color;
}

/*!
    Sets the contents pointed to by \a c, \a m, \a y, \a k, and \a a,
    to the cyan, magenta, yellow, black, and alpha-channel
    (transparency) components of the CMYK value.

    \sa setCmyk() getRgb() getHsv()
*/
void QColor::getCmyk(int *c, int *m, int *y, int *k, int *a)
{
    if (!c || !m || !y || !k)
        return;

    if (cspec != Invalid && cspec != Cmyk) {
        toCmyk().getCmyk(c, m, y, k, a);
        return;
    }

    *c = ct.acmyk.cyan >> 8;
    *m = ct.acmyk.magenta >> 8;
    *y = ct.acmyk.yellow >> 8;
    *k = ct.acmyk.black >> 8;

    if (a)
        *a = ct.acmyk.alpha >> 8;
}

/*!
    \overload

    Sets the contents pointed to by \a c, \a m, \a y, \a k, and \a a,
    to the cyan, magenta, yellow, black, and alpha-channel
    (transparency) components of the CMYK value.

    \sa setCmyk() getRgb() getHsv()
*/
void QColor::getCmykF(qreal *c, qreal *m, qreal *y, qreal *k, qreal *a)
{
    if (!c || !m || !y || !k)
        return;

    if (cspec != Invalid && cspec != Cmyk) {
        toCmyk().getCmykF(c, m, y, k, a);
        return;
    }

    *c = ct.acmyk.cyan    / qreal(USHRT_MAX);
    *m = ct.acmyk.magenta / qreal(USHRT_MAX);
    *y = ct.acmyk.yellow  / qreal(USHRT_MAX);
    *k = ct.acmyk.black   / qreal(USHRT_MAX);

    if (a)
        *a = ct.acmyk.alpha / qreal(USHRT_MAX);
}

/*!
    Sets the color to CMYK values, \a c (cyan), \a m (magenta), \a y (yellow),
    \a k (black), and \a a (alpha-channel, i.e. transparency).

    All the values must be in the range 0-255.

    \sa getCmyk() setRgb() setHsv()
*/
void QColor::setCmyk(int c, int m, int y, int k, int a)
{
    if (c < 0 || c > 255
        || m < 0 || m > 255
        || y < 0 || y > 255
        || k < 0 || k > 255
        || a < 0 || a > 255) {
        qWarning("QColor::setCmyk: CMYK paramaters out of range");
        return;
    }

    cspec = Cmyk;
    ct.acmyk.alpha   = a * 0x101;
    ct.acmyk.cyan    = c * 0x101;
    ct.acmyk.magenta = m * 0x101;
    ct.acmyk.yellow  = y * 0x101;
    ct.acmyk.black   = k * 0x101;
}

/*!
    \overload

    Sets the color to CMYK values, \a c (cyan), \a m (magenta), \a y (yellow),
    \a k (black), and \a a (alpha-channel, i.e. transparency).

    All the values must be in the range 0.0-1.0.

    \sa getCmyk() setRgb() setHsv()
*/
void QColor::setCmykF(qreal c, qreal m, qreal y, qreal k, qreal a)
{
    if (c < 0.0 || c > 1.0
        || m < 0.0 || m > 1.0
        || y < 0.0 || y > 1.0
        || k < 0.0 || k > 1.0
        || a < 0.0 || a > 1.0) {
        qWarning("QColor::setCmyk: CMYK paramaters out of range");
        return;
    }

    cspec = Cmyk;
    ct.acmyk.alpha   = qRound(a * USHRT_MAX);
    ct.acmyk.cyan    = qRound(c * USHRT_MAX);
    ct.acmyk.magenta = qRound(m * USHRT_MAX);
    ct.acmyk.yellow  = qRound(y * USHRT_MAX);
    ct.acmyk.black   = qRound(k * USHRT_MAX);
}

/*!
    Static convenience function that returns a QColor constructed from
    the CMYK color values, \a c (cyan), \a m (magenta), \a y (yellow),
    \a k (black), and \a a (alpha-channel, i.e. transparency).

    All the values must be in the range 0-255.

    \sa toCmyk() fromHsv() fromRgb()
*/
QColor QColor::fromCmyk(int c, int m, int y, int k, int a)
{
    if (c < 0 || c > 255
        || m < 0 || m > 255
        || y < 0 || y > 255
        || k < 0 || k > 255
        || a < 0 || a > 255) {
        qWarning("QColor::fromCmyk: CMYK paramaters out of range");
        return QColor();
    }

    QColor color;
    color.cspec = Cmyk;
    color.ct.acmyk.alpha   = a * 0x101;
    color.ct.acmyk.cyan    = c * 0x101;
    color.ct.acmyk.magenta = m * 0x101;
    color.ct.acmyk.yellow  = y * 0x101;
    color.ct.acmyk.black   = k * 0x101;
    return color;
}

/*!
    \overload

    Static convenience function that returns a QColor constructed from
    the CMYK color values, \a c (cyan), \a m (magenta), \a y (yellow),
    \a k (black), and \a a (alpha-channel, i.e. transparency).

    All the values must be in the range 0.0-1.0.

    \sa toCmyk() fromHsv() fromRgb()
*/
QColor QColor::fromCmykF(qreal c, qreal m, qreal y, qreal k, qreal a)
{
    if (c < 0.0 || c > 1.0
        || m < 0.0 || m > 1.0
        || y < 0.0 || y > 1.0
        || k < 0.0 || k > 1.0
        || a < 0.0 || a > 1.0) {
        qWarning("QColor::fromCmyk: CMYK paramaters out of range");
        return QColor();
    }

    QColor color;
    color.cspec = Cmyk;
    color.ct.acmyk.alpha   = qRound(a * USHRT_MAX);
    color.ct.acmyk.cyan    = qRound(c * USHRT_MAX);
    color.ct.acmyk.magenta = qRound(m * USHRT_MAX);
    color.ct.acmyk.yellow  = qRound(y * USHRT_MAX);
    color.ct.acmyk.black   = qRound(k * USHRT_MAX);
    return color;
}

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

    QColor hsv = toHsv();
    int s = hsv.ct.ahsv.saturation;
    int v = hsv.ct.ahsv.value;

    v = (factor*v)/100;
    if (v > USHRT_MAX) {
        // overflow... adjust saturation
        s -= v - USHRT_MAX;
        if (s < 0)
            s = 0;
        v = USHRT_MAX;
    }

    hsv.ct.ahsv.saturation = s;
    hsv.ct.ahsv.value = v;

    // convert back to same color spec as original color
    return hsv.convertTo(cspec);
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

    QColor hsv = toHsv();
    hsv.ct.ahsv.value = (hsv.ct.ahsv.value * 100) / factor;

    // convert back to same color spec as original color
    return hsv.convertTo(cspec);
}

/*!
    Assigns a copy of the color \a color to this color, and returns a
    reference to it.
*/
QColor &QColor::operator=(const QColor &color)
{
    cspec = color.cspec;
    ct.argb = color.ct.argb;
    return *this;
}

/*! \overload
    Assigns a copy of the \a color and returns a reference to this color.
 */
QColor &QColor::operator=(Qt::GlobalColor color)
{
    return operator=(QColor(color));
}

/*!
    Returns true if this color has the same RGB value as the color \a
    color; otherwise returns false.
*/
bool QColor::operator==(const QColor &color) const
{
    return (cspec == color.cspec
            && ct.argb.alpha == color.ct.argb.alpha
            && ct.argb.red   == color.ct.argb.red
            && ct.argb.green == color.ct.argb.green
            && ct.argb.blue  == color.ct.argb.blue
            && ct.argb.pad   == color.ct.argb.pad);
}

/*!
    Returns true if this color has a different RGB value from the
    color \a color; otherwise returns false.
*/
bool QColor::operator!=(const QColor &color) const
{ return !operator==(color); }


/*!
   Returns the color as a QVariant
*/
QColor::operator QVariant() const
{
    extern bool qRegisterGuiVariant();
    static const bool b = qRegisterGuiVariant();
    Q_UNUSED(b)
    return QVariant(QVariant::Color, this);
}

/*! \internal

    Marks the color as invalid and sets all components to zero (alpha
    is set to fully opaque for compatibility with Qt 3).
*/
void QColor::invalidate()
{
    cspec = Invalid;
    ct.argb.alpha = USHRT_MAX;
    ct.argb.red = 0;
    ct.argb.green = 0;
    ct.argb.blue = 0;
    ct.argb.pad = 0;
}

#ifdef QT3_SUPPORT

/*!
    Returns the pixel value.

    This value is used by the underlying window system to refer to a
    color. It can be thought of as an index into the display
    hardware's color table, but the value is an arbitrary 32-bit
    value.

    The \a screen parameter is only used under X11 to specify the X11
    screen.

    \sa alloc()
*/
uint QColor::pixel(int screen) const
{
    QColormap cmap = QColormap::instance(screen);
    return cmap.pixel(*this);
}

#endif // QT3_SUPPORT

/*****************************************************************************
  QColor stream functions
 *****************************************************************************/

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QColor &c)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QColor(" << c.name() << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QColor to QDebug");
    return dbg;
    Q_UNUSED(c);
#endif
}
#endif

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QColor &color)

    \relates QColor
    Writes the \a color to the \a stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator<<(QDataStream &stream, const QColor &color)
{
    if (stream.version() < 7) {
        quint32 p = (quint32)color.rgb();
        if (stream.version() == 1) // Swap red and blue
            p = ((p << 16) & 0xff0000) | ((p >> 16) & 0xff) | (p & 0xff00ff00);
        return stream << p;
    }

    qint8   s = color.cspec;
    quint16 a = color.ct.argb.alpha;
    quint16 r = color.ct.argb.red;
    quint16 g = color.ct.argb.green;
    quint16 b = color.ct.argb.blue;
    quint16 p = color.ct.argb.pad;

    stream << s;
    stream << a;
    stream << r;
    stream << g;
    stream << b;
    stream << p;

    return stream;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QColor &color)

    \relates QColor
    Reads the \a color from the \a stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>(QDataStream &stream, QColor &color)
{
    if (stream.version() < 7) {
        quint32 p;
        stream >> p;
        if (stream.version() == 1) // Swap red and blue
            p = ((p << 16) & 0xff0000) | ((p >> 16) & 0xff) | (p & 0xff00ff00);
        color.setRgb(p);
        return stream;
    }

    qint8 s;
    quint16 a, r, g, b, p;
    stream >> s;
    stream >> a;
    stream >> r;
    stream >> g;
    stream >> b;
    stream >> p;

    color.cspec = QColor::Spec(s);
    color.ct.argb.alpha = a;
    color.ct.argb.red   = r;
    color.ct.argb.green = g;
    color.ct.argb.blue  = b;
    color.ct.argb.pad   = p;

    return stream;
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

    Returns the RGB triplet (\a{r}, \a{g}, \a{b}).

    The return type QRgb is equivalent to \c unsigned \c int.

    \sa qRgba(), qRed(), qGreen(), qBlue()
*/

/*!
    \fn QRgb qRgba(int r, int g, int b, int a)
    \relates QColor

    Returns the RGBA quadruplet (\a{r}, \a{g}, \a{b}, \a{a}).

    The return type QRgba is equivalent to \c unsigned \c int.

    \sa qRgb(), qRed(), qGreen(), qBlue()
*/

/*!
    \fn int qGray(int r, int g, int b)
    \relates QColor

    Returns a gray value (0 to 255) from the (\a r, \a g, \a b) triplet.

    The gray value is calculated using the formula (r*11 + g*16 +
    b*5)/32.
*/

/*!
    \fn int qGray(QRgb rgb)
    \overload
    \relates QColor

    Returns a gray value (0 to 255) from the given RGB triplet \a rgb.
*/

/*!
    \fn QColor::QColor(int x, int y, int z, Spec colorSpec)

    Use one of the other QColor constructors, or one of the static
    convenience functions, instead.
*/

/*!
    \fn QColor::rgb(int *r, int *g, int *b) const

    Use getRgb() instead.
*/

/*!
    \fn QColor::hsv(int *h, int *s, int *v) const

    Use getHsv() instead.
*/

/*!
    \fn QColor QColor::convertTo(Spec colorSpec) const

    Converts the color to the color format specified by \a colorSpec
*/
