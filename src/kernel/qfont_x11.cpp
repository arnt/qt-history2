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
#include "qcomplextext_p.h"
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
#include "../codecs/qfontcodecs_p.h"

#ifndef QT_NO_XFTFREETYPE
# include "qintdict.h"
# include "qpixmap.h"
# include "qsettings.h"
#endif // QT_NO_XFTFREETYPE

#include "qt_x11.h"

#include <ctype.h>
#include <stdlib.h>
#include <time.h>


#if !defined(QT_NO_XFTFREETYPE) && defined(QT_NO_XFTNAMEUNPARSE)
// Assume XFree86 4.0.3 implementation which is missing XftInitFtLibrary and
// XftNameUnparse
extern "C" {
Bool XftInitFtLibrary();
Bool XftNameUnparse (XftPattern *, char *, int);
}
#endif


// #define QFONTLOADER_DEBUG
// #define QFONTLOADER_DEBUG_VERBOSE


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
static const char * const hiragana_encodings[] = { "jisx0208.1983-0", 0 };
static const char * const katakana_encodings[] = { "jisx0208.1983-0", 0 };
static const char * const hangul_encodings[] = { "ksc5601.1987-0", 0 };
static const char * const bopomofo_encodings[] = { "gb2312.1980-0", 0 };
static const char * const unicode_encodings[] = { "iso10646-1", "unicode-*", 0 };
static const char * const hanx11_encodings[] = { "gb18030-0", /* "gb18030.2000-1", */ "gb18030.2000-0", "big5-0", "big5hkscs-0" };
static const char * const latinA2_encodings[] = { "iso8859-2", 0 };
static const char * const latinA3_encodings[] = { "iso8859-3", 0 };
static const char * const latinA4_encodings[] = { "iso8859-4", 0 };
static const char * const latinA14_encodings[] = { "iso8859-14", 0 };
static const char * const latinA15_encodings[] = { "iso8859-15", 0 };

// we select one of these at initialization time for Han use
static const char * const hancn_encodings[] =
    { "gbk-0", "gb2312.1980-0", "big5-0", "big5hkscs-0", "jisx0208.1983-0", "ksc5601.1987-0", 0 };
static const char * const hancngb18030_encodings[] =
    { "gb18030-0", /* "gb18030.2000-1", */ "gb18030.2000-0", "gbk-0", "gb2312.1980-0", "big5-0", "jisx0208.1983-0", "ksc5601.1987-0", 0 };
static const char * const hanhk_encodings[] =
    { "big5hkscs-0", "hkscs-1", "big5-0", "gb2312.1980-0", "gbk-0", "jisx0208.1983-0", "ksc5601.1987-0", "gb18030-0", 0 };
static const char * const hanjp_encodings[] =
    { "jisx0208.1983-0", "gb2312.1980-0", "big5-0", "ksc5601.1987-0", "big5hkscs-0", "gbk-0", 0 };
static const char * const hankr_encodings[] =
    { "ksc5601.1987-0", "jisx0208.1983-0", "gb2312.1980-0", "big5-0", "big5hkscs-0", "gbk-0", 0 };
static const char * const hantw_encodings[] =
    { "big5-0", "gb2312.1980-0", "jisx0208.1983-0", "ksc5601.1987-0", "big5hkscs-0", "gbk-0", 0 };

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
    { 0, empty_encodings },
    // Tibetan
    { 0, empty_encodings },
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
    { 0, empty_encodings },

    // Ethiopic
    { 0, empty_encodings },
    // Cherokee
    { 0, empty_encodings },
    // CanadianAboriginal
    { 0, empty_encodings },
    // Mongolian
    { 0, empty_encodings },
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
    { 0, latinA15_encodings }
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
    memset( latinCache, 0,  256*sizeof( uchar ) );
}

