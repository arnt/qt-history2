/****************************************************************************
** $Id: $
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
#include "qdict.h"
#include "qcache.h"
#include "qdatastream.h"
#include "qapplication.h"
#include "qcleanuphandler.h"
#include "qstringlist.h"

#include <ctype.h>
#include <limits.h>


// #define QFONTCACHE_DEBUG


/*! \class QFont qfont.h

  \brief The QFont class specifies a font used for drawing text.

  \ingroup graphics
  \ingroup appearance
  \ingroup shared
  \mainclass

    When you create a QFont object you specify various attributes that
    you want the font to have. Qt will use the font with the specified
    attributes, or if no matching font exists, Qt will use the closest
    matching installed font. The attributes of the font that is actually
    used are retrievable from a QFontInfo object. If the window system
    provides an exact match exactMatch() returns TRUE. Use QFontMetrics
    to get measurements, e.g. the pixel length of a string using
    QFontMetrics::width().

    Use QApplication::setFont() to set the application's default font.

    If a choosen X11 font does not include all the characters that need
    to be displayed, QFont will try to find the characters in the
    nearest equivalent fonts. When a QPainter draws a character from a
    font the QFont will report whether or not it has the character; if
    it does not, QPainter will draw an unfilled square.

    Create QFonts like this:
    \code
    QFont serifFont( "Times", 10, Bold );
    QFont sansFont( "Helvetica [Cronyx]", 12 );
    \endcode

    The attributes set in the constructor can also be set later, e.g.
    setFamily(), setPointSize(), setPointSizeFloat(), setWeight() and
    setItalic(). The remaining attributes must be set after
    contstruction, e.g. setBold(), setUnderline(), setStrikeOut() and
    setFixedPitch(). QFontInfo objects should be created \e after the
    font's attributes have been set. A QFontInfo object will not change,
    even if you change the font's attributes. The corresponding "get"
    functions, e.g. family(), pointSize(), etc., return the values that
    were set, even though the values used may differ. The actual values
    are available from a QFontInfo object.

    If the requested font family is unavailable you can influence the
    \link #fontmatching font matching algorithm\endlink by choosing a
    particular \l{QFont::StyleHint} and \l{QFont::StyleStrategy} with
    setStyleHint(). The default family (corresponding to the current
    style hint) is returned by defaultFamily().

    The font-matching algorithm has a lastResortFamily() and
    lastResortFont() in cases where a suitable match cannot be found.
    You can provide substitutions for font family names using
    insertSubstitution() and insertSubstitutions(). Substitutions can be
    removed with removeSubstitution(). Use substitute() to retrieve a
    family's first substitute, or the family name itself if it has no
    substitutes. Use substitutes() to retrieve a list of a family's
    substitutes (which may be empty).

    Every QFont has a key() which you can use, for example, as the key
    in a cache or dictionary. If you want to store a user's font
    preferences you could use QSettings, writing the font information
    with toString() and reading it back with fromString(). The
    operator<<() and operator>>() functions are also available, but they
    work on a data stream.

    It is possible to set the height of characters shown on the screen
    to a specified number of pixels with setPixelSize(); however using
    setPointSize() has a similar effect and provides device
    independence.

    Under the X Window System you can set a font using its system
    specific name with setRawName().

    Loading fonts can be expensive, especially on X11. QFont contains
    extensive optimizations to make the copying of QFont objects fast,
    and to cache the results of the slow window system functions it
    depends upon.

  \target fontmatching
  The font matching algorithm works as follows:
  \list 1
  \i The specified font family is searched for.
  \i If not found, the styleHint() is used to select a replacement
  family.
  \i  Each replacement font family is searched for.
  \i If none of these are found or there was no styleHint(), "helvetica"
  will be searched for.
  \i If "helvetica" isn't found Qt will try the lastResortFamily().
  \i If the lastResortFamily() isn't found Qt will try the
  lastResortFont() which will always return a name of some kind.
  \endlist

  Once a font is found, the remaining attributes are matched in order of
  priority:
  \list 1
  \i fixedPitch()
  \i pointSize() (see below)
  \i weight()
  \i italic()
  \endlist

    If you have a font which matches on family, even if none of the
    other attributes match, this font will be chosen in preference to a
    font which doesn't match on family but which does match on the other
    attributes. This is because font family is the dominant search
    criteria.

  The point size is defined to match if it is within 20% of the
  requested point size. When several fonts match and are only
  distinguished by point size, the font with the closest point size to
  the one requested will be chosen.

    The actual family, font size, weight and other font attributes used
    for drawing text will depend on what's available for the chosen
    family under the window system. A QFontInfo object can be used to
    determine the actual values used for drawing the text.

  Examples:

    \code
    QFont f("Helvetica");
    \endcode
    If you had both an Adobe and a Cronyx Helvetica, you might get
    either.

    \code
    QFont f1( "Helvetica [Cronyx]" );  // Qt 3.x
    QFont f2( "Cronyx-Helvetica" );    // Qt 2.x compatibility
    \endcode
    You can specify the foundry you want in the family name. Both fonts,
    f1 and f2, in the above example will be set to  "Helvetica
    [Cronyx]".

    To determine the attributes of the font actually used in the window
    system, use a QFontInfo object, e.g.
    \code
    QFontInfo info( f1 );
    QString family = info.family();
    \endcode

    To find out font metrics use a QFontMetrics object, e.g.
    \code
    QFontMetrics fm( f1 );
    int pixelWidth = fm.width( "How many pixels wide is this text?" );
    int pixelHeight = fm.height();
    \endcode

  For more general information on fonts, see the
  \link http://www.nwalsh.com/comp.fonts/FAQ/ comp.fonts FAQ.\endlink
  Information on encodings can be found from
  \link http://czyborra.com/ Roman Czyborra's\endlink page.

  \sa QFontMetrics QFontInfo QFontDatabase QApplication::setFont()
  QWidget::setFont() QPainter::setFont() QFont::StyleHint
  QFont::Weight
*/

