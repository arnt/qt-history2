/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.cpp#148 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes
**
** Created : 941207
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qfont.h"
#include "qfontdata_p.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qpainter.h"
#include "qpainter_p.h"

#include <qdict.h>
#include <qcache.h>
#include <qdatastream.h>
#include <qapplication.h>
#include <qcleanuphandler.h>
#include <qstringlist.h>
#include <ctype.h>
#include <limits.h>


// #define QFONTCACHE_DEBUG


// REVISED: brad
/*!
  \class QFont qfont.h

  \brief The QFont class specifies a font used for drawing text.

  \ingroup drawing
  \ingroup appearance
  \ingroup shared

  QFont, more precisely, is a collection of attributes of a font.
  When Qt needs to draw text, it will look up and load the closest
  matching installed font and draw using that.

  The most important attributes of a QFont are its family(),
  pointSize(), weight() and whether it is italic() or not.  There are
  QFont constructors that take these attributes as arguments, as shown
  in this example:

  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
        QPainter p( this );

        // times, 12pt, normal
        p.setFont( QFont( "times" ) );
        p.drawText( 10, 20, "Text1" );

        // helvetica, 18pt, normal
        p.setFont( QFont( "helvetica", 18 ) );
        p.drawText( 10, 120, "Text2" );

        // courier, 24pt, bold
        p.setFont( QFont( "courier", 24, QFont::Bold ) );
        p.drawText( 10, 220, "Text3" );

        // lucida, 36pt, bold, italic
        p.setFont( QFont( "lucida", 36, QFont::Bold, TRUE ) );
        p.drawText( 10, 320, "Text4" );
    }
  \endcode

  The default QFont constructor makes a copy of application's default
  font, QApplication::font().

  You can also change these attributes of an existing QFont object
  using functions such as setFamily(), setPointSize(), setWeight() and
  setItalic().

  There are also some less-used attributes.  setUnderline() decides
  whether the font is underlined or not; setStrikeOut() can be used to get
  overstrike (a horizontal line through the middle of the characters);
  setFixedPitch() determines whether Qt should give preference to
  fixed-pitch (also known as fixed-width) or variable-pitch fonts when it
  needs to choose an installed font; setStyleHint() can be used to offer
  more general help to the font matching algorithm, and on X11
  setRawName() can be used to bypass the entire font matching and use an
  X11 XLFD.

  Of course there is also a reader function for each of these set*()
  functions.  Note that the reader functions return the values
  previously set, \e not the attributes of the actual window system
  font that will be used for drawing.  You can get information about
  the font that will be used for drawing by using QFontInfo, but be
  aware that QFontInfo may be slow and that its results depend on what
  fonts are installed.

  In general font handling and loading are costly operations,
  especially on X11.  The QFont class contains extensive optimizations
  to make copying of QFont objects fast, and to cache the results of
  the slow window system functions it uses.

  QFont also offers a few static functions, mostly to tune the font
  matching algorithm: You can control what happens if a font's family
  isn't installed using insertSubstitution() and removeSubstitution(),
  ask what happens for a single family using substitute() and you can
  get a complete list of the fallback families using substitutions().

  cacheStatistics() offers cache effectiveness information; this is
  useful mostly for debugging.

  Finally, QApplication::setFont() allows you to set the default font.
  The default default font is chosen at application startup from a set
  of common installed fonts that support the correct character set for
  the current locale. Of course, the initialization algorithm has a
  default, too: The default default default font!

  The font matching algorithm works as follows:

  First an available font family is found. If the requested is not
  available the styleHint() is used to select a replacement family. If
  the style hint has not been set, "helvetica" will be used.

  If even the replacement family is not found, "helvetica" is searched
  for, if that too is not found Qt will search for a last resort font,
  i.e.  a specific font to match to, ignoring the attribute
  settings. Qt searches through a built-in list of very common
  fonts. If none of these are available, Qt gives you an error message
  and aborts (of course this only happens if you are using fonts and
  Qt \e has to load a font). We have not been able to find a case
  where this happens. Please <a href="bughowto.html">report it as a
  bug</a> if it does, preferably with a list of the fonts you have
  installed.

  The following attributes are then matched, in order of priority: <ol>
  <li> charSet()
  <li> fixedPitch()
  <li> pointSize() (see below)
  <li> weight()
  <li> italic()
  </ol>

  If, for example, a font with the correct character set is found, but
  with all other attributes in the list unmatched, it will be chosen
  before a font with the wrong character set but with all other
  attributes correct.

  The point size is defined to match if it is within 20% of the
  requested point size. Of course, when several fonts match and only
  point size differs the closest point size to the one requested will
  be chosen.

  For more general information on fonts, see the
  <a href="http://www.nwalsh.com/comp.fonts/FAQ/">comp.fonts FAQ</a>
  and for more general information on encodings, see
  <a href="http://czyborra.com/charsets/">Roman Czyborra's page</a>
  about that.

  \sa QFontMetrics QFontInfo QApplication::setFont()
  QWidget::setFont() QPainter::setFont() QFont::StyleHint
  QFont::CharSet QFont::Weight
*/

/*! \enum QFont::CharSet

  The following character set encodings are available: <ul>
  <li> \c QFont::ISO_8859_1 - Latin1 , common in much of Europe
  <li> \c QFont::ISO_8859_2 - Latin2, Central and Eastern European character set
  <li> \c QFont::ISO_8859_3 - Latin3, less common European character set
  <li> \c QFont::ISO_8859_4 - Latin4, less common European character set
  <li> \c QFont::ISO_8859_5, Cyrillic
  <li> \c QFont::ISO_8859_6, Arabic
  <li> \c QFont::ISO_8859_7, Greek
  <li> \c QFont::ISO_8859_8, Hebrew
  <li> \c QFont::ISO_8859_9, Turkish
  <li> \c QFont::ISO_8859_10..15, other ISO 8859 characters sets
  <li> \c QFont::KOI8R - KOI8-R, Cyrillic, defined in
       <a href="ftp://ftp.nordu.net/rfc/rfc1489.txt">RFC 1489.</a>
   <li> \c QFont::KOI8U - KOI8-U, Cyrillic/Ukrainian, defined in
       <a href="ftp://ftp.nordu.net/rfc/rfc2319.txt">RFC 2319.</a>
   <li> \c QFont::AnyCharSet - whatever is handiest.
  <li> \c QFont::Set_Ja, Japanese
  <li> \c QFont::Set_Ko, Korean
  <li> \c QFont::Set_Th_TH
  <li> \c QFont::Set_Zh
  <li> \c QFont::Set_Zh_TW
  <li> \c QFont::Unicode, Unicode character set
  <li> \c QFont::Set_GBK
  <li> \c QFont::Set_Big5
  </ul>

*/


/*!
  \internal
  Constructs a font that gets by default a
  \link shclass.html deep copy \endlink of \e data.
  If \a deep is false, the copy will be shallow.
*/
QFont::QFont( QFontPrivate *data, bool deep )
{
    if ( deep ) {
	d = new QFontPrivate( *data );
	Q_CHECK_PTR( d );

	// now a single reference
	d->count = 1;
    } else {
	d = data;
	d->ref();
    }
}


/*!
  \internal
  Detaches the font object from common font data.
*/
void QFont::detach()
{
    if (d->count != 1) {
	*this = QFont(d);
    }
}


/*!
  Constructs a font object that refers to the default font.

  \sa QApplication::setFont(), QApplication::font()
*/
QFont::QFont()
{
    const QFont appfont = QApplication::font();
    d = appfont.d;
    d->ref();
}


/*!
  Constructs a font object with the specified \e family, \e pointSize,
  \e weight and \e italic settings.  The charSet() is copied from the
  \link QApplication::font() default font \endlink and the rest of the
  settings are set reasonably.

  If \e pointSize is less than or equal to 0 it is set to 1.

  \sa setFamily(), setPointSize(), setWeight(), setItalic()
*/
QFont::QFont( const QString &family, int pointSize, int weight, bool italic )
{
    if (pointSize <= 0) pointSize = 1;

    d = new QFontPrivate;
    Q_CHECK_PTR( d );

    d->request.family = family;
    d->request.pointSize = pointSize * 10;
    d->request.pixelSize = pixelSize();
    d->request.weight = weight;
    d->request.italic = italic;
}


/*! \obsolete
  Constructs a font object with the specified family, pointSize, weight,
  italic and charSet settings. If pointSize is less than or equal to 0 it
  is set to 1.
 */
QFont::QFont(const QString &family, int pointSize, int weight, bool italic, CharSet)
{
    if (pointSize <= 0) pointSize = 1;

    d = new QFontPrivate;
    Q_CHECK_PTR( d );

    d->request.family = family;
    d->request.pointSize = pointSize * 10;
    d->request.pixelSize = pixelSize();
    d->request.weight = weight;
    d->request.italic = italic;
}


/*!
  Constructs a font that is a copy of \e font.
*/
QFont::QFont( const QFont &font )
{
    d = font.d;
    d->ref();
}

