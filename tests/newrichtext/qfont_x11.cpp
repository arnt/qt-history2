/****************************************************************************
** $Id$
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for X11
**
** Created : 940515
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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

// REVISED: brad

#include "qplatformdefs.h"

#include "qfont.h"
#include "qfontdata_p.h"
#include <private/qcomplextext_p.h>
#include "qfontinfo.h"
#include "qfontdatabase.h"
#include "qfontmetrics.h"
#include "qpainter.h"
#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qregexp.h"
#include "qdict.h"
#include "qtextcodec.h"
#include "qcleanuphandler.h"
#include <private/qfontcodecs_p.h>

#ifndef QT_NO_XFTFREETYPE
# include "qintdict.h"
# include "qpixmap.h"
# include "qsettings.h"
#endif // QT_NO_XFTFREETYPE

#include "fontengine.h"
#include "fontenginexlfd.h"
#include "qtextdata.h"
#include "qt_x11.h"

#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#if !defined(QT_NO_XFTFREETYPE) && defined(QT_NO_XFTNAMEUNPARSE)
// Assume XFree86 4.0.3 implementation which is missing XftInitFtLibrary and
// XftNameUnparse
extern "C" {
Bool XftInitFtLibrary();
Bool XftNameUnparse (XftPattern *, char *, int);
}
#endif


#define QFONTLOADER_DEBUG
#define QFONTLOADER_DEBUG_VERBOSE

#define CN_ENCODINGS "gb18030-0", "gb18030.2000-0", "gbk-0", "gb2312.1980-0"
#define JP_ENCODINGS "jisx0208.1997-0", "jisx0208.1990-0", "jisx0208.1983-0"
#define KR_ENCODINGS "ksc5601.1987-0"
#define HK_ENCODINGS "big5hkscs-0", "hkscs-1"
#define TW_ENCODINGS "big5*-*", "big5-0"

static const int MAXFONTSIZE = 128;

// to get which font encodings are installed:
// xlsfonts | egrep -- "^-.*-.*-.*$" | cut -f 14- -d- | sort | uniq

// because of the way the font matcher works
// these lists must have at least 2 entries
static const char * const empty_encodings[] = { 0, 0 };
static const char * const latin_encodings[] = { "iso8859-1", "ascii-0", "adobe-fontspecific",
						"microsoft-symbol", 0 };
static const char * const greek_encodings[] = { "iso8859-7", 0 };
static const char * const cyrillic_encodings[] = { "koi8-ru", "koi8-u", "koi8-r",
						   "iso8859-5", "microsoft-cp1251", 0 };
static const char * const hebrew_encodings[] = { "iso8859-8", 0 };
static const char * const arabic_encodings[] = { "iso8859-6.8x" /*, "iso8859-6"*/, 0 };
static const char * const tamil_encodings[] = { "tscii-*", 0 };
static const char * const thai_encodings[] = { "tis620*-0", "iso8859-11", 0 };
static const char * const hiragana_encodings[] = { JP_ENCODINGS, 0 };
static const char * const katakana_encodings[] = { JP_ENCODINGS,  0 };
static const char * const hangul_encodings[] = { KR_ENCODINGS, 0 };
static const char * const bopomofo_encodings[] = { CN_ENCODINGS, 0 };
static const char * const unicode_encodings[] = { "iso10646-1", "unicode-*", 0 };
static const char * const hanx11_encodings[] =
    { CN_ENCODINGS, TW_ENCODINGS, HK_ENCODINGS, 0 };
static const char * const latinA2_encodings[] = { "iso8859-2", 0 };
static const char * const latinA3_encodings[] = { "iso8859-3", 0 };
static const char * const latinA4_encodings[] = { "iso8859-4", 0 };
static const char * const latinA14_encodings[] = { "iso8859-14", 0 };
static const char * const latinA15_encodings[] = { "iso8859-15", 0 };
static const char * const jisx0201_encodings[] = { "jisx0201*-0", 0 };
static const char * const gb18030_encodings[] = { "gb18030-0", "gb18030.2000-0", 0 };
static const char * const lao_encodings[] = { "mulelao-1", 0 };

// we select one of these at initialization time for Han use
static const char * const hancn_encodings[] =
    { CN_ENCODINGS,
      TW_ENCODINGS,
      HK_ENCODINGS,
      JP_ENCODINGS,
      KR_ENCODINGS, 0 };
static const char * const hanjp_encodings[] =
    { JP_ENCODINGS,
      CN_ENCODINGS,
      TW_ENCODINGS,
      HK_ENCODINGS,
      KR_ENCODINGS, 0 };
static const char * const hankr_encodings[] =
    { KR_ENCODINGS,
      JP_ENCODINGS,
      CN_ENCODINGS,
      TW_ENCODINGS,
      HK_ENCODINGS, 0 };
static const char * const hanhk_encodings[] =
    { HK_ENCODINGS,
      TW_ENCODINGS,
      CN_ENCODINGS,
      JP_ENCODINGS,
      KR_ENCODINGS, 0 };
static const char * const hantw_encodings[] =
    { TW_ENCODINGS,
      HK_ENCODINGS,
      CN_ENCODINGS,
      JP_ENCODINGS,
      KR_ENCODINGS, 0 };

static struct
{
    int index;
    const char * const * list;
}  script_table[QFont::LastPrivateScript] = {
    // Latin
    { 0, latin_encodings },
    // Greek
    { 0, greek_encodings },
    // Cyrillic
    { 0, cyrillic_encodings },
    // Armenian
    { 0, empty_encodings },
    // Georgian
    { 0, empty_encodings },
    // Runic
    { 0, empty_encodings },
    // Ogham
    { 0, empty_encodings },
    // SpacingModifiers
    { 0, empty_encodings },
    // CombiningMarks
    { 0, empty_encodings },

    // Hebrew
    { 0, hebrew_encodings },
    // Arabic
    { 0, arabic_encodings },
    // Syriac
    { 0, empty_encodings },
    // Thaana
    { 0, empty_encodings },

    // Devanagari
    { 0, empty_encodings },
    // Bengali
    { 0, empty_encodings },
    // Gurmukhi
    { 0, empty_encodings },
    // Gujarati
    { 0, empty_encodings },
    // Oriya
    { 0, empty_encodings },
    // Tamil
    { 0, tamil_encodings },
    // Telugu
    { 0, empty_encodings },
    // Kannada
    { 0, empty_encodings },
    // Malayalam
    { 0, empty_encodings },
    // Sinhala
    { 0, empty_encodings },
    // Thai
    { 0, thai_encodings },
    // Lao
    { 0, lao_encodings },
    // Tibetan
    { 0, gb18030_encodings },
    // Myanmar
    { 0, empty_encodings },
    // Khmer
    { 0, empty_encodings },

    // Han
    { 0, empty_encodings },
    // Hiragana
    { 0, hiragana_encodings },
    // Katakana
    { 0, katakana_encodings },
    // Hangul
    { 0, hangul_encodings },
    // Bopomofo
    { 0, bopomofo_encodings },
    // Yi
    { 0, gb18030_encodings },

    // Ethiopic
    { 0, empty_encodings },
    // Cherokee
    { 0, empty_encodings },
    // CanadianAboriginal
    { 0, empty_encodings },
    // Mongolian
    { 0, gb18030_encodings },
    // CurrencySymbols
    { 0, empty_encodings },
    // LetterlikeSymbols
    { 0, empty_encodings },
    // NumberForms
    { 0, empty_encodings },
    // MathematicalOperators
    { 0, empty_encodings },
    // TechnicalSymbols
    { 0, empty_encodings },
    // GeometricSymbols
    { 0, empty_encodings },
    // MiscellaneousSymbols
    { 0, empty_encodings },
    // EnclosedAndSquare
    { 0, empty_encodings },
    // Braille
    { 0, empty_encodings },

    // Unicode
    { 0, unicode_encodings },

    // UnknownScript
    { 0, empty_encodings },
    // NoScript
    { 0, empty_encodings },

    // HanX11
    { 0, hanx11_encodings },

    // LatinBasic == Latin
    // LatinExtendedA_2
    { 0, latinA2_encodings },
    // LatinExtendedA_3
    { 0, latinA3_encodings },
    // LatinExtendedA_4
    { 0, latinA4_encodings },
    // LatinExtendedA_14
    { 0, latinA14_encodings },
    // LatinExtendedA_15
    { 0, latinA15_encodings },
    // KatakanaHalfWidth
    { 0, jisx0201_encodings }
};