/*! \enum QFont::Script

  This enum represents \link unicode.html Unicode \endlink
  allocated scripts. For exhaustive
  coverage see \link http://www.amazon.com/exec/obidos/ASIN/0201616335/trolltech/t
  The Unicode Standard Version 3.0 \endlink.
  The following scripts are supported:

  Modern European alphabetic scripts (left to right):

  \value Latin consists of most alphabets based on the original Latin alphabet.
  \value Greek covers ancient and modern Greek and Coptic.
  \value Cyrillic covers the Slavic and non-Slavic languages using
	cyrillic alphabets.
  \value Armenian contains the Armenian alphabet used with the Armenian language.
  \value Georgian covers at least the language Georgian.
  \value Runic covers the known constituents of the Runic alphabets used
	 by the early and medieval societies in the Germanic,
	 Scandinavian, and Anglo-Saxon areas.
  \value Ogham is an alphabetical script used to write a very early form of Irish.
  \value SpacingModifiers are small signs indicating modifications to the preceeding letter.
  \value CombiningMarks consist of diacritical marks not specific to a particular alphabet,
	 diacritical marks used in combination with mathematical and technical symbols,
	 and glyph encodings applied to multiple letterforms.

  Middle Eastern scripts (right to left):

  \value Hebrew is used for writing Hebrew, Yiddish, and some other languages.
  \value Arabic covers the Arabic language as well as Persian, Urdu, Kurdish and some
	 others.
  \value Syriac is used to write the active liturgical languages and dialects of several
	 Middle Eastern and Southeast Indian communities.
  \value Thaana is used to write the Maledivian Dhivehi language.

  South and Southeast Asian scripts (left to right with few historical exceptions):

  \value Devanagari covers classical Sanskrit and modern Hindi as well as several other
	 languages.
  \value Bengali is a relative to Devanagari employed to write the Bengali language
	 used in West Bengal/India and Bangladesh as well as several minority languages.
  \value Gurmukhi is another Devanagari relative used to write Punjabi.
  \value Gujarati is closely related to Devanagari and used to write the Gujarati
	 language of the Gujarat state in India.
  \value Oriya is used to write the Oriya language of Orissa state/India.
  \value Tamil is used to write the Tamil language of Tamil Nadu state/India,
	 Sri Lanka, Singapore and parts of Malaysia as well as some minority languages.
  \value Telugu is used to write the Telugu language of Andhra Pradesh state/India
	 and some minority languages.
  \value Kannada is another South Indian script used to write the Kannada language of
	 Karnataka state/India and some minority languages.
  \value Malayalam is used to write the Malayalam language of Kerala state/India.
  \value Sinhala is used for Sri Lanka's majority language Sinhala and is also employed
	 to write Pali, Sanskrit, and Tamil.
  \value Thai is used to write Thai and other Southeast Asian languages.
  \value Lao is a language and script quite similar to Thai.
  \value Tibetan is the script used to write Tibetan in several countries like Tibet,
	 the bordering Indian regions and Nepal. It is also used in the Buddist
	 philosophy and liturgy of the Mongolian cultural area.
  \value Myanmar is mainly used to write the Burmese language of Myanmar (former Burma).
  \value Khmer is the official language of Kampuchea.

  East Asian scripts (traditionally top-down, right to left, modern often horizontal
	 left to right):

  \value Han consists of the CJK (Chinese, Japanese, Korean) idiographic characters.
  \value Hiragana is a cursive syllabary used to indicate phonetics and pronounciation
	 of Japanese words.
  \value Katakana is a non-cursive syllabic script used to write Japanese words with
	 visual emphasis and non-Japanese words in a phonetical manner.
  \value Hangul is a Korean script consisting of alphabetic components.
  \value Bopomofo is a phonetic alphabet for Chinese
	 (mainly Mandarin).
  \value Yi (also called Cuan or Wei) is a syllabary used to write the Yi language
	 of Southwestern China, Myanmar, Laos, and Vietnam.

  Additional scripts that do not fit well into the script categories above:

  \value Ethiopic is a syllabary used by several Central East African languages.
  \value Cherokee is a left-to-right syllabic script used to write the Cherokee language.
  \value CanadianAboriginal consists of the syllabics used by some Canadian aboriginal societies.
  \value Mongolian is the traditional (and recently reintroduced) script used to write Mongolian.

  Symbols:

  \value CurrencySymbols contains currency symbols not encoded in other scripts.
  \value LetterlikeSymbols consists of symbols derived  from ordinary letters of an
	 alphabetical script.
  \value NumberForms are provided for compatibility with other existing character sets.
  \value MathematicalOperators consists of encodings for operators,
	 relations and other symbols like arrows used in a mathematical context.
  \value TechnicalSymbols contains representations for control codes, the space symbol,
	 APL symbols and other symbols mainly used in the context of electronic data
	 processing.
  \value GeometricSymbols covers block elements and geometric shapes.
  \value MiscellaneousSymbols consists of a heterogeneous collection of symbols that
	 do not fit any other Unicode character block, e.g. Dingbats.
  \value EnclosedAndSquare is provided for compatibility with some East Asian standards.
  \value Braille is an international writing system used by blind people. This script encodes
	 the 256 eight-dot patterns with the 64 six-dot patterns as a subset.


  \value Unicode includes all the above scripts.

  The values below are provided for completeness and must not be used in user programs.

  \value HanX11 For internal use only.
  \value LatinBasic For internal use only.
  \value LatinExtendedA_2 For internal use only.
  \value LatinExtendedA_3 For internal use only.
  \value LatinExtendedA_4 For internal use only.
  \value LatinExtendedA_14 For internal use only.
  \value LatinExtendedA_15 For internal use only.

  \value LastPrivateScript For internal use only.

  \value NScripts For internal use only.
  \value NoScript For internal use only.
  \value UnknownScript For internal use only.
*/

/*! \internal

  Constructs a font that gets by default a
  \link shclass.html deep copy \endlink of \a data.
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

QFont::QFont( QFontPrivate *data, QPaintDevice *pd )
{
    d = new QFontPrivate( *data, pd );
    d->request.dirty = TRUE;
    Q_CHECK_PTR( d );

    // now a single reference
    d->count = 1;
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


/*! Constructs a font object that uses the application's default font.

  \sa QApplication::setFont(), QApplication::font()
*/
QFont::QFont()
{
    const QFont appfont = QApplication::font();
    d = appfont.d;
    d->ref();
}


/*! Constructs a font object with the specified \a family, \a pointSize,
  \a weight and \a italic settings.

  If \a pointSize is <= 0 it is set to 1.

    The \a family name may optionally also include a foundry name, e.g.
    "Helvetica [Cronyx]". (The Qt 2.x syntax, i.e. "Cronyx-Helvetica",
    is also supported.) If the \a family is available from more than one
    foundry and the foundry isn't specified, an arbitrary foundry is
    chosen. If the family isn't available a family will be set using the
    \link #fontmatching font matching\endlink algorithm.

  \sa Weight, setFamily(), setPointSize(), setWeight(), setItalic(), setStyleHint() QApplication::font()
*/
QFont::QFont( const QString &family, int pointSize, int weight, bool italic )
{
    if (pointSize <= 0) pointSize = 1;

    d = new QFontPrivate;
    Q_CHECK_PTR( d );

    d->request.family = family;
    d->request.pointSize = pointSize * 10;
    d->request.pixelSize = -1;
    d->request.weight = weight;
    d->request.italic = italic;
}


/*! Constructs a font that is a copy of \a font.
*/
QFont::QFont( const QFont &font )
{
    d = font.d;
    d->ref();
}

/*! Destroys the font object and frees all allocated resources.
*/
QFont::~QFont()
{
    if ( d->deref() ) {
	delete d;
    }
}


/*! Assigns \a font to this font and returns a reference to it.
*/
QFont &QFont::operator=( const QFont &font )
{
    if ( font.d != d ) {
	if (d->deref()) delete d;

	d = font.d;
	d->ref();
    }

    return *this;
}


/*! Returns the requested font family name, i.e. the name set in the
 constructor or the last setFont() call.

  \sa setFamily() substitutes() substitute()
*/
QString QFont::family() const
{
    return d->request.family;
}


/*!
    Sets the family name of the font. The name is case insensitive and
    may include a foundry name.

    The \a family name may optionally also include a foundry name, e.g.
    "Helvetica [Cronyx]". (The Qt 2.x syntax, i.e. "Cronyx-Helvetica",
    is also supported.) If the \a family is available from more than one
    foundry and the foundry isn't specified, an arbitrary foundry is
    chosen. If the family isn't available a family will be set using the
    \link #fontmatching font matching\endlink algorithm.

    \sa family(), setStyleHint(), QFontInfo
*/
void QFont::setFamily( const QString &family )
{
    if (d->request.family == family) return;

    detach();
    d->request.family = family;
    d->request.dirty  = TRUE;
}


/*! Returns the point size in 1/10ths of a point.

  The returned value will be -1 if the font size has been specified in
  pixels.

  \sa pointSize() pointSizeFloat()
  */
int QFont::deciPointSize() const
{
    return d->request.pointSize;
}


/*!
    Returns the point size of the font. Returns -1 if the font size was
    specified in pixels.

  \sa setPointSize() deciPointSize() pointSizeFloat()
*/
int QFont::pointSize() const
{
    return d->request.pointSize == -1 ? -1 : d->request.pointSize/ 10;
}