/*!
  Destructs the font object.
*/
QFont::~QFont()
{
    if ( d->deref() ) {
	delete d;
    }
}


/*!
  Assigns \e font to this font and returns a reference to this font.
*/
QFont &QFont::operator=( const QFont &font )
{
    if (d->deref()) delete d;

    d = font.d;
    d->ref();

    return *this;
}


/*!
  Returns the family name set by setFamily().

  Use QFontInfo to find the family name of the window system font that
  is actually used for drawing.

  Example:
  \code
    QFont     font( "Nairobi" );
    QFontInfo info( font );
    qDebug( "Font family requested is    : \"%s\"", font.family() );
    qDebug( "Font family actually used is: \"%s\"", info.family() );
  \endcode
  \sa setFamily(), substitute()
*/
QString QFont::family() const
{
    return d->request.family;
}


/*!
  Sets the family name of the font (e.g. "Helvetica" or "times").

  The family name is case insensitive.

  If the family is not available a default family is used.

  \sa family(), setStyleHint(), QFontInfo
*/
void QFont::setFamily( const QString &family )
{
    if (d->request.family == family) return;

    detach();
    d->request.family = family;
    d->request.dirty  = TRUE;
}


/*!
  Returns the point size in 1/10ths of a point.
  \sa pointSize()
  */
int QFont::deciPointSize() const
{
    return d->request.pointSize;
}


/*!
  Returns the point size set by setPointSize().

  Use QFontInfo to find the point size of the window system font
  actually used.

  Example of use:
  \code
    QFont     font( "helvetica" );
    QFontInfo info( font );
    font.setPointSize( 53 );
    qDebug( "Font size requested is    : %d", font.pointSize() );
    qDebug( "Font size actually used is: %d", info.pointSize() );
  \endcode

  \sa setPointSize() deciPointSize()
*/
int QFont::pointSize() const
{
    return d->request.pointSize / 10;
}


/*!
  Sets the point size to \a pointSize. The point size must be greater
  than zero.

  Example:
  \code
    QFont font( "courier" );
    font.setPointSize( 18 );
  \endcode

  \sa pointSize(), QFontInfo
*/
void QFont::setPointSize( int pointSize )
{
    if ( pointSize <= 0 ) {

#if defined(CHECK_RANGE)
	qWarning( "QFont::setPointSize: Point size <= 0 (%d)", pointSize );
#endif

	return;
    }

    pointSize *= 10;
    if (d->request.pointSize == pointSize) return;

    detach();
    d->request.pointSize = (short) pointSize;
    d->request.pixelSize = pixelSize();
    d->request.dirty = TRUE;
}


/*!
  Sets the point size to \a pointSize. The point size must be greater
  than zero. The requested precision may not be achieved on all platforms.
*/
void QFont::setPointSizeFloat( float pointSize )
{
    if ( pointSize <= 0 ) {

#if defined(CHECK_RANGE)
	qWarning( "QFont::setPointSize: Point size <= 0 (%f)", pointSize );
#endif

	return;
    }

    int ps = int(pointSize * 10.0 + 0.5);
    if (d->request.pointSize == ps) return;

    detach();
    d->request.pointSize = (short) ps;
    d->request.pixelSize = pixelSize();
    d->request.dirty = TRUE;
}


/*!
  Returns the height of characters in the font in points (1/72 inch).

  \sa pointSize()
*/
float QFont::pointSizeFloat() const
{
    return float(d->request.pointSize) / 10.0;
}


/*!
  Sets the logical height of characters in the font if shown on
  the screen.
*/
void QFont::setPixelSize( int pixelSize )
{
    setPixelSizeFloat(float(pixelSize));
}


/*!
  Returns the value set by setItalic().

  Use QFontInfo to find the italic value of the window system font actually
  used.
  \sa setItalic()
*/
bool QFont::italic() const
{
    return d->request.italic;
}


/*!
  Sets italic on or off.

  \sa italic(), QFontInfo
*/
void QFont::setItalic( bool enable )
{

    if ((bool) d->request.italic == enable) return;

    detach();
    d->request.italic = enable;
    d->request.dirty = TRUE;
}


/*!
  Returns the weight set by setWeight().

  Use QFontInfo to find the weight of the window system font actually used.
  \sa setWeight(), QFontInfo
*/
int QFont::weight() const
{
    return d->request.weight;
}


/*!
  \enum QFont::Weight

  Contains the predefined font weights:<ul>
  <li> \c QFont::Light (25)
  <li> \c QFont::Normal (50)
  <li> \c QFont::DemiBold (63)
  <li> \c QFont::Bold (75)
  <li> \c QFont::Black (87)
  </ul>
*/

/*!
  Sets the weight (or boldness), which should be a value
  from the QFont::Weight enumeration.

  Example:
  \code
    QFont font( "courier" );
    font.setWeight( QFont::Bold );
  \endcode

  Strictly speaking you can use all values in the range [0,99] (where
  0 is ultralight and 99 is extremely black), but there is perhaps
  asking too much of the underlying window system.

  If the specified weight is not available the closest available will
  be used. Use QFontInfo to check the actual weight.

  \sa weight(), QFontInfo
*/
void QFont::setWeight( int weight )
{
    if ( weight < 0 || weight > 99 ) {

#if defined(CHECK_RANGE)
	qWarning( "QFont::setWeight: Value out of range (%d)", weight );
#endif

	return;
    }

    if ((int) d->request.weight == weight) return;

    detach();
    d->request.weight = weight;
    d->request.dirty = TRUE;
}


/*!
  \fn bool QFont::bold() const

  Returns TRUE if weight() is a value greater than \c QFont::Normal,
  otherwise FALSE.

  \sa weight(), setBold(), QFontInfo::bold()
*/

/*!
  \fn void QFont::setBold( bool enable )

  Sets the weight to \c QFont::Bold if \e enable is TRUE, or to
  \c QFont::Normal if \e enable is FALSE.

  Use setWeight() to set the weight to other values.

  \sa bold(), setWeight()
*/


/*!
  Returns the value set by setUnderline().

  Use QFontInfo to find the underline value of the window system font
  actually used for drawing.

  \sa setUnderline(), QFontInfo::underline()
*/
bool QFont::underline() const
{
    return d->request.underline;
}


/*!
  Sets underline on or off.

  \sa underline(), QFontInfo
*/
void QFont::setUnderline( bool enable )
{
    if ((bool) d->request.underline == enable) return;

    detach();
    d->request.underline = enable;
    d->request.dirty = TRUE;
}


/*!
  Returns the value set by setStrikeOut().

  Use QFontInfo to find the strike out value of the window system font
  actually used.

  \sa setStrikeOut(), QFontInfo::strikeOut().
*/
bool QFont::strikeOut() const
{
    return d->request.strikeOut;
}


/*!
  Sets strike out on or off.

  \sa strikeOut(), QFontInfo
*/
void QFont::setStrikeOut( bool enable )
{
    if ((bool) d->request.strikeOut == enable) return;

    detach();
    d->request.strikeOut = enable;
    d->request.dirty = TRUE;
}


/*!
  Returns the value set by setFixedPitch().

  Use QFontInfo to find the fixed pitch value of the window system font
  actually used.
  \sa setFixedPitch(), QFontInfo::fixedPitch()
*/
bool QFont::fixedPitch() const
{
    return d->request.fixedPitch;
}


/*!
  Sets fixed pitch on or off.

  A fixed pitch font is a font where all characters have the same width.

  \sa fixedPitch(), QFontInfo
*/
void QFont::setFixedPitch( bool enable )
{
    if ((bool) d->request.fixedPitch == enable) return;

    detach();
    d->request.fixedPitch = enable;
    d->request.dirty = TRUE;
}


/*!
  Returns the StyleStratgie set by setStyleHint()

  \sa setStyleHint()
*/
QFont::StyleStrategy QFont::styleStrategy() const
{
    return (StyleStrategy) d->request.styleStrategy;
}


/*!
  Returns the StyleHint set by setStyleHint().

  \sa setStyleHint(), QFontInfo::styleHint()
*/
QFont::StyleHint QFont::styleHint() const
{
    return (StyleHint) d->request.styleHint;
}


/*!
  \enum QFont::StyleHint

  Style hints are used by the font matching algorithm when a selected
  font family cannot be found and is used to find an appropriate
  default family.

  The style hint value of \c AnyStyle leaves the task of finding a
  good default family to the font matching algorithm.

  The other available style hints are \c QFont::SansSerif, \c
  QFont::TypeWriter, \c QFont::OldEnglish, \c QFont::System
*/

/*!
  \enum QFont::StyleStrategy

  The style strategy tells the font matching algorithm what type
  of fonts should be used to find an appropriate default family.

  The algorithm won't prefer any type of font if \c NoStratgie is
  provided.

  The other available strategys are \c QFont::PreferBitmap, \c
  QFont::PreferDevice, \c QFont::PreferOutline, \c QFont::ForceOutline

  Any of these may be ORed with a indicator whether exact matching or
  good quality should be preferred.

  \c QFont::PreferMatch, \c QFont::PreferQuality
*/