bool qt_has_xft = FALSE;
bool qt_use_antialiasing = FALSE;

static inline float pixelSize( const QFontDef &request, QPaintDevice *paintdevice,
			       int scr )
{
    float pSize;
    if ( request.pointSize != -1 ) {
	if ( paintdevice )
	    pSize = request.pointSize *
		    QPaintDeviceMetrics( paintdevice ).logicalDpiY() / 720.;
	else if (QPaintDevice::x11AppDpiY( scr ) == 75)
	    pSize = request.pointSize / 10.;
	else
	    pSize = request.pointSize * QPaintDevice::x11AppDpiY( scr ) / 720.;
    } else {
	pSize = request.pixelSize;
    }
    return pSize;

}

static inline float pointSize( const QFontDef &fd, QPaintDevice *paintdevice,
			       int scr )
{
    float pSize;
    if ( fd.pointSize == -1 ) {
	if ( paintdevice )
	    pSize = fd.pixelSize * 720. /
		    QPaintDeviceMetrics( paintdevice ).logicalDpiY();
	else if (QPaintDevice::x11AppDpiY( scr ) == 75)
	    pSize = fd.pixelSize * 10;
	else
	    pSize = fd.pixelSize * 720. / QPaintDevice::x11AppDpiY( scr );
    } else {
	pSize = fd.pointSize;
    }
    return pSize;
}

// class definition in qfontdata_p.h
QFontX11Data::QFontX11Data()
{
    for ( int i = 0; i < QFont::LastPrivateScript; i++ ) {
	fontstruct[i] = 0;
    }
}

QFontX11Data::~QFontX11Data()
{
    FontEngineIface *qfs;

    for ( int i = 0; i < QFont::LastPrivateScript; i++ ) {
	qfs = fontstruct[i];
	fontstruct[i] = 0;

	if ( qfs && qfs != (FontEngineIface *) -1 ) {
	    qfs->deref();
	}
    }
}

class QXFontName
{
public:
    QXFontName( const QCString &n, bool e, bool c )
	: name(n), exactMatch(e), useCore(c)
    { ; }

    QCString name;
    bool exactMatch;
    bool useCore;
};


typedef QDict<QXFontName> QFontNameDict;


// dict of matched font names default character set
static QFontNameDict *fontNameDict = 0;


/*
  Clears the internal cache of mappings from QFont instances to X11
  (XLFD) font names. Called from QPaintDevice::x11App
  */
void qX11ClearFontNameCache()
{
    if (fontNameDict)
	fontNameDict->clear();
}




// **********************************************************************
// QFontPrivate static methods
// **********************************************************************

// Returns the family name that corresponds to the current style hint.
QString QFontPrivate::defaultFamily() const
{
    switch (request.styleHint) {
    case QFont::Times:
	return QString::fromLatin1("times");

    case QFont::Courier:
	return QString::fromLatin1("courier");

    case QFont::Decorative:
	return QString::fromLatin1("old english");

    case QFont::Helvetica:
    case QFont::System:
    default:
	return QString::fromLatin1("helvetica");
    }
}


// Returns a last resort family name for the font matching algorithm.
QString QFontPrivate::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}


// Returns a last resort raw font name for the font matching algorithm.
// This is used if even the last resort family is not available. It
// returns \e something, almost no matter what.
// The current implementation tries a wide variety of common fonts,
// returning the first one it finds. The implementation may change at
// any time.
static const char * const tryFonts[] = {
    "-*-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-courier-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-times-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-lucida-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-helvetica-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-courier-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-times-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-lucida-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-helvetica-*-*-*-*-*-*-*-*-*-*-*-*",
    "-*-courier-*-*-*-*-*-*-*-*-*-*-*-*",
    "-*-times-*-*-*-*-*-*-*-*-*-*-*-*",
    "-*-lucida-*-*-*-*-*-*-*-*-*-*-*-*",
    "-*-fixed-*-*-*-*-*-*-*-*-*-*-*-*",
    "6x13",
    "7x13",
    "8x13",
    "9x15",
    "fixed",
    0
};

QString QFontPrivate::lastResortFont() const
{
    static QString last;

    // already found
    if ( !last.isNull() )
	return last;

    int i = 0;
    const char* f;

    while ( (f = tryFonts[i]) ) {
	last = QString::fromLatin1(f);

	if ( fontExists(last) )
	    return last;

	i++;
    }

#if defined(CHECK_NULL)
    qFatal( "QFontPrivate::lastResortFont: Cannot find any reasonable font" );
#endif

    return last;
}


// Get an array of X font names that matches a pattern
char **QFontPrivate::getXFontNames( const char *pattern, int *count )
{
    static int maxFonts = 256;
    char **list;

    for (;;) {
	list = XListFonts( QPaintDevice::x11AppDisplay(), (char*)pattern,
			   maxFonts, count );

	// I know precisely why 32768 is 32768.
	if ( *count != maxFonts || maxFonts >= 32768 )
	    return list;

	XFreeFontNames( list );

	maxFonts *= 2;
    }
}


// Returns TRUE if the font exists, FALSE otherwise
bool QFontPrivate::fontExists( const QString &fontName )
{
    char **fontNames;
    int count;

    fontNames = getXFontNames( fontName.latin1(), &count );

    XFreeFontNames( fontNames );

    return count != 0;
}


// Splits an X font name into fields separated by '-'
bool QFontPrivate::parseXFontName(char *fontName, char **tokens)
{
    if ( ! fontName || fontName[0] == '0' || fontName[0] != '-' ) {
	tokens[0] = 0;
	return FALSE;
    }

    int	  i;
    ++fontName;
    for ( i = 0; i < NFontFields && fontName && fontName[0]; ++i ) {
	tokens[i] = fontName;
	for ( ;; ++fontName ) {
	    if ( *fontName == '-' )
		break;
	    if ( ! *fontName ) {
		fontName = 0;
		break;
	    }
	}

	if ( fontName ) *fontName++ = '\0';
    }

    if ( i < NFontFields ) {
	for ( int j = i ; j < NFontFields; ++j )
	    tokens[j] = 0;
	return FALSE;
    }

    return TRUE;
}

/*
  Removes wildcards from an XLFD.

  Returns \a xlfd with all wildcards removed if a match for \a xlfd is
  found, otherwise it returns \a xlfd.
*/
QCString QFontPrivate::fixXLFD( const QCString &xlfd )
{
    QCString ret = xlfd;
    int count = 0;
    char **fontNames = getXFontNames( xlfd, &count );
    if ( count > 0 )
	ret = fontNames[0];
    XFreeFontNames( fontNames );
    return ret ;
}

/*
  Fills in a font definition (QFontDef) from the font properties in an
  XFontStruct.

  Returns TRUE if the QFontDef could be filled with properties from
  the XFontStruct.  The fields lbearing and rbearing are not given any
  values.
*/
bool QFontPrivate::fillFontDef( XFontStruct *fs, QFontDef *fd, int screen )
{
    unsigned long value;
    if ( ! XGetFontProperty( fs, XA_FONT, &value ) )
	return FALSE;

    char *n = XGetAtomName( QPaintDevice::x11AppDisplay(), value );
    QCString xlfd( n );
    if ( n )
	XFree( n );
    return fillFontDef( xlfd.lower(), fd, screen );
}