/*! Sets the point size to \a pointSize. The point size must be greater
  than zero.

  \sa pointSize() setPointSizeFloat()
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
    d->request.pixelSize = -1;
    d->request.dirty = TRUE;
}


/*! Sets the point size to \a pointSize. The point size must be greater
  than zero. The requested precision may not be achieved on all platforms.

  \sa pointSizeFloat() setPointSize() setPixelSize()
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
    d->request.pixelSize = -1;
    d->request.dirty = TRUE;
}


/*!
    Returns the point size of the font. Returns -1 if the font size was
    specified in pixels.

  \sa pointSize() setPointSizeFloat() pixelSize() QFontInfo::pointSize() QFontInfo::pixelSize()
*/
float QFont::pointSizeFloat() const
{
    return float(d->request.pointSize == -1 ? -10 : d->request.pointSize) / 10.0;
}


/*! Sets the font size to \a pixelSize pixels.

    Using this function makes the font device dependent. Use
    setPointSize() or setPointSizeFloat() to set the size of the font in
    a device independent manner.

  \sa pixelSize()
*/
void QFont::setPixelSize( int pixelSize )
{
    if ( pixelSize <= 0 ) {
#if defined(CHECK_RANGE)
	qWarning( "QFont::setPointSize: Point size <= 0 (%f)", pointSize );
#endif
	return;
    }
    if (d->request.pixelSize == pixelSize) return;

    detach();
    d->request.pixelSize = pixelSize;
    d->request.pointSize = -1;
    d->request.dirty = TRUE;
}

/*!
    Returns the pixel size of the font if it was set with
    setPixelSize(). Returns -1 if the size was set with setPointSize()
    or setPointSizeFloat().

  \sa setPixelSize() pointSize() QFontInfo::pointSize() QFontInfo::pixelSize()
*/
int QFont::pixelSize() const
{
    return d->request.pixelSize;
}


/*! \obsolete

  Sets the logical pixel height of font characters when shown on
  the screen to \a pixelSize.
*/
void QFont::setPixelSizeFloat( float pixelSize )
{
    setPixelSize( (int)pixelSize );
}


/*!
    Returns TRUE if italic has been set; otherwise returns FALSE.

  \sa setItalic()
*/
bool QFont::italic() const
{
    return d->request.italic;
}


/*!
    If \a enable is TRUE, italic is set on; otherwise italic is set off.

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
    Returns the weight of the font which is one of the enumerated values
    from \l{QFont::Weight}.

  \sa setWeight(), Weight, QFontInfo
*/
int QFont::weight() const
{
    return d->request.weight;
}


/*! \enum QFont::Weight

  Qt uses a weighting scale from 0 to 99 similar to, but not
  the same as, the scales used in Windows or CSS. A weight of
  0 is ultralight, whilst 99 will be an extremely black.

  This enum contains the predefined font weights:

  \value Light 25
  \value Normal 50
  \value DemiBold 63
  \value Bold 75
  \value Black 87
*/

/*! Sets the weight the font to \a weight, which should be a value
  from the \l QFont::Weight enumeration.

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


/*! \fn bool QFont::bold() const

  Returns TRUE if weight() is a value greater than \link Weight
  QFont::Normal \endlink; otherwise returns FALSE.

  \sa weight(), setBold(), QFontInfo::bold()
*/

/*! \fn void QFont::setBold( bool enable )

  If \a enable is true sets the font's weight to
  \link Weight QFont::Bold \endlink; otherwise sets the weight to
  \link Weight QFont::Normal\endlink.

  For finer boldness control use setWeight().

  \sa bold(), setWeight()
*/


/*!
    Returns TRUE if underline has been set; otherwise returns FALSE.

  \sa setUnderline(), QFontInfo::underline()
*/
bool QFont::underline() const
{
    return d->request.underline;
}


/*!
    If \a enable is TRUE, sets underline on; otherwise sets underline
    off.

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
    Returns TRUE if strikeout has been set; otherwise returns FALSE.

  \sa setStrikeOut(), QFontInfo::strikeOut().
*/
bool QFont::strikeOut() const
{
    return d->request.strikeOut;
}


/*!
    If \a enable is TRUE, sets strikeout on; otherwise sets strikeout
    off.

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
    Returns TRUE if fixed pitch has been set; otherwise returns FALSE.

  \sa setFixedPitch(), QFontInfo::fixedPitch()
*/
bool QFont::fixedPitch() const
{
    return d->request.fixedPitch;
}


/*!

    If \a enable is TRUE, sets fixed pitch on; otherwise sets fixed
    pitch off.

  \sa fixedPitch(), QFontInfo
*/
void QFont::setFixedPitch( bool enable )
{
    if ((bool) d->request.fixedPitch == enable) return;

    detach();
    d->request.fixedPitch = enable;
    d->request.dirty = TRUE;
}


/*! Returns the StyleStrategy.

    The style strategy affects the \link #fontmatching font
    matching\endlink algorithm. See \l QFont::StyleStrategy for the list
    of strategies.

  \sa setStyleHint() QFont::StyleHint
*/
QFont::StyleStrategy QFont::styleStrategy() const
{
    return (StyleStrategy) d->request.styleStrategy;
}


/*! Returns the StyleHint.

    The style hint affects the \link #fontmatching font
    matching\endlink algorithm. See \l QFont::StyleHint for the list
    of strategies.

  \sa setStyleHint(), QFont::StyleStrategy QFontInfo::styleHint()
*/
QFont::StyleHint QFont::styleHint() const
{
    return (StyleHint) d->request.styleHint;
}


/*! \enum QFont::StyleHint

  Style hints are used by the \link #fontmatching font matching\endlink
  algorithm to find an appropriate default family if a selected font
  family is not available.

  \value AnyStyle leaves the font matching algorithm to choose the
  family. This is the default.

  \value SansSerif the font matcher prefer sans serif fonts.
  \value Helvetica is a synonym for \c SansSerif.

  \value Serif the font matcher prefers serif fonts.
  \value Times is a synonym for \c Serif.

  \value TypeWriter the font matcher prefers fixed pitch fonts.
  \value Courier a synonym for \c TypeWriter.

  \value OldEnglish the font matcher prefers decorative fonts.
  \value Decorative is a synonym for \c OldEnglish.

  \value System the font matcher prefers system fonts.
*/

/*! \enum QFont::StyleStrategy

  The style strategy tells the \link #fontmatching font matching\endlink
  algorithm what type of fonts should be used to find an appropriate
  default family.

  The following strategies are available:

  \value PreferDefault the default style strategy. It does not prefer
	 any type of font.
  \value PreferBitmap prefers bitmap fonts (as opposed to outline fonts).
  \value PreferDevice prefers device fonts.
  \value PreferOutline prefers outline fonts (as opposed to bitmap fonts).
  \value ForceOutline forces the use of outline fonts.
  \value NoAntialias don't antialias the fonts.
  \value PreferAntialias antialias if possible.

  Any of these may be OR-ed with one of these flags:
  \value PreferMatch prefer an exact match. The font matcher will try to
  use the exact font size that has been specified.
  \value PreferQuality prefer the best quality font. The font matcher
  will use the nearest standard point size that the font supports.

  Whilst all strategies work on Windows, they are currently ignored
  under X11.
*/

/*! Sets the style hint and strategy to \a hint and \a strategy,
  respectively.

    If these aren't set explicitly the style hint will default to
    \c AnyStyle and the style strategy to \c PreferDefault.

  \sa StyleHint, styleHint(), StyleStrategy, styleStrategy(), QFontInfo
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

/*!
  Sets the style strategy for the font to \a s.

  \sa QFont::StyleStrategy
*/
void QFont::setStyleStrategy( StyleStrategy s )
{
    if ( s == (StyleStrategy)d->request.styleStrategy ) return;
    detach();
    d->request.styleStrategy = s;
    d->request.dirty = TRUE;
}