/*!
  Sets the style hint and strategy.

  The style hint has a default value of \c AnyStyle which leaves the
  task of finding a good default family to the font matching
  algorithm.

  The style strategy has a default value of \c PreferDefault which tells
  the algorithm not to prefer any type of font.
  Right now, the X version only supports bitmap fonts.

  In the example below the push button will
  display its text label with the Bavaria font family if this family
  is available, if not it will display its text label with another
  serif font:
  \code
    #include <qapplication.h>
    #include <qpushbutton.h>
    #include <qfont.h>

    int main( int argc, char **argv )
    {
        QApplication app( argc, argv );
        QPushButton  push("Push me");

        QFont font( "Bavaria", 18 );        // preferred family is Bavaria
        font.setStyleHint( QFont::Serif )    // can also use any serif font

        push.setFont( font );
        return app.exec( &push );
    }
  \endcode

  \sa QFont::StyleHint, styleHint(), QFont::StyleStrategy, styleStrategy(), QFontInfo
*/
void QFont::setStyleHint( StyleHint hint, StyleStrategy strategy )
{
    if ((StyleHint) d->request.styleHint == hint) return;

    detach();
    d->request.styleHint = hint;
    d->request.styleStrategy = strategy;
    d->request.hintSetByUser = TRUE;
    d->request.dirty = TRUE;
}


/*!  Turns raw mode on if \a enable is TRUE, or turns it off if \a
  enable is FALSE.

  Calling this function only has effect under X windows.  If raw mode
  is enabled, Qt will search for an X font with a complete font name
  matching the family name, ignoring all other values set for the
  QFont.  If the font name matches several fonts, Qt will use the
  first font returned by X.  QFontInfo \e cannot be used to fetch
  information about a QFont using raw mode (it will return the values
  set in the QFont for all parameters, including the family name).

  \warning Do not use raw mode unless you really, really need it! In
  most (if not all) cases, setRawName() is a much better choice.

  \sa rawMode(), setRawName()
*/
void QFont::setRawMode( bool enable )
{
    if ((bool) d->request.rawMode == enable) return;

    detach();
    d->request.rawMode = enable;
    d->request.dirty = TRUE;
}


/*!
  Returns TRUE if a window system font exactly matching the settings
  of this font is available.

  \sa QFontInfo
*/
bool QFont::exactMatch() const
{
    d->load();

    return d->exactMatch;
}


/*!
  Returns TRUE if the this font is equal to \e f, or FALSE if they are
  different.

  Two QFonts are equal if their font attributes are equal.  If
  rawMode() is enabled for both fonts, then only the family fields are
  compared.

  \sa operator!=()
*/
bool QFont::operator==( const QFont &f ) const
{
    return f.d == d || f.key() == key();
}


/*!
  Returns TRUE if the this font is different from \e f, or FALSE if they are
  equal.

  Two QFonts are different if their font attributes are different.  If
  rawMode() is enabled for both fonts, then only the family fields are
  compared.

  \sa operator==()
*/
bool QFont::operator!=( const QFont &f ) const
{
    return !(operator==( f ));
}


/*!
  Returns TRUE if this font and \a f are copies of each other,
  i.e. one of them was created as a copy of the other and neither was
  subsequently modified.  This is much stricter than equality.

  \sa operator=, operator==
*/
bool QFont::isCopyOf( const QFont & f ) const
{
    return d && d == f.d;
}


/*!
  Returns the family name that corresponds to the current style hint.
*/
QString QFont::defaultFamily() const
{
    return d->defaultFamily();
}


/*!
 Returns a last resort family name for the font matching algorithm.
*/
QString QFont::lastResortFamily() const
{
    return d->lastResortFamily();
}


/*!
  Returns a last resort raw font name for the font matching algorithm.
  This is used if even the last resort family is not available. It returns
  \e something, almost no matter what.

  The current implementation tries a wide variety of common fonts, returning
  the first one it finds. The implementation may change at any time.
*/
QString QFont::lastResortFont() const
{
    return d->lastResortFont();
}


/*! \obsolete
  Returns the character set by setCharSet().

  Use QFontInfo to find the CharSet of the window system font actually used.
  \sa setCharSet()
*/
QFont::CharSet QFont::charSet() const
{
    return d->charsetcompat;
}


/*! \obsolete
  Used to set the character set encoding (e.g. \c Latin1).

  Does nothing anymore in Qt-3, as Qt-3 is completely based on Unicode
*/
void QFont::setCharSet( CharSet charset )
{
    d->charsetcompat = charset;
}


/*! \obsolete
  For Qt 3.0 and higher, this method always returns Unicode.
*/
QFont::CharSet QFont::charSetForLocale()
{
    return Unicode;
}


/*!
  Returns the value set by setRawMode().
  \sa setRawMode()
*/
bool QFont::rawMode() const
{
    return d->request.rawMode;
}


/*! \obsolete
  Returns the encoding name of a character set, e.g. QFont::ISO_8859_1
  returns "iso8859-1" and QFont::Unicode returns "iso10646".
  */
QString QFont::encodingName( CharSet cs )
{
    QString result;
    switch( cs ) {
    case QFont::ISO_8859_1:
        result = "iso8859-1";
        break;
    case QFont::ISO_8859_2:
        result = "iso8859-2";
        break;
    case QFont::ISO_8859_3:
        result = "iso8859-3";
        break;
    case QFont::ISO_8859_4:
        result = "iso8859-4";
        break;
    case QFont::ISO_8859_5:
        result = "iso8859-5";
        break;
    case QFont::ISO_8859_6:
        result = "iso8859-6";
        break;
    case QFont::ISO_8859_7:
        result = "iso8859-7";
        break;
    case QFont::ISO_8859_8:
        result = "iso8859-8";
        break;
    case QFont::ISO_8859_9:
        result = "iso8859-9";
        break;
    case QFont::ISO_8859_10:
        result = "iso8859-10";
        break;
    case QFont::ISO_8859_11:
        result = "iso8859-11";
        break;
    case QFont::ISO_8859_12:
        result = "iso8859-12";
        break;
    case QFont::ISO_8859_13:
        result = "iso8859-13";
        break;
    case QFont::ISO_8859_14:
        result = "iso8859-14";
        break;
    case QFont::ISO_8859_15:
        result = "iso8859-15";
        break;
    case QFont::KOI8R:
        result = "KOI8-R";
        break;
    case QFont::KOI8U:
        result = "KOI8-U";
        break;
    case QFont::Set_Ja:
        result = "Set_Ja";
        break;
    case QFont::Set_Ko:
        result = "Set_Ko";
        break;
    case QFont::Set_Th_TH:
        result = "Set_Th_TH";
        break;
    case QFont::Set_Zh:
        result = "Set_Zh";
        break;
    case QFont::Set_Zh_TW:
        result = "Set_Zh_TW";
        break;
    case QFont::Set_GBK:
        result = "Set_GBK";
        break;
    case QFont::Set_Big5:
        result = "Set_Big5";
        break;
    case QFont::AnyCharSet:
        result = "AnyCharSet";
        break;
    case QFont::Unicode:
        result = "iso10646";
        break;
    case JIS_X_0201:
	result = "jisx0201";
        break;
    case JIS_X_0208:
	result = "jisx0208";
        break;
    case KSC_5601:
	result = "ksc5601";
        break;
    case GB_2312:
	result = "gb2312";
        break;
    case Big5:
	result = "big5";
        break;
    case QFont::TSCII:
	result = "TSCII";
	break;
    }
    return result;
}


/*! \obsolete
  Returns the QApplication default font.

  This function will be removed in a future version of Qt. Please use
  QApplication::font() instead.
*/
QFont QFont::defaultFont()
{
    return QApplication::font();
}


/*!
  \obsolete

  Please use QApplication::setFont() instead.
*/
void  QFont::setDefaultFont( const QFont &f )
{
    QApplication::setFont( f );
}








/*****************************************************************************
  QFont substitution management
 *****************************************************************************/

typedef QDict<QStringList> QFontSubst;
static QFontSubst *fontSubst = 0;
QCleanupHandler<QFontSubst> qfont_cleanup_fontsubst;


// create substitution dict
static void initFontSubst()
{
    // default substitutions
    static const char *initTbl[] = {

#if defined(Q_WS_X11)
        "arial",        "helvetica",
        "helv",         "helvetica",
        "tms rmn",      "times",
#elif defined(Q_WS_WIN)
        "times",        "Times New Roman",
        "courier",      "Courier New",
        "helvetica",    "Arial",
#endif

        0,              0
    };

    if (fontSubst) return;

    fontSubst = new QFontSubst();
    Q_CHECK_PTR( fontSubst );
    qfont_cleanup_fontsubst.add(fontSubst);

    for ( int i=0; initTbl[i] != 0; i += 2 )
	QFont::insertSubstitution(QString::fromLatin1(initTbl[i]),
				  QString::fromLatin1(initTbl[i+1]));
}