/*
  Fills in a font definition (QFontDef) from an XLFD (X Logical Font
  Description).

  Returns TRUE if the the given xlfd is valid.  The fields lbearing
  and rbearing are not given any values.
*/
bool QFontPrivate::fillFontDef( const QCString &xlfd, QFontDef *fd, int screen )
{
    char *tokens[QFontPrivate::NFontFields];
    QCString buffer = xlfd.copy();
    if ( ! parseXFontName(buffer.data(), tokens) )
	return FALSE;

    fd->family = QString::fromLatin1(tokens[Family]);
    QString foundry = QString::fromLatin1(tokens[Foundry]);
    if ( ! foundry.isEmpty() && foundry != QString::fromLatin1("*") )
	fd->family += QString::fromLatin1(" [") + foundry + QString::fromLatin1("]");

    if ( qstrlen( tokens[AddStyle] ) > 0 )
	fd->addStyle = QString::fromLatin1(tokens[AddStyle]);
    else
	fd->addStyle = QString::null;

    fd->pointSize = atoi(tokens[PointSize]);
    fd->styleHint = QFont::AnyStyle;	// ### any until we match families

    char slant = tolower( (uchar) tokens[Slant][0] );
    fd->italic = ( slant == 'o' || slant == 'i' );
    char fixed = tolower( (uchar) tokens[Spacing][0] );
    fd->fixedPitch = ( fixed == 'm' || fixed == 'c' );
    fd->weight = getFontWeight( tokens[Weight] );

    int r = atoi(tokens[ResolutionY]);
    fd->pixelSize = atoi(tokens[PixelSize]);
    // not "0" or "*", or required DPI
    if ( r && fd->pixelSize && QPaintDevice::x11AppDpiY( screen ) &&
	 r != QPaintDevice::x11AppDpiY( screen ) ) {
	// calculate actual pointsize for display DPI
	fd->pointSize = (int) ((fd->pixelSize * 720.) /
			       QPaintDevice::x11AppDpiY( screen ) + 0.5);
    } else if ( fd->pixelSize == 0 && fd->pointSize ) {
	// calculate pixel size from pointsize/dpi
	fd->pixelSize = ( fd->pointSize * QPaintDevice::x11AppDpiY( screen ) ) / 720;
    }

    fd->underline     = FALSE;
    fd->strikeOut     = FALSE;
    fd->hintSetByUser = FALSE;
    fd->rawMode       = FALSE;
    fd->dirty         = FALSE;

    return TRUE;
}




// **********************************************************************
// QFontPrivate member methods
// **********************************************************************




// Scoring constants
#define exactScore           0xfffe
#define exactNonUnicodeScore 0xffff

#define PitchScore	     0x102
#define CJKPitchScore 0x100
#define SizeScore	     0x80
#define NotScaledBitmapScore 0x40
#define ResolutionScore	     0x20
#define WeightScore	     0x10
#define SlantScore	     0x08
#define WidthScore	     0x04
#define NonUnicodeScore	     0x01

// Returns an XLFD for the font and sets exact to TRUE if the font found matches
// the font queried
QCString QFontPrivate::findFont(QFont::Script script, bool *exact, double *scale) const
{
    *scale = 1.;
    QString familyName = request.family;

    // assume exact match
    *exact = TRUE;

    if ( familyName.isEmpty() ) {
	familyName = defaultFamily();
	*exact = FALSE;
    }

    QString foundryName;
    QFontDatabase::parseFontName(familyName, foundryName, familyName);
    if ( foundryName == "x11" )
	foundryName = QString::null;

    QString addStyle = request.addStyle;
    if (addStyle.isEmpty())
	addStyle = "*";

    int score;

    QCString bestName;
    bool done = FALSE;
    int start_index = script_table[script].index;

    while (! done) {
	bestName = bestFamilyMember(script, foundryName, familyName, addStyle, &score, scale);

	if (bestName.isNull()) {
	    if (! script_table[script].list[++script_table[script].index])
		script_table[script].index = 0;

	    if (script_table[script].index == start_index)
		done = TRUE;
	} else
	    done = TRUE;
    }

    if ( score < exactScore )
	*exact = FALSE;

    if (! bestName.isNull())
	return bestName;

    // try substitution
    QStringList list = QFont::substitutes( familyName );
    QStringList::Iterator sit = list.begin();

    while (sit != list.end() && bestName.isNull()) {
	familyName = *sit++;

	if (request.family != familyName) {
	    done = FALSE;
	    script_table[script].index = start_index;
	    QFontDatabase::parseFontName(familyName, foundryName, familyName);

	    while (! done) {
		bestName = bestFamilyMember(script, foundryName, familyName,
					    addStyle, &score, scale);

		if (bestName.isNull()) {
		    if (! script_table[script].list[++script_table[script].index])
			script_table[script].index = 0;

		    if (script_table[script].index == start_index)
			done = TRUE;
		} else
		    done = TRUE;
	    }
	}
    }

    if (script == QFont::Unicode)
	return bestName;
    if (! bestName.isNull())
	return bestName;

    // try default family for style
    QString f = defaultFamily();

    if ( request.family != f ) {
	familyName = f;
	done = FALSE;
	script_table[script].index = start_index;

	while (! done) {
	    bestName = bestFamilyMember(script, foundryName, familyName,
					addStyle, &score, scale);

	    if (bestName.isNull()) {
		if (! script_table[script].list[++script_table[script].index])
		    script_table[script].index = 0;

		if (script_table[script].index == start_index)
		    done = TRUE;
	    } else
		done = TRUE;
	}
    }

    if (! bestName.isNull())
	return bestName;

    // try system default family
    f = lastResortFamily();

    if ( request.family != f ) {
	familyName = f;
	done = FALSE;
	script_table[script].index = start_index;

	while (! done) {
	    bestName = bestFamilyMember(script, foundryName, familyName,
					addStyle, &score, scale);

	    if (bestName.isNull()) {
		if (! script_table[script].list[++script_table[script].index])
		    script_table[script].index = 0;

		if (script_table[script].index == start_index)
		    done = TRUE;
	    } else
		done = TRUE;
	}
    }

    if (! bestName.isNull())
	return bestName;

    // try *any* family
    f = "*";

    if (request.family != f) {
	familyName = f;
	done = FALSE;
	script_table[script].index = start_index;

	while (! done) {
	    bestName = bestFamilyMember(script, foundryName, familyName,
					addStyle, &score, scale);

	    if (bestName.isNull()) {
		if (! script_table[script].list[++script_table[script].index])
		    script_table[script].index = 0;

		if (script_table[script].index == start_index)
		    done = TRUE;
	    } else
		done = TRUE;
	}
    }

    // no matching fonts found
    if (bestName.isNull() && script == defaultScript)
	bestName = lastResortFont().latin1();

    return bestName;
}


QCString QFontPrivate::bestFamilyMember(QFont::Script script,
					const QString& foundry,
					const QString& family,
					const QString& addStyle,
					int *score, double *scale ) const
{
    const int prettyGoodScore = CJKPitchScore | SizeScore | WeightScore |
				SlantScore | WidthScore;

    int bestScore = 0;
    double bestScale = 1.;
    QCString result;

    if ( !foundry.isEmpty() ) {
	QString pattern
	    = "-" + foundry + "-" + family + "-*-*-*-" + addStyle + "-*-*-*-*-*-*-" +
	    (script_table[script].list)[(script_table[script].index)];
	result = bestMatch(pattern.latin1(), &bestScore, script, &bestScale);
    }

    if ( bestScore < prettyGoodScore ) {
	QRegExp alt( "[,;]" );
	int alternator = 0;
	int next;
	int bias = 0;
	int testScore = 0;
	double testScale = 1.;
	QCString testResult;

	while ( alternator < (int)family.length() ) {
	    next = family.find( alt, alternator );

	    if ( next < alternator )
		next = family.length();

	    QString fam = family.mid( alternator, next-alternator );
	    QString pattern = "-*-" + fam + "-*-*-*-" + addStyle + "-*-*-*-*-*-*-" +
			      (script_table[script].list)[(script_table[script].index)];
	    testResult = bestMatch( pattern.latin1(), &testScore, script, &testScale );
	    bestScore -= bias;

	    if ( testScore > bestScore ) {
		bestScore = testScore;
		bestScale = testScale;
		result = testResult;
	    }

	    if ( family[next] == ';' )
		bias += 1;

	    alternator = next + 1;
	}
    }
    if ( score )
	*score = bestScore;
    *scale = bestScale;

    return result;
}