QFontX11Data::~QFontX11Data()
{
    QFontStruct *qfs;

    for ( int i = 0; i < QFont::LastPrivateScript; i++ ) {
	qfs = fontstruct[i];
	fontstruct[i] = 0;

	if ( qfs && qfs != (QFontStruct *) -1 ) {
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
// QFontStruct member methods
// **********************************************************************

QFontStruct::~QFontStruct()
{
    if (handle) {
	XFreeFont(QPaintDevice::x11AppDisplay(), (XFontStruct *) handle);
	handle = 0;
    }

#ifndef QT_NO_XFTFREETYPE
    if (xfthandle) {
	XftFontClose(QPaintDevice::x11AppDisplay(), (XftFont *) xfthandle);
	xfthandle = 0;
    }

    if (xftpattern) {
	XftPatternDestroy((XftPattern *) xftpattern);
	xftpattern = 0;
    }
#endif // QT_NO_XFTFREETYPE

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

    QCString xlfd = XGetAtomName( QPaintDevice::x11AppDisplay(), value );
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

// returns TRUE if the character doesn't exist (ie. zero bounding box)
static inline bool charNonExistent(XCharStruct *xcs)
{
    return (xcs == (XCharStruct *) -1 || !xcs ||
	    (xcs->width == 0 && xcs->ascent + xcs->descent == 0));
}


// return the XCharStruct for the specified cell in the single dimension font xfs
static inline XCharStruct *getCharStruct1d(XFontStruct *xfs, uint c)
{
    XCharStruct *xcs = (XCharStruct *) -1;
    if (c >= xfs->min_char_or_byte2 &&
	c <= xfs->max_char_or_byte2) {
	if (xfs->per_char != NULL) {
	    xcs = xfs->per_char + (c - xfs->min_char_or_byte2);
	    if (charNonExistent(xcs))
		xcs = (XCharStruct *) -1;
	} else
	    xcs = &(xfs->min_bounds);
    }

    return xcs;
}


// return the XCharStruct for the specified row/cell in the 2 dimension font xfs
static inline XCharStruct *getCharStruct2d(XFontStruct *xfs, uint r, uint c)
{
    XCharStruct *xcs = (XCharStruct *) -1;

    if (r >= xfs->min_byte1 &&
	r <= xfs->max_byte1 &&
	c >= xfs->min_char_or_byte2 &&
	c <= xfs->max_char_or_byte2) {
	if (xfs->per_char != NULL) {
	    xcs = xfs->per_char + ((r - xfs->min_byte1) *
				   (xfs->max_char_or_byte2 -
				    xfs->min_char_or_byte2 + 1)) +
		  (c - xfs->min_char_or_byte2);
	    if (charNonExistent(xcs))
		xcs = (XCharStruct *) -1;
	} else
	    xcs = &(xfs->min_bounds);
    }

    return xcs;
}


// returns a XCharStruct based for the character at pos in str
static inline XCharStruct *getCharStruct(QFontStruct *qfs, const QString &str, int pos)
{
    XFontStruct *xfs;
    XCharStruct *xcs;
    unsigned short ch;

    if (! qfs || qfs == (QFontStruct *) -1 ||
    	! (xfs = (XFontStruct *) qfs->handle)) {
	xcs = (XCharStruct *) -1;
	goto end;
    }

    if (qfs->codec)
	ch = qfs->codec->characterFromUnicode(str, pos);
    else
	ch = QComplexText::shapedCharacter( str, pos ).unicode();

    if (ch == 0) {
	xcs = 0;
	goto end;
    }

    if (! xfs->max_byte1)
	// single row font
	xcs = getCharStruct1d(xfs, ch);
    else
	xcs = getCharStruct2d(xfs, (ch>>8), ch&0xff);

 end:
    return xcs;
}


#ifndef QT_NO_XFTFREETYPE

// returns a XGlyphInfo for the character as pos in str. this function can return:
//   -2 (meaning this fontstruct doesn't contain an xft font)
//   -1 (meaning the glyph doesn't exist in the font)
//    0 (meaning a zero width character)
//    a valid XGlyphInfo
static inline XGlyphInfo *getGlyphInfo(QFontStruct *qfs, const QString &str, int pos)
{
    XftFont *xftfs;
    XGlyphInfo *xgi;
    XftChar16 c;

    if (! qfs || qfs == (QFontStruct *) -1 ||
	! (xftfs = (XftFont *) qfs->xfthandle)) {
	xgi = (XGlyphInfo *) -2;
	goto end;
    }

    // no need for codec, all Xft fonts are in unicode mapping
    c = QComplexText::shapedCharacter(str, pos).unicode();
    if (c == 0) {
	xgi = 0;
	goto end;
    }

    // load the glyph if it's not in the font
    if (XftGlyphExists(QPaintDevice::x11AppDisplay(), xftfs, c)) {
	static XGlyphInfo metrics;
	XftTextExtents16(QPaintDevice::x11AppDisplay(), xftfs, &c, 1, &metrics);
	xgi = &metrics;
    } else {
	xgi = (XGlyphInfo *) -1;
    }

 end:
    return xgi;
}


// ditto
static inline XGlyphInfo *getGlyphInfo(QFontStruct *qfs, const QChar &ch)
{
    XftFont *xftfs;
    XGlyphInfo *xgi;
    XftChar16 c;

    if (! qfs || qfs == (QFontStruct *) -1 ||
	! (xftfs = (XftFont *) qfs->xfthandle)) {
	xgi = (XGlyphInfo *) -2;
	goto end;
    }

    c = ch.unicode();
    if (XftGlyphExists(QPaintDevice::x11AppDisplay(), xftfs, c)) {
	static XGlyphInfo metrics;
	XftTextExtents16(QPaintDevice::x11AppDisplay(), xftfs, &c, 1, &metrics);
	xgi = &metrics;
    } else {
	xgi = (XGlyphInfo *) -1;
    }

 end:
    return xgi;
}

#endif // QT_NO_XFTFREETYPE


// comment me
QRect QFontPrivate::boundingRect( const QChar &ch )
{
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch, this );

    QFontStruct *qfs = 0;
    XCharStruct *xcs = 0;
    QRect r;

    if (script != QFont::UnknownScript) {
	load(script);
	qfs = x11data.fontstruct[script];
    }
    double scale = (qfs != (QFontStruct *)-1) ? qfs->scale : 1.;
#ifndef QT_NO_XFTFREETYPE
    XGlyphInfo *xgi = 0;
    if ((xgi = getGlyphInfo(qfs, ch)) != (XGlyphInfo *) -2) {
	if ( xgi == (XGlyphInfo *) -1)
	    r.setRect( 0, actual.pixelSize * -3 / 4,
		       actual.pixelSize * 3 / 4, actual.pixelSize * 3 / 4);
	else if ( xgi )
	    r.setRect( int(-xgi->x*scale), int(-xgi->y*scale),
		       int(xgi->width*scale), int(xgi->height*scale) );
    } else
#endif // QT_NO_XFTFREETYPE

	{
	    xcs = getCharStruct(qfs, QString(ch), 0);
	    if ( xcs == (XCharStruct *) -1)
		r.setRect( 0, actual.pixelSize * -3 / 4,
			   actual.pixelSize * 3 / 4, actual.pixelSize * 3 / 4);
	    else if ( xcs )
		r.setRect( int(xcs->lbearing*scale), int(-xcs->ascent*scale),
			   int((xcs->rbearing - xcs->lbearing)*scale),
			   int((xcs->descent + xcs->ascent)*scale) );
	}

    return r;
}


// returns the width of the string in pixels (replaces XTextWidth)
// assumes the string is already shaped
int QFontPrivate::textWidth( const QString &str, int pos, int len )
{
    const QChar *chars = str.unicode() + pos;
    QFont::Script current = QFont::NoScript, tmp;
    int i;
    int w = 0;
    int tmpw = 0;
    double scale = 1.;

    QFontStruct *qfs = 0;
    XCharStruct *xcs = 0;

#ifndef QT_NO_XFTFREETYPE
    XGlyphInfo *xgi = 0;
#endif // QT_NO_XFTFREETYPE

    for (i = 0; i < len; i++) {
	// combiningClass is always 0 for unicode < 0x400
	if (chars->unicode() < 0x400 || chars->combiningClass() == 0 || pos + i == 0) {
	    SCRIPT_FOR_CHAR( tmp, *chars, this );

	    if (tmp != current) {
		// new script, make sure the font is loaded
		if (tmp != QFont::UnknownScript) {
		    load(tmp);
		    qfs = x11data.fontstruct[tmp];
		} else
		    qfs = 0;

		w += (int) (tmpw*scale);
		tmpw = 0;
		current = tmp;
		scale =  (qfs != (QFontStruct *)-1) ? qfs->scale : 1.;
	    }

	    int width = 0;
	    if ( tmp == QFont::LatinBasic )
		width = x11data.latinCache[chars->unicode()];

	    if ( !width ) {
#ifndef QT_NO_XFTFREETYPE
		if ((xgi = getGlyphInfo(qfs, *chars)) != (XGlyphInfo *) -2) {
		    if (xgi == (XGlyphInfo *) -1) {
			// character isn't in the font, set the script to UnknownScript
			tmp = current = QFont::UnknownScript;
			w += (int) (tmpw*scale);
			tmpw = 0;
			width = actual.pixelSize * 3 / 4;
			scale = 1.;
		    } else if (xgi)
			width = xgi->xOff;
		} else
#endif // QT_NO_XFTFREETYPE
		{
		    xcs = getCharStruct(qfs, str, pos + i);
		    if (xcs == (XCharStruct *) -1) {
			// character isn't in the font, set the script to UnknownScript
			tmp = current = QFont::UnknownScript;
			w += (int) (tmpw*scale);
			width = actual.pixelSize * 3 / 4;
			scale = 1.;
		    } else if (xcs)
			width = xcs->width;
		}
		if ( tmp == QFont::LatinBasic && width < 0x100 )
		    x11data.latinCache[chars->unicode()] = (uchar)width;
	    }
	    tmpw += width;
	}

	chars++;
    }

    w += (int) (tmpw*scale);
    return w;
}


// returns the width of the string in pixels (replaces XTextWidth)... this function
// also sets up a TextRun cache, which is used by QFontPrivate::drawText below
int QFontPrivate::textWidth( const QString &str, int pos, int len,
			     QFontPrivate::TextRun *cache )
{
    const QChar *chars = str.unicode() + pos, *last = 0;
    QFont::Script current = QFont::NoScript;
    QFont::Script tmp = QFont::NoScript;

    int i, w = 0, pw = 0, lastlen = 0;

    // for non-spacing marks
    QPointArray markpos;
    bool markexists;
    int nmarks = 0;

    QFontStruct *qfs = 0;
    XCharStruct *xcs = 0;

#ifndef QT_NO_XFTFREETYPE
    XGlyphInfo *xgi = 0;
#endif // QT_NO_XFTFREETYPE

    for (i = 0; i < len; i++) {
	// combiningClass is always 0 for unicode < 0x400
	if (chars->unicode() < 0x400 || chars->combiningClass() == 0 || pos + i == 0) {
	    SCRIPT_FOR_CHAR( tmp, *chars, this );

	    if (tmp != current ||
		// X11 doesn't draw strings wider than 32768px
		// we use a smaller value here, since no screen is wider than
		// 4000 px, so we can optimise drawing
		w > pw + 4096 ||

		// RENDER has problems drawing strings longer than 250 chars (253
		// seems to be the length that breaks). Split the cache every
		// 250 characters so we don't run into this. (note: this is with
		// XFree86 4.1.0)
		lastlen > 250) {
		if (last && lastlen) {
		    if (qfs && qfs != (QFontStruct *) -1 && qfs->codec)
			cache->mapped =
			    qfs->codec->fromUnicode(str, pos + i - lastlen, lastlen);
		    cache->setParams(pw, 0, w, last, lastlen, current);

		    cache->next = new QFontPrivate::TextRun();
		    cache = cache->next;
		    pw = w;
		}

		last = chars;
		lastlen = 0;

		// new script, make sure the font is loaded
		if (tmp != QFont::UnknownScript) {
		    load(tmp);
		    qfs = x11data.fontstruct[tmp];
		} else
		    qfs = 0;

		current = tmp;
	    }

#ifndef QT_NO_XFTFREETYPE
	    if ((xgi = getGlyphInfo(qfs, str, pos + i)) != (XGlyphInfo *) -2) {
		if (xgi == (XGlyphInfo *) -1) {
		    if (qfs && qfs != (QFontStruct *) -1) {
			// character isn't in the font, so we insert a cache break
			// and set the script to UnknownScript

			if (last && lastlen) {
			    if (qfs && qfs != (QFontStruct *) -1 && qfs->codec)
				cache->mapped =
				    qfs->codec->fromUnicode(str, pos + i - lastlen,
							    lastlen);
			    cache->setParams(pw, 0, w, last, lastlen, current);
			}

			cache->next = new QFontPrivate::TextRun();
			cache = cache->next;
			pw = w;

			last = chars;
			lastlen = 0;

			tmp = current = QFont::UnknownScript;
		    }

		    w += actual.pixelSize * 3 / 4;
		} else if (xgi)
		    w += xgi->xOff;
	    } else
#endif // QT_NO_XFTFREETYPE
	    {
		xcs = getCharStruct(qfs, str, pos + i);
		if (xcs == (XCharStruct *) -1) {
		    if (qfs && qfs != (QFontStruct *) -1) {
			// character isn't in the font, so we insert a cache break
			// and set the script to UnknownScript

			if (last && lastlen) {
			    if (qfs && qfs != (QFontStruct *) -1 && qfs->codec)
				cache->mapped =
				    qfs->codec->fromUnicode(str, pos + i - lastlen,
							    lastlen);
			    cache->setParams(pw, 0, w, last, lastlen, current);
			}

			cache->next = new QFontPrivate::TextRun();
			cache = cache->next;
			pw = w;

			last = chars;
			lastlen = 0;

			tmp = current = QFont::UnknownScript;
		    }

		    w += actual.pixelSize * 3 / 4;
		} else if (xcs)
		    w += xcs->width;
	    }

	    chars++;
	    lastlen++;
	} else {
	    // start of a new set of marks
	    if (last && lastlen) {
		// force a cache break when starting a set of marks
		if (qfs && qfs != (QFontStruct *) -1 && qfs->codec)
		    cache->mapped =
			qfs->codec->fromUnicode(str, pos + i - lastlen, lastlen);
		cache->setParams(pw, 0, w, last, lastlen, tmp);

		cache->next = new QFontPrivate::TextRun();
		cache = cache->next;
		pw = w;
	    }

	    last = chars;
	    lastlen = 0;
	    markpos = QComplexText::positionMarks(this, str, pos + i - 1);
	    nmarks = markpos.size();

	    // deal with all marks
	    for (int n = 0; n < nmarks; n++) {
		// make sure font is loaded for mark
		SCRIPT_FOR_CHAR( tmp, *chars, this );
		if (tmp != QFont::UnknownScript) {
		    load(tmp);
		    qfs = x11data.fontstruct[tmp];
		} else
		    qfs = 0;

		markexists = inFont(*chars);

		QPoint p = markpos[n];

		// advance one character
		chars++;
		lastlen++;

		if (last && lastlen) {
		    if (markexists) {
			if (qfs && qfs != (QFontStruct *) -1 && qfs->codec)
			    cache->mapped =
				qfs->codec->fromUnicode(str, pos + i - lastlen, lastlen);
			// set the position for the mark
			cache->setParams(pw + p.x(), p.y(), w, last, lastlen, tmp);

			// advance to the next mark/character
			cache->next = new QFontPrivate::TextRun();
			cache = cache->next;

			current = QFont::UnknownScript;
		    }
		}

		last = chars;
		lastlen = 0;
	    }

	    if (nmarks > 1)
		i += nmarks - 1;
	}
    }

    if (last && lastlen) {
	if (qfs && qfs != (QFontStruct *) -1 && qfs->codec)
	    cache->mapped =
		qfs->codec->fromUnicode(str, pos + i - lastlen, lastlen);
	cache->setParams( pw, 0, w, last, lastlen, current );
    }

    return w;
}

// needed to get printer font metrics right, and because
// XCharStruct is limited to shorts.
struct QCharStruct
{
    QCharStruct() {
	ascent = -1000000;
	descent = -1000000;
	lbearing = 1000000;
	rbearing = -1000000;
	width = 0;
    }
    float ascent;
    float descent;
    float lbearing;
    float rbearing;
    float width;
};


// return the text extents in overall, replaces XTextExtents
void QFontPrivate::textExtents( const QString &str, int pos, int len,
				QCharStruct *overall )
{
    const QChar *chars = str.unicode() + pos;
    QFont::Script current = QFont::NoScript, tmp;
    int i;

    // for non-spacing marks
    QPointArray markpos;
    bool markexists;
    int nmarks = 0;

    QFontStruct *qfs = 0;
    XCharStruct *xcs = 0;

#ifndef QT_NO_XFTFREETYPE
    XGlyphInfo *xgi = 0;
#endif // QT_NO_XFTFREETYPE

    float scale = 1;
    for (i = 0; i < len; i++) {
	// combiningClass is always 0 for unicode < 0x400
	if (chars->unicode() < 0x400 || chars->combiningClass() == 0 || pos + i == 0) {
	    SCRIPT_FOR_CHAR( tmp, *chars, this );

	    if (tmp != current) {
		// new script, make sure the font is loaded
		if (tmp != QFont::UnknownScript) {
		    load(tmp);
		    qfs = x11data.fontstruct[tmp];
		    if (qfs && qfs != (QFontStruct *) -1 )
			scale = qfs->scale;
		    else
			scale = 1.;
		} else {
		    qfs = 0;
		    scale = 1.;
		}

		current = tmp;
	    }

#ifndef QT_NO_XFTFREETYPE
	    if ((xgi = getGlyphInfo(qfs, str, pos + i)) != (XGlyphInfo *) -2) {
		if (xgi == (XGlyphInfo *) -1) {
		    // character isn't in the font, set the script to UnknownScript
		    tmp = current = QFont::UnknownScript;

		    float size = (actual.pixelSize * 3. / 4.);
		    overall->ascent = QMAX(overall->ascent, size);
		    overall->descent = QMAX(overall->descent, 0);
		    overall->lbearing = QMIN(overall->lbearing, 0);
		    overall->width += size;
		    overall->rbearing = QMAX(overall->rbearing, overall->width);
		} else {
		    overall->ascent = QMAX(overall->ascent, xgi->y*scale);
		    overall->descent = QMAX(overall->descent, (xgi->height - xgi->y)*scale);
		    overall->lbearing = QMIN(overall->lbearing, -xgi->x*scale);
		    overall->rbearing = QMAX(overall->rbearing, overall->width +
					     (xgi->width - xgi->x)*scale);
		    overall->width += xgi->xOff;
		}
	    } else
#endif // QT_NO_XFTFREETYPE
	    {
		xcs = getCharStruct(qfs, str, pos + i);
		if (xcs == (XCharStruct *) -1) {
		    // character isn't in the font, set the script to UnknownScript
		    tmp = current = QFont::UnknownScript;

		    int size = (actual.pixelSize * 3 / 4);
		    overall->ascent = QMAX(overall->ascent, size);
		    overall->descent = QMAX(overall->descent, 0);
		    overall->lbearing = QMIN(overall->lbearing, 0);
		    overall->width += size;
		    overall->rbearing = QMAX(overall->rbearing, overall->width);
		} else if (xcs) {
		    overall->ascent = QMAX(overall->ascent, xcs->ascent*scale);
		    overall->descent = QMAX(overall->descent, xcs->descent*scale);
		    overall->lbearing = QMIN(overall->lbearing,
					     overall->width + xcs->lbearing*scale);
		    overall->rbearing = QMAX(overall->rbearing,
					     overall->width + xcs->rbearing*scale);
		    overall->width += xcs->width*scale;
		}
	    }

	    chars++;
	} else {
	    // start of a new set of marks
	    markpos = QComplexText::positionMarks(this, str, pos + i - 1);
	    nmarks = markpos.size();

	    // deal with all marks
	    for (int n = 0; n < nmarks; n++) {
		// make sure font is loaded for mark
		SCRIPT_FOR_CHAR( tmp, *chars, this );
		if (tmp != QFont::UnknownScript) {
		    load(tmp);
		    qfs = x11data.fontstruct[tmp];
		} else
		    qfs = 0;

		markexists = inFont(*chars);

		QPoint p = markpos[n];

		if (markexists) {
		    // yes, really do both, this makes sure that marks that rise above
		    // the box expand it up, and marks below the box expand it down
		    overall->ascent = QMAX(overall->ascent, p.y()*scale);
		    overall->descent = QMAX(overall->descent, p.y()*scale);

		    current = QFont::UnknownScript;
		}

		// advance one character
		chars++;
	    }

	    if (nmarks > 1)
    		i += nmarks - 1;
	}
    }
}


// draw the text run cache... nonspacing marks, bidi reordering and all compositions
// will have already been done by the time we get here
void QFontPrivate::drawText( Display *dpy, int screen, Qt::HANDLE hd, Qt::HANDLE rendhd,
			     GC gc, const QColor &pen, Qt::BGMode bgmode,
			     const QColor& bgcolor, int x, int y,
			     const QFontPrivate::TextRun *cache, int pdWidth )
{
    Q_UNUSED(screen);
    Q_UNUSED(rendhd);
    Q_UNUSED(pen);
    Q_UNUSED(bgcolor);

    Qt::HANDLE fid_last = 0;

    while ( cache ) {
	QFontStruct *qfs = x11data.fontstruct[cache->script];
	XFontStruct *xfs = 0;

#ifndef QT_NO_XFTFREETYPE
	XftFont *xftfs = 0;
#endif // QT_NO_XFTFREETYPE

    	if ( cache->script < QFont::LastPrivateScript &&
	     cache->length > 0 &&
	     qfs && qfs != (QFontStruct *) -1 ) {
	    xfs = (XFontStruct *) qfs->handle;

#ifndef QT_NO_XFTFREETYPE
	    xftfs = (XftFont *) qfs->xfthandle;
#endif // QT_NO_XFTFREETYPE

	}
	// clip away invisible parts. Saves some drawing operations.
	if ( x + cache->xoff < pdWidth && x + cache->xoff + cache->x2off > 0 ) {

#ifndef QT_NO_XFTFREETYPE
	    if (xftfs) {
		XftDraw *draw = (XftDraw *) rendhd;

		if (bgmode != Qt::TransparentMode) {
		    XftColor col;
		    col.color.red = bgcolor.red()     | bgcolor.red() << 8;
		    col.color.green = bgcolor.green() | bgcolor.green() << 8;
		    col.color.blue = bgcolor.blue()   | bgcolor.blue() << 8;
		    col.color.alpha = 0xffff;
		    col.pixel = bgcolor.pixel();
		    XftDrawRect(draw, &col, x + cache->xoff,  y - xftfs->ascent,
				cache->x2off - cache->xoff,
				xftfs->ascent + xftfs->descent);
		}

		XftColor col;
		col.color.red = pen.red () | pen.red() << 8;
		col.color.green = pen.green () | pen.green() << 8;
		col.color.blue = pen.blue () | pen.blue() << 8;
		col.color.alpha = 0xffff;
		col.pixel = pen.pixel();
		XftDrawString16 (draw, &col, xftfs,
				 x + cache->xoff, y + cache->yoff,
				 (unsigned short *)cache->string, cache->length);
	    } else
#endif // QT_NO_XFTFREETYPE
		if (xfs) {
		    if (xfs->fid != fid_last) {
			XSetFont(dpy, gc, xfs->fid);
			fid_last = xfs->fid;
		    }

		    if ( xfs->max_byte1 || ! qfs->codec ) {
			XChar2b *chars;
			if ( qfs->codec )
			    chars = (XChar2b *) cache->mapped.data();
			else {
			    chars = new XChar2b[cache->length];
			    int i;
			    for (i = 0; i < cache->length; i++) {
				chars[i].byte1 = cache->string[i].row();
				chars[i].byte2 = cache->string[i].cell();
			    }
			}

			if (bgmode != Qt::TransparentMode)
			    XDrawImageString16(dpy, hd, gc, x + cache->xoff, y + cache->yoff,
					       chars, cache->length );
			else
			    XDrawString16(dpy, hd, gc, x + cache->xoff, y + cache->yoff,
					  chars, cache->length );
			if ( !qfs->codec )
			    delete [] chars;
		    } else {
			const char *chars = cache->mapped.data();
			if ( chars ) {
			    if (bgmode != Qt::TransparentMode)
				XDrawImageString(dpy, hd, gc, x + cache->xoff,
						 y + cache->yoff, chars, cache->length );
			    else
				XDrawString(dpy, hd, gc, x + cache->xoff,
					    y + cache->yoff, chars, cache->length );
			} else
			    qWarning( "internal error in QFontPrivate::drawText()" );
		    }
		} else {
		    int l = cache->length;
		    XRectangle *rects = new XRectangle[l];
		    int inc = actual.pixelSize * 3 / 4;

		    for (int k = 0; k < l; k++) {
			rects[k].x = x + cache->xoff + (k * inc);
			rects[k].y = y - inc + 2;
			rects[k].width = rects[k].height = inc - 3;
		    }

		    XDrawRectangles(dpy, hd, gc, rects, l);
		    delete [] rects;
		}
	}

	cache = cache->next;
    }
}


// returns TRUE if the character exists in the font, FALSE otherwise
bool QFontPrivate::inFont( const QChar &ch )
{
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch, this );

    if (script == QFont::UnknownScript)
	return FALSE;

    load(script);

    QFontStruct *qfs = x11data.fontstruct[script];

#ifndef QT_NO_XFTFREETYPE
    if (qfs && qfs != (QFontStruct *) -1 && qfs->xfthandle)
	return XftGlyphExists(QPaintDevice::x11AppDisplay(),
			      (XftFont *) qfs->xfthandle, ch.unicode());
#endif // QT_NO_XFTFREETYPE

    XCharStruct *xcs = getCharStruct(qfs, QString(ch), 0);
    return (! charNonExistent(xcs));
}


#ifndef QT_NO_XFTFREETYPE

static XftPattern *checkXftFont( XftPattern *match, const QString &familyName, const QChar &sample )
{
#ifndef QT_XFT2
    char * family_value;
    XftPatternGetString (match, XFT_FAMILY, 0, &family_value);
    QString fam = family_value;
    //qDebug("got family %s for request %s", fam.latin1(), familyName.latin1() );

    if ( fam.lower() != familyName.lower() ) {
	//qDebug("pattern don't match");
	XftPatternDestroy(match);
	match = 0;
    }
#else
    Q_UNUSED( familyName );
#endif

    if (match && sample.unicode() != 0 ) {
	// check if the character is actually in the font - this does result in
	// a font being loaded, but since Xft is completely client side, we can
	// do this efficiently
#ifdef QT_XFT2
    	FcCharSet   *c;
 	if (FcPatternGetCharSet(match, FC_CHARSET, 0, &c) == FcResultMatch) {
 	    if (!FcCharSetHasChar(c, sample.unicode())) {
		XftPatternDestroy(match);
 		match = 0;
 	    }
 	}
#else
	XftFontStruct *xftfs = XftFreeTypeOpen(QPaintDevice::x11AppDisplay(), match);

	if (xftfs) {
	    if ( ! XftFreeTypeGlyphExists(QPaintDevice::x11AppDisplay(), xftfs,
					  sample.unicode())) {
		XftPatternDestroy(match);
		match = 0;
	    }

	    XftFreeTypeClose(QPaintDevice::x11AppDisplay(), xftfs);
	}
#endif
    }

    return match;
}

// returns an XftPattern for the font or zero if no found supporting the script could
// be found
XftPattern *QFontPrivate::findXftFont(const QChar &sample, bool *exact, double *scale) const
{
    // look for foundry/family
    QString familyName;
    QString foundryName;

    QFontDatabase::parseFontName(request.family, foundryName, familyName);
    XftPattern *match = bestXftPattern(familyName, foundryName, sample, scale);

    if ( match )
	match = checkXftFont( match, familyName, sample );

    *exact = TRUE;

    if (match)
    	return match;

    *exact = FALSE;

    // try font substitutions
    QStringList list = QFont::substitutes(request.family);
    QStringList::Iterator sit = list.begin();

    while (sit != list.end() && ! match) {
	familyName = *sit++;

	if (request.family != familyName) {
	    QFontDatabase::parseFontName(familyName, foundryName, familyName);
	    match = bestXftPattern(familyName, foundryName, sample, scale);

	    if ( match )
		match = checkXftFont( match, familyName, sample );
	}
    }

    return match;
}

// finds an XftPattern best matching the familyname, foundryname and other
// requested pieces of the font
XftPattern *QFontPrivate::bestXftPattern(const QString &familyName,
					 const QString &foundryName,
					 const QChar &sample, double *scale) const
{
    QCString generic_value;
    int weight_value;
    int slant_value;
    double size_value;
    int mono_value;
    const char *sizeFormat;

    weight_value = request.weight;
    if (weight_value == 0)
	weight_value = XFT_WEIGHT_MEDIUM;
    else if (weight_value < (QFont::Light + QFont::Normal) / 2)
	weight_value = XFT_WEIGHT_LIGHT;
    else if (weight_value < (QFont::Normal + QFont::DemiBold) / 2)
	weight_value = XFT_WEIGHT_MEDIUM;
    else if (weight_value < (QFont::DemiBold + QFont::Bold) / 2)
	weight_value = XFT_WEIGHT_DEMIBOLD;
    else if (weight_value < (QFont::Bold + QFont::Black) / 2)
	weight_value = XFT_WEIGHT_BOLD;
    else
	weight_value = XFT_WEIGHT_BLACK;

    if (request.italic)
	slant_value = XFT_SLANT_ITALIC;
    else
	slant_value = XFT_SLANT_ROMAN;

    if ( paintdevice &&
	 (QPaintDeviceMetrics( paintdevice ).logicalDpiY() !=
	  QPaintDevice::x11AppDpiY( x11Screen )) ) {
	size_value = pixelSize( request, paintdevice, x11Screen );
	sizeFormat = XFT_PIXEL_SIZE;
	//qDebug("requesting scaled font, dpy=%d, pixelsize=%f",
	// QPaintDeviceMetrics( paintdevice ).logicalDpiY(), size_value);
    } else if ( request.pointSize != -1 ) {
	size_value = request.pointSize / 10.;
	sizeFormat = XFT_SIZE;
    } else {
	size_value = request.pixelSize;
	sizeFormat = XFT_PIXEL_SIZE;
    }
    if ( size_value > MAXFONTSIZE ) {
	*scale = (double)size_value/(double)MAXFONTSIZE;
	size_value = MAXFONTSIZE;
    } else {
	*scale = 1.;
    }

    mono_value = request.fixedPitch ? XFT_MONO : XFT_PROPORTIONAL;

    switch (request.styleHint) {
    case QFont::SansSerif:
    default:
	generic_value = "sans";
	break;
    case QFont::Serif:
	generic_value = "serif";
	break;
    case QFont::TypeWriter:
	generic_value = "mono";
	mono_value = XFT_MONO;
	break;
    }

    XftResult res;
    XftPattern *pattern = 0, *result = 0;

    pattern = XftPatternCreate();
    if ( ! pattern ) return 0;

#ifndef QT_XFT2
    XftPatternAddString (pattern, XFT_ENCODING, "iso10646-1");
#endif
    if (! foundryName.isNull())
	XftPatternAddString (pattern, XFT_FOUNDRY, foundryName.latin1());
    if (! familyName.isNull())
	XftPatternAddString (pattern, XFT_FAMILY, familyName.latin1());
    XftPatternAddString (pattern, XFT_FAMILY, generic_value.data());

    if (mono_value >= XFT_MONO)
 	XftPatternAddInteger (pattern, XFT_SPACING, mono_value);

    XftPatternAddInteger (pattern, XFT_WEIGHT, weight_value);
    XftPatternAddInteger (pattern, XFT_SLANT, slant_value);
    XftPatternAddDouble (pattern, sizeFormat, size_value);

#ifdef QT_XFT2
    if ( sample.unicode() != 0 ) {
	FcCharSet *cs = FcCharSetCreate ();
	FcCharSetAddChar (cs, sample.unicode());
	FcPatternAddCharSet (pattern, FC_CHARSET, cs);
	FcCharSetDestroy (cs); // let pattern hold last reference
    }
#else
    Q_UNUSED( sample );
#endif

    if ( !qt_use_antialiasing || request.styleStrategy & ( QFont::PreferAntialias |
							   QFont::NoAntialias) ) {
	bool requestAA;
	if ( !qt_use_antialiasing || request.styleStrategy & QFont::NoAntialias )
	    requestAA = FALSE;
	else
	    requestAA = TRUE;
	XftPatternAddBool( pattern, XFT_ANTIALIAS,requestAA );
    }

    result = XftFontMatch(QPaintDevice::x11AppDisplay(),
			  x11Screen, pattern, &res);
    XftPatternDestroy(pattern);

    return result;
}

#endif // QT_NO_XFTFREETYPE


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


// fill the actual fontdef with data from the loaded font
void QFontPrivate::initFontInfo(QFont::Script script, double scale)
{
    // set the scale value for each font correctly...
    if ( scale > 0 && x11data.fontstruct[script] != (QFontStruct *) -1 ) {
	x11data.fontstruct[script]->scale = scale;
    }
    if ((script != QFont::Unicode && script != defaultScript) || !actual.dirty ||
	x11data.fontstruct[script] == (QFontStruct *) -1) {
	// make sure the pixel size is correct, so that we can draw the missing char
	// boxes in the correct size...
	if (request.pixelSize == -1) {
	    actual.pointSize = request.pointSize;
	    actual.pixelSize = (int)(pixelSize( actual, paintdevice, x11Screen ) +.5);
	}
	return;
    }

    if ( paintdevice &&
	 (QPaintDeviceMetrics( paintdevice ).logicalDpiY() !=
	  QPaintDevice::x11AppDpiY( x11Screen )) ) {
	// we have a printer font
	actual = request;
	float _pointSize = pointSize( actual, paintdevice, x11Screen );
	float _pixelSize = pixelSize( actual, paintdevice, x11Screen );
	if ( actual.pointSize == -1 )
	    actual.pointSize = (int)(_pointSize + 0.5);
	else
	    actual.pixelSize = (int) (_pixelSize + 0.5);

	QFontDef font;
	if ( fillFontDef(x11data.fontstruct[script]->name, &font, x11Screen ) ) {
	    if ( font.pixelSize != 0 )
		x11data.fontstruct[script]->scale *= _pixelSize/((float) font.pixelSize);
	    //qDebug("setting scale to %f requested pixel=%f got %d",
	    // x11data.fontstruct[script]->scale, _pixelSize, font.pixelSize);
	}
	return;
    }

    actual.lbearing = SHRT_MIN;
    actual.rbearing = SHRT_MIN;

    if (exactMatch) {
	actual = request;
	actual.dirty = FALSE;

	if ( actual.pointSize == -1 )
	    actual.pointSize = (int)(pointSize( actual, paintdevice, x11Screen ) +.5);
	else
	    actual.pixelSize = (int)(pixelSize( actual, paintdevice, x11Screen ) +.5);

#ifndef   QT_NO_XFTFREETYPE
	if (x11data.fontstruct[script]->xftpattern) {
	    // parse the pattern
	    XftPattern *pattern =
		(XftPattern *) x11data.fontstruct[script]->xftpattern;

	    char *family_value;
	    int slant_value;
	    int weight_value;
	    int spacing_value = XFT_PROPORTIONAL;
	    XftPatternGetString (pattern, XFT_FAMILY, 0, &family_value);
	    XftPatternGetInteger (pattern, XFT_SLANT, 0, &slant_value);
	    XftPatternGetInteger (pattern, XFT_WEIGHT, 0, &weight_value);
	    XftPatternGetInteger (pattern, XFT_SPACING, 0, &spacing_value);
	    if (weight_value == XFT_WEIGHT_LIGHT)
		weight_value = QFont::Light;
	    else if (weight_value <= XFT_WEIGHT_MEDIUM)
		weight_value = QFont::Normal;
	    else if (weight_value <= XFT_WEIGHT_DEMIBOLD)
		weight_value = QFont::DemiBold;
	    else if (weight_value <= XFT_WEIGHT_BOLD)
		weight_value = QFont::Bold;
	    else if ( weight_value <= XFT_WEIGHT_BLACK)
		weight_value = QFont::Black;
	    else
		weight_value = QFont::Normal;

	    actual.family = family_value;
	    actual.weight = weight_value;
	    actual.italic = (slant_value != XFT_SLANT_ROMAN);
	    actual.fixedPitch = (spacing_value >= XFT_MONO);
	} else
#endif // QT_NO_XFTFREETYPE
	    {
		QFontDef def;
		if ( ! fillFontDef( (XFontStruct *) x11data.fontstruct[script]->handle,
				    &def, x11Screen ) &&
		     ! fillFontDef( x11data.fontstruct[script]->name,
				    &def, x11Screen ) ) {
		    // failed to parse the XLFD of the exact match font...
		    // this should never happen...
		    exactMatch = FALSE;
		} else {
		    QString dfoundry, dfamily, afoundry, afamily;
		    QFontDatabase::parseFontName( def.family, dfoundry, dfamily );
		    QFontDatabase::parseFontName( actual.family, afoundry, afamily );

		    if ( dfamily        != afamily            ||
			 ( !dfoundry.isEmpty() &&
			   !afoundry.isEmpty() &&
			   dfoundry     != afoundry )         ||
			 ( !def.addStyle.isEmpty() &&
			   !actual.addStyle.isEmpty() &&
			   def.addStyle   != actual.addStyle ) ) {
			// the foundry/family/addStyle do not match between
			// these 2 fontdefs... we have most likely made an
			// exact match with a font alias... fix it...
			actual.family = def.family;
			actual.addStyle = def.addStyle;
			exactMatch = FALSE;

		    }
		}
		// if we have a scaled font, we fake actual to show the correct size
		// value nevertheless....
		actual.pointSize = (int) (actual.pointSize*scale);
		actual.pixelSize = (int) (actual.pixelSize*scale);
	    }

	return;
    }

    if ( ! fillFontDef( (XFontStruct *) x11data.fontstruct[script]->handle,
			&actual, x11Screen ) &&
	 ! fillFontDef( x11data.fontstruct[script]->name,
			&actual, x11Screen ) ) {
	// zero fontdef
	actual = QFontDef();

	actual.family = QString::fromLatin1(x11data.fontstruct[script]->name);
	actual.rawMode = TRUE;
	actual.pointSize = request.pointSize;
	actual.pixelSize = request.pixelSize;
	exactMatch = FALSE;

	if ( actual.pointSize == -1 )
	    actual.pointSize = (int)(pointSize( actual, paintdevice, x11Screen ) +.5);
	else
	    actual.pixelSize = (int)(pixelSize( actual, paintdevice, x11Screen ) +.5);
    }

    actual.pointSize = (int)(actual.pointSize*scale);
    actual.pixelSize = (int)(actual.pixelSize*scale);
    actual.underline = request.underline;
    actual.strikeOut = request.strikeOut;
    actual.dirty = FALSE;
}


QFont::Script QFontPrivate::hanHack( const QChar &c )
{
    QFontStruct *f;

    load( QFont::Han, TRUE );
    if ( (f = x11data.fontstruct[QFont::Han]) != (QFontStruct *) -1 ) {
	Q_ASSERT( f != 0 );
	if ( !f->codec || f->codec->canEncode( c ) )
	    return QFont::Han;
    }
    // Han didn't do it, let's try the other ones...

    // japanese
    load( QFont::Hiragana, FALSE );
    if ( (f = x11data.fontstruct[QFont::Hiragana]) != (QFontStruct *) -1 ) {
	Q_ASSERT( f != 0 );
	if ( !f->codec || f->codec->canEncode( c ) )
	    return QFont::Hiragana;
    }

    // korean
    load( QFont::Hangul, FALSE );
    if ( (f = x11data.fontstruct[QFont::Hangul]) != (QFontStruct *) -1 ) {
	Q_ASSERT( f != 0 );
	if ( !f->codec || f->codec->canEncode( c ) )
	    return QFont::Hangul;
    }

    // traditional chinese
    load( QFont::Bopomofo, FALSE );
    if ( (f = x11data.fontstruct[QFont::Bopomofo]) != (QFontStruct *) -1 ) {
	Q_ASSERT( f != 0 );
	if ( !f->codec || f->codec->canEncode( c ) )
	    return QFont::Bopomofo;
    }

    // simplified chinese
    load( QFont::HanX11, FALSE );
    if ( (f = x11data.fontstruct[QFont::HanX11]) != (QFontStruct *) -1 ) {
	Q_ASSERT( f != 0 );
	if ( !f->codec || f->codec->canEncode( c ) )
	    return QFont::HanX11;
    }
    return QFont::Han;
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
    case QFont::Gurmukhi:                  ch = 0xa010; break;
    case QFont::Gujarati:                  ch = 0x0a90; break;
    case QFont::Oriya:                     ch = 0x0b10; break;
    case QFont::Tamil:                     ch = 0x0b90; break;
    case QFont::Telugu:                    ch = 0x0c10; break;
    case QFont::Kannada:                   ch = 0x0c90; break;
    case QFont::Malayalam:                 ch = 0x0d10; break;
    case QFont::Sinhala:                   ch = 0x0d90; break;
    case QFont::Thai:                      ch = 0x0e10; break;
    case QFont::Lao:                       ch = 0xe081; break;
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
    QFontStruct *qfs = x11data.fontstruct[QFont::Unicode];

    if (! qfs) {

#ifdef QFONTLOADER_DEBUG_VERBOSE
	qDebug("QFontLoader: trying to load unicode font");
#endif

	load(QFont::Unicode, FALSE);
	qfs = x11data.fontstruct[QFont::Unicode];
    }

    if (qfs && qfs != (QFontStruct *) -1) {
	if (qfs->handle) {
	    XFontStruct *xfs = (XFontStruct *) qfs->handle;

	    if (xfs->per_char) {
		XCharStruct *xcs = getCharStruct2d(xfs, 0xff, 0xfe);

#ifdef QFONTLOADER_DEBUG_VERBOSE
		qDebug("QFontLoader: unicode font has individual charstructs");
		qDebug("QFontLoader: m1 %d x1 %d m2 %d x2 %d",
		       xfs->min_byte1,
		       xfs->max_byte1,
		       xfs->min_char_or_byte2,
		       xfs->max_char_or_byte2);

		if (xcs && xcs != (XCharStruct *) -1) {
		    qDebug("QFontLoader: bounding box for undefined char: "
			   "%d %d %d",
			   xcs->width, xcs->ascent, xcs->descent);
		} else {
		    qDebug("QFontLoader: unicode font doesn't have undefined "
			   "char?");
		}

#endif

		if (charNonExistent(xcs) && sample.row() + sample.cell() != 0) {
		    xcs = getCharStruct2d(xfs, sample.row(), sample.cell());
		    hasChar = (! charNonExistent(xcs));
		}
	    }
	}
#ifndef QT_NO_XFTFREETYPE
	else {
	    hasChar = XftGlyphExists(QPaintDevice::x11AppDisplay(),
				     (XftFont *) qfs->xfthandle,
				     sample.unicode());
	}
#endif // QT_NO_XFTFREETYPE

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
	QFontStruct *qfs = x11data.fontstruct[script];
	x11data.fontstruct[script] = 0;

	if (qfs && qfs != (QFontStruct *) -1) {
	    // only deref here... we will let the cache take care of cleaning up
	    // while the application is running
	    qfs->deref();
	}

	// dirty unicode also if the font is dirty
	qfs = x11data.fontstruct[QFont::Unicode];
	x11data.fontstruct[QFont::Unicode] = 0;

	if (qfs && qfs != (QFontStruct *) -1) {
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

    QFontStruct *qfs = 0;
    QCString fontname;
    QTextCodec *codec = 0;
    XFontStruct *xfs = 0;

#ifndef QT_NO_XFTFREETYPE
    XftFont *xftfs = 0;
    XftPattern *xftmatch = 0;
#endif // QT_NO_XFTFREETYPE

    QString k(key() + script_table[script].list[script_table[script].index]);
    k += "/scr" + QString::number( x11Screen );
    if ( paintdevice )
	k += "/res" + QString::number(QPaintDeviceMetrics( paintdevice ).logicalDpiY());
    else
	k += "/res" + QString::number( QPaintDevice::x11AppDpiY( x11Screen ) );

    // Look for font name in fontNameDict based on QFont::key()
    QXFontName *qxfn = fontNameDict->find(k);
    double scale = -1.;
    if (! qxfn) {
	// if we don't find the name in the dict, we need to find a font name

#ifdef QFONTLOADER_DEBUG_VERBOSE
	qDebug("QFontLoader: getting font name for family %s", request.family.latin1());
#endif

	QString name;
	bool match;
	bool use_core = TRUE;

#ifndef QT_NO_XFTFREETYPE
	if (qt_has_xft && ! (request.styleStrategy & QFont::PreferBitmap) &&
            ! request.rawMode ) {
	    xftmatch = findXftFont(sample, &match, &scale);

	    if (xftmatch) {
		use_core = FALSE;

		char fn[1024];
		XftNameUnparse(xftmatch, fn, sizeof(fn));
		fn[1023] = '\0'; // just in case

		name = fn;
	    }
	}
#endif // QT_NO_XFTFREETYPE

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
	    x11data.fontstruct[script] = (QFontStruct *) -1;
	    initFontInfo(script, scale);
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

	if (qfs != (QFontStruct *) -1) {
	    qfs->ref();
	    initFontInfo(script, scale);
	}

	request.dirty = FALSE;

	return;
    }

#ifndef QT_NO_XFTFREETYPE
    // with XftFreeType support - we always load a font using Unicode, so we never
    // need a codec

    if (! qxfn->useCore) {
	if (! xftmatch) {
	    XftResult res;
	    xftmatch = XftNameParse(qxfn->name.data());
	    xftmatch = XftFontMatch(QPaintDevice::x11AppDisplay(),
				    x11Screen, xftmatch, &res);
	}

#ifdef QFONTLOADER_DEBUG
	qDebug("QFontLoader: loading xft font '%s'", fontname.data());
#endif

	// We pass a duplicate to XftFontOpenPattern because either xft font
	// will own the pattern after the call or the pattern will be
	// destroyed.
	XftPattern *dup = XftPatternDuplicate( xftmatch );
	xftfs = XftFontOpenPattern(QPaintDevice::x11AppDisplay(), dup);
    } else if ( xftmatch ) {
	qFatal( "this should not happen" );
    }
#endif // QT_NO_XFTFREETYPE

    if (qxfn->useCore) {
	// if we have no way to map this script, we give up
	if (! script_table[script].list[script_table[script].index]) {

#ifdef QFONTLOADER_DEBUG_VERBOSE
	    qDebug("QFontLoader: no nothing about script %d, giving up", script);
#endif

	    x11data.fontstruct[script] = (QFontStruct *) -1;
	    initFontInfo(script, scale);
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

		x11data.fontstruct[script] = (QFontStruct *) -1;
		initFontInfo(script, scale);
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
		x11data.fontstruct[script] = (QFontStruct *) -1;
		initFontInfo(script, scale);
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
#ifndef QT_NO_XFTFREETYPE
    else if (xftfs) {
	cost = ((xftfs->ascent + xftfs->descent) *
		(xftfs->max_advance_width * 256 / 8));
    }
#endif // QT_NO_XFTFREETYPE
    else {
	// couldn't load the font...
	x11data.fontstruct[script] = (QFontStruct *) -1;
	initFontInfo(script, scale);
	fontCache->insert(k, x11data.fontstruct[script], 1);
	return;
    }

    qfs = new QFontStruct((Qt::HANDLE) xfs,
#ifndef QT_NO_XFTFREETYPE
			  (Qt::HANDLE) xftfs, (Qt::HANDLE) xftmatch,
#else
			  0, 0,
#endif // QT_NO_XFTFREETYPE
			  fontname, codec, cost);
    x11data.fontstruct[script] = qfs;

    initFontInfo(script, scale);
    request.dirty = FALSE;

    // Insert font into the font cache and font dict
    if ( !fontCache->insert(k, qfs, qfs->cache_cost) )
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
	(void) new QFontJis0208Codec;
	(void) new QFontKsc5601Codec;
	(void) new QFontGb2312Codec;
	(void) new QFontGbkCodec;
	(void) new QFontGb18030_0Codec;
	(void) new QFontBig5Codec;
	(void) new QFontBig5hkscsCodec;
	(void) new QFontArabic68Codec;
	codecs_once = TRUE;
    }
#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS

#ifndef QT_NO_XFTFREETYPE
    qt_has_xft = FALSE;

    if (qt_use_xrender &&
	XftInit(0) && XftInitFtLibrary()) {
	QSettings settings;
	qt_has_xft = settings.readBoolEntry( "/qt/enableXft", TRUE );
	qt_use_antialiasing = QSettings().readBoolEntry( "/qt/useXft", TRUE );
    }
#endif // QT_NO_XFTFREETYPE

    QTextCodec *codec = QTextCodec::codecForLocale();
    // we have a codec for the locale - lets see if it's one of the CJK codecs,
    // and change the script_table[Han].list to an appropriate list
    if (codec) {
	switch (codec->mibEnum()) {
	case 2025: // GB2312
	case 57: // gb2312.1980-0
	case 113: // GBK
	case -113: // gbk-0
	    script_table[QFont::Han].list = hancn_encodings;
	    break;

	case 114: // GB18030
	case -114: // gb18030-0
	    script_table[QFont::Han].list = hancngb18030_encodings;
	    script_table[QFont::Mongolian].list = hancngb18030_encodings;
	    script_table[QFont::Tibetan].list = hancngb18030_encodings;
	    script_table[QFont::Yi].list = hancngb18030_encodings;
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

    time_t ttmp = time(NULL);
    struct tm *tt = 0;
    char samp[64];
    QString sample;

    if (ttmp != -1 && (tt = localtime(&ttmp)) != 0 &&
	strftime(samp, 64, "%A%B", tt) > 0) {

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
    d->load(QFontPrivate::defaultScript);

    // find the first font id and return that
    for (int i = 0; i < QFont::NScripts; i++) {
	QFontStruct *qfs = d->x11data.fontstruct[i];

	if (qfs) {
	    d->load((QFont::Script) i);

	    qfs = d->x11data.fontstruct[i];

	    if (qfs && qfs != (QFontStruct *) -1) {
		if (qfs->xfthandle)
		    return qfs->xfthandle;
		return ((XFontStruct *) qfs->handle)->fid;
	    }
	}
    }

    // no font ids in the font, so we return an invalid handle
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

    QFontStruct *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (QFontStruct *) -1) {
	return QString::null;
    }

    return QString::fromLatin1(qfs->name);
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

    QFontStruct *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (QFontStruct *) -1)
	return d->actual.pixelSize * 3 / 4;

#ifndef QT_NO_XFTFREETYPE
    XftFont *xftfs = (XftFont *) qfs->xfthandle;
    if (xftfs)
	return (int) (xftfs->ascent * qfs->scale);
#endif // QT_NO_XFTFREETYPE

    XFontStruct *f = (XFontStruct *) qfs->handle;
    return (int) (QMIN(f->ascent, f->max_bounds.ascent) * qfs->scale);
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

    QFontStruct *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (QFontStruct *) -1)
	return 0;

#ifndef QT_NO_XFTFREETYPE
    XftFont *xftfs = (XftFont *) qfs->xfthandle;
    if (xftfs)
	return (int) ((xftfs->descent - 1)*qfs->scale);
#endif // QT_NO_XFTFREETYPE

    XFontStruct *f = (XFontStruct *) qfs->handle;
    return (int) ((QMIN(f->descent, f->max_bounds.descent) - 1)*qfs->scale);
}


/*!
    Returns TRUE if character \a ch is a valid character in the font;
    otherwise returns FALSE.
*/
bool QFontMetrics::inFont(QChar ch) const
{
    return d->inFont( ch );
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
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch, d );

    if (script == QFont::UnknownScript)
	return 0;

    d->load(script);

    QFontStruct *qfs = d->x11data.fontstruct[script];
    XCharStruct *xcs = getCharStruct(qfs, QString(ch), 0);
    if (! xcs || xcs == (XCharStruct *) -1)
	return 0;
    return (int) (xcs->lbearing * qfs->scale);
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
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch, d );

    if (script == QFont::UnknownScript)
	return 0;

    d->load(script);

    QFontStruct *qfs = d->x11data.fontstruct[script];
    XCharStruct *xcs = getCharStruct(qfs, QString(ch), 0);
    if (! xcs || xcs == (XCharStruct *) -1)
	return 0;
    return (int) (xcs->width - xcs->rbearing * qfs->scale);
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
    if ( d->actual.lbearing == SHRT_MIN ) {
	d->load(QFontPrivate::defaultScript);

	QFontStruct *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
	if (! qfs || qfs == (QFontStruct *) -1)
	    return 0;

	XFontStruct *f = (XFontStruct *) qfs->handle;
	if (! f)
	    return 0;

	if ( f->per_char ) {
	    XCharStruct *cs = f->per_char;
	    int nc = maxIndex(f) + 1;
	    int mx = cs->lbearing;

	    for (int c = 1; c < nc; c++) {
		int nmx = cs[c].lbearing;

		if (nmx < mx)
		    mx = nmx;
	    }

	    d->actual.lbearing = (int) (mx * qfs->scale);
	} else
	    d->actual.lbearing = f->min_bounds.lbearing;
    }

    return d->actual.lbearing;
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
    if ( d->actual.rbearing == SHRT_MIN ) {
	d->load(QFontPrivate::defaultScript);

	QFontStruct *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
	if (! qfs || qfs == (QFontStruct *) -1)
	    return 0;

	XFontStruct *f = (XFontStruct *) qfs->handle;
	if (! f)
	    return 0;

	if ( f->per_char ) {
	    XCharStruct *c = f->per_char;
	    int nc = maxIndex(f) + 1;
	    int mx = c->width - c->rbearing;

	    for (int i = 1; i < nc; i++) {
		int nmx = c[i].width - c[i].rbearing;

		if (nmx < mx)
		    mx = nmx;
	    }

	    d->actual.rbearing = (int) (mx * qfs->scale);
	} else
	    d->actual.rbearing = f->max_bounds.width - f->max_bounds.rbearing;
    }

    return d->actual.rbearing;
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

    QFontStruct *qfs =  d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (QFontStruct *) -1)
	return (d->actual.pixelSize * 3 / 4) + 1;

#ifndef QT_NO_XFTFREETYPE
    XftFont *xftfs = (XftFont *) qfs->xfthandle;
    if (xftfs)
	return (int) ((xftfs->ascent + xftfs->descent)*qfs->scale);
#endif // QT_NO_XFTFREETYPE

    XFontStruct *f = (XFontStruct *) qfs->handle;
    return (int) ((QMIN(f->ascent, f->max_bounds.ascent)
		   + QMIN(f->descent, f->max_bounds.descent)) * qfs->scale);
}


/*!
    Returns the leading of the font.

    This is the natural inter-line spacing.

    \sa height(), lineSpacing()
*/
int QFontMetrics::leading() const
{
    d->load(QFontPrivate::defaultScript);

    QFontStruct *qfs =  d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (QFontStruct *) -1)
	return 0;

    XFontStruct *f = (XFontStruct *) qfs->handle;
    int l;

#ifndef QT_NO_XFTFREETYPE
    XftFont *xftfs = (XftFont *) qfs->xfthandle;
    if (xftfs)
	l = (int) QMIN( xftfs->height - (xftfs->ascent + xftfs->descent),
			((xftfs->ascent + xftfs->descent) >> 4)*qfs->scale );
    else
#endif // QT_NO_XFTFREETYPE
	l = qRound((QMIN(f->ascent, f->max_bounds.ascent)
		     + QMIN(f->descent, f->max_bounds.descent)) * qfs->scale *0.15 );

    return (l > 0) ? l : 1;
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
    int w = 0;
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch, d );

    if (script == QFont::UnknownScript)
	return d->actual.pixelSize * 3 / 4;

    d->load(script);

    QFontStruct *qfs = d->x11data.fontstruct[script];

    if ( script == QFont::LatinBasic )
	w = d->x11data.latinCache[ch.unicode()];

    if ( !w ) {
	if ( ch.combiningClass() > 0 )
	    return 0;

#ifndef QT_NO_XFTFREETYPE
	XGlyphInfo *xgi = getGlyphInfo(qfs, ch);
	if (xgi != (XGlyphInfo *) -2) {
	    if (xgi == (XGlyphInfo *) -1)
		w = d->actual.pixelSize * 3 / 4;
	    else if (! xgi)
		w = 0;
	    else
		w = xgi->xOff;
	} else
#endif // QT_NO_XFTFREETYPE
	{
	    XCharStruct *xcs = getCharStruct(qfs, QString(ch), 0);
	    if (xcs == (XCharStruct *) -1)
		w = d->actual.pixelSize * 3 / 4;
	    else if (! xcs)
		w = 0;
	    else
		w = xcs->width;
	}
	if ( script == QFont::LatinBasic && w < 0x100 )
	    d->x11data.latinCache[ch.unicode()] = (uchar)w;
    }

    if ( qfs && qfs != (QFontStruct *)-1 && w )
	w = (int)( w * qfs->scale );

    return w;
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
    int w = 0;

    const QChar &ch = str.unicode()[pos];
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch, d );

    if (script == QFont::UnknownScript)
	return d->actual.pixelSize * 3 / 4;

    d->load(script);

    QFontStruct *qfs = d->x11data.fontstruct[script];

    if ( script == QFont::LatinBasic )
	w = d->x11data.latinCache[ch.unicode()];

    if ( !w ) {
	if ( ch.combiningClass() > 0 )
	    return 0;

#ifndef QT_NO_XFTFREETYPE
	XGlyphInfo *xgi = getGlyphInfo(qfs, str, pos);
	if (xgi != (XGlyphInfo *) -2) {
	    if (xgi == (XGlyphInfo *) -1)
		w = d->actual.pixelSize * 3 / 4;
	    else if (! xgi)
		w = 0;
	    else
		w = xgi->xOff;
	} else
#endif // QT_NO_XFTFREETYPE
	{
	    XCharStruct *xcs = getCharStruct(qfs, str, pos);
	    if (xcs == (XCharStruct *) -1)
		w = d->actual.pixelSize * 3 / 4;
	    else if (! xcs)
		w = 0;
	    else
		w = xcs->width;
	}
	if ( script == QFont::LatinBasic && w < 0x100 )
	    d->x11data.latinCache[ch.unicode()] = (uchar)w;
    }

    if ( qfs && qfs != (QFontStruct *)-1 && w )
	w = (int)( w * qfs->scale );

    return w;
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

    return d->textWidth( shaped, 0, len );
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

    // this algorithm is similar to width(const QString &, int)
    bool simple = str.simpleText();
    QString shaped = simple ? str : QComplexText::shapedString( str, 0, len, QPainter::Auto, this);
    if ( !simple )
	len = shaped.length();

    QCharStruct overall;

    d->textExtents( shaped, 0, len, &overall );

    bool underline;
    bool strikeOut;
    int startX = (int)overall.lbearing;
    int width = (int)(overall.rbearing - startX);
    int ascent = (int)overall.ascent;
    int descent = (int)overall.descent;

    if ( painter ) {
	underline = painter->cfont.underline();
	strikeOut = painter->cfont.strikeOut();
    } else {
	underline = underlineFlag();
	strikeOut = strikeOutFlag();
    }

    if ( underline || strikeOut ) {
	if ( startX > 0 )
	    startX = 0;

	if ( overall.rbearing < overall.width )
	    width =  (int)(overall.width - startX);
	else
	    width =  (int)(overall.rbearing - startX);

	if ( underline && len != 0 ) {
	    int ulTop = underlinePos();
	    int ulBot = ulTop + lineWidth(); // X descent is 1

	    if ( descent < ulBot )	// more than logical descent, so don't
		descent = ulBot;	// subtract 1 here!

	    if ( ascent < -ulTop )
		ascent = -ulTop;
	}

	if ( strikeOut && len != 0 ) {
	    int soTop = strikeOutPos();
	    int soBot = soTop - lineWidth(); // same as above

	    if ( descent < -soBot )
		descent = -soBot;

	    if ( ascent < soTop )
		ascent = soTop;
	}
    }

    return QRect(startX, -ascent, width, descent + ascent);
}


/*!
    Returns the width of the widest character in the font.
*/
int QFontMetrics::maxWidth() const
{
    QFontStruct *qfs;
    int w = 0, ww;

    for (int i = 0; i < QFont::LastPrivateScript - 1; i++) {
	if (! d->x11data.fontstruct[i])
	    continue;

	d->load((QFont::Script) i);

	qfs = d->x11data.fontstruct[i];
	if (! qfs || qfs == (QFontStruct *) -1)
	    continue;

#ifndef QT_NO_XFTFREETYPE
	if (d->x11data.fontstruct[i]->xfthandle) {
	    XftFont *xftfs =
		(XftFont *) qfs->xfthandle;
	    ww = (int) (xftfs->max_advance_width * qfs->scale);
	} else
#endif // QT_NO_XFTFREETYPE
	    {
		XFontStruct *f =
		    (XFontStruct *) qfs->handle;
		ww = (int) (f->max_bounds.width * qfs->scale);
	    }

	if (ww > w)
	    w = ww;
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