/*!
  Returns the font family name to be used whenever \e familyName is
  specified. The lookup is case insensitive.

  If there is no substitution for \e familyName, then \e familyName is
  returned.

  Example:
  \code
    QFont::insertSubstitution( "NewYork", "London" );
    QFont::insertSubstitution( "Paris",   "Texas" );

    QFont::substitute( "NewYork" );     // returns "London"
    QFont::substitute( "PARIS" );       // returns "Texas"
    QFont::substitute( "Rome" );        // returns "Rome"

    QFont::removeSubstitution( "newyork" );
    QFont::substitute( "NewYork" );     // returns "NewYork"
  \endcode

  \sa setFamily(), insertSubstitution(), removeSubstitution()
*/
QString QFont::substitute( const QString &familyName )
{
    initFontSubst();

    QStringList *list = fontSubst->find(familyName);
    if (list && list->count() > 0) {
        return *(list->at(0));
    }

    return familyName;
}


/*!
  Returns a list of fonts to be used whenever \e familyName is
  specified.  The lookup is case insensitive.

  If there is no substitution for \e familyName, then an empty
  list is returned.
 */
QStringList QFont::substitutes(const QString &familyName)
{
    initFontSubst();

    QStringList ret, *list = fontSubst->find(familyName);
    if (list) ret += *list;
    return ret;
}


/*!
  Inserts a new font family name substitution in the family substitution
  table.

  If \e familyName already exists in the substitution table, it will
  be replaced with this new substitution.

  \sa removeSubstitution(), substitutions(), substitute()
*/
void QFont::insertSubstitution(const QString &familyName,
                                const QString &substituteName)
{
    initFontSubst();

    QStringList *list = fontSubst->find(familyName);
    if (list) {
	list->remove(substituteName);
    } else {
	list = new QStringList;
	fontSubst->insert(familyName, list);
    }

    list->prepend(substituteName);
}


/*!
  Removes a font family name substitution from the family substitution
  table.

  \sa insertSubstitution(), substitutions(), substitute()
*/
void QFont::removeSubstitution( const QString &familyName )
{
    initFontSubst();

    fontSubst->remove(familyName);
}


#ifndef QT_NO_STRINGLIST

/*!
  Returns a sorted list of substituted family names.

  \sa insertSubstitution(), removeSubstitution(), substitute()
*/
QStringList QFont::substitutions()
{
    initFontSubst();

    QStringList ret;
    QDictIterator<QStringList> it(*fontSubst);

    while (it.current()) {
	ret.append(it.currentKey());
	++it;
    }

    ret.sort();

    return ret;
}

#endif // QT_NO_STRINGLIST


/*
  Internal function. Converts boolean font settings (except dirty)
  to an unsigned 8-bit number. Used for serialization etc.
*/
static Q_UINT8 get_font_bits( const QFontDef &f )
{
    Q_UINT8 bits = 0;
    if ( f.italic )
        bits |= 0x01;
    if ( f.underline )
        bits |= 0x02;
    if ( f.strikeOut )
        bits |= 0x04;
    if ( f.fixedPitch )
        bits |= 0x08;
    if ( f.hintSetByUser )
        bits |= 0x10;
    if ( f.rawMode )
        bits |= 0x20;
    return bits;
}


#ifndef QT_NO_DATASTREAM

/*
  Internal function. Sets boolean font settings (except dirty)
  from an unsigned 8-bit number. Used for serialization etc.
*/
static void set_font_bits( Q_UINT8 bits, QFontDef *f )
{
    f->italic        = (bits & 0x01) != 0;
    f->underline     = (bits & 0x02) != 0;
    f->strikeOut     = (bits & 0x04) != 0;
    f->fixedPitch    = (bits & 0x08) != 0;
    f->hintSetByUser = (bits & 0x10) != 0;
    f->rawMode       = (bits & 0x20) != 0;
}

#endif


/*!
  Returns the font's key, which is a textual representation of the font
  settings. It is typically used to insert and find fonts in a
  dictionary or a cache.
  \sa QMap
*/
QString QFont::key() const
{
    return d->key();
}


/*!
  Returns a description of this font.  The description is a comma-separated
  list of the various attributes, perfectly suited for use in QSettings.

  \sa fromString()
 */
QString QFont::toString() const
{
    QStringList l;
    l.append(family());
    l.append(QString::number(pointSize()));
    l.append(QString::number((int)styleHint()));
    l.append(QString::number(weight()));
    l.append(QString::number((int)italic()));
    l.append(QString::number((int)underline()));
    l.append(QString::number((int)strikeOut()));
    l.append(QString::number((int)fixedPitch()));
    l.append(QString::number((int)rawMode()));
    return l.join(",");
}


/*!
  Sets this font to match the description \a descrip.  The description
  is a comma-separated list of the attributes, as returned by toString().

  \sa toString()
 */
bool QFont::fromString(const QString &descrip)
{
    QStringList l(QStringList::split(',', descrip));

    if (l.count() != 9) {

#ifdef QT_CHECK_STATE
	qWarning("QFont::fromString: invalid description '%s'", descrip.latin1());
#endif

	return FALSE;
    }

    setFamily(l[0]);
    setPointSize(l[1].toInt());
    setStyleHint((StyleHint) l[2].toInt());
    setWeight(l[3].toInt());
    setItalic(l[4].toInt());
    setUnderline(l[5].toInt());
    setStrikeOut(l[6].toInt());
    setFixedPitch(l[7].toInt());
    setRawMode(l[8].toInt());

    return TRUE;
}


/*! \internal
  Internal function that dumps font cache statistics.
*/
#if !defined( Q_WS_QWS ) && !defined( Q_WS_MAC )
void QFont::cacheStatistics()
{

#if defined(QT_DEBUG)

    QFontPrivate::fontCache->statistics();

    QFontCacheIterator it(*QFontPrivate::fontCache);
    QFontStruct *qfs;
    qDebug( "{" );
    while ( (qfs = it.current()) ) {
	++it;
#ifdef Q_WS_X11
	qDebug( "   [%s]", (const char *) qfs->name );
#else
	qDebug( "   [%s]", (const char *) qfs->key() );
#endif
    }
    qDebug( "}" );

#endif

}
#endif



/*****************************************************************************
  QFont stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM

/*!
  \relates QFont
  Writes a font to the stream.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator<<( QDataStream &s, const QFont &font )
{
    if ( s.version() == 1 ) {
	QCString fam( font.d->request.family.latin1() );
	s << fam;
    } else {
	s << font.d->request.family;
    }

    return s << (Q_INT16) font.d->request.pointSize
	     << (Q_UINT8) font.d->request.styleHint
	     << (Q_UINT8) font.d->charsetcompat
	     << (Q_UINT8) font.d->request.weight
	     << get_font_bits(font.d->request);
}


/*!
  \relates QFont
  Reads a font from the stream.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>( QDataStream &s, QFont &font )
{
    if (font.d->deref()) delete font.d;

    font.d = new QFontPrivate;

    Q_INT16 pointSize;
    Q_UINT8 styleHint, charSet, weight, bits;

    if ( s.version() == 1 ) {
	QCString fam;
	s >> fam;
	font.d->request.family = QString( fam );
    } else {
	s >> font.d->request.family;
    }

    s >> pointSize;
    s >> styleHint;
    s >> charSet;
    s >> weight;
    s >> bits;

    font.d->request.pointSize = pointSize;
    font.d->request.pixelSize = font.pixelSize();
    font.d->request.styleHint = styleHint;
    font.d->request.weight = weight;
    font.d->request.dirty = TRUE;
    font.d->charsetcompat = (QFont::CharSet) charSet;

    set_font_bits( bits, &(font.d->request) );

    return s;
}

#endif //QT_NO_DATASTREAM




/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

// invariant: this list contains pointers to ALL QFontMetrics objects
// with non-null painter pointers, and no other objects.  Callers of
// these functions must maintain this invariant.

typedef QList<QFontMetrics> QFontMetricsList;
static QFontMetricsList *fm_list = 0;

QCleanupHandler<QFontMetricsList> qfont_cleanup_fontmetricslist;

static void insertFontMetrics( QFontMetrics *fm ) {
    if ( !fm_list ) {
        fm_list = new QFontMetricsList;
        Q_CHECK_PTR( fm_list );
	qfont_cleanup_fontmetricslist.add( fm_list );
    }
    fm_list->append( fm );
}

static void removeFontMetrics( QFontMetrics *fm )
{
    if ( !fm_list ) {
#if defined(CHECK_NULL)
        qWarning( "QFontMetrics::~QFontMetrics: Internal error" );
#endif
        return;
    }
    fm_list->removeRef( fm );
}


/*!
  Resets all pointers to \a painter in all font metrics objects in the
  application.
*/
void QFontMetrics::reset( const QPainter *painter )
{
    if ( fm_list ) {
        QListIterator<QFontMetrics> it( *fm_list );
        QFontMetrics * fm;
        while( (fm=it.current()) != 0 ) {
            ++it;
            if ( fm->painter == painter ) {
                fm->painter = 0;                // detach from painter
                removeFontMetrics( fm );
            }
        }
    }
}