struct QFontMatchData { // internal for bestMatch
    QFontMatchData()
    {
	score=0; name=0; pointDiff=99; weightDiff=99; smooth=FALSE, scale = 1.;
    }

    int	    score;
    char   *name;
    float   pointDiff;
    int	    weightDiff;
    bool    smooth;
    double scale;
};

QCString QFontPrivate::bestMatch( const char *pattern, int *score,
				  QFont::Script script, double *scale ) const
{
    QFontMatchData best;
    QFontMatchData bestScalable;

    QCString	matchBuffer( 256 );	// X font name always <= 255 chars
    char **	xFontNames;
    int		fcount;
    int		sc;
    float	pointDiff;	// difference in % from requested point size
    int		weightDiff;	// difference from requested weight
    bool	scalable       = FALSE;
    bool	smoothScalable = FALSE;
    int		i;
    double tmpScale;

    xFontNames = getXFontNames( pattern, &fcount );

    for( i = 0; i < fcount; i++ ) {
	sc = fontMatchScore( xFontNames[i], matchBuffer,
			     &pointDiff, &weightDiff,
			     &scalable, &smoothScalable, script, &tmpScale );

	if ( scalable ) {
	    if ( sc > bestScalable.score ||
		 sc == bestScalable.score &&
		 weightDiff < bestScalable.weightDiff ||
		 sc == bestScalable.score &&
		 weightDiff == bestScalable.weightDiff &&
		 smoothScalable && !bestScalable.smooth ) {
		bestScalable.score = sc;
		bestScalable.name = xFontNames[i];
		bestScalable.pointDiff = pointDiff;
		bestScalable.weightDiff = weightDiff;
		bestScalable.smooth = smoothScalable;
	    }
	} else {
	    if ( sc > best.score ||
		 sc == best.score && pointDiff < best.pointDiff ||
		 sc == best.score && pointDiff == best.pointDiff &&
		 weightDiff < best.weightDiff ) {
		best.score = sc;
		best.name = xFontNames[i];
		best.pointDiff = pointDiff;
		best.weightDiff = weightDiff;
		best.scale = tmpScale;
	    }
	}
    }

    QCString bestName;
    char *tokens[NFontFields];
    if ( bestScalable.score > best.score ||
	 bestScalable.score == best.score &&
	 bestScalable.pointDiff < best.pointDiff ||
	 bestScalable.score == best.score &&
	 bestScalable.pointDiff == best.pointDiff &&
	 bestScalable.weightDiff < best.weightDiff ) {
	qstrcpy( matchBuffer.data(), bestScalable.name );

	if ( parseXFontName( matchBuffer.data(), tokens ) ) {
	    int resx;
	    int resy;
	    int pSize;

	    if ( bestScalable.smooth ) {
		// X will scale the font accordingly
		resx = 0;
		resy = 0;
	    } else {
		resx = atoi(tokens[ResolutionX]);
		resy = atoi(tokens[ResolutionY]);
	    }
	    double pxs = pixelSize( request, paintdevice, x11Screen );
	    if ( pxs > (double)MAXFONTSIZE ) {
		bestScalable.scale = pxs/(double)MAXFONTSIZE;
		pxs = MAXFONTSIZE;
	    }
	    pSize = (int) ( pxs + 0.5 );

	    bestName.sprintf( "-%s-%s-%s-%s-%s-%s-%i-*-%i-%i-%s-*-%s-%s",
			      tokens[Foundry],
			      tokens[Family],
			      tokens[Weight],
			      tokens[Slant],
			      tokens[Width],
			      tokens[AddStyle],
			      pSize,
			      resx, resy,
			      tokens[Spacing],
			      tokens[CharsetRegistry],
			      tokens[CharsetEncoding] );
	    best.scale = bestScalable.score;
	    best.scale = bestScalable.scale;
	    best.name = bestName.data();
	}
    }

    *score = best.score;
    bestName = best.name;
    *scale = best.scale;

    XFreeFontNames( xFontNames );
    return bestName;
}


// Returns a score describing how well a font name matches the contents
// of a font.
int QFontPrivate::fontMatchScore( const char *fontName, QCString &buffer,
				   float *pixelSizeDiff, int  *weightDiff,
				   bool	 *scalable     , bool *smoothScalable,
				  QFont::Script script, double *scale ) const
{
    char *tokens[NFontFields];
    bool   exactmatch = TRUE;
    int	   score      = NonUnicodeScore;
    *scalable	      = FALSE;
    *smoothScalable   = FALSE;
    *weightDiff	      = 0;
    *pixelSizeDiff    = 0;

    qstrcpy( buffer.data(), fontName );	// NOTE: buffer must be large enough
    if ( ! parseXFontName( buffer.data(), tokens ) )
	return 0;	// Name did not conform to X Logical Font Description

    if ( isScalable( tokens ) ) {
	*scalable = TRUE;			// scalable font
	if ( isSmoothlyScalable( tokens ) )
	    *smoothScalable = TRUE;
    }

    // we no longer score the charset, since we query the X server for fonts based
    // on the foundry, family and charset now

    char pitch = tolower( (uchar) tokens[Spacing][0] );
    if ( script >= QFont::Han && script <= QFont::Yi ||
	 script == QFont::HanX11 ) {
	// basically we treat cell spaced and proportional fonts the same for asian.
	// Proportional ones put a heavy load on the server, so we make them less
	// favorable.
	if ( pitch != 'p' )
	    score |= PitchScore;
	else {
	    score |= CJKPitchScore;
	    exactmatch = FALSE;
	}
    } else if ( request.fixedPitch ) {
	if ( pitch == 'm' || pitch == 'c' )
	    score |= PitchScore;
	else
	    exactmatch = FALSE;
    } else {
	if ( pitch != 'p' )
	    exactmatch = FALSE;
    }

    // ### fix scaled bitmap fonts
    float diff;
    *scale = 1.;
    if ( *scalable ) {
	diff = 0.9;	// choose scalable font over >= 0.9 point difference
	if ( *smoothScalable ) {
	    score |= SizeScore;
	    if ( request.styleStrategy & (QFont::PreferDefault | QFont::PreferOutline |
					  QFont::ForceOutline) )
		diff = 0;
	} else {
	    // scaled bitmap fonts look just ugly. Never give them a size score if not
	    // PreferMatch
	    exactmatch = FALSE;
	    if ( ! (request.styleStrategy & QFont::PreferQuality) )
		score |= SizeScore;
	}
    } else {
	int pSize;
	float percentDiff;
	pSize = atoi(tokens[PixelSize]);

	int reqPSize = (int) (pixelSize( request, paintdevice, x11Screen ) + 0.5);

	if ( reqPSize != 0 ) {
	    diff = (float)QABS(pSize - reqPSize);
	    percentDiff = diff/reqPSize*100.0F;
	} else {
	    diff = (float)pSize;
	    percentDiff = 100;
	}
	if ( reqPSize > MAXFONTSIZE ) {
	    *scale = (double)reqPSize/(double)MAXFONTSIZE;
	    reqPSize = MAXFONTSIZE;
	}
	if ( diff == 0 && (request.styleStrategy & (QFont::PreferOutline |
						    QFont::ForceOutline) ) )
	    diff = 0.9;

	if ( percentDiff < 10 &&
	     (!(request.styleStrategy & QFont::PreferMatch) ||
	      request.styleStrategy & QFont::PreferQuality) ) {
	    score |= SizeScore;

	}
	if ( !(request.styleStrategy & QFont::PreferMatch) )
	    score |= NotScaledBitmapScore;
	if ( pSize != reqPSize ) {
	    exactmatch = FALSE;
	}
    }

    if ( pixelSizeDiff )
	*pixelSizeDiff = diff;

    int weightVal = getFontWeight(tokens[Weight], TRUE);
    if ( weightVal == (int) request.weight )
	score |= WeightScore;
    else
	exactmatch = FALSE;

    *weightDiff = QABS( weightVal - (int) request.weight );
    char slant = tolower( (uchar) tokens[Slant][0] );

    if ( request.italic ) {
	if ( slant == 'o' || slant == 'i' )
	    score |= SlantScore;
	else
	    exactmatch = FALSE;
    } else {
	if ( slant == 'r' )
	    score |= SlantScore;
	else
	    exactmatch = FALSE;
    }

    if ( qstricmp( tokens[Width], "normal" ) == 0 )
	score |= WidthScore;
    else
	exactmatch = FALSE;

    return exactmatch ? (exactScore | (score&NonUnicodeScore)) : score;
}