/*!
    If \a enable is TRUE, turns raw mode on; otherwise turns raw mode
    off. This function only has an affect under X11.

  If raw mode is enabled, Qt will search for an X font with a complete
  font name matching the family name, ignoring all other values set for
  the QFont.  If the font name matches several fonts, Qt will use the
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

    Returns TRUE if this font is equal to \a f; otherwise returns FALSE.

  Two QFonts are considered equal if their font attributes are equal.  If
  rawMode() is enabled for both fonts, only the family fields are
  compared.

  \sa operator!=() isCopyOf()
*/
bool QFont::operator==( const QFont &f ) const
{
    return f.d == d || f.key() == key();
}


/*!

    Returns TRUE if this font is different from \a f; otherwise returns
    FALSE.

  Two QFonts are considered to be different if their font attributes are
  different.  If rawMode() is enabled for both fonts, only the family
  fields are compared.

  \sa operator==()
*/
bool QFont::operator!=( const QFont &f ) const
{
    return !(operator==( f ));
}


/*!

    Returns TRUE if this font and \a f are copies of each other, i.e.
    one of them was created as a copy of the other and neither has been
    modified since.  This is much stricter than equality.

  \sa operator=() operator==()
*/
bool QFont::isCopyOf( const QFont & f ) const
{
    return d && d == f.d;
}


/*! Returns the family name that corresponds to the current style hint.

  \sa StyleHint styleHint() setStyleHint()
*/
QString QFont::defaultFamily() const
{
    return d->defaultFamily();
}


/*!
    Returns the "last resort" font family name.

    The current implementation tries a wide variety of common fonts,
    returning the first one it finds. Is is possible that no family is
    found in which case a null string is returned.

  \sa lastResortFont()
*/
QString QFont::lastResortFamily() const
{
    return d->lastResortFamily();
}


/*!
    Returns a "last resort" font name for the font matching algorithm.
    This is used if the last resort family is not available. It will
    always return a name, if necessary returning something like "fixed"
    or "system".

  The current implementation tries a wide variety of common fonts,
  returning the first one it finds. This implementation may change at
  any time, but this function will always return a string containing
  something.

  It is theoretically possible that there really isn't a
  lastResortFont() in which case Qt will abort with an error message. We
  have not been able to identify a case where this happens. Please <a
  href="bughowto.html">report it as a bug</a> if it does, preferably
  with a list of the fonts you have installed.

  \sa lastResortFamily() rawName()
*/
QString QFont::lastResortFont() const
{
    return d->lastResortFont();
}


/*!
    Returns TRUE if raw mode is used for font name matching; otherwise
    returns FALSE.

  \sa setRawMode() rawName()
*/
bool QFont::rawMode() const
{
    return d->request.rawMode;
}


#ifndef QT_NO_COMPAT

/*! \obsolete

  Please use QApplication::font() instead.
*/
QFont QFont::defaultFont()
{
    return QApplication::font();
}


/*! \obsolete

  Please use QApplication::setFont() instead.
*/
void  QFont::setDefaultFont( const QFont &f )
{
    QApplication::setFont( f );
}


#endif




#ifndef QT_NO_STRINGLIST

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

    if (fontSubst)
	return;

    fontSubst = new QFontSubst(17, FALSE);
    Q_CHECK_PTR( fontSubst );
    fontSubst->setAutoDelete( TRUE );
    qfont_cleanup_fontsubst.add(&fontSubst);

    for ( int i=0; initTbl[i] != 0; i += 2 )
	QFont::insertSubstitution(QString::fromLatin1(initTbl[i]),
				  QString::fromLatin1(initTbl[i+1]));
}


/*! Returns the first family name to be used whenever \a familyName is
  specified. The lookup is case insensitive.

  If there is no substitution for \a familyName, \a familyName is
  returned.

  To obtain a list of substitutions use substitutes().

  \sa setFamily() insertSubstitutions() insertSubstitution() removeSubstitution()
*/
QString QFont::substitute( const QString &familyName )
{
    initFontSubst();

    QStringList *list = fontSubst->find(familyName);
    if (list && list->count() > 0)
	return *(list->at(0));

    return familyName;
}


/*! Returns a list of family names to be used whenever \a familyName is
  specified.  The lookup is case insensitive.

  If there is no substitution for \a familyName, an empty
  list is returned.

   \sa substitute() insertSubstitutions() insertSubstitution() removeSubstitution()
 */
QStringList QFont::substitutes(const QString &familyName)
{
    initFontSubst();

    QStringList ret, *list = fontSubst->find(familyName);
    if (list)
	ret += *list;
    return ret;
}


/*!
    Inserts the family name \a substituteName into the substitution
    table for \a familyName.

  \sa insertSubstitutions() removeSubstitution() substitutions() substitute() substitutes()
*/
void QFont::insertSubstitution(const QString &familyName,
			       const QString &substituteName)
{
    initFontSubst();

    QStringList *list = fontSubst->find(familyName);
    if (! list) {
	list = new QStringList;
	fontSubst->insert(familyName, list);
    }

    if (! list->contains(substituteName))
	list->append(substituteName);
}


/*! Inserts the list of families \a substituteNames into the substitution
  list for \a familyName.

  \sa insertSubstitution(), removeSubstitution(), substitutions(), substitute()
*/
void QFont::insertSubstitutions(const QString &familyName,
				const QStringList &substituteNames)
{
    initFontSubst();

    QStringList *list = fontSubst->find(familyName);
    if (! list) {
	list = new QStringList;
	fontSubst->insert(familyName, list);
    }

    QStringList::ConstIterator it = substituteNames.begin();
    while (it != substituteNames.end()) {
	if (! list->contains(*it))
	    list->append(*it);
	it++;
    }
}

// ### mark: should be called removeSubstitutions()
/*!
    Removes all the substitutions for \a familyName.

  \sa insertSubstitutions(), insertSubstitution(), substitutions(), substitute()
*/
void QFont::removeSubstitution( const QString &familyName )
{ // ### function name should be removeSubstitutions() or
  // ### removeSubstitutionList()
    initFontSubst();

    fontSubst->remove(familyName);
}


/*! Returns a sorted list of substituted family names.

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
    \internal
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
    \internal
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
    Returns the font's key, a textual representation of a font. It is
    typically used as the key for a cache or dictionary of fonts.

  \sa QMap
*/
QString QFont::key() const
{
    return d->key();
}

#ifndef QT_NO_STRINGLIST
/*! Returns a description of the font.  The description is a comma-separated
  list of the attributes, perfectly suited for use in QSettings.

  \sa fromString() operator<<()
 */