/*!
  \class QFontMetrics qfontmetrics.h
  \brief The QFontMetrics class provides font metrics information about fonts.

  \ingroup fonts
  \ingroup shared

  QFontMetrics functions calculate size of characters and strings for
  a given font. There are three ways you can create a QFontMetrics object:

  The QFontMetrics constructor with a QFont creates a font metrics
  object for a screen-compatible font, i.e. the font can not be a
  printer font.

  QWidget::fontMetrics() returns the font metrics for a widget's font.
  This is equivalent to QFontMetrics(widget->font()).  Setting a new
  font for the widget later does not affect the font metrics object.

  QPainter::fontMetrics() returns the font metrics for a painter's
  current font. The font metrics object is automatically updated if
  somebody sets a new painter font (unlike the two above cases, which
  take a "snapshot" of a font).

  Once created, the object provides functions to access the individual
  metrics of the font, its characters, and for strings rendered in
  this font.

  There are several functions that operate on the font: ascent(),
  descent(), height(), leading() and lineSpacing() return the basic
  size properties of the font, and underlinePos(), strikeOutPos() and
  lineWidth() return properties of the line that underlines or strikes
  out the characters.  These functions are all fast.

  There are also some functions that operate on the set of glyphs in
  the font: minLeftBearing(), minRightBearing() and maxWidth().  These
  are by necessity slow, and we recommend avoiding them if possible.

  For each character, you can get its width(), leftBearing() and
  rightBearing() and find out whether it is in the font using
  inFont().  You can also treat the character as a string, and use the
  string functions on it.

  The string functions include width(), to return the width of a
  string in pixels (or points, for a printer), boundingRect(), to
  return the rectangle necessary to render a string, and size(), to
  return the size of that rectangle.

  Example:
  \code
    QFont font("times",24);
    QFontMetrics fm(font);
    int w = fm.width("What's the width of this text");
    int h = fm.height();
  \endcode

  \sa QFont QFontInfo
*/


/*!
  Constructs a font metrics object for \a font.

  The font must be screen-compatible, i.e. a font you use when drawing
  text in QWidget or QPixmap objects, not QPicture or QPrinter.
  If \a font is a printer font, you'll probably get wrong results.

  Use QPainter::fontMetrics() to get the font metrics when painting.
  This is a little slower than using this constructor, but it always
  gives correct results.
*/
QFontMetrics::QFontMetrics( const QFont &font )
{
    d = font.d;
    d->ref();

    d->load();

    painter = 0;
    flags = 0;

    if (font.underline())
	setUnderlineFlag();
    if (font.strikeOut())
	setStrikeOutFlag();
}


/*!
  \internal
  Constructs a font metrics object for a painter.
*/
QFontMetrics::QFontMetrics( const QPainter *p )
{
    painter = (QPainter *) p;

#if defined(CHECK_STATE)
    if ( !painter->isActive() )
	qWarning( "QFontMetrics: Get font metrics between QPainter::begin() "
		  "and QPainter::end()" );
#endif

    painter->setf(QPainter::FontMet);
    d = painter->cfont.d;
    d->ref();

    d->load();

    flags = 0;

    insertFontMetrics( this );
}


/*!
  Constructs a copy of \e fm.
*/
QFontMetrics::QFontMetrics( const QFontMetrics &fm )
    : d(fm.d), painter(fm.painter), flags(fm.flags)
{
    d->ref();
    if ( painter )
        insertFontMetrics( this );
}


/*!
  Destructs the font metrics object.
*/
QFontMetrics::~QFontMetrics()
{
    if ( painter )
        removeFontMetrics( this );
    if ( d->deref() )
	delete d;
}


/*!
  Font metrics assignment.
*/
QFontMetrics &QFontMetrics::operator=( const QFontMetrics &fm )
{
    if ( painter )
        removeFontMetrics( this );
    if ( d != fm.d ) {
	if ( d->deref() )
	    delete d;
	d = fm.d;
	d->ref();
    }
    painter = fm.painter;
    flags = fm.flags;
    if ( painter )
        insertFontMetrics( this );
    return *this;
}


/*!
  Returns the bounding rectangle of \e ch relative to the leftmost
  point on the base line.

  Note that the bounding rectangle may extend to the left of (0,0),
  e.g. for italicized fonts, and that the text output may cover \e all
  pixels in the bounding rectangle.

  Note that the rectangle usually extends both above and below the
  base line.

  \sa width()
*/
QRect QFontMetrics::boundingRect( QChar ch ) const
{
    return d->boundingRect( ch );
}


/*!
  Returns the bounding rectangle of the first \e len characters of \e str,
  which is the set of pixels the text would cover if drawn at (0,0). The
  drawing, and hence the bounding rectangle, is constrained to the rectangle
  \a (x,y,w,h).

  If \a len is negative (default value), the whole string is used.

  The \a flgs argument is
  the bitwise OR of the following flags:  <ul>
  <li> \c AlignAuto aligns to the left border for all languages except hebrew and arabic where it aligns to the right..
  <li> \c AlignLeft aligns to the left border.
  <li> \c AlignRight aligns to the right border.
  <li> \c AlignJustify produces justified text.
  <li> \c AlignHCenter aligns horizontally centered.
  <li> \c AlignTop aligns to the top border.
  <li> \c AlignBottom aligns to the bottom border.
  <li> \c AlignVCenter aligns vertically centered
  <li> \c AlignCenter (= \c AlignHCenter | AlignVCenter)
  <li> \c SingleLine ignores newline characters in the text.
  <li> \c ExpandTabs expands tabulators.
  <li> \c ShowPrefix interprets "&x" as "x" underlined.
  <li> \c WordBreak breaks the text to fit the rectangle.
  </ul>

  Horizontal alignment defaults to AlignAuto and vertical alignment
  defaults to AlignTop.

  If several of the horizontal or several of the vertical alignment flags
  are set, the resulting alignment is undefined.

  These flags are defined in qnamespace.h.

  If \c ExpandTabs is set in \a flgs, then:
  if \a tabarray is non.zero, it specifies a 0-terminated sequence
  of pixel-positions for tabs; otherwise
  if \a tabstops is non-zero, it is used as the tab spacing (in pixels).

  Note that the bounding rectangle may extend to the left of (0,0),
  e.g. for italicized fonts, and that the text output may cover \e all
  pixels in the bounding rectangle.

  Newline characters are processed as linebreaks.

  Despite the different actual character heights, the heights of the
  bounding rectangles of "Yes" and "yes" are the same.

  The bounding rectangle given by this function is somewhat larger
  than that calculated by the simpler boundingRect() function.  This
  function uses the \link minLeftBearing() maximum left \endlink and
  \link minRightBearing() right \endlink font bearings as is necessary
  for multi-line text to align correctly.  Also, fontHeight() and
  lineSpacing() are used to calculate the height, rather than
  individual character heights.

  The \a intern argument is for internal purposes.

  \sa width(), QPainter::boundingRect(), Qt::AlignmentFlags
*/
QRect QFontMetrics::boundingRect( int x, int y, int w, int h, int flgs,
                                  const QString& str, int len, int tabstops,
                                  int *tabarray, QTextParag **intern ) const
{
    if ( len < 0 )
        len = str.length();

    int tabarraylen=0;
    if (tabarray)
        while (tabarray[tabarraylen])
            tabarraylen++;

    QRect rb;
    QRect r(x, y, w, h);
    qt_format_text( QFont( d, FALSE ), r, flgs, str, len, &rb,
                    tabstops, tabarray, tabarraylen, intern, 0 );

    return rb;
}


/*!
  Returns the size in pixels of the first \e len characters of \e str.

  If \a len is negative (default value), the whole string is used.

  The \a flgs argument is
  the bitwise OR of the following flags:  <ul>
  <li> \c SingleLine ignores newline characters in the text.
  <li> \c ExpandTabs expands tabulators.
  <li> \c ShowPrefix interprets "&x" as "x" underlined.
  <li> \c WordBreak breaks the text to fit the rectangle.
  </ul>

  These flags are defined in qnamespace.h.

  If \c ExpandTabs is set in \a flgs, then:
  if \a tabarray is non.zero, it specifies a 0-terminated sequence
  of pixel-positions for tabs; otherwise
  if \a tabstops is non-zero, it is used as the tab spacing (in pixels).

  Newline characters are processed as linebreaks.

  Despite the different actual character heights, the heights of the
  bounding rectangles of "Yes" and "yes" are the same.

  The \a intern argument is for internal purposes.

  \sa boundingRect()
*/
QSize QFontMetrics::size( int flgs, const QString &str, int len, int tabstops,
                          int *tabarray, QTextParag **intern ) const
{
    return boundingRect(0,0,1,1,flgs,str,len,tabstops,tabarray,intern).size();
}


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