// Computes the line width (underline,strikeout)
void QFontPrivate::computeLineWidth()
{
    int nlw;
    int weight = actual.weight;
    int pSize  = actual.pixelSize;

    // ad hoc algorithm
    int score = pSize * weight;
    nlw = ( score ) / 700;

    // looks better with thicker line for small pointsizes
    if ( nlw < 2 && score >= 1050 ) nlw = 2;
    if ( nlw == 0 ) nlw = 1;

    if (nlw > lineWidth) lineWidth = nlw;
}


// Loads the font for the specified script
static inline int maxIndex(XFontStruct *f) {
    return (((f->max_byte1 - f->min_byte1) *
	     (f->max_char_or_byte2 - f->min_char_or_byte2 + 1)) +
	    f->max_char_or_byte2 - f->min_char_or_byte2);
}

// returns a sample unicode character for the specified script
static QChar sampleCharacter(QFont::Script script)
{
    ushort ch;

    switch (script) {
    case QFont::Latin:                     ch = 0x0030; break;
    case QFont::Greek:                     ch = 0x0390; break;
    case QFont::Cyrillic:                  ch = 0x0410; break;
    case QFont::Armenian:                  ch = 0x0540; break;
    case QFont::Georgian:                  ch = 0x10a0; break;
    case QFont::Runic:                     ch = 0x16a0; break;
    case QFont::Ogham:                     ch = 0x1680; break;
    case QFont::CombiningMarks:            ch = 0x0300; break;

    case QFont::Hebrew:                    ch = 0x05d0; break;
    case QFont::Arabic:                    ch = 0x0630; break;
    case QFont::Syriac:                    ch = 0x0710; break;
    case QFont::Thaana:                    ch = 0x0780; break;

    case QFont::Devanagari:                ch = 0x0910; break;
    case QFont::Bengali:                   ch = 0x0990; break;
    case QFont::Gurmukhi:                  ch = 0x0a10; break;
    case QFont::Gujarati:                  ch = 0x0a90; break;
    case QFont::Oriya:                     ch = 0x0b10; break;
    case QFont::Tamil:                     ch = 0x0b90; break;
    case QFont::Telugu:                    ch = 0x0c10; break;
    case QFont::Kannada:                   ch = 0x0c90; break;
    case QFont::Malayalam:                 ch = 0x0d10; break;
    case QFont::Sinhala:                   ch = 0x0d90; break;
    case QFont::Thai:                      ch = 0x0e10; break;
    case QFont::Lao:                       ch = 0x0e81; break;
    case QFont::Tibetan:                   ch = 0x0f00; break;
    case QFont::Myanmar:                   ch = 0x1000; break;
    case QFont::Khmer:                     ch = 0x1780; break;

    case QFont::Han:                       ch = 0x4e00; break;
    case QFont::Hiragana:                  ch = 0x3050; break;
    case QFont::Katakana:                  ch = 0x30b0; break;
    case QFont::Hangul:                    ch = 0xac00; break;
    case QFont::Bopomofo:                  ch = 0x3110; break;
    case QFont::Yi:                        ch = 0xa000; break;

    case QFont::Ethiopic:                  ch = 0x1200; break;
    case QFont::Cherokee:                  ch = 0x13a0; break;
    case QFont::CanadianAboriginal:        ch = 0x1410; break;
    case QFont::Mongolian:                 ch = 0x1800; break;

    case QFont::CurrencySymbols:           ch = 0x20aa; break;
    case QFont::LetterlikeSymbols:         ch = 0x2122; break;
    case QFont::NumberForms:               ch = 0x215b; break;
    case QFont::MathematicalOperators:     ch = 0x222b; break;
    case QFont::TechnicalSymbols:          ch = 0x2440; break;
    case QFont::GeometricSymbols:          ch = 0x2500; break;
    case QFont::MiscellaneousSymbols:      ch = 0x2600; break;
    case QFont::EnclosedAndSquare:         ch = 0x2460; break;
    case QFont::Braille:                   ch = 0x2800; break;

    case QFont::LatinExtendedA_2:          ch = 0x0102; break;
    case QFont::LatinExtendedA_3:          ch = 0x0108; break;
    case QFont::LatinExtendedA_4:          ch = 0x0100; break;
    case QFont::LatinExtendedA_14:         ch = 0x0174; break;
    case QFont::LatinExtendedA_15:         ch = 0x0152; break;

    default:
 	ch = 0;
    }

    return QChar(ch);
}


bool QFontPrivate::loadUnicode(QFont::Script script, const QChar &sample)
{
    bool hasChar = FALSE;
    FontEngineIface *qfs = x11data.fontstruct[QFont::Unicode];

    if (!qfs) {

#ifdef QFONTLOADER_DEBUG_VERBOSE
	qDebug("QFontLoader: trying to load unicode font");
#endif

	load(QFont::Unicode, FALSE);
	qfs = x11data.fontstruct[QFont::Unicode];
    }

    if (qfs && qfs != (FontEngineIface *) -1) {
	QChar chs[2] = { QChar(0xfffe), sample };
	hasChar = !qfs->canRender( chs, 1 ) && qfs->canRender( chs+1, 1 );

	if (hasChar) {
#ifdef QFONTLOADER_DEBUG_VERBOSE
	    qDebug("QFontLoader: unicode font has char 0x%02x%02x for %d %s",
		   sample.row(), sample.cell(), script,
		   script_table[script].list[script_table[script].index]);
#endif

	    x11data.fontstruct[script] = qfs;
	    qfs->ref();
	    request.dirty = FALSE;
	}
    }

    return hasChar;
}