QString QFont::toString() const
{
    QStringList l;
    l.append(family());
    l.append(QString::number(pointSize()));
    l.append(QString::number(pixelSize()));
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
    is a comma-separated list of the font attributes, as returned by
    toString().

  \sa toString() operator>>()
 */
bool QFont::fromString(const QString &descrip)
{
    QStringList l(QStringList::split(',', descrip));

    int count = l.count();
    if (count != 10 && count != 9) {

#ifdef QT_CHECK_STATE
	qWarning("QFont::fromString: invalid description '%s'", descrip.latin1());
#endif

	return FALSE;
    }

    setFamily(l[0]);
    setPointSize(l[1].toInt());
    if ( count == 9 ) {
	setStyleHint((StyleHint) l[2].toInt());
	setWeight(l[3].toInt());
	setItalic(l[4].toInt());
	setUnderline(l[5].toInt());
	setStrikeOut(l[6].toInt());
	setFixedPitch(l[7].toInt());
	setRawMode(l[8].toInt());
    } else {
	setPixelSize(l[2].toInt());
	setStyleHint((StyleHint) l[3].toInt());
	setWeight(l[4].toInt());
	setItalic(l[5].toInt());
	setUnderline(l[6].toInt());
	setStrikeOut(l[7].toInt());
	setFixedPitch(l[8].toInt());
	setRawMode(l[9].toInt());
    }

    return TRUE;
}
#endif // QT_NO_STRINGLIST

#if !defined( Q_WS_QWS ) // && !defined( Q_WS_MAC )
/*! \internal

  Internal function that dumps font cache statistics.
*/
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
#elif defined(Q_WS_MAC)
	qDebug( "   [we need to implement this]"); //XXX
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

/*! \relates QFont

  Writes the font \a font to the data stream \a s. (toString() writes to
  a text stream.)

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

    if ( s.version() <= 3 ) {
	Q_INT16 pointSize = (Q_INT16) font.d->request.pointSize;
	if ( pointSize == -1 )
	    pointSize = (Q_INT16)QFontInfo( font ).pointSize() * 10;
	s << pointSize;
    } else {
	s << (Q_INT16) font.d->request.pointSize;
	s << (Q_INT16) font.d->request.pixelSize;
    }

    return s << (Q_UINT8) font.d->request.styleHint
	     << (Q_UINT8) 0
	     << (Q_UINT8) font.d->request.weight
	     << get_font_bits(font.d->request);
}


/*! \relates QFont
  Reads the font \a font from the data stream \a s. (fromString() reads from
  a text stream.)

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>( QDataStream &s, QFont &font )
{
    if (font.d->deref()) delete font.d;

    font.d = new QFontPrivate;

    Q_INT16 pointSize, pixelSize = -1;
    Q_UINT8 styleHint, charSet, weight, bits;

    if ( s.version() == 1 ) {
	QCString fam;
	s >> fam;
	font.d->request.family = QString( fam );
    } else {
	s >> font.d->request.family;
    }

    s >> pointSize;
    if ( s.version() >= 4 )
	s >> pixelSize;
    s >> styleHint;
    s >> charSet;
    s >> weight;
    s >> bits;

    font.d->request.pointSize = pointSize;
    font.d->request.pixelSize = pixelSize;
    font.d->request.styleHint = styleHint;
    font.d->request.weight = weight;
    font.d->request.dirty = TRUE;

    set_font_bits( bits, &(font.d->request) );

    return s;
}

#endif // QT_NO_DATASTREAM




/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

// invariant: this list contains pointers to ALL QFontMetrics objects
// with non-null painter pointers, and no other objects.  Callers of
// these functions must maintain this invariant.

typedef QPtrList<QFontMetrics> QFontMetricsList;
static QFontMetricsList *fm_list = 0;

QCleanupHandler<QFontMetricsList> qfont_cleanup_fontmetricslist;

static void insertFontMetrics( QFontMetrics *fm ) {
    if ( !fm_list ) {
	fm_list = new QFontMetricsList;
	Q_CHECK_PTR( fm_list );
	qfont_cleanup_fontmetricslist.add( &fm_list );
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
	QPtrListIterator<QFontMetrics> it( *fm_list );
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


/*! \class QFontMetrics qfontmetrics.h
  \brief The QFontMetrics class provides font metrics information.

  \ingroup graphics
  \ingroup shared

  QFontMetrics functions calculate size of characters and strings for
  a given font. There are three ways you can create a QFontMetrics object:

    \list 1
  \i Calling the QFontMetrics constructor with a QFont creates a font
  metrics object for a screen-compatible font, i.e. the font cannot be a
  printer font<sup>*</sup>. If the font is changed later, the font
  metrics object is \e not updated.

  \i QWidget::fontMetrics() returns the font metrics for a widget's font.
  This is equivalent to QFontMetrics(widget->font()).  If the widget's
  font is changed later, the font metrics object is \e not updated.

  \i QPainter::fontMetrics() returns the font metrics for a painter's
  current font. The font metrics object is \e automatically updated if
  you set a new painter font.
  \endlist

    <sup>*</sup> If you use a printer font the values returned will
    almost certainly be inaccurate. Printer fonts are not always
    accessible so the nearest screen font is used if a printer font is
    supplied.

  Once created, the object provides functions to access the individual
  metrics of the font, its characters, and for strings rendered in
  the font.

  There are several functions that operate on the font: ascent(),
  descent(), height(), leading() and lineSpacing() return the basic
  size properties of the font. The underlinePos(), strikeOutPos() and
  lineWidth() functions, return the properties of the line that
  underlines or strikes out the characters.  These functions are all
  fast.

  There are also some functions that operate on the set of glyphs in
  the font: minLeftBearing(), minRightBearing() and maxWidth().  These
  are by necessity slow, and we recommend avoiding them if possible.

  For each character, you can get its width(), leftBearing() and
  rightBearing() and find out whether it is in the font using
  inFont().  You can also treat the character as a string, and use the
  string functions on it.

  The string functions include width(), to return the width of a
  string in pixels (or points, for a printer), boundingRect(), to
  return a rectangle large enough to contain the rendered string, and
  size(), to return the size of that rectangle.

  Example:
  \code
    QFont font( "times", 24 );
    QFontMetrics fm (font );
    int pixelsWide = fm.width( "What's the width of this text?" );
    int pixelsHigh = fm.height();
  \endcode

  \sa QFont QFontInfo QFontDatabase
*/


/*! Constructs a font metrics object for \a font.

  The font must be screen-compatible, i.e. a font you use when drawing
  text in QWidget or QPixmap objects, not QPicture or QPrinter.

  The font metrics object holds the information for the font that is
  passed in the constructor at the time it is created, and is not
  updated if the font's attributes are changed later.

  Use QPainter::fontMetrics() to get the font metrics when painting.
  This is a little slower than using this constructor, but it always
  gives correct results because the font info data is updated.
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


/*! \internal

  Constructs a font metrics object for the painter \a p.
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
    if ( painter->testf(QPainter::DirtyFont) )
	painter->updateFont();
    if ( painter->pfont )
	d = painter->pfont->d;
    else
	d = painter->cfont.d;
    d->ref();

    d->load();

    flags = 0;

    insertFontMetrics( this );
}


/*! Constructs a copy of \a fm.
*/
QFontMetrics::QFontMetrics( const QFontMetrics &fm )
    : d(fm.d), painter(fm.painter), flags(fm.flags)
{
    d->ref();
    if ( painter )
	insertFontMetrics( this );
}


/*! Destroys the font metrics object and frees all allocated resources.
*/
QFontMetrics::~QFontMetrics()
{
    if ( painter )
	removeFontMetrics( this );
    if ( d->deref() )
	delete d;
}


/*! Assigns the font metrics \a fm.
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


/*! \overload

  Returns the bounding rectangle of the character \a ch relative to the
  left-most point on the base line.

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


/*! \overload

  Returns the bounding rectangle of the first \e len characters of \e str,
  which is the set of pixels the text would cover if drawn at (0,0). The
  drawing, and hence the bounding rectangle, is constrained to the rectangle
  (\a x, \a y, \a w, \a h).

  If \a len is negative (which is the default), the entire string is used.

  The \a flgs argument is the bitwise OR of the following flags:
  <ul>
  <li> \c AlignAuto aligns to the left border for all languages except Hebrew and Arabic where it aligns to the right.
  <li> \c AlignLeft aligns to the left border.
  <li> \c AlignRight aligns to the right border.
  <li> \c AlignJustify produces justified text.
  <li> \c AlignHCenter aligns horizontally centered.
  <li> \c AlignTop aligns to the top border.
  <li> \c AlignBottom aligns to the bottom border.
  <li> \c AlignVCenter aligns vertically centered
  <li> \c AlignCenter (= \c AlignHCenter | AlignVCenter)
  <li> \c SingleLine ignores newline characters in the text.
  <li> \c ExpandTabs expands tabs (see below)
  <li> \c ShowPrefix interprets "&x" as "x" underlined.
  <li> \c WordBreak breaks the text to fit the rectangle.
  </ul>

  Horizontal alignment defaults to AlignAuto and vertical alignment
  defaults to AlignTop.

  If several of the horizontal or several of the vertical alignment flags
  are set, the resulting alignment is undefined.

  These flags are defined in qnamespace.h.

  If \c ExpandTabs is set in \a flgs, then:
  if \a tabarray is non-null, it specifies a 0-terminated sequence
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

  The \a intern argument should not be used.

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
    qt_format_text( QFont( d, (bool)FALSE ), r, flgs, str, len, &rb,
		    tabstops, tabarray, tabarraylen, intern, 0 );

    return rb;
}


/*! Returns the size in pixels of the first \a len characters of \a str.

  If \a len is negative (the default), the entire string is used.

  The \a flgs argument is the bitwise OR of the following flags:
  <ul>
  <li> \c SingleLine ignores newline characters.
  <li> \c ExpandTabs expands tabs (see below)
  <li> \c ShowPrefix interprets "&x" as "x" underlined.
  <li> \c WordBreak breaks the text to fit the rectangle.
  </ul>

  These flags are defined in qnamespace.h.

  If \c ExpandTabs is set in \a flgs, then:
  if \a tabarray is non-null, it specifies a 0-terminated sequence
  of pixel-positions for tabs; otherwise
  if \a tabstops is non-zero, it is used as the tab spacing (in pixels).

  Newline characters are processed as linebreaks.

  Despite the different actual character heights, the heights of the
  bounding rectangles of "Yes" and "yes" are the same.

  The \a intern argument should not be used.

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

typedef QPtrList<QFontInfo> QFontInfoList;
static QFontInfoList *fi_list = 0;

QCleanupHandler<QFontInfoList> qfont_cleanup_fontinfolist;

static void insertFontInfo( QFontInfo *fi )
{
    if ( !fi_list ) {
	fi_list = new QFontInfoList;
	Q_CHECK_PTR( fi_list );
	qfont_cleanup_fontinfolist.add( &fi_list );
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
	QPtrListIterator<QFontInfo> it( *fi_list );
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


/*! \class QFontInfo qfontinfo.h

  \brief The QFontInfo class provides general information about fonts.

  \ingroup graphics
  \ingroup shared

  The QFontInfo class provides the same access functions as QFont, e.g.
  family(), pointSize(), italic(), weight(), fixedPitch(), styleHint()
  etc. But whilst the QFont access functions return the values that were
  set, a QFontInfo object returns the values that apply to the font that
  will actually be used to draw the text.

  For example, when the program asks for a 25pt Courier font on a
  machine that has a 24pt Courier font but not a scalable one, QFont
  will (normally) use the 24pt Courier for rendering.  In this case,
  QFont::pointSize() returns 25 and QFontInfo::pointSize() 24.

  There are three ways to create a QFontInfo object.
    \list 1
  \i Calling the QFontInfo constructor with a QFont creates a font info
  object for a screen-compatible font, i.e. the font cannot be a printer
  font<sup>*</sup>. If the font is changed later, the font info object
  is \e not updated.

  \i QWidget::fontInfo() returns the font info for a widget's font.  This
  is equivalent to calling QFontInfo(widget->font()). If the widget's
  font is changed later, the font info object is \e not updated.

  \i QPainter::fontInfo() returns the font info for a painter's current
  font. The font info object is \e automatically updated if you set
  a new painter font.
  \endlist

    <sup>*</sup> If you use a printer font the values returned will
    almost certainly be inaccurate. Printer fonts are not always
    accessible so the nearest screen font is used if a printer font is
    supplied.

  \sa QFont QFontMetrics QFontDatabase
*/


/*! Constructs a font info object for \a font.

  The font must be screen-compatible, i.e. a font you use when drawing
  text in \link QWidget widgets\endlink or \link QPixmap pixmaps\endlink.

  The font info object holds the information for the font that is
  passed in the constructor at the time it is created, and is not
  updated if the font's attributes are changed later.

  Use the QPainter::fontInfo() function to get the font info when
  painting. This is a little slower than using this constructor, but it
  always gives correct results because the font info data is updated.
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


/*! \internal

  Constructs a font info object for the painter \a p.
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
    if ( painter->testf(QPainter::DirtyFont) )
	painter->updateFont();
    if ( painter->pfont )
	d = painter->pfont->d;
    else
	d = painter->cfont.d;
    d->ref();

    d->load();

    flags = 0;

    insertFontInfo( this );
}


/*! Constructs a copy of \a fi.
*/
QFontInfo::QFontInfo( const QFontInfo &fi )
    : d(fi.d), painter(fi.painter), flags(fi.flags)
{
    d->ref();
    if ( painter )
	insertFontInfo( this );
}


/*! Destroys the font info object.
*/
QFontInfo::~QFontInfo()
{
    if ( painter )
	removeFontInfo( this );
    if (d->deref())
	delete d;
}


/*! Assigns the font info in \a fi.
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
    Returns the pixel size of the matched window system font.

  \sa QFont::pointSize()
*/
int QFontInfo::pixelSize() const
{
    return d->actual.pixelSize;
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


/*! \fn bool QFontInfo::bold() const

  Returns TRUE if weight() would return a value greater than
  \c QFont::Normal; otherwise returns FALSE.

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
    Returns the strikeout value of the matched window system font.

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

  \sa QFont::fixedPitch()
*/
bool QFontInfo::fixedPitch() const
{
    return d->actual.fixedPitch;
}


/*!
    Returns the style of the matched window system font.

  Currently only returns the style hint set in QFont.

  \sa QFont::styleHint() QFont::StyleHint
*/
QFont::StyleHint QFontInfo::styleHint() const
{
    return (QFont::StyleHint) d->actual.styleHint;
}


/*! Returns TRUE if the font is a raw mode font.

  If it is a raw mode font, all other functions in QFontInfo will return the
  same values set in the QFont, regardless of the font actually used.

  \sa QFont::rawMode()
*/
bool QFontInfo::rawMode() const
{
    return d->actual.rawMode;
}


/*!
    Returns TRUE if the matched window system font is exactly the same
    as the one specified by the font.

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

// when debugging the font cache - clean it out more aggressively
#ifndef QFONTCACHE_DEBUG
static const int qtFontCacheFastTimeout =  30000;
static const int qtFontCacheSlowTimeout = 300000;
#else // !QFONTCACHE_DEBUG
static const int qtFontCacheFastTimeout = 10000;
static const int qtFontCacheSlowTimeout = 30000;
#endif // QFONTCACHE_DEBUG

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

	if (qfs == (QFontStruct *) -1)
	    take(key);
    }
}


bool QFontCache::insert(const QString &key, const QFontStruct *qfs, int cost)
{

#ifdef QFONTCACHE_DEBUG
    qDebug("QFC::insert: inserting %p w/ cost %d\n%s", qfs, cost,
	   (qfs == (QFontStruct *) -1) ? "negative cache item" : qfs->name.data());
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
    if (qfs == (QFontStruct *) -1)
	return;

    if (qfs->count == 0) {

#ifdef QFONTCACHE_DEBUG
	qDebug("QFC::deleteItem: removing %p from cache\n%s", qfs,
	       (qfs == (QFontStruct *) -1) ? "negative cache item" : qfs->name.data());
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
	    if (qfs->count > 0)
		nmcost += qfs->cache_cost;
	} else
	    // keep negative cache items in the cache
	    nmcost++;
    }

    nmcost = QMAX(tqcost, nmcost);
    if (nmcost < qtFontCacheMin)
	nmcost = qtFontCacheMin;

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
	return request.family;

#if 1
    int len = (request.family.length() * 2) +
	      (request.addStyle.length() * 2) +
	      2 +  // point size
	      2 +  // pixel size
	      1 +  // font bits
	      1 +  // weight
	      1;   // hint

    QByteArray buf(len);
    uchar *p = (uchar *) buf.data();

    memcpy((char *) p, (char *) request.family.unicode(),
	   (request.family.length() * 2));
    p += request.family.length() * 2;

    if (request.addStyle.length() > 0) {
	memcpy((char *) p, (char *) request.addStyle.unicode(),
	       (request.addStyle.length() * 2));
	p += request.addStyle.length() * 2;
    }

    *((Q_UINT16 *) p) = request.pointSize; p += 2;
    *((Q_UINT16 *) p) = request.pixelSize; p += 2;
    *p++ = get_font_bits( request );
    *p++ = request.weight;
    *p++ = (request.hintSetByUser ?
	    (int) request.styleHint : (int) QFont::AnyStyle);

    return QString((QChar *) buf.data(), buf.size() / 2);
#else
    // this version is for debugging as it gives better readable strings.
    QString k = request.family;
    if ( request.addStyle.length() )
	k += request.addStyle;
    k += "%1/%2/%3/%4/%5";
    k = k.arg( request.pointSize ).arg( request.pixelSize ).arg( get_font_bits( request ) ).arg( request.weight )
	.arg( (request.hintSetByUser ? (int) request.styleHint : (int) QFont::AnyStyle) );
    return k;
#endif
}

QFont::Script QFontPrivate::scriptForChar( const QChar &c )
{
#ifndef Q_WS_QWS
    uchar row = c.row();
    uchar cell = c.cell();

    // Thankfully LatinBasic is more or less == ISO-8859-1
    if (! row)
	return QFont::LatinBasic;

    switch ( row ) {
    case 0x01:
	// There are no typos here... really...
	switch (cell) {
	case 0x00: return QFont::LatinExtendedA_4;
	case 0x01: return QFont::LatinExtendedA_4;
	case 0x02: return QFont::LatinExtendedA_2;
	case 0x03: return QFont::LatinExtendedA_2;
	case 0x04: return QFont::LatinExtendedA_2;
	case 0x05: return QFont::LatinExtendedA_2;
	case 0x06: return QFont::LatinExtendedA_2;
	case 0x07: return QFont::LatinExtendedA_2;
	case 0x08: return QFont::LatinExtendedA_3;
	case 0x09: return QFont::LatinExtendedA_3;
	case 0x0A: return QFont::LatinExtendedA_3;
	case 0x0B: return QFont::LatinExtendedA_3;
	case 0x0C: return QFont::LatinExtendedA_2;
	case 0x0D: return QFont::LatinExtendedA_2;
	case 0x0E: return QFont::LatinExtendedA_2;
	case 0x0F: return QFont::LatinExtendedA_2;
	case 0x10: return QFont::LatinExtendedA_2;
	case 0x11: return QFont::LatinExtendedA_2;
	case 0x12: return QFont::LatinExtendedA_4;
	case 0x13: return QFont::LatinExtendedA_4;
	case 0x16: return QFont::LatinExtendedA_4;
	case 0x17: return QFont::LatinExtendedA_4;
	case 0x18: return QFont::LatinExtendedA_2;
	case 0x19: return QFont::LatinExtendedA_2;
	case 0x1A: return QFont::LatinExtendedA_2;
	case 0x1B: return QFont::LatinExtendedA_2;
	case 0x1C: return QFont::LatinExtendedA_3;
	case 0x1D: return QFont::LatinExtendedA_3;
	case 0x1E: return QFont::LatinExtendedA_3;
	case 0x1F: return QFont::LatinExtendedA_3;
	case 0x20: return QFont::LatinExtendedA_3;
	case 0x21: return QFont::LatinExtendedA_3;
	case 0x22: return QFont::LatinExtendedA_4;
	case 0x23: return QFont::LatinExtendedA_4;
	case 0x24: return QFont::LatinExtendedA_3;
	case 0x25: return QFont::LatinExtendedA_3;
	case 0x26: return QFont::LatinExtendedA_3;
	case 0x27: return QFont::LatinExtendedA_3;
	case 0x28: return QFont::LatinExtendedA_4;
	case 0x29: return QFont::LatinExtendedA_4;
	case 0x2A: return QFont::LatinExtendedA_4;
	case 0x2B: return QFont::LatinExtendedA_4;
	case 0x2E: return QFont::LatinExtendedA_4;
	case 0x2F: return QFont::LatinExtendedA_4;
	case 0x30: return QFont::LatinExtendedA_3;
	case 0x31: return QFont::LatinExtendedA_3;
	case 0x34: return QFont::LatinExtendedA_3;
	case 0x35: return QFont::LatinExtendedA_3;
	case 0x36: return QFont::LatinExtendedA_4;
	case 0x37: return QFont::LatinExtendedA_4;
	case 0x38: return QFont::LatinExtendedA_4;
	case 0x39: return QFont::LatinExtendedA_2;
	case 0x3A: return QFont::LatinExtendedA_2;
	case 0x3B: return QFont::LatinExtendedA_4;
	case 0x3C: return QFont::LatinExtendedA_4;
	case 0x3D: return QFont::LatinExtendedA_2;
	case 0x3E: return QFont::LatinExtendedA_2;
	case 0x41: return QFont::LatinExtendedA_2;
	case 0x42: return QFont::LatinExtendedA_2;
	case 0x43: return QFont::LatinExtendedA_2;
	case 0x44: return QFont::LatinExtendedA_2;
	case 0x45: return QFont::LatinExtendedA_4;
	case 0x46: return QFont::LatinExtendedA_4;
	case 0x47: return QFont::LatinExtendedA_2;
	case 0x48: return QFont::LatinExtendedA_2;
	case 0x4A: return QFont::LatinExtendedA_4;
	case 0x4B: return QFont::LatinExtendedA_4;
	case 0x4C: return QFont::LatinExtendedA_4;
	case 0x4D: return QFont::LatinExtendedA_4;
	case 0x50: return QFont::LatinExtendedA_2;
	case 0x51: return QFont::LatinExtendedA_2;
	case 0x52: return QFont::LatinExtendedA_15;
	case 0x53: return QFont::LatinExtendedA_15;
	case 0x54: return QFont::LatinExtendedA_2;
	case 0x55: return QFont::LatinExtendedA_2;
	case 0x56: return QFont::LatinExtendedA_4;
	case 0x57: return QFont::LatinExtendedA_4;
	case 0x58: return QFont::LatinExtendedA_2;
	case 0x59: return QFont::LatinExtendedA_2;
	case 0x5A: return QFont::LatinExtendedA_2;
	case 0x5B: return QFont::LatinExtendedA_2;
	case 0x5C: return QFont::LatinExtendedA_3;
	case 0x5D: return QFont::LatinExtendedA_3;
	case 0x5E: return QFont::LatinExtendedA_2;
	case 0x5F: return QFont::LatinExtendedA_2;
	case 0x60: return QFont::LatinExtendedA_2;
	case 0x61: return QFont::LatinExtendedA_2;
	case 0x62: return QFont::LatinExtendedA_2;
	case 0x63: return QFont::LatinExtendedA_2;
	case 0x64: return QFont::LatinExtendedA_2;
	case 0x65: return QFont::LatinExtendedA_2;
	case 0x66: return QFont::LatinExtendedA_4;
	case 0x67: return QFont::LatinExtendedA_4;
	case 0x68: return QFont::LatinExtendedA_4;
	case 0x69: return QFont::LatinExtendedA_4;
	case 0x6A: return QFont::LatinExtendedA_4;
	case 0x6B: return QFont::LatinExtendedA_4;
	case 0x6C: return QFont::LatinExtendedA_3;
	case 0x6D: return QFont::LatinExtendedA_3;
	case 0x6E: return QFont::LatinExtendedA_2;
	case 0x6F: return QFont::LatinExtendedA_2;
	case 0x70: return QFont::LatinExtendedA_2;
	case 0x71: return QFont::LatinExtendedA_2;
	case 0x72: return QFont::LatinExtendedA_4;
	case 0x73: return QFont::LatinExtendedA_4;
	case 0x74: return QFont::LatinExtendedA_14;
	case 0x75: return QFont::LatinExtendedA_14;
	case 0x76: return QFont::LatinExtendedA_14;
	case 0x77: return QFont::LatinExtendedA_14;
	case 0x78: return QFont::LatinExtendedA_15;
	case 0x79: return QFont::LatinExtendedA_2;
	case 0x7A: return QFont::LatinExtendedA_2;
	case 0x7B: return QFont::LatinExtendedA_2;
	case 0x7C: return QFont::LatinExtendedA_2;
	case 0x7D: return QFont::LatinExtendedA_2;
	case 0x7E: return QFont::LatinExtendedA_2;
	}
	return QFont::Latin;

    case 0x02:
	if (cell <= 0xaf)
	    return QFont::Latin;
	return QFont::SpacingModifiers;

    case 0x03:
	if (cell <= 0x6f)
	    return QFont::CombiningMarks;
	return QFont::Greek;

    case 0x04:
	return QFont::Cyrillic;

    case 0x05:
	if (cell <= 0x2f)
	    break;
	if (cell <= 0x8f)
	    return QFont::Armenian;
	return QFont::Hebrew;

    case 0x06:
	return QFont::Arabic;

    case 0x07:
	if (cell <= 0x4f)
	    return QFont::Syriac;
	if (cell <= 0x7f)
	    break;
	if (cell <= 0xbf)
	    return QFont::Thaana;
	break;

    case 0x09:
	if (cell <= 0x7f)
	    return QFont::Devanagari;
	return QFont::Bengali;

    case 0x0a:
	if (cell <= 0x7f)
	    return QFont::Gurmukhi;
	return QFont::Gujarati;

    case 0x0b:
	if ( cell <= 0x7f )
	    return QFont::Oriya;
	return QFont::Tamil;

    case 0x0c:
	if (cell <= 0x7f)
	    return QFont::Telugu;
	return QFont::Kannada;

    case 0x0d:
	if (cell <= 0x7f)
	    return QFont::Malayalam;
	return QFont::Sinhala;

    case 0x0e:
	if (cell <= 0x7f)
	    return QFont::Thai;
	return QFont::Lao;

    case 0x0f:
	if (cell <= 0xbf)
	    return QFont::Tibetan;
	break;

    case 0x10:
	if (cell <= 0x9f)
	    return QFont::Myanmar;
	return QFont::Georgian;

    case 0x11:
	return QFont::Hangul;

    case 0x12:
	return QFont::Ethiopic;

    case 0x13:
	if (cell <= 0x7f)
	    return QFont::Ethiopic;
	if (cell <= 0x8f)
	    break;
	return QFont::Cherokee;

    case 0x14:
    case 0x15:
	return QFont::CanadianAboriginal;

    case 0x16:
	if (cell <= 0x7f)
	    return QFont::CanadianAboriginal;
	if (cell <= 0x9f)
	    return QFont::Ogham;
	if (cell <= 0xf0)
	    return QFont::Runic;
	break;

    case 0x17:
	if (cell <= 0x7f)
	    break;
	return QFont::Khmer;

    case 0x18:
	if (cell <= 0xaf)
	    return QFont::Mongolian;
	break;

    case 0x1e:
	return QFont::Latin;

    case 0x1f:
	return QFont::Greek;

    case 0x20:
	if (cell <= 0x6f)
	    break;
	if (cell <= 0x9f)
	    return QFont::NumberForms;
	if (cell <= 0xcf)
	    return QFont::CurrencySymbols;
	return QFont::CombiningMarks;

    case 0x21:
	if (cell <= 0x4f)
	    return QFont::LetterlikeSymbols;
	if (cell <= 0x8f)
	    return QFont::NumberForms;
	return QFont::MathematicalOperators;

    case 0x22:
	return QFont::MathematicalOperators;

    case 0x23:
	return QFont::TechnicalSymbols;

    case 0x24:
	if (cell <= 0x5f)
	    return QFont::TechnicalSymbols;
	return QFont::EnclosedAndSquare;

    case 0x25:
	return QFont::GeometricSymbols;

    case 0x26:
    case 0x27:
	return QFont::MiscellaneousSymbols;

    case 0x28:
	return QFont::Braille;

    case 0x2e:
	if (cell <= 0x7f)
	    break;
#ifdef Q_WS_X11
	return hanHack( c );
#else
	return QFont::Han;
#endif

    case 0x2f:
	if (cell <= 0xd5) {
#ifdef Q_WS_X11
	    return hanHack( c );
#else
	    return QFont::Han;
#endif
	}
	if (cell <= 0xef)
	    break;
#ifdef Q_WS_X11
	return hanHack( c );
#else
	return QFont::Han;
#endif

    case 0x30:
	if (cell <= 0x3f) {
	    // Unified Han Symbols and Punctuation
#ifdef Q_WS_X11
	    return hanHack( c );
#else
	    return QFont::Han;
#endif
	}
	if (cell <= 0x9f)
	    return QFont::Hiragana;
	return QFont::Katakana;

    case 0x31:
	if (cell <= 0x2f)
	    return QFont::Bopomofo;

	// Hangul Compatibility Jamo
	if (cell <= 0x8f)
	    return QFont::Hangul;
	if (cell <= 0x9f) {
#ifdef Q_WS_X11
	    return hanHack( c );
#else
	    return QFont::Han;
#endif
	}
	break;

    case 0x32:
    case 0x33:
	return QFont::EnclosedAndSquare;

    case 0xa0:
    case 0xa1:
    case 0xa2:
    case 0xa3:
	return QFont::Yi;
    case 0xa4:
	if (cell <= 0xcf)
	    return QFont::Yi;
	break;

    case 0xfb:
	if (cell <= 0x06)
	    return QFont::Latin;
	if (cell <= 0x1c)
	    break;
	if (cell <= 0x4f)
	    return QFont::Hebrew;
	return QFont::Arabic;

    case 0xfc:
    case 0xfd:
	return QFont::Arabic;

    case 0xfe:
	if (cell <= 0x1f)
	    break;
	if (cell <= 0x2f)
	    return QFont::CombiningMarks;
	if (cell <= 0x6f)
	    break;
	return QFont::Arabic;

    case 0xff:
	// Hiragana half/full width forms block
	if (cell <= 0xef)
	    return QFont::Hiragana;
	break;
    }

    // Canadian Aboriginal Syllabics
    if (row >= 0x14 && (row < 0x16 || (row == 0x16 && cell <= 0x7f))) {
	return QFont::CanadianAboriginal;
    }

    // Hangul Syllables
    if (row >= 0xac && (row < 0xd7 || (row == 0xd7 && cell <= 0xa3))) {
	return QFont::Hangul;
    }

    if (// Unified Han + Extension-A
	(row >= 0x34 && row <= 0x9f) ||
	// Unified Han Compatibility
	(row >= 0xf9 && row <= 0xfa)
	) {
#ifdef Q_WS_X11
	return hanHack( c );
#else
	return QFont::Han;
#endif
    }

    // return QFont::UnknownScript;
#endif //Q_WS_QWS
    // "Qt/Embedded is Unicode throughout..."
    return QFont::Unicode;
}