// invariant: this list contains pointers to ALL QFontInfo objects
// with non-null painter pointers, and no other objects.  Callers of
// these functions must maintain this invariant.

typedef QList<QFontInfo> QFontInfoList;
static QFontInfoList *fi_list = 0;

QCleanupHandler<QFontInfoList> qfont_cleanup_fontinfolist;

static void insertFontInfo( QFontInfo *fi )
{
    if ( !fi_list ) {
        fi_list = new QFontInfoList;
        Q_CHECK_PTR( fi_list );
	qfont_cleanup_fontinfolist.add( fi_list );
    }
    fi_list->append( fi );
}

static void removeFontInfo( QFontInfo *fi )
{
    if ( !fi_list ) {
#if defined(CHECK_NULL)
        qWarning( "QFontInfo::~QFontInfo: Internal error" );
#endif
        return;
    }
    fi_list->removeRef( fi );
}


/*!
  Resets all pointers to \a painter in all font metrics objects in the
  application.
*/
void QFontInfo::reset( const QPainter *painter )
{
    if ( fi_list ) {
        QListIterator<QFontInfo> it( *fi_list );
        QFontInfo * fi;
        while( (fi=it.current()) != 0 ) {
            ++it;
            if ( fi->painter == painter ) {
                fi->painter = 0;                // detach from painter
                removeFontInfo( fi );
            }
        }
    }
}


/*!
  \class QFontInfo qfontinfo.h

  \brief The QFontInfo class provides general information about fonts.

  \ingroup fonts
  \ingroup shared

  The QFontInfo class mirrors QFont exactly, but where QFont access
  functions returns set values, QFontInfo returns the values that
  apply to the font in use.

  For example, when the program asks for a 25pt Courier font on a
  machine that has a 24pt Courier font but not a scalable one, QFont
  will (normally) use the 24pt Courier for rendering.  In this case,
  QFont::pointSize() returns 25 and QFontInfo::pointSize() 24.

  The access functions in QFontInfo mirror QFont exactly, except for
  this difference.

  There are three ways to create a QFontInfo object.

  The QFontInfo constructor with a QFont creates a font info object
  for a screen-compatible font, i.e. the font can not be a printer
  font.

  QWidget::fontInfo() returns the font info for a widget's font.  This
  is equivalent to QFontInfo(widget->font()).  Setting a new font for
  the widget later does not affect the font info object.

  QPainter::fontInfo() returns the font info for a painter's current
  font. The font info object is automatically updated if somebody sets
  a new painter font, unlike the two above cases, which take a
  "snapshot" of a font.

  \sa QFont QFontMetrics
*/


/*!
  Constructs a font info object for \a font.

  The font must be screen-compatible, i.e. a font you use when drawing
  text in \link QWidget widgets\endlink or \link QPixmap pixmaps\endlink.
  If \a font is a printer font, you'll probably get wrong results.

  Use the QPainter::fontInfo() to get the font info when painting.
  This is a little slower than using this constructor, but it always
  gives correct results.
*/
QFontInfo::QFontInfo( const QFont &font )
{
    d = font.d;
    d->ref();

    d->load();

    painter = 0;
    flags = 0;

    if ( font.underline() )
	setUnderlineFlag();
    if ( font.strikeOut() )
	setStrikeOutFlag();
    if ( font.exactMatch() )
	setExactMatchFlag();
}


/*!
  \internal
  Constructs a font info object for a painter.
*/
QFontInfo::QFontInfo( const QPainter *p )
{
    painter = (QPainter *) p;

#if defined(CHECK_STATE)
    if ( !painter->isActive() )
	qWarning( "QFontInfo: Get font info between QPainter::begin() "
		  "and QPainter::end()" );
#endif

    painter->setf( QPainter::FontInf );
    d = painter->cfont.d;
    d->ref();

    d->load();

    flags = 0;

    insertFontInfo( this );
}


/*!
  Constructs a copy of \e fi.
*/
QFontInfo::QFontInfo( const QFontInfo &fi )
    : d(fi.d), painter(fi.painter), flags(fi.flags)
{
    d->ref();
    if ( painter )
        insertFontInfo( this );
}


/*!
  Destructs the font info object.
*/
QFontInfo::~QFontInfo()
{
    if ( painter )
        removeFontInfo( this );
    if (d->deref())
	delete d;
}


/*!
  Font info assignment.
*/
QFontInfo &QFontInfo::operator=( const QFontInfo &fi )
{
    if ( painter )
        removeFontInfo( this );
    if (d != fi.d) {
	if (d->deref())
	    delete d;
	d = fi.d;
	d->ref();
    }
    painter = fi.painter;
    flags = fi.flags;
    if ( painter )
        insertFontInfo( this );
    return *this;
}


/*!
  Returns the family name of the matched window system font.
  \sa QFont::family()
*/
QString QFontInfo::family() const
{
    return d->actual.family;
}


/*!
  Returns the point size of the matched window system font.
  \sa QFont::pointSize()
*/
int QFontInfo::pointSize() const
{
    return d->actual.pointSize / 10;
}


/*!
  Returns the italic value of the matched window system font.
  \sa QFont::italic()
*/
bool QFontInfo::italic() const
{
    return d->actual.italic;
}


/*!
  Returns the weight of the matched window system font.

  \sa QFont::weight(), bold()
*/
int QFontInfo::weight() const
{
    return d->actual.weight;
}


/*!
  \fn bool QFontInfo::bold() const

  Returns TRUE if weight() would return a greater than
  \c QFont::Normal, and FALSE otherwise.

  \sa weight(), QFont::bold()
*/

/*!
  Returns the underline value of the matched window system font.
  \sa QFont::underline()

  \internal

  Here we read the underline flag directly from the QFont.
  This is OK for X11 and for Windows because we always get what we want.
*/
bool QFontInfo::underline() const
{
    return painter ? painter->font().underline() : underlineFlag();
}


/*!
  Returns the strike out value of the matched window system font.
  \sa QFont::strikeOut()

  \internal Here we read the strikeOut flag directly from the QFont.
  This is OK for X11 and for Windows because we always get what we want.
*/
bool QFontInfo::strikeOut() const
{
    return painter ? painter->font().strikeOut() : strikeOutFlag();
}


/*!
  Returns the fixed pitch value of the matched window system font.
  A fixed pitch font is a font that has constant character pixel width.
  \sa QFont::fixedPitch()
*/
bool QFontInfo::fixedPitch() const
{
    return d->actual.fixedPitch;
}


/*!
  Returns the style of the matched window system font.

  Currently only returns the hint set in QFont.

  \sa QFont::styleHint()
*/
QFont::StyleHint QFontInfo::styleHint() const
{
    return (QFont::StyleHint) d->actual.styleHint;
}


/*! \obsolete
  Returns the character set of the matched window system font.

  \sa QFont::charSet()
*/
QFont::CharSet QFontInfo::charSet() const
{
    return d->charsetcompat;
}


/*!
  Returns TRUE if the font is a raw mode font.

  If it is a raw mode font, all other functions in QFontInfo will return the
  same values set in the QFont, regardless of the font actually used.

  \sa QFont::rawMode()
*/
bool QFontInfo::rawMode() const
{
    return d->actual.rawMode;
}


/*!
  Returns TRUE if the matched window system font is exactly the one specified
  by the font.

  \sa QFont::exactMatch()
*/
bool QFontInfo::exactMatch() const
{
    return painter ? painter->font().exactMatch() : exactMatchFlag();
}



#ifndef Q_WS_QWS
// **********************************************************************
// QFontCache
// **********************************************************************

static const int qtFontCacheMin = 2*1024*1024;
static const int qtFontCacheSize = 61;
static const int qtFontCacheFastTimeout =  30000;
static const int qtFontCacheSlowTimeout = 300000;


QFontCache *QFontPrivate::fontCache = 0;


QFontCache::QFontCache() :
    QObject(0, "global font cache"),
    QCache<QFontStruct>(qtFontCacheMin, qtFontCacheSize),
    timer_id(0), fast(FALSE)
{
    setAutoDelete(TRUE);
}


QFontCache::~QFontCache()
{
    // remove negative cache items
    QFontCacheIterator it(*this);
    QString key;
    QFontStruct *qfs;

    while ((qfs = it.current())) {
	key = it.currentKey();
	++it;

	if (qfs == (QFontStruct *) -1) {
	    take(key);
	}
    }
}