void QFontPrivate::load(QFont::Script script, bool tryUnicode)
{
    // Make sure fontCache is initialized
    if (! fontCache) {
#ifdef QT_CHECK_STATE
	qFatal( "QFont: Must construct a QApplication before a QFont" );
#endif // QT_CHECK_STATE

	return;
    }

    if (script == QFont::NoScript)
	script = defaultScript;

    if (script > QFont::LastPrivateScript)
	qFatal("QFontLoader: script %d is out of range", script);

    if (x11data.fontstruct[script] && ! request.dirty)
	return;

    if (request.dirty) {
	// dirty font needs to have the fontstruct deref'ed
	FontEngineIface *qfs = x11data.fontstruct[script];
	x11data.fontstruct[script] = 0;

	if (qfs && qfs != (FontEngineIface *) -1) {
	    // only deref here... we will let the cache take care of cleaning up
	    // while the application is running
	    qfs->deref();
	}

	// dirty unicode also if the font is dirty
	qfs = x11data.fontstruct[QFont::Unicode];
	x11data.fontstruct[QFont::Unicode] = 0;

	if (qfs && qfs != (FontEngineIface *) -1) {
	    // only deref here... we will let the cache take care of cleaning up
	    // while the application is running
	    qfs->deref();
	}

	// make sure to recalculate fontinfo
	actual.dirty = TRUE;
    }

    QChar sample = sampleCharacter(script);
    // look for a unicode font first, and see if it has the script that we want...
    if (tryUnicode && loadUnicode(script, sample)) {
	request.dirty = FALSE;
	return;
    }

    FontEngineIface *qfs = 0;
    QCString fontname;
    QTextCodec *codec = 0;
    XFontStruct *xfs = 0;


    QString k(key() + script_table[script].list[script_table[script].index]);
    k += "/scr" + QString::number( x11Screen );
    if ( paintdevice )
	k += "/res" + QString::number(QPaintDeviceMetrics( paintdevice ).logicalDpiY());
    else
	k += "/res" + QString::number( QPaintDevice::x11AppDpiY( x11Screen ) );

    // Look for font name in fontNameDict based on QFont::key()
    QXFontName *qxfn = fontNameDict->find(k);
    double scale = 1.;
    if (! qxfn) {
	// if we don't find the name in the dict, we need to find a font name

#ifdef QFONTLOADER_DEBUG_VERBOSE
	qDebug("QFontLoader: getting font name for family %s", request.family.latin1());
#endif

	QString name;
	bool match;
	bool use_core = TRUE;

	if (use_core) {
	    if (request.rawMode) {
		name = QFont::substitute(request.family);
		match = fontExists(name);

		if (! match && script == QFontPrivate::defaultScript)
		    name = lastResortFont();
		else if ( script != QFontPrivate::defaultScript )
		    name = QString::null;
	    } else {
		name = findFont(script, &match, &scale);
	    }
	}

	bool fail = FALSE;
	if (name.isNull()) {
	    // no font name... this can only happen with Unicode
	    // qDebug("QFontLoader: no font name - this must be unicode (%d %s)",
	    // script, script_table[script].list[script_table[script].index]);

	    name = k + "NU";
	    fail = TRUE;
	}

#ifdef QFONTLOADER_DEBUG_VERBOSE
	qDebug("QFontLoader: putting '%s' (%d) into name dict", name.latin1(),
	       name.length());
#endif

	// Put font name into fontNameDict
	qxfn = new QXFontName(name.latin1(), match, use_core);
	Q_CHECK_PTR(qxfn);
	fontNameDict->insert(k, qxfn);

	if (fail) {
	    // we don't have a font name, so we need to negative cache a font struct
	    // the reason we do this here, is because Exceed substitutes the fixed
	    // font for any name it can't find...
	    x11data.fontstruct[script] = (FontEngineIface *) -1;
// 	    initFontInfo(script, scale);
	    fontCache->insert(k, x11data.fontstruct[script], 1);
	    return;
	}
    }

#ifdef QFONTLOADER_DEBUG_VERBOSE
    qDebug("QFont::load: using name '%s'", qxfn->name.data());
#endif

    exactMatch = qxfn->exactMatch;
    fontname = qxfn->name;

    // Look in fontCache for font
    qfs = fontCache->find(k);
    if (qfs) {
	// Found font in either cache or dict...
	x11data.fontstruct[script] = qfs;

	if (qfs != (FontEngineIface *) -1) {
	    qfs->ref();
// 	    initFontInfo(script, scale);
	}

	request.dirty = FALSE;

	return;
    }

    if (qxfn->useCore) {
	// if we have no way to map this script, we give up
	if (! script_table[script].list[script_table[script].index]) {

#ifdef QFONTLOADER_DEBUG_VERBOSE
	    qDebug("QFontLoader: no nothing about script %d, giving up", script);
#endif

	    x11data.fontstruct[script] = (FontEngineIface *) -1;
// 	    initFontInfo(script, scale);
	    fontCache->insert(k, x11data.fontstruct[script], 1);
	    return;
	}

	// get unicode -> font encoding codec
	if (script < QFont::Unicode || script > QFont::NoScript) {
	    if ( script == QFont::Hebrew )
		codec = QTextCodec::codecForName( "ISO 8859-8-I" );
	    else if ( script == QFont::Latin )
		codec = QTextCodec::codecForName("ISO-8859-1");
	    else
		codec =
		    QTextCodec::codecForName(script_table[script].list[script_table[script].index]);

#ifdef QFONTLOADER_DEBUG
	    if (codec) {
		qDebug("QFontLoader: got codec %s for script %d %s",
		       codec->name(), script,
		       script_table[script].list[script_table[script].index]);
	    }
#endif

	    // if we don't have a codec for the font, don't even bother loading it
	    if (! codec) {
#ifdef QFONTLOADER_DEBUG
		qDebug("QFontLoader: no codec for script %d %s",
		       script,
		       script_table[script].list[script_table[script].index]);
#endif

		x11data.fontstruct[script] = (FontEngineIface *) -1;
// 		initFontInfo(script, scale);
		fontCache->insert(k, x11data.fontstruct[script], 1);
		return;
	    }
	}

	// font was never loaded, we need to do that now
#ifdef QFONTLOADER_DEBUG
	qDebug("QFontLoader: %p loading font for %d %s\n\t%s", this,
	       script, script_table[script].list[script_table[script].index],
	       fontname.data());
#endif

	if (! (xfs = XLoadQueryFont(QPaintDevice::x11AppDisplay(),
				    fontname.data()))) {
	    if (script != QFont::Unicode && script == QFontPrivate::defaultScript) {

#ifdef QFONTLOADER_DEBUG
		qDebug("QFontLoader: load failed, trying last resort");
#endif

		exactMatch = FALSE;

		if (! (xfs = XLoadQueryFont(QPaintDevice::x11AppDisplay(),
					    lastResortFont().latin1()))) {
		    qFatal("QFontLoader: Internal error");
		}
	    } else {
		// Didn't get unicode/script font, set to sentinel and return
		x11data.fontstruct[script] = (FontEngineIface *) -1;
// 		initFontInfo(script, scale);
		fontCache->insert(k, x11data.fontstruct[script], 1);
		return;
	    }
	}
    }
    // calculate cost of this item in the fontCache
    int cost = 1;
    if (xfs) {
	cost = maxIndex(xfs);
	if ( cost > 3000 ) {
	    // If the user is using large fonts, we assume they have
	    // turned on the Xserver option deferGlyphs, and that they
	    // have more memory available to the server.
	    cost = 3000;
	}
	cost = ((xfs->max_bounds.ascent + xfs->max_bounds.descent) *
		(xfs->max_bounds.width * cost / 8));
    }
    else {
	// couldn't load the font...
	x11data.fontstruct[script] = (FontEngineIface *) -1;
// 	initFontInfo(script, scale);
	fontCache->insert(k, x11data.fontstruct[script], 1);
	return;
    }

    // ### fix index
    qfs = new FontEngineXLFD( xfs, fontname, codec, 0);
    x11data.fontstruct[script] = qfs;

//     initFontInfo(script, scale);
    request.dirty = FALSE;

    // Insert font into the font cache and font dict
    if ( !fontCache->insert(k, qfs, cost) )
    {
#ifdef QT_CHECK_STATE
	qFatal("QFont::load: font cache overflow error");
#endif // QT_CHECK_STATE
    }
}




// **********************************************************************
// QFont methods
// **********************************************************************


/*!
    Returns TRUE if the font attributes have been changed and the font
    has to be (re)loaded; otherwise returns FALSE.
*/
bool QFont::dirty() const
{
    return d->request.dirty;
}


// **********************************************************************
// QFont static methods
// **********************************************************************

QFont::Script QFontPrivate::defaultScript = QFont::UnknownScript;
QSingleCleanupHandler<QFontCache> cleanup_fontcache;
QSingleCleanupHandler<QFontNameDict> cleanup_fontnamedict;

extern bool qt_use_xrender; // defined in qapplication_x11.cpp

/*!
  Internal function that initializes the font system.

  \internal
  The font cache and font dict do not alloc the keys. The key is a QString
  which is shared between QFontPrivate and QXFontName.
*/
void QFont::initialize()
{
    // create font cache and name dict
    if ( ! QFontPrivate::fontCache ) {
	QFontPrivate::fontCache = new QFontCache();
	Q_CHECK_PTR(QFontPrivate::fontCache);
	cleanup_fontcache.set(&QFontPrivate::fontCache);
    }

    if ( ! fontNameDict ) {
	fontNameDict = new QFontNameDict(QFontPrivate::fontCache->size());
	Q_CHECK_PTR(fontNameDict);
	fontNameDict->setAutoDelete(TRUE);
	cleanup_fontnamedict.set(&fontNameDict);
    }

#ifndef QT_NO_CODECS
#ifndef QT_NO_BIG_CODECS
    static bool codecs_once = FALSE;
    if ( ! codecs_once ) {
	(void) new QFontJis0201Codec;
	(void) new QFontJis0208Codec;
	(void) new QFontKsc5601Codec;
	(void) new QFontGb2312Codec;
	(void) new QFontGbkCodec;
	(void) new QFontGb18030_0Codec;
	(void) new QFontBig5Codec;
	(void) new QFontBig5hkscsCodec;
	(void) new QFontArabic68Codec;
	(void) new QFontLaoCodec;
	codecs_once = TRUE;
    }
#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS

#if 0
    QTextCodec *codec = QTextCodec::codecForLocale();
    // we have a codec for the locale - lets see if it's one of the CJK codecs,
    // and change the script_table[Han].list to an appropriate list
    if (codec) {
	switch (codec->mibEnum()) {
	case 2025: // GB2312
	case 57: // gb2312.1980-0
	case 113: // GBK
	case -113: // gbk-0
	case 114: // GB18030
	case -114: // gb18030-0
	    script_table[QFont::Han].list = hancn_encodings;
	    break;

	case 2026: // Big5
	case -2026: // big5-0, big5-eten.0
	    script_table[QFont::Han].list = hantw_encodings;
	    break;

	case 2101: // Big5-HKSCS
	case -2101: // big5hkscs-0, hkscs-1
	    script_table[QFont::Han].list = hanhk_encodings;
	    break;

	case 36: // KS C 5601
	case 38: // EUC KR
	    script_table[QFont::Han].list = hankr_encodings;
	    break;

	case 16: // JIS7
	case 17: // SJIS
	case 18: // EUC JP
	case 63: // JIS X 0208
	default:
	    script_table[QFont::Han].list = hanjp_encodings;
	    break;
	}
    } else
	script_table[QFont::Han].list = hanjp_encodings;

    // get some sample text based on the users locale. we use this to determine the
    // default script for the font system
    QCString oldlctime = setlocale(LC_TIME, 0);
    QCString lctime = setlocale(LC_TIME, "");

    time_t ttmp = time(0);
    struct tm *tt = 0;
    char samp[64];
    QString sample;

    if (ttmp != -1) {
#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
	// use the reentrant versions of localtime() where available
	tm res;
	tt = localtime_r(&ttmp, &res);
#else
	tt = localtime(&ttmp);
#endif // QT_THREAD_SUPPORT && _POSIX_THREAD_SAFE_FUNCTIONS
	if (tt != 0 && strftime(samp, 64, "%A%B", tt) > 0)
	    if (codec)
		sample = codec->toUnicode(samp);
    }

    if (! sample.isNull() && ! sample.isEmpty()) {
	QFont::Script cs = QFont::NoScript, tmp;
	const QChar *uc = sample.unicode();
	QFontPrivate *priv = new QFontPrivate;

	for (uint i = 0; i < sample.length(); i++) {
	    SCRIPT_FOR_CHAR( tmp, *uc, priv );
	    uc++;
	    if (tmp != cs && tmp != QFont::UnknownScript) {
		cs = tmp;
		break;
	    }
	}
	delete priv;

	if (cs != QFont::UnknownScript)
	    QFontPrivate::defaultScript = cs;
    }

    setlocale(LC_TIME, oldlctime.data());
#else
    QFontPrivate::defaultScript = QFont::Latin;
#endif
}


/*! \internal

  Internal function that cleans up the font system.
*/
void QFont::cleanup()
{
    // we don't delete the fontcache/namedict here because that is taken
    // care of by our cleanup handlers
    QFontPrivate::fontCache->setAutoDelete(TRUE);
    QFontPrivate::fontCache->clear();
    fontNameDict->clear();
}





// **********************************************************************
// QFont member methods
// **********************************************************************

/*!
    Returns the window system handle to the font, for low-level
    access. Using this function is \e not portable.
*/
Qt::HANDLE QFont::handle() const
{
    return 0;
}


/*!
    Returns the name of the font within the underlying window system.
    On Windows, this is usually just the family name of a TrueType
    font. Under X, it is an XLFD (X Logical Font Description). Using
    the return value of this function is usually \e not \e portable.

    \sa setRawName()
*/
QString QFont::rawName() const
{
    d->load(QFontPrivate::defaultScript);

    FontEngineIface *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (FontEngineIface *) -1) {
	return QString::null;
    }

    return QString::fromLatin1(qfs->name());
}


/*!
    Sets a font by its system specific name. The function is
    particularly useful under X, where system font settings (for
    example X resources) are usually available in XLFD (X Logical Font
    Description) form only. You can pass an XLFD as \a name to this
    function.

    In Qt 2.0 and later, a font set with setRawName() is still a
    full-featured QFont. It can be queried (for example with italic())
    or modified (for example with setItalic()) and is therefore also
    suitable for rendering rich text.

    If Qt's internal font database cannot resolve the raw name, the
    font becomes a raw font with \a name as its family.

    Note that the present implementation does not handle wildcards in
    XLFDs well, and that font aliases (file \c fonts.alias in the font
    directory on X11) are not supported.

    \sa rawName(), setRawMode(), setFamily()
*/
void QFont::setRawName( const QString &name )
{
    detach();

    bool validXLFD = QFontPrivate::fillFontDef( QFontPrivate::fixXLFD( name.latin1() ),
						&d->request, d->x11Screen ) ;
    d->request.dirty = TRUE;

    if ( !validXLFD ) {

#ifdef QT_CHECK_STATE
	qWarning("QFont::setRawMode(): Invalid XLFD: \"%s\"", name.latin1());
#endif // QT_CHECK_STATE

	setFamily( name );
	setRawMode( TRUE );
    }
}

/*!
  \internal
  X11 Only: Associate the font with the specified \a screen.
*/
void QFont::x11SetScreen( int screen )
{
    if ( screen < 0 ) // assume default
	screen = QPaintDevice::x11AppScreen();

    if ( screen == d->x11Screen )
	return; // nothing to do

    detach();
    d->x11Screen = screen;
    d->request.dirty = TRUE;
}

/*!
  \internal
  X11 Only: Returns the screen with which this font is associated.
*/
int QFont::x11Screen() const
{
    return d->x11Screen;
}


FontEngineIface *QFont::engineForScript( QFont::Script script ) const
{
    d->load(script);

    return d->x11data.fontstruct[script];
}

// **********************************************************************
// QFontMetrics member methods
// **********************************************************************