bool QFontCache::insert(const QString &key, const QFontStruct *qfs, int cost)
{

#ifdef QFONTCACHE_DEBUG
    qDebug("QFC::insert: inserting %p w/ cost %d", qfs, cost);
#endif // QFONTCACHE_DEBUG

    if (totalCost() + cost > maxCost()) {

#ifdef QFONTCACHE_DEBUG
	qDebug("QFC::insert: adjusting max cost to %d (%d %d)",
	       totalCost() + cost, totalCost(), maxCost());
#endif // QFONTCACHE_DEBUG

	setMaxCost(totalCost() + cost);
    }

    bool ret = QCache<QFontStruct>::insert(key, qfs, cost);

    if (ret && (! timer_id || ! fast)) {
	if (timer_id) {

#ifdef QFONTCACHE_DEBUG
	    qDebug("QFC::insert: killing old timer");
#endif // QFONTCACHE_DEBUG

	    killTimer(timer_id);
	}

#ifdef QFONTCACHE_DEBUG
	qDebug("QFC::insert: starting timer");
#endif // QFONTCACHE_DEBUG

	timer_id = startTimer(qtFontCacheFastTimeout);
	fast = TRUE;
    }

    return ret;
}


void QFontCache::deleteItem(Item d)
{
    QFontStruct *qfs = (QFontStruct *) d;

    // don't try to delete negative cache items
    if (qfs == (QFontStruct *) -1) {
	return;
    }

    if (qfs->count == 0) {

#ifdef QFONTCACHE_DEBUG
	qDebug("QFC::deleteItem: removing %s from cache", (const char *) qfs->name);
#endif // QFONTCACHE_DEBUG

    	delete qfs;
    }
}


void QFontCache::timerEvent(QTimerEvent *)
{
    if (maxCost() <= qtFontCacheMin) {

#ifdef QFONTCACHE_DEBUG
	qDebug("QFC::timerEvent: cache max cost is less than min, killing timer");
#endif // QFONTCACHE_DEBUG

	setMaxCost(qtFontCacheMin);

	killTimer(timer_id);
	timer_id = 0;
	fast = TRUE;

	return;
    }

    QFontCacheIterator it(*this);
    QString key;
    QFontStruct *qfs;
    int tqcost = maxCost() * 3 / 4;
    int nmcost = 0;

    while ((qfs = it.current())) {
	key = it.currentKey();
	++it;

	if (qfs != (QFontStruct *) -1) {
	    if (qfs->count > 0) {
		nmcost += qfs->cache_cost;
	    }
	} else {
	    // keep negative cache items in the cache
	    nmcost++;
	}
    }

    nmcost = QMAX(tqcost, nmcost);
    if (nmcost < qtFontCacheMin) nmcost = qtFontCacheMin;

    if (nmcost == totalCost()) {
	if (fast) {

#ifdef QFONTCACHE_DEBUG
	    qDebug("QFC::timerEvent: slowing timer");
#endif // QFONTCACHE_DEBUG

	    killTimer(timer_id);

	    timer_id = startTimer(qtFontCacheSlowTimeout);
	    fast = FALSE;
	}
    } else if (! fast) {
	// cache size is changing now, but we're still on the slow timer... time to
	// drop into passing gear

#ifdef QFONTCACHE_DEBUG
	qDebug("QFC::timerEvent: dropping into passing gear");
#endif // QFONTCACHE_DEBUG

	killTimer(timer_id);
	timer_id = startTimer(qtFontCacheFastTimeout);
	fast = TRUE;
    }

#ifdef QFONTCACHE_DEBUG
    qDebug("QFC::timerEvent: before cache cost adjustment: %d %d",
	   totalCost(), maxCost());
#endif // QFONTCACHE_DEBUG

    setMaxCost(nmcost);

#ifdef QFONTCACHE_DEBUG
    qDebug("QFC::timerEvent:  after cache cost adjustment: %d %d",
	   totalCost(), maxCost());
#endif // QFONTCACHE_DEBUG

}
#endif




// **********************************************************************
// QFontPrivate member methods
// **********************************************************************

// Converts a weight string to a value
int QFontPrivate::getFontWeight(const QCString &weightString, bool adjustScore)
{
    // Test in decreasing order of commonness
    if ( weightString == "medium" )       return QFont::Normal;
    else if ( weightString == "bold" )    return QFont::Bold;
    else if ( weightString == "demibold") return QFont::DemiBold;
    else if ( weightString == "black" )   return QFont::Black;
    else if ( weightString == "light" )   return QFont::Light;

    QCString s(weightString.lower());

    if ( s.contains("bold") ) {
	if ( adjustScore )
	    return (int) QFont::Bold - 1;  // - 1, not sure that this IS bold
	else
	    return (int) QFont::Bold;
    }

    if ( s.contains("light") ) {
	if ( adjustScore )
	    return (int) QFont::Light - 1; // - 1, not sure that this IS light
	else
	    return (int) QFont::Light;
    }

    if ( s.contains("black") ) {
	if ( adjustScore )
	    return (int) QFont::Black - 1; // - 1, not sure this IS black
	else
	    return (int) QFont::Black;
    }

    if ( adjustScore )
	return (int) QFont::Normal - 2;	   // - 2, we hope it's close to normal

    return (int) QFont::Normal;
}

QString QFontPrivate::key() const
{
    if (request.rawMode)
	return request.family.lower();

    QString family = request.family.lower();

    int len = (family.length() * 2) +
	      2 +  // point size
	      1 +  // font bits
	      1 +  // weight
	      1;   // hint

    QByteArray buf(len);
    uchar *p = (uchar *) buf.data();

    memcpy((char *) p, (char *) family.unicode(), (family.length() * 2));

    p += family.length()*2;

    *((Q_UINT16 *) p) = request.pointSize; p += 2;
    *p++ = get_font_bits( request );
    *p++ = request.weight;
    *p++ = (request.hintSetByUser ?
	    (int) request.styleHint : (int) QFont::AnyStyle);

    return QString((QChar *) buf.data(), buf.size() / 2);
}

QFontPrivate::Script QFontPrivate::scriptForChar( const QChar &c )
{
    uchar row = c.row();

    // Thankfully BASICLATIN is more or less == ISO 8859-1
    if (! row) return QFontPrivate::BasicLatin;

    switch ( row ) {
    case 0x01:
	// There are no typos here... really...
	switch (c.cell()) {
	case 0x00: return QFontPrivate::LatinExtA4;
	case 0x01: return QFontPrivate::LatinExtA4;
	case 0x02: return QFontPrivate::LatinExtA2;
	case 0x03: return QFontPrivate::LatinExtA2;
	case 0x04: return QFontPrivate::LatinExtA2;
	case 0x05: return QFontPrivate::LatinExtA2;
	case 0x06: return QFontPrivate::LatinExtA2;
	case 0x07: return QFontPrivate::LatinExtA2;
	case 0x08: return QFontPrivate::LatinExtA3;
	case 0x09: return QFontPrivate::LatinExtA3;
	case 0x0A: return QFontPrivate::LatinExtA3;
	case 0x0B: return QFontPrivate::LatinExtA3;
	case 0x0C: return QFontPrivate::LatinExtA2;
	case 0x0D: return QFontPrivate::LatinExtA2;
	case 0x0E: return QFontPrivate::LatinExtA2;
	case 0x0F: return QFontPrivate::LatinExtA2;
	case 0x10: return QFontPrivate::LatinExtA2;
	case 0x11: return QFontPrivate::LatinExtA2;
	case 0x12: return QFontPrivate::LatinExtA4;
	case 0x13: return QFontPrivate::LatinExtA4;
	case 0x16: return QFontPrivate::LatinExtA4;
	case 0x17: return QFontPrivate::LatinExtA4;
	case 0x18: return QFontPrivate::LatinExtA2;
	case 0x19: return QFontPrivate::LatinExtA2;
	case 0x1A: return QFontPrivate::LatinExtA2;
	case 0x1B: return QFontPrivate::LatinExtA2;
	case 0x1C: return QFontPrivate::LatinExtA3;
	case 0x1D: return QFontPrivate::LatinExtA3;
	case 0x1E: return QFontPrivate::LatinExtA3;
	case 0x1F: return QFontPrivate::LatinExtA3;
	case 0x20: return QFontPrivate::LatinExtA3;
	case 0x21: return QFontPrivate::LatinExtA3;
	case 0x22: return QFontPrivate::LatinExtA4;
	case 0x23: return QFontPrivate::LatinExtA4;
	case 0x24: return QFontPrivate::LatinExtA3;
	case 0x25: return QFontPrivate::LatinExtA3;
	case 0x26: return QFontPrivate::LatinExtA3;
	case 0x27: return QFontPrivate::LatinExtA3;
	case 0x28: return QFontPrivate::LatinExtA4;
	case 0x29: return QFontPrivate::LatinExtA4;
	case 0x2A: return QFontPrivate::LatinExtA4;
	case 0x2B: return QFontPrivate::LatinExtA4;
	case 0x2E: return QFontPrivate::LatinExtA4;
	case 0x2F: return QFontPrivate::LatinExtA4;
	case 0x30: return QFontPrivate::LatinExtA3;
	case 0x31: return QFontPrivate::LatinExtA3;
	case 0x34: return QFontPrivate::LatinExtA3;
	case 0x35: return QFontPrivate::LatinExtA3;
	case 0x36: return QFontPrivate::LatinExtA4;
	case 0x37: return QFontPrivate::LatinExtA4;
	case 0x38: return QFontPrivate::LatinExtA4;
	case 0x39: return QFontPrivate::LatinExtA2;
	case 0x3A: return QFontPrivate::LatinExtA2;
	case 0x3B: return QFontPrivate::LatinExtA4;
	case 0x3C: return QFontPrivate::LatinExtA4;
	case 0x3D: return QFontPrivate::LatinExtA2;
	case 0x3E: return QFontPrivate::LatinExtA2;
	case 0x41: return QFontPrivate::LatinExtA2;
	case 0x42: return QFontPrivate::LatinExtA2;
	case 0x43: return QFontPrivate::LatinExtA2;
	case 0x44: return QFontPrivate::LatinExtA2;
	case 0x45: return QFontPrivate::LatinExtA4;
	case 0x46: return QFontPrivate::LatinExtA4;
	case 0x47: return QFontPrivate::LatinExtA2;
	case 0x48: return QFontPrivate::LatinExtA2;
	case 0x4A: return QFontPrivate::LatinExtA4;
	case 0x4B: return QFontPrivate::LatinExtA4;
	case 0x4C: return QFontPrivate::LatinExtA4;
	case 0x4D: return QFontPrivate::LatinExtA4;
	case 0x50: return QFontPrivate::LatinExtA2;
	case 0x51: return QFontPrivate::LatinExtA2;
	case 0x52: return QFontPrivate::LatinExtA15;
	case 0x53: return QFontPrivate::LatinExtA15;
	case 0x54: return QFontPrivate::LatinExtA2;
	case 0x55: return QFontPrivate::LatinExtA2;
	case 0x56: return QFontPrivate::LatinExtA4;
	case 0x57: return QFontPrivate::LatinExtA4;
	case 0x58: return QFontPrivate::LatinExtA2;
	case 0x59: return QFontPrivate::LatinExtA2;
	case 0x5A: return QFontPrivate::LatinExtA2;
	case 0x5B: return QFontPrivate::LatinExtA2;
	case 0x5C: return QFontPrivate::LatinExtA3;
	case 0x5D: return QFontPrivate::LatinExtA3;
	case 0x5E: return QFontPrivate::LatinExtA2;
	case 0x5F: return QFontPrivate::LatinExtA2;
	case 0x60: return QFontPrivate::LatinExtA2;
	case 0x61: return QFontPrivate::LatinExtA2;
	case 0x62: return QFontPrivate::LatinExtA2;
	case 0x63: return QFontPrivate::LatinExtA2;
	case 0x64: return QFontPrivate::LatinExtA2;
	case 0x65: return QFontPrivate::LatinExtA2;
	case 0x66: return QFontPrivate::LatinExtA4;
	case 0x67: return QFontPrivate::LatinExtA4;
	case 0x68: return QFontPrivate::LatinExtA4;
	case 0x69: return QFontPrivate::LatinExtA4;
	case 0x6A: return QFontPrivate::LatinExtA4;
	case 0x6B: return QFontPrivate::LatinExtA4;
	case 0x6C: return QFontPrivate::LatinExtA3;
	case 0x6D: return QFontPrivate::LatinExtA3;
	case 0x6E: return QFontPrivate::LatinExtA2;
	case 0x6F: return QFontPrivate::LatinExtA2;
	case 0x70: return QFontPrivate::LatinExtA2;
	case 0x71: return QFontPrivate::LatinExtA2;
	case 0x72: return QFontPrivate::LatinExtA4;
	case 0x73: return QFontPrivate::LatinExtA4;
	case 0x74: return QFontPrivate::LatinExtA14;
	case 0x75: return QFontPrivate::LatinExtA14;
	case 0x76: return QFontPrivate::LatinExtA14;
	case 0x77: return QFontPrivate::LatinExtA14;
	case 0x78: return QFontPrivate::LatinExtA15;
	case 0x79: return QFontPrivate::LatinExtA2;
	case 0x7A: return QFontPrivate::LatinExtA2;
	case 0x7B: return QFontPrivate::LatinExtA2;
	case 0x7C: return QFontPrivate::LatinExtA2;
	case 0x7D: return QFontPrivate::LatinExtA2;
	case 0x7E: return QFontPrivate::LatinExtA2;
	}

	return QFontPrivate::LatinExtB;

	// TODO: support for Latin Extended-B
    case 0x02:
	if (c.cell() <= 0x4f)
	    return QFontPrivate::LatinExtB;
	if (c.cell() <= 0xaf)
	    return QFontPrivate::IPAExt;
	break;

    case 0x03:
	if (c.cell() <= 0x6f)
	    return QFontPrivate::Diacritical;
	return QFontPrivate::Greek;

    case 0x04:
	// Cyrillic (Russian/Ukrainian)
	if (c.cell() >= 0x8c)
	    return QFontPrivate::CyrillicExt;
	if (c.cell() >= 0x60)
	    return QFontPrivate::CyrillicHistoric;
	return QFontPrivate::Cyrillic;

    case 0x05:
	if( c.cell() >= 0x90 )
	    return QFontPrivate::Hebrew;
	return QFontPrivate::Armenian;

    case 0x06:
	// probably won't work like this because of shaping...
	return QFontPrivate::Arabic;

    case 0x07:
	if (c.cell() <= 0x4f)
	    return QFontPrivate::Syriac;
	if (c.cell() >= 0x80 && c.cell() <= 0xbf)
	    return QFontPrivate::Thaana;
	break;

    case 0x09:
	if (c.cell() <= 0x80)
	    return QFontPrivate::Bengali;
	return QFontPrivate::Devanagari;

    case 0x0a:
	if (c.cell() <= 0x80)
	    return QFontPrivate::Gurmukhi;
	return QFontPrivate::Gujarati;

    case 0x0b:
	if ( c.cell() >= 0x80 )
	    return QFontPrivate::Tamil;
	return QFontPrivate::Oriya;

    case 0x0c:
	if (c.cell() >= 0x80)
	    return QFontPrivate::Kannada;
	return QFontPrivate::Telugu;

    case 0x0d:
	if (c.cell() >= 0x80)
	    return QFontPrivate::Sinhala;
	return QFontPrivate::Malayalam;

    case 0x0e:
	if (c.cell() >= 0x80)
	    return QFontPrivate::Lao;
	return QFontPrivate::Thai;

    case 0x0f:
	if (c.cell() <= 0xbf)
	    return QFontPrivate::Tibetan;
	break;

    case 0x10:
	if (c.cell() <= 0x9f)
	    return QFontPrivate::Myanmar;
	return QFontPrivate::Georgian;

    case 0x11:
	return QFontPrivate::Hangul;

    case 0x12:
	return QFontPrivate::Ethiopic;

    case 0x13:
	if (c.cell() <= 0x7f)
	    return QFontPrivate::Ethiopic;
	break;

    case 0x17:
	if (c.cell() >= 0x80)
	    return QFontPrivate::Khmer;
	break;

    case 0x1e:
	return QFontPrivate::LatinExtADDL;

    case 0x1f:
	return QFontPrivate::GreekExt;

    case 0x30:
	if (c.cell() >= 0xa0)
	    return QFontPrivate::Katakana;
	if (c.cell() >= 0x40)
	    return QFontPrivate::Hiragana;

	// Unified Han Symbols and Punctuation
#ifdef Q_WS_X11
	return hanHack( c );
#else
	return QFontPrivate::Han;
#endif
    case 0x31:
	if (c.cell() <= 0x2f)
	    return QFontPrivate::Bopomofo;

	// Hangul Compatibility Jamo
	if (c.cell() <= 0x8f)
	    return QFontPrivate::Hangul;
	break;

    case 0xfb:
	if (c.cell() >= 0x50)
//	    return QFontPrivate::ArabicPresentationA;
		    return QFontPrivate::Arabic;
	break;

    case 0xfe:
	if (c.cell() >= 0x70)
		    return QFontPrivate::Arabic;
//	    return QFontPrivate::ArabicPresentationB;
	break;

    case 0xff:
	// Hiragana half/full width forms block
	if (c.cell() <= 0xef)
	    return QFontPrivate::Hiragana;
	break;
    }

    // Canadian Aboriginal Syllabics
    if (row >= 0x14 && (row < 0x16 || (row == 0x16 && c.cell() <= 0x7f))) {
	return QFontPrivate::CanadianAboriginal;
    }

    // Hangul Syllables
    if (row >= 0xac && (row < 0xd7 || (row == 0xd7 && c.cell() <= 0xa3))) {
	return QFontPrivate::Hangul;
    }

    if (// Unified Han + Extension-A
	(row >= 0x34 && row <= 0x9f) ||
	// Unified Han Compatibility
	(row >= 0xf9 && row <= 0xfa)
	) {
#ifdef Q_WS_X11
	return hanHack( c );
#else
	return QFontPrivate::Han;
#endif
    }

    // qDebug("QFP::scriptForChar: unknown character U+%04x", c.unicode());
    // return QFontPrivate::UnknownScript;
    return QFontPrivate::Unicode;
}