/*!
    Returns the ascent of the font.

    The ascent of a font is the distance from the baseline to the
    highest position characters extend to. In practice, some font
    designers break this rule, e.g. when they put more than one accent
    on top of a character, or to accommodate an unusual character in
    an exotic language, so it is possible (though rare) that this
    value will be too small.

    \sa descent()
*/
int QFontMetrics::ascent() const
{

    d->load(QFontPrivate::defaultScript);

    FontEngineIface *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (FontEngineIface *) -1)
	return 0;
    return qfs->ascent();
}


/*!
    Returns the descent of the font.

    The descent is the distance from the base line to the lowest point
    characters extend to. (Note that this is different from X, which
    adds 1 pixel.) In practice, some font designers break this rule,
    e.g. to accommodate an unusual character in an exotic language, so
    it is possible (though rare) that this value will be too small.

    \sa ascent()
*/
int QFontMetrics::descent() const
{
    d->load(QFontPrivate::defaultScript);

    FontEngineIface *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (FontEngineIface *) -1)
	return 0;
    return qfs->descent();
}


/*!
    Returns TRUE if character \a ch is a valid character in the font;
    otherwise returns FALSE.
*/
bool QFontMetrics::inFont(QChar ch) const
{
    return TRUE; //####
}


/*!
    Returns the left bearing of character \a ch in the font.

    The left bearing is the right-ward distance of the left-most pixel
    of the character from the logical origin of the character. This
    value is negative if the pixels of the character extend to the
    left of the logical origin.

    See width(QChar) for a graphical description of this metric.

    \sa rightBearing(), minLeftBearing(), width()
*/
int QFontMetrics::leftBearing(QChar ch) const
{
    // ####
    return 0;
}


/*!
    Returns the right bearing of character \a ch in the font.

    The right bearing is the left-ward distance of the right-most
    pixel of the character from the logical origin of a subsequent
    character. This value is negative if the pixels of the character
    extend to the right of the width() of the character.

    See width() for a graphical description of this metric.

    \sa leftBearing(), minRightBearing(), width()
*/
int QFontMetrics::rightBearing(QChar ch) const
{
    // ####
    return 0;
}


/*!
    Returns the minimum left bearing of the font.

    This is the smallest leftBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minRightBearing(), leftBearing()
*/
int QFontMetrics::minLeftBearing() const
{
    d->load(QFontPrivate::defaultScript);

    FontEngineIface *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (FontEngineIface *) -1)
	return 0;
    return 0;
}


/*!
    Returns the minimum right bearing of the font.

    This is the smallest rightBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minLeftBearing(), rightBearing()
*/
int QFontMetrics::minRightBearing() const
{
    d->load(QFontPrivate::defaultScript);

    FontEngineIface *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (FontEngineIface *) -1)
	return 0;
    return 0;
}


/*!
    Returns the height of the font.

    This is always equal to ascent()+descent()+1 (the 1 is for the
    base line).

    \sa leading(), lineSpacing()
*/
int QFontMetrics::height() const
{
    d->load(QFontPrivate::defaultScript);

    FontEngineIface *qfs =  d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (FontEngineIface *) -1)
	return (d->actual.pixelSize * 3 / 4) + 1;

    return qfs->ascent() + qfs->descent();
}


/*!
    Returns the leading of the font.

    This is the natural inter-line spacing.

    \sa height(), lineSpacing()
*/
int QFontMetrics::leading() const
{
    d->load(QFontPrivate::defaultScript);

    FontEngineIface *qfs =  d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (FontEngineIface *) -1)
	return 0;

    return qfs->leading();
}


/*!
    Returns the distance from one base line to the next.

    This value is always equal to leading()+height().

    \sa height(), leading()
*/
int QFontMetrics::lineSpacing() const
{
    return leading() + height();
}


/*! \fn int QFontMetrics::width( char c ) const

  \overload
  \obsolete

  Provided to aid porting from Qt 1.x.
*/


/*!
    \overload

    <img src="bearings.png" align=right>

    Returns the logical width of character \a ch in pixels. This is a
    distance appropriate for drawing a subsequent character after \a
    ch.

    Some of the metrics are described in the image to the right. The
    central dark rectangles cover the logical width() of each
    character. The outer pale rectangles cover the leftBearing() and
    rightBearing() of each character. Notice that the bearings of "f"
    in this particular font are both negative, while the bearings of
    "o" are both positive.

    \warning This function will produce incorrect results for Arabic
    characters or non spacing marks in the middle of a string, as the
    glyph shaping and positioning of marks that happens when
    processing strings cannot be taken into account. Use charWidth()
    instead if you aren't looking for the width of isolated
    characters.

    \sa boundingRect(), charWidth()
*/
int QFontMetrics::width(QChar ch) const
{
    // ####
    return 0;
}


/*!
    Returns the width of the character at position \a pos in the
    string \a str.

    The whole string is needed, as the glyph drawn may change
    depending on the context (the letter before and after the current
    one) for some languages (e.g. Arabic).

    This function also takes non spacing marks and ligatures into
    account.
*/
int QFontMetrics::charWidth( const QString &str, int pos ) const
{
    // ###
    return 0;
}


/*!
    Returns the width in pixels of the first \a len characters of \a
    str. If \a len is negative (the default), the entire string is
    used.

    Note that this value is \e not equal to boundingRect().width();
    boundingRect() returns a rectangle describing the pixels this
    string will cover whereas width() returns the distance to where
    the next string should be drawn.

    \sa boundingRect()
*/
int QFontMetrics::width( const QString &str, int len ) const
{
    if (len < 0)
	len = str.length();
    if (len == 0)
	return 0;

    // this algorithm is similar to the one used for painting
    bool simple = str.simpleText();
    QString shaped = simple ? str : QComplexText::shapedString( str, 0, len, QPainter::Auto, this );
    if ( !simple )
	len = shaped.length();

    return 0; //####
}


/*!
    Returns the bounding rectangle of the first \a len characters of
    \a str, which is the set of pixels the text would cover if drawn
    at (0, 0).

    If \a len is negative (the default), the entire string is used.

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts, and that the text output may cover \e
    all pixels in the bounding rectangle.

    Newline characters are processed as normal characters, \e not as
    linebreaks.

    Due to the different actual character heights, the height of the
    bounding rectangle of e.g. "Yes" and "yes" may be different.

    \sa width(), QPainter::boundingRect()
*/
QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
    if (len < 0)
	len = str.length();
    if (len == 0)
	return QRect();

    return QRect();
}


/*!
    Returns the width of the widest character in the font.
*/
int QFontMetrics::maxWidth() const
{
    FontEngineIface *qfs;
    int w = 0;

    for (int i = 0; i < QFont::LastPrivateScript - 1; i++) {
	if (! d->x11data.fontstruct[i])
	    continue;

	d->load((QFont::Script) i);

	qfs = d->x11data.fontstruct[i];
	if (! qfs || qfs == (FontEngineIface *) -1)
	    continue;
	w = QMAX( w, qfs->maxCharWidth() );
    }
    return w;
}

/*!
    Returns the distance from the base line to where an underscore
    should be drawn.

    \sa strikeOutPos(), lineWidth()
*/
int QFontMetrics::underlinePos() const
{
    int pos = ((lineWidth() * 2) + 3) / 6;

    return pos ? pos : 1;
}


/*!
    Returns the distance from the base line to where the strikeout
    line should be drawn.

    \sa underlinePos(), lineWidth()
*/
int QFontMetrics::strikeOutPos() const
{
    int pos = ascent() / 3;

    return pos > 0 ? pos : 1;
}


/*!
    Returns the width of the underline and strikeout lines, adjusted
    for the point size of the font.

    \sa underlinePos(), strikeOutPos()
*/
int QFontMetrics::lineWidth() const
{
    // lazy computation of linewidth
    d->computeLineWidth();

    return d->lineWidth;
}
