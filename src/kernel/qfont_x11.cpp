/****************************************************************************
** $Id: $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for X11
**
** Created : 940515
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
Bool XftInitFtLibrary(void);
Bool XftNameUnparse (XftPattern *, char *, int);
}
#endif


// #define QFONTLOADER_DEBUG
// #define QFONTLOADER_DEBUG_VERBOSE


// to get which font encodings are installed:
// xlsfonts | egrep -- "^-.*-.*-.*$" | cut -f 14- -d- | sort | uniq

// because of the way the font matcher works
// these lists must have at least 2 entries
static const char * const empty_encodings[] = { 0, 0 };
static const char * const latin_encodings[] = { "iso8859-1", 0 };
static const char * const greek_encodings[] = { "iso8859-7", 0 };
static const char * const cyrillic_encodings[] = { "iso8859-5", "koi8-r", "koi8-ru", 0 };
static const char * const hebrew_encodings[] = { "iso8859-8", 0 };
static const char * const arabic_encodings[] = { "iso8859-6.8x", 0 };
static const char * const tamil_encodings[] = { "tscii-*", 0 };
static const char * const thai_encodings[] = { "tis620*-0", "iso8859-11", 0 };
static const char * const hiragana_encodings[] = { "jisx0208.1983-0", 0 };
static const char * const katakana_encodings[] = { "jisx0208.1983-0", 0 };
static const char * const hangul_encodings[] = { "ksc5601.1987-0", 0 };
static const char * const bopomofo_encodings[] = { "gb2312.1980-0", 0 };
static const char * const unicode_encodings[] = { "iso10646-1", "unicode-*", 0 };
static const char * const hanx11_encodings[] = { "big5*-*", 0 };
static const char * const latinA2_encodings[] = { "iso8859-2", 0 };
static const char * const latinA3_encodings[] = { "iso8859-3", 0 };
static const char * const latinA4_encodings[] = { "iso8859-4", 0 };
static const char * const latinA14_encodings[] = { "iso8859-14", 0 };
static const char * const latinA15_encodings[] = { "iso8859-15", 0 };

// we select on of these at initialization time for Han use
static const char * const hancn_encodings[] =
    { "gb2312.1980-0", "big5*-*", "jisx0208.1983-0", "ksc5601.1987-0", 0 };
static const char * const hanjp_encodings[] =
    { "jisx0208.1983-0", "gb2312.1980-0", "big5*-*", "ksc5601.1987-0", 0 };
static const char * const hankr_encodings[] =
    { "ksc5601.1987-0", "jisx0208.1983-0", "gb2312.1980-0", "big5*-*", 0 };
static const char * const hantw_encodings[] =
    { "big5*-*", "gb2312.1980-0", "jisx0208.1983-0", "ksc5601.1987-0", 0 };

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

// Data needed for XftFreeType support
#ifndef QT_NO_XFTFREETYPE
typedef QIntDict<QPixmap> QPixmapDict;
static QPixmapDict *qt_xft_render_sources = 0;
QCleanupHandler<QPixmapDict> cleanup_pixmapdict;
#endif // QT_NO_XFTFREETYPE


static inline float pixelSize( const QFontDef &request, QPaintDevice *paintdevice )
{
    float pSize;
    if ( request.pointSize != -1 ) {
	if ( paintdevice )
	    pSize = request.pointSize *
		    QPaintDeviceMetrics( paintdevice ).logicalDpiY() / 720.;
	else
	    pSize = request.pointSize * QPaintDevice::x11AppDpiY() / 720.;
    } else {
	pSize = request.pixelSize;
    }
    return pSize;

}

static inline float pointSize( const QFontDef &fd, QPaintDevice *paintdevice )
{
    float pSize;
    if ( fd.pointSize == -1 ) {
	if ( paintdevice )
	    pSize = fd.pixelSize * 10 *
		    QPaintDeviceMetrics( paintdevice ).logicalDpiY() / 72.;
	else
	    pSize = fd.pixelSize * 10 * QPaintDevice::x11AppDpiY() / 72.;
    } else {
	pSize = fd.pointSize;
    }
    return pSize;
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
	XftFreeTypeClose(QPaintDevice::x11AppDisplay(), (XftFontStruct *) xfthandle);
	xfthandle = 0;
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
// This is used if even the last resort family is not available.  It
// returns \e something, almost no matter what.
// The current implementation tries a wide variety of common fonts,
// returning the first one it finds.  The implementation may change at
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

    fontNames = getXFontNames( fontName.ascii(), &count );

    XFreeFontNames( fontNames );

    return count != 0;
}


// Splits an X font name into fields separated by '-'
bool QFontPrivate::parseXFontName(const QCString &fontName, char **tokens)
{
    if (fontName.isEmpty() || fontName[0] != '-') {
	tokens[0] = 0;
	return FALSE;
    }

    int	  i;
    char *f = fontName.data() + 1;
    for (i = 0; i < NFontFields && f && f[0]; i++) {
	tokens[i] = f;
	f = strchr( f, '-' );

	if (f) *f++ = '\0';
    }

    if (i < NFontFields) {
	for(int j = i ; j < NFontFields; j++)
	    tokens[j] = 0;

	return FALSE;
    }

    return TRUE;
}


// Fills in a font definition (QFontDef) from an XLFD (X Logical Font Description).
// Returns TRUE if the the given xlfd is valid. If the xlfd is valid the encoding
// name (charset registry + "-" + charset encoding) is returned in encodingName if
// encodingName is non-zero. The fileds lbearing and rbearing are not given any values.
bool QFontPrivate::fillFontDef( const QCString &xlfd, QFontDef *fd,
				QCString *encodingName )
{
    char *tokens[QFontPrivate::NFontFields];
    QCString buffer = xlfd.copy();
    if ( ! parseXFontName(buffer, tokens) )
	return FALSE;

    if ( encodingName ) {
	*encodingName = tokens[QFontPrivate::CharsetRegistry];
	*encodingName += '-';
	*encodingName += tokens[QFontPrivate::CharsetEncoding];
    }

    fd->family = QString::fromLatin1(tokens[Family]);
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
    if ( r && fd->pixelSize && QPaintDevice::x11AppDpiY() &&
	 r != QPaintDevice::x11AppDpiY() ) {
	// calculate actual pointsize for display DPI
	fd->pointSize = (int) ((fd->pixelSize * 720.) /
			       QPaintDevice::x11AppDpiY() + 0.5);
    } else if ( fd->pixelSize == 0 && fd->pointSize ) {
	// calculate pixel size from pointsize/dpi
	fd->pixelSize = ( fd->pointSize * QPaintDevice::x11AppDpiY() ) / 720;
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
    return (xcs == (XCharStruct *) -1 ||
	    (xcs && xcs->width == 0 && xcs->ascent + xcs->descent == 0));
}


// return the XCharStruct for the specified cell in the single dimension font xfs
static inline XCharStruct *getCharStruct1d(XFontStruct *xfs, uint c)
{
    XCharStruct *xcs = (XCharStruct *) -1;
    if (c >= xfs->min_char_or_byte2 &&
	c <= xfs->max_char_or_byte2) {
	if (xfs->per_char != NULL) {
	    xcs = &(xfs->per_char[(c - xfs->min_char_or_byte2)]);
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
	    xcs = &(xfs->per_char[((r - xfs->min_byte1) *
				   (xfs->max_char_or_byte2 -
				    xfs->min_char_or_byte2 + 1)) +
				  (c - xfs->min_char_or_byte2)]);
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
    QChar ch;

    if (! qfs || qfs == (QFontStruct *) -1 ||
    	! (xfs = (XFontStruct *) qfs->handle)) {
	xcs = (XCharStruct *) -1;
	goto end;
    }

    if (qfs->codec)
	ch = QChar(qfs->codec->characterFromUnicode(str, pos));
    else
	ch = QComplexText::shapedCharacter(str, pos);

    if (ch.unicode() == 0) {
	xcs = 0;
	goto end;
    }

    if (! xfs->max_byte1)
	// single row font
	xcs = getCharStruct1d(xfs, ch.cell());
    else
	xcs = getCharStruct2d(xfs, ch.row(), ch.cell());

 end:
    return xcs;
}


#ifndef QT_NO_XFTFREETYPE

// returns a XGlyphInfo for the character as pos in str.  this function can return:
//   -2 (meaning this fontstruct doesn't contain an xft font)
//   -1 (meaning the glyph doesn't exist in the font)
//    0 (meaning a zero width character)
//    a valid XGlyphInfo
static inline XGlyphInfo *getGlyphInfo(QFontStruct *qfs, const QString &str, int pos)
{
    XftFontStruct *xftfs;
    XGlyphInfo *xgi;
    QChar ch;
    unsigned int missing[1];
    int nmissing = 0;

    if (! qfs || qfs == (QFontStruct *) -1 ||
	! (xftfs = (XftFontStruct *) qfs->xfthandle)) {
	xgi = (XGlyphInfo *) -2;
	goto end;
    }

    // no need for codec, all Xft fonts are in unicode mapping
    ch = QComplexText::shapedCharacter(str, pos);

    if (ch.unicode() == 0) {
	xgi = 0;
	goto end;
    }

    // load the glyph if it's not in the font
    XftGlyphCheck(QPaintDevice::x11AppDisplay(), xftfs, ch.unicode(),
		  missing, &nmissing);
    if (nmissing)
	XftGlyphLoad(QPaintDevice::x11AppDisplay(), xftfs, missing, nmissing);

    if (ch.unicode() > xftfs->nrealized) {
	xgi = (XGlyphInfo *) -1;
	goto end;
    }
    xgi = xftfs->realized[ch.unicode()];
    if (! xgi)
	xgi = (XGlyphInfo *) -1;

 end:
    return xgi;
}


// ditto
static inline XGlyphInfo *getGlyphInfo(QFontStruct *qfs, const QChar &ch)
{
    XftFontStruct *xftfs;
    XGlyphInfo *xgi;
    unsigned int missing[1];
    int nmissing = 0;

    if (! qfs || qfs == (QFontStruct *) -1 ||
	! (xftfs = (XftFontStruct *) qfs->xfthandle)) {
	xgi = (XGlyphInfo *) -2;
	goto end;
    }

    // load the glyph if it's not in the font
    XftGlyphCheck(QPaintDevice::x11AppDisplay(), xftfs, ch.unicode(),
		  missing, &nmissing);
    if (nmissing)
	XftGlyphLoad(QPaintDevice::x11AppDisplay(), xftfs, missing, nmissing);

    if (ch.unicode() > xftfs->nrealized) {
	xgi = (XGlyphInfo *) -1;
	goto end;
    }
    xgi = xftfs->realized[ch.unicode()];
    if (! xgi)
	xgi = (XGlyphInfo *) -1;

 end:
    return xgi;
}

#endif // QT_NO_XFTFREETYPE


// comment me
QRect QFontPrivate::boundingRect( const QChar &ch )
{
    QFont::Script script = scriptForChar( ch );

    QFontStruct *qfs = 0;
    XCharStruct *xcs = 0;
    QRect r;

    if (script != QFont::UnknownScript) {
	load(script);
	qfs = x11data.fontstruct[script];
    }

#ifndef QT_NO_XFTFREETYPE
    XGlyphInfo *xgi = 0;
    if ((xgi = getGlyphInfo(qfs, ch)) != (XGlyphInfo *) -2) {
	if ( xgi == (XGlyphInfo *) -1)
	    r.setRect( 0, actual.pixelSize * -3 / 4,
		       actual.pixelSize * 3 / 4, actual.pixelSize * 3 / 4);
	else if ( xgi )
	    r.setRect( -xgi->x, -xgi->y, xgi->width, xgi->height);
    } else
#endif // QT_NO_XFTFREETYPE

	{
	    xcs = getCharStruct(qfs, QString(ch), 0);
	    if ( xcs == (XCharStruct *) -1)
		r.setRect( 0, actual.pixelSize * -3 / 4,
			   actual.pixelSize * 3 / 4, actual.pixelSize * 3 / 4);
	    else if ( xcs )
		r.setRect( xcs->lbearing, -xcs->ascent,
			   xcs->rbearing - xcs->lbearing, xcs->descent + xcs->ascent );
	}

    return r;
}


// returns the width of the string in pixels (replaces XTextWidth)
int QFontPrivate::textWidth( const QString &str, int pos, int len )
{
    const QChar *chars = str.unicode() + pos;
    QFont::Script current = QFont::NoScript, tmp;
    int i;
    float w = 0;

    QFontStruct *qfs = 0;
    XCharStruct *xcs = 0;

#ifndef QT_NO_XFTFREETYPE
    XGlyphInfo *xgi = 0;
#endif // QT_NO_XFTFREETYPE

    for (i = 0; i < len; i++) {
	if (chars->combiningClass() == 0 || pos + i == 0) {
	    tmp = scriptForChar(*chars);

	    if (tmp != current) {
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
		    // character isn't in the font, set the script to UnknownScript
		    tmp = current = QFont::UnknownScript;
		    w += actual.pixelSize * 3 / 4;
		} else if (xgi)
		    w += xgi->xOff;
	    } else
#endif // QT_NO_XFTFREETYPE
		{
		    xcs = getCharStruct(qfs, str, pos + i);
		    if (xcs == (XCharStruct *) -1) {
			// character isn't in the font, set the script to UnknownScript
			tmp = current = QFont::UnknownScript;
			w += actual.pixelSize * 3 / 4;
		    } else if (xcs)
			w += xcs->width * qfs->scale;
		}
	}

	chars++;
    }

    return (int)w;
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
	if (chars->combiningClass() == 0 || pos + i == 0) {
	    tmp = scriptForChar(*chars);

	    if (tmp != current ||
		// X11 doesn't draw strings wider than 32768px
		// we use a smaller value here, since no screen is wider than
		// 4000 px, so we can optimise drawing
		w > pw + 4096 ||

		// RENDER has problems drawing strings longer than 250 chars (253
		// seems to be the length that breaks).  Split the cache every
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
		tmp = scriptForChar(*chars);
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
	if (chars->combiningClass() == 0 || pos + i == 0) {
	    tmp = scriptForChar(*chars);

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
		    overall->ascent = QMAX(overall->ascent, xgi->y);
		    overall->descent = QMAX(overall->descent, xgi->height - xgi->y);
		    overall->lbearing = QMIN(overall->lbearing, xgi->x);
		    overall->rbearing = QMAX(overall->rbearing, overall->width +
					     (xgi->width - xgi->x));
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
		tmp = scriptForChar(*chars);
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
	XftFontStruct *xftfs = 0;
#endif // QT_NO_XFTFREETYPE

    	if ( cache->script < QFont::LastPrivateScript &&
	     cache->length > 0 &&
	     qfs && qfs != (QFontStruct *) -1 ) {
	    xfs = (XFontStruct *) qfs->handle;

#ifndef QT_NO_XFTFREETYPE
	    xftfs = (XftFontStruct *) qfs->xfthandle;
#endif // QT_NO_XFTFREETYPE

	}
	// clip away invisible parts. Saves some drawing operations.
	if ( x + cache->xoff < pdWidth && x + cache->xoff + cache->x2off > 0 ) {

#ifndef QT_NO_XFTFREETYPE
	if (xftfs) {
	    QPixmap *pm = qt_xft_render_sources->find(screen);
	    if (! pm) {
		pm = new QPixmap(1, 1);
		// fix the render Picture to tile the source
		XRenderPictureAttributes pattr;
		pattr.repeat = TRUE;
		XRenderChangePicture(dpy, pm->x11RenderHandle(), CPRepeat, &pattr);

		qt_xft_render_sources->insert(screen, pm);
	    }

	    // fill pixmap with pen color
	    pm->fill(pen);

	    if (bgmode != Qt::TransparentMode) {
		XRenderColor col;
		col.red = bgcolor.red() | bgcolor.red() << 8;
		col.green = bgcolor.green() | bgcolor.green() << 8;
		col.blue = bgcolor.blue() | bgcolor.blue() << 8;
		col.alpha = 0xffff;
		XRenderFillRectangle(dpy, PictOpSrc, rendhd, &col,
				     x + cache->xoff,  y - xftfs->ascent,
				     cache->x2off - cache->xoff,
				     xftfs->ascent + xftfs->descent);
	    }

	    XftRenderString16(dpy, pm->x11RenderHandle(), xftfs, rendhd, 0, 0,
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
    QFont::Script script = scriptForChar( ch );

    if (script == QFont::UnknownScript)
	return FALSE;

    load(script);

    QFontStruct *qfs = x11data.fontstruct[script];

#ifndef QT_NO_XFTFREETYPE
    if (qfs && qfs != (QFontStruct *) -1 && qfs->xfthandle)
	return XftFreeTypeGlyphExists(QPaintDevice::x11AppDisplay(),
				      (XftFontStruct *) qfs->xfthandle, ch.unicode());
#endif // QT_NO_XFTFREETYPE

    XCharStruct *xcs = getCharStruct(qfs, QString(ch), 0);
    return (! charNonExistent(xcs));
}


#ifndef QT_NO_XFTFREETYPE

// returns an XftPattern for the font or zero if no found supporting the script could
// be found
XftPattern *QFontPrivate::findXftFont(const QChar &sample, bool *exact) const
{
    // look for foundry/family
    QString familyName;
    QString foundryName;
    QFontDatabase::parseFontName(request.family, foundryName, familyName);

    XftPattern *match = bestXftPattern(familyName, foundryName);

    if (sample.unicode() != 0 && match) {
	// check if the character is actually in the font - this does result in
	// a font being loaded, but since Xft is completely client side, we can
	// do this efficiently
	XftFontStruct *xftfs = XftFreeTypeOpen(QPaintDevice::x11AppDisplay(),
					       match);

	if (xftfs) {
	    if (! XftFreeTypeGlyphExists(QPaintDevice::x11AppDisplay(), xftfs,
					 sample.unicode())) {
		XftPatternDestroy(match);
		match = 0;
	    }

	    XftFreeTypeClose(QPaintDevice::x11AppDisplay(), xftfs);
	}
    }

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
	    match = bestXftPattern(familyName, foundryName);

	    if (sample.unicode() != 0 && match) {
		// check if the character is actually in the font - this does result in
		// a font being loaded, but since Xft is completely client side, we can
		// do this efficiently
		XftFontStruct *xftfs = XftFreeTypeOpen(QPaintDevice::x11AppDisplay(),
						       match);

		if (xftfs) {
		    if (! XftFreeTypeGlyphExists(QPaintDevice::x11AppDisplay(), xftfs,
						 sample.unicode())) {
			XftPatternDestroy(match);
			match = 0;
		    }

		    XftFreeTypeClose(QPaintDevice::x11AppDisplay(), xftfs);
		}
	    }
	}
    }

    return match;
}

// finds an XftPattern best matching the familyname, foundryname and other
// requested pieces of the font
XftPattern *QFontPrivate::bestXftPattern(const QString &familyName,
					 const QString &foundryName) const
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
	  QPaintDevice::x11AppDpiY()) ) {
	size_value = pixelSize( request, paintdevice );
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

    if (mono_value >= XFT_MONO) {
	if (! foundryName.isNull())
	    pattern = XftPatternBuild(0,
				      XFT_ENCODING, XftTypeString, "iso10646-1",
				      XFT_FOUNDRY, XftTypeString, foundryName.latin1(),
				      XFT_FAMILY, XftTypeString, familyName.latin1(),
				      XFT_FAMILY, XftTypeString, generic_value.data(),
				      XFT_WEIGHT, XftTypeInteger, weight_value,
				      XFT_SLANT, XftTypeInteger, slant_value,
				      sizeFormat, XftTypeDouble, size_value,
				      XFT_SPACING, XftTypeInteger, mono_value,
				      0);
	else if (! familyName.isNull())
	    pattern = XftPatternBuild(0,
				      XFT_ENCODING, XftTypeString, "iso10646-1",
				      XFT_FAMILY, XftTypeString, familyName.latin1(),
				      XFT_FAMILY, XftTypeString, generic_value.data(),
				      XFT_WEIGHT, XftTypeInteger, weight_value,
				      XFT_SLANT, XftTypeInteger, slant_value,
				      sizeFormat, XftTypeDouble, size_value,
				      XFT_SPACING, XftTypeInteger, mono_value,
				      0);
	else
	    pattern = XftPatternBuild(0,
				      XFT_ENCODING, XftTypeString, "iso10646-1",
				      XFT_FAMILY, XftTypeString, generic_value.data(),
				      XFT_WEIGHT, XftTypeInteger, weight_value,
				      XFT_SLANT, XftTypeInteger, slant_value,
				      sizeFormat, XftTypeDouble, size_value,
				      XFT_SPACING, XftTypeInteger, mono_value,
				      0);
    } else {
	if (! foundryName.isNull())
	    pattern = XftPatternBuild(0,
				      XFT_ENCODING, XftTypeString, "iso10646-1",
				      XFT_FOUNDRY, XftTypeString, foundryName.latin1(),
				      XFT_FAMILY, XftTypeString, familyName.latin1(),
				      XFT_FAMILY, XftTypeString, generic_value.data(),
				      XFT_WEIGHT, XftTypeInteger, weight_value,
				      XFT_SLANT, XftTypeInteger, slant_value,
				      sizeFormat, XftTypeDouble, size_value,
				      0);
	else if (! familyName.isNull())
	    pattern = XftPatternBuild(0,
				      XFT_ENCODING, XftTypeString, "iso10646-1",
				      XFT_FAMILY, XftTypeString, familyName.latin1(),
				      XFT_FAMILY, XftTypeString, generic_value.data(),
				      XFT_WEIGHT, XftTypeInteger, weight_value,
				      XFT_SLANT, XftTypeInteger, slant_value,
				      sizeFormat, XftTypeDouble, size_value,
				      0);
	else
	    pattern = XftPatternBuild(0,
				      XFT_ENCODING, XftTypeString, "iso10646-1",
				      XFT_FAMILY, XftTypeString, generic_value.data(),
				      XFT_WEIGHT, XftTypeInteger, weight_value,
				      XFT_SLANT, XftTypeInteger, slant_value,
				      sizeFormat, XftTypeDouble, size_value,
				      0);
    }

    static bool useAA = QSettings().readBoolEntry("/qt/useXft");
    if ( !useAA || request.styleStrategy & ( QFont::PreferAntialias |
					     QFont::NoAntialias) ) {
	bool requestAA;
	if ( !useAA || request.styleStrategy & QFont::NoAntialias )
	    requestAA = FALSE;
	else
	    requestAA = TRUE;
	XftPatternAddBool( pattern, XFT_ANTIALIAS,requestAA );
    }

    if (pattern) {
	result = XftFontMatch(QPaintDevice::x11AppDisplay(),
			      QPaintDevice::x11AppScreen(), pattern, &res);
	XftPatternDestroy(pattern);
    }

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
QCString QFontPrivate::findFont(QFont::Script script, bool *exact) const
{
    QString familyName = request.family;

    // assume exact match
    *exact = TRUE;

    if ( familyName.isEmpty() ) {
	familyName = defaultFamily();
	*exact = FALSE;
    }

    QString foundryName;
    QFontDatabase::parseFontName(familyName, foundryName, familyName);

    QString addStyle = request.addStyle;
    if (addStyle.isEmpty())
	addStyle = "*";

    int score;

    QCString bestName;
    bool done = FALSE;
    int start_index = script_table[script].index;

    while (! done) {
	bestName = bestFamilyMember(script, foundryName, familyName, addStyle, &score);

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
					    addStyle, &score);

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
					addStyle, &score);

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
					addStyle, &score);

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
					addStyle, &score);

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
					int *score ) const
{
    const int prettyGoodScore = CJKPitchScore | SizeScore | WeightScore |
				SlantScore | WidthScore;

    int testScore = 0;
    QCString testResult;
    int bestScore = 0;
    QCString result;

    if ( !foundry.isEmpty() ) {
	QString pattern
	    = "-" + foundry + "-" + family + "-*-*-*-" + addStyle + "-*-*-*-*-*-*-" +
	    (script_table[script].list)[(script_table[script].index)];
	result = bestMatch(pattern.latin1(), &bestScore, script);
    }

    if ( bestScore < prettyGoodScore ) {
	QRegExp alt( "[,;]" );
	int alternator = 0;
	int next;
	int bias = 0;

	while ( alternator < (int)family.length() ) {
	    next = family.find( alt, alternator );

	    if ( next < alternator )
		next = family.length();

	    QString fam = family.mid( alternator, next-alternator );
	    QString pattern = "-*-" + fam + "-*-*-*-" + addStyle + "-*-*-*-*-*-*-" +
			      (script_table[script].list)[(script_table[script].index)];
	    testResult = bestMatch( pattern.latin1(), &testScore, script );
	    bestScore -= bias;

	    if ( testScore > bestScore ) {
		bestScore = testScore;
		result = testResult;
	    }

	    if ( family[next] == ';' )
		bias += 1;

	    alternator = next + 1;
	}
    }

    if ( score )
	*score = bestScore;

    return result;
}


struct QFontMatchData { // internal for bestMatch
    QFontMatchData()
    {
	score=0; name=0; pointDiff=99; weightDiff=99; smooth=FALSE;
    }

    int	    score;
    char   *name;
    float   pointDiff;
    int	    weightDiff;
    bool    smooth;
};

QCString QFontPrivate::bestMatch( const char *pattern, int *score,
				  QFont::Script script ) const
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

    xFontNames = getXFontNames( pattern, &fcount );

    for( i = 0; i < fcount; i++ ) {
	sc = fontMatchScore( xFontNames[i], matchBuffer,
			     &pointDiff, &weightDiff,
			     &scalable, &smoothScalable, script );

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

	if ( parseXFontName( matchBuffer, tokens ) ) {
	    int resx;
	    int resy;
	    int pSize;

	    if ( bestScalable.smooth ) {
		// X will scale the font accordingly
		resx = QPaintDevice::x11AppDpiX();
		resy = QPaintDevice::x11AppDpiY();
	    } else {
		resx = atoi(tokens[ResolutionX]);
		resy = atoi(tokens[ResolutionY]);
	    }
	    pSize = (int) (pixelSize( request, paintdevice ) + 0.5 );

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
	    best.name = bestName.data();
	    best.score = bestScalable.score;
	}
    }

    *score = best.score;
    bestName = best.name;

    XFreeFontNames( xFontNames );
    return bestName;
}


// Returns a score describing how well a font name matches the contents
// of a font.
int QFontPrivate::fontMatchScore( const char *fontName, QCString &buffer,
				   float *pixelSizeDiff, int  *weightDiff,
				   bool	 *scalable     , bool *smoothScalable,
				  QFont::Script script ) const
{
    char *tokens[NFontFields];
    bool   exactmatch = TRUE;
    int	   score      = NonUnicodeScore;
    *scalable	      = FALSE;
    *smoothScalable   = FALSE;
    *weightDiff	      = 0;
    *pixelSizeDiff    = 0;

    qstrcpy( buffer.data(), fontName );	// NOTE: buffer must be large enough
    if ( ! parseXFontName( buffer, tokens ) )
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
	    if ( request.styleStrategy & QFont::PreferMatch )
		score |= SizeScore;
	}
    } else {
	int pSize;
	float percentDiff;
	pSize = atoi(tokens[PixelSize]);

	int reqPSize = (int) (pixelSize( request, paintdevice ) + 0.5);

	if ( reqPSize != 0 ) {
	    diff = (float)QABS(pSize - reqPSize);
	    percentDiff = diff/reqPSize*100.0F;
	} else {
	    diff = (float)pSize;
	    percentDiff = 100;
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
void QFontPrivate::initFontInfo(QFont::Script script)
{
    if ( paintdevice &&
	 (QPaintDeviceMetrics( paintdevice ).logicalDpiY() !=
	  QPaintDevice::x11AppDpiY()) ) {
	// we have a printer font
	actual = request;
	float _pointSize = pointSize( actual, paintdevice );
	float _pixelSize = pixelSize( actual, paintdevice );
	if ( actual.pointSize == -1 )
	    actual.pointSize = (int)(_pointSize + 0.5);
	else
	    actual.pixelSize = (int) (_pixelSize + 0.5);

	QFontDef font;
	if ( fillFontDef(x11data.fontstruct[script]->name, &font, 0)) {
	    if ( font.pixelSize != 0 )
		x11data.fontstruct[script]->scale = _pixelSize/((float) font.pixelSize);
	    //qDebug("setting scale to %f requested pixel=%f got %d",
	    // x11data.fontstruct[script]->scale, _pixelSize, font.pixelSize);
	}
	return;
    }

    if (script != QFont::Unicode && script != defaultScript || !actual.dirty)
	return;

    actual.lbearing = SHRT_MIN;
    actual.rbearing = SHRT_MIN;

    if (exactMatch) {
	// ### this is not 100% correct for Xft, as the matched font could be
	// of a different family!
	actual = request;
	actual.dirty = FALSE;

	if ( actual.pointSize == -1 )
	    actual.pointSize = (int)(pointSize( actual, paintdevice ) +.5);
	else
	    actual.pixelSize = (int)(pixelSize( actual, paintdevice ) +.5);

	return;
    }

    if (! fillFontDef(x11data.fontstruct[script]->name, &actual, 0)) {
	// zero fontdef
	actual = QFontDef();

	actual.family = QString::fromLatin1(x11data.fontstruct[script]->name);
	actual.rawMode = TRUE;
	actual.pointSize = request.pointSize;
	actual.pixelSize = request.pixelSize;
	exactMatch = FALSE;

	if ( actual.pointSize == -1 )
	    actual.pointSize = (int)(pointSize( actual, paintdevice ) +.5);
	else
	    actual.pixelSize = (int)(pixelSize( actual, paintdevice ) +.5);
    }

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
    QChar ch;
    uchar row, cell;

    switch (script) {
    case QFont::Latin:                     row = 0x00; cell = 0x30; break;
    case QFont::Greek:                     row = 0x03; cell = 0x90; break;
    case QFont::Cyrillic:                  row = 0x04; cell = 0x10; break;
    case QFont::Armenian:                  row = 0x05; cell = 0x40; break;
    case QFont::Georgian:                  row = 0x10; cell = 0xa0; break;
    case QFont::Runic:                     row = 0x16; cell = 0xa0; break;
    case QFont::Ogham:                     row = 0x16; cell = 0x80; break;
    case QFont::CombiningMarks:            row = 0x03; cell = 0x00; break;

    case QFont::Hebrew:                    row = 0x05; cell = 0xd0; break;
    case QFont::Arabic:                    row = 0x06; cell = 0x30; break;
    case QFont::Syriac:                    row = 0x07; cell = 0x10; break;
    case QFont::Thaana:                    row = 0x07; cell = 0x80; break;

    case QFont::Devanagari:                row = 0x09; cell = 0x10; break;
    case QFont::Bengali:                   row = 0x09; cell = 0x90; break;
    case QFont::Gurmukhi:                  row = 0xa0; cell = 0x10; break;
    case QFont::Gujarati:                  row = 0x0a; cell = 0x90; break;
    case QFont::Oriya:                     row = 0x0b; cell = 0x10; break;
    case QFont::Tamil:                     row = 0x0b; cell = 0x90; break;
    case QFont::Telugu:                    row = 0x0c; cell = 0x10; break;
    case QFont::Kannada:                   row = 0x0c; cell = 0x90; break;
    case QFont::Malayalam:                 row = 0x0d; cell = 0x10; break;
    case QFont::Sinhala:                   row = 0x0d; cell = 0x90; break;
    case QFont::Thai:                      row = 0x0e; cell = 0x10; break;
    case QFont::Lao:                       row = 0xe0; cell = 0x81; break;
    case QFont::Tibetan:                   row = 0x0f; cell = 0x00; break;
    case QFont::Myanmar:                   row = 0x10; cell = 0x00; break;
    case QFont::Khmer:                     row = 0x17; cell = 0x80; break;

    case QFont::Han:                       row = 0x4e; cell = 0x00; break;
    case QFont::Hiragana:                  row = 0x30; cell = 0x50; break;
    case QFont::Katakana:                  row = 0x30; cell = 0xb0; break;
    case QFont::Hangul:                    row = 0xac; cell = 0x00; break;
    case QFont::Bopomofo:                  row = 0x31; cell = 0x10; break;
    case QFont::Yi:                        row = 0xa0; cell = 0x00; break;

    case QFont::Ethiopic:                  row = 0x12; cell = 0x00; break;
    case QFont::Cherokee:                  row = 0x13; cell = 0xa0; break;
    case QFont::CanadianAboriginal:        row = 0x14; cell = 0x10; break;
    case QFont::Mongolian:                 row = 0x18; cell = 0x00; break;

    case QFont::CurrencySymbols:           row = 0x20; cell = 0xa0; break;
    case QFont::LetterlikeSymbols:         row = 0x21; cell = 0x00; break;
    case QFont::NumberForms:               row = 0x21; cell = 0x60; break;
    case QFont::MathematicalOperators:     row = 0x22; cell = 0x2b; break;
    case QFont::TechnicalSymbols:          row = 0x24; cell = 0x40; break;
    case QFont::GeometricSymbols:          row = 0x25; cell = 0x00; break;
    case QFont::MiscellaneousSymbols:      row = 0x26; cell = 0x00; break;
    case QFont::EnclosedAndSquare:         row = 0x24; cell = 0x60; break;
    case QFont::Braille:                   row = 0x28; cell = 0x00; break;

    case QFont::LatinExtendedA_2:          row = 0x01; cell = 0x02; break;
    case QFont::LatinExtendedA_3:          row = 0x01; cell = 0x08; break;
    case QFont::LatinExtendedA_4:          row = 0x01; cell = 0x00; break;
    case QFont::LatinExtendedA_14:         row = 0x01; cell = 0x74; break;
    case QFont::LatinExtendedA_15:         row = 0x01; cell = 0x52; break;

    default:
 	row = cell = 0;
    }

    ch.setCell( cell );
    ch.setRow( row );
    return ch;
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

		if (xcs) {
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
	    hasChar = XftFreeTypeGlyphExists(QPaintDevice::x11AppDisplay(),
					     (XftFontStruct *) qfs->xfthandle,
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
    XftFontStruct *xftfs = 0;
    XftPattern *xftmatch = 0;
#endif // QT_NO_XFTFREETYPE

    // Look for font name in fontNameDict based on QFont::key()
    QString k(key() + script_table[script].list[script_table[script].index]);
    if ( paintdevice )
	k += "/" + QString::number(QPaintDeviceMetrics( paintdevice ).logicalDpiY());
    QXFontName *qxfn = fontNameDict->find(k);

    if (! qxfn) {
	// if we don't find the name in the dict, we need to find a font name

#ifdef QFONTLOADER_DEBUG_VERBOSE
	qDebug("QFontLoader: getting font name for family %s", request.family.latin1());
#endif

	QString name;
	bool match;
	bool use_core = TRUE;

#ifndef QT_NO_XFTFREETYPE
	if (qt_has_xft) {
	    xftmatch = findXftFont(sample, &match);

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
		else
		    name = QString::null;
	    } else {
		name = findFont(script, &match);
	    }
	}

	if (name.isNull()) {
	    // no font name... this can only happen with Unicode
	    // qDebug("QFontLoader: no font name - this must be unicode (%d %s)",
	    // script, script_table[script].list[script_table[script].index]);

	    name = k + "NU";
	}

#ifdef QFONTLOADER_DEBUG_VERBOSE
	qDebug("QFontLoader: putting '%s' (%d) into name dict", name.latin1(),
	       name.length());
#endif

	// Put font name into fontNameDict
	qxfn = new QXFontName(name.latin1(), match, use_core);
	Q_CHECK_PTR(qxfn);
	fontNameDict->insert(k, qxfn);
    }

#ifdef QFONTLOADER_DEBUG_VERBOSE
    qDebug("QFont::load: using name '%s'", qxfn->name.data());
#endif

    exactMatch = qxfn->exactMatch;

    fontname = qxfn->name;

    // Look in fontCache for font
    qfs = fontCache->find(fontname.data());
    if (qfs) {
	// Found font in either cache or dict...
	x11data.fontstruct[script] = qfs;

	if (qfs != (QFontStruct *) -1) {
	    qfs->ref();
	    initFontInfo(script);
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
				    QPaintDevice::x11AppScreen(), xftmatch, &res);
	}

#ifdef QFONTLOADER_DEBUG
	qDebug("QFontLoader: loading xft font '%s'", fontname.data());
#endif

	xftfs = XftFreeTypeOpen(QPaintDevice::x11AppDisplay(),
				xftmatch);

	XftPatternDestroy(xftmatch);
    }
#endif // QT_NO_XFTFREETYPE

    if (qxfn->useCore) {
	// if we have no way to map this script, we give up
	if (! script_table[script].list[script_table[script].index]) {

#ifdef QFONTLOADER_DEBUG_VERBOSE
	    qDebug("QFontLoader: no nothing about script %d, giving up", script);
#endif

	    x11data.fontstruct[script] = (QFontStruct *) -1;
	    return;
	}

	// get unicode -> font encoding codec
	if (script < QFont::Unicode || script > QFont::NoScript) {
	    if ( script == QFont::Hebrew )
		codec = QTextCodec::codecForName( "ISO 8859-8-I" );
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
		fontCache->insert(fontname.data(), x11data.fontstruct[script], 1);
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
	    if (script != QFont::Unicode) {

#ifdef QFONTLOADER_DEBUG
		qDebug("QFontLoader: load failed, trying last resort");
#endif

		exactMatch = FALSE;

		if (! (xfs = XLoadQueryFont(QPaintDevice::x11AppDisplay(),
					    lastResortFont().latin1()))) {
		    qFatal("QFontLoader: Internal error");
		}
	    } else {
		// Didn't get unicode font, set to sentinel and return
		//qDebug("no unicode font, doing negative caching");
		x11data.fontstruct[script] = (QFontStruct *) -1;
		fontCache->insert(fontname.data(), x11data.fontstruct[script], 1);

		return;
	    }
	}
    }

    // calculate cost of this item in the fontCache
    int cost = 1;
    if (xfs) {
	cost = maxIndex(xfs);
	if ( cost > 5000 ) {
	    // If the user is using large fonts, we assume they have
	    // turned on the Xserver option deferGlyphs, and that they
	    // have more memory available to the server.
	    cost = 5000;
	}
	cost = ((xfs->max_bounds.ascent + xfs->max_bounds.descent) *
		(xfs->max_bounds.width * cost / 8));
    }
#ifndef QT_NO_XFTFREETYPE
    else if (xftfs) {
	cost = (xftfs->max_char - xftfs->min_char);
	if (cost <= 0)
	    cost = 256;
	else if (cost > 5000)
	    cost = 5000;
	cost = ((xftfs->ascent + xftfs->descent) *
		(xftfs->max_advance_width * cost / 8));
    }
#endif // QT_NO_XFTFREETYPE

    qfs = new QFontStruct((Qt::HANDLE) xfs,
#ifndef QT_NO_XFTFREETYPE
			  (Qt::HANDLE) xftfs,
#else
			  0,
#endif // QT_NO_XFTFREETYPE
			  fontname, codec, cost);
    x11data.fontstruct[script] = qfs;

    initFontInfo(script);
    request.dirty = FALSE;

    // Insert font into the font cache and font dict
    bool inserted;
    inserted = fontCache->insert(qfs->name, qfs, qfs->cache_cost);

#ifdef QT_CHECK_STATE
    if (! inserted)
	qFatal("QFont::load: font cache overflow error");
#endif // QT_CHECK_STATE

}




// **********************************************************************
// QFont methods
// **********************************************************************


/*!
  Returns TRUE if the font attributes have been changed and the font has to
  be (re)loaded; otherwise returns FALSE.
*/
bool QFont::dirty() const
{
    return d->request.dirty;
}


// **********************************************************************
// QFont static methods
// **********************************************************************

QFont::Script QFontPrivate::defaultScript = QFont::UnknownScript;
QCleanupHandler<QFontCache> cleanup_fontcache;
QCleanupHandler<QFontNameDict> cleanup_fontnamedict;

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
    QFontPrivate::fontCache = new QFontCache();
    Q_CHECK_PTR(QFontPrivate::fontCache);
    cleanup_fontcache.add(&QFontPrivate::fontCache);

    fontNameDict = new QFontNameDict(QFontPrivate::fontCache->size(), FALSE);
    Q_CHECK_PTR(fontNameDict);
    fontNameDict->setAutoDelete(TRUE);
    cleanup_fontnamedict.add(&fontNameDict);

#ifndef QT_NO_CODECS
#ifndef QT_NO_BIG_CODECS
    (void) new QFontJis0208Codec;
    (void) new QFontKsc5601Codec;
    (void) new QFontGB2312Codec;
    (void) new QFontBig5Codec;
    (void) new QFontArabic68Codec;
#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS

#ifndef QT_NO_XFTFREETYPE
    qt_has_xft = FALSE;

    if (qt_use_xrender &&
	XftInit(0) && XftInitFtLibrary()) {
	qt_has_xft = TRUE;
	qt_xft_render_sources = new QPixmapDict();
	cleanup_pixmapdict.add(&qt_xft_render_sources);
    }
#endif // QT_NO_XFTFREETYPE

    QTextCodec *codec = QTextCodec::codecForLocale();
    // we have a codec for the locale - lets see if it's one of the CJK codecs,
    // and change the script_table[Han].list to an appropriate list
    if (codec) {
	switch (codec->mibEnum()) {
	case 57: // GB 2312-1980
	case 2027: // GBK
	    script_table[QFont::Han].list = hancn_encodings;
	    break;

	case 2026: // Big5
	    script_table[QFont::Han].list = hantw_encodings;
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

    // get some sample text based on the users locale.  we use this to determine the
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
	    tmp = priv->scriptForChar(*uc++);
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
    QFontPrivate::fontCache = 0;
    fontNameDict = 0;

#ifndef QT_NO_XFTFREETYPE
    if (qt_xft_render_sources)
	qt_xft_render_sources->setAutoDelete(TRUE);
    qt_xft_render_sources = 0;
#endif // QT_NO_XFTFREETYPE

}





// **********************************************************************
// QFont member methods
// **********************************************************************

/*!
  Returns the window system handle to the font, for low-level
  access.  Using this function is \e not portable.
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
  On Windows, this is usually just the family name of a truetype
  font. Under X, it is an XLFD (X Logical Font Description). Using the
  return value of this function is usually \e not \e portable.

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
  Sets a font by its system specific name. The function is in
  particular useful under X, where system font settings (for example
  X resources) are usually available in XLFD (X Logical Font
  Description) form only. You can pass an XLFD as \a name to this
  function.

  In Qt 2.0 and later, a font set with setRawName() is still a
  full-featured QFont. It can be queried (for example with italic())
  or modified (for example with setItalic() ) and is therefore also
  suitable for rendering rich text.

  If Qt's internal font database cannot resolve the raw name, the font
  becomes a raw font with \a name as its family.

  Note that the present implementation does not handle
  wildcards in XLFDs well, and that font aliases (file \c fonts.alias
  in the font directory on X11) are not supported.

  \sa rawName(), setRawMode(), setFamily()
*/
void QFont::setRawName( const QString &name )
{
    detach();

    bool validXLFD = QFontPrivate::fillFontDef(name.latin1(), &d->request, 0);
    d->request.dirty = TRUE;

    if ( !validXLFD ) {

#ifdef QT_CHECK_STATE
	qWarning("QFont::setRawMode(): Invalid XLFD: \"%s\"", name.latin1());
#endif // QT_CHECK_STATE

	setFamily( name );
	setRawMode( TRUE );
    }
}


// **********************************************************************
// QFontMetrics member methods
// **********************************************************************

/*!
  Returns the maximum ascent of the font.

  The ascent is the distance from the base line to the uppermost line
  where pixels may be drawn.

  \sa descent()
*/
int QFontMetrics::ascent() const
{

    d->load(QFontPrivate::defaultScript);

    QFontStruct *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (QFontStruct *) -1)
	return d->actual.pixelSize * 3 / 4;

#ifndef QT_NO_XFTFREETYPE
    XftFontStruct *xftfs = (XftFontStruct *) qfs->xfthandle;
    if (xftfs)
	return xftfs->ascent;
#endif // QT_NO_XFTFREETYPE

    XFontStruct *f = (XFontStruct *) qfs->handle;
    return (int) (f->max_bounds.ascent * qfs->scale);
}


/*!
  Returns the maximum descent of the font.

  The descent is the distance from the base line to the lowermost line
  where pixels may be drawn. (Note that this is different from X, which
  adds 1 pixel.)

  \sa ascent()
*/
int QFontMetrics::descent() const
{
    d->load(QFontPrivate::defaultScript);

    QFontStruct *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (QFontStruct *) -1)
	return 0;

#ifndef QT_NO_XFTFREETYPE
    XftFontStruct *xftfs = (XftFontStruct *) qfs->xfthandle;
    if (xftfs)
	return xftfs->descent;
#endif // QT_NO_XFTFREETYPE

    XFontStruct *f = (XFontStruct *) qfs->handle;
    return (int) ((f->max_bounds.descent - 1)*qfs->scale);
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
  of the character from the logical origin of the character.
  This value is negative if the pixels of the character extend
  to the left of the logical origin.

  See width(QChar) for a graphical description of this metric.

  \sa rightBearing(), minLeftBearing(), width()
*/
int QFontMetrics::leftBearing(QChar ch) const
{
    QFont::Script script = d->scriptForChar(ch);

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

  The right bearing is the left-ward distance of the right-most pixel
  of the character from the logical origin of a subsequent character.
  This value is negative if the pixels of the character extend
  to the right of the width() of the character.

  See width() for a graphical description of this metric.

  \sa leftBearing(), minRightBearing(), width()
*/
int QFontMetrics::rightBearing(QChar ch) const
{
    QFont::Script script = d->scriptForChar(ch);

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

  This is the smallest leftBearing(char) of all characters in the font.

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

  This is always equal to ascent()+descent()+1 (the 1 is for the base line).

  \sa leading(), lineSpacing()
*/
int QFontMetrics::height() const
{
    d->load(QFontPrivate::defaultScript);

    QFontStruct *qfs =  d->x11data.fontstruct[QFontPrivate::defaultScript];
    if (! qfs || qfs == (QFontStruct *) -1)
	return (d->actual.pixelSize * 3 / 4) + 1;

#ifndef QT_NO_XFTFREETYPE
    XftFontStruct *xftfs = (XftFontStruct *) qfs->xfthandle;
    if (xftfs)
	return xftfs->ascent + xftfs->descent;
#endif // QT_NO_XFTFREETYPE

    XFontStruct *f = (XFontStruct *) qfs->handle;
    return (int) ((f->max_bounds.ascent + f->max_bounds.descent) * qfs->scale);
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
    XftFontStruct *xftfs = (XftFontStruct *) qfs->xfthandle;
    if (xftfs)
	l = xftfs->height - (xftfs->ascent + xftfs->descent);
    else
#endif // QT_NO_XFTFREETYPE
	l = (int) ((f->ascent + f->descent - f->max_bounds.ascent -
		    f->max_bounds.descent) * qfs->scale);

    return (l > 0) ? l : 0;
}


/*! Returns the distance from one base line to the next.

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


/*! \overload

  <img src="bearings.png" align=right>

  Returns the logical width of character \a ch in pixels.  This is a
  distance appropriate for drawing a subsequent character after \a ch.

  Some of the metrics are described in the image to the right.  The
  tall dark rectangle covers the logical width() of a character.  The
  shorter pale rectangles cover leftBearing() and rightBearing() of
  the characters.  Notice that the bearings of "f" in this particular
  font are both negative, while the bearings of "o" are both positive.

  \warning This function will produce incorrect results for Arabic
  characters or non spacing marks in the middle of a string, as the
  glyph shaping  and positioning of marks  that happens when processing
  strings cannot be taken into account. Use charWidth() instead if you
  aren't looking for the width of isolated characters.

  \sa boundingRect(), charWidth()
*/
int QFontMetrics::width(QChar ch) const
{
    if ( ch.combiningClass() > 0 )
	return 0;

    QFont::Script script = d->scriptForChar(ch);

    if (script == QFont::UnknownScript)
	return d->actual.pixelSize * 3 / 4;

    d->load(script);

    int w = 0;
    QFontStruct *qfs = d->x11data.fontstruct[script];

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
		w = (int) (xcs->width * qfs->scale);
	}

    return w;
}


/*!
    Returns the width of the character at position \a pos in the string
    \a str.

  The whole string is needed, as the glyph drawn may change depending on the
  context (the letter before and after the current one) for some languages
  (e.g. Arabic).

  This function also takes non spacing marks and ligatures into account.
*/
int QFontMetrics::charWidth( const QString &str, int pos ) const
{
    QChar ch = str[pos];
    if ( ch.combiningClass() > 0 )
	return 0;

    QFont::Script script = d->scriptForChar(ch);

    if (script == QFont::UnknownScript)
	return d->actual.pixelSize * 3 / 4;

    d->load(script);

    int w = 0;
    QFontStruct *qfs = d->x11data.fontstruct[script];

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
		w = (int) (xcs->width * qfs->scale);
	}

    return w;
}


/*!
    Returns the width in pixels of the first \a len characters of \a
    str. If \a len is negative (the default), the entire string is used.

  Note that this value is \e not equal to boundingRect().width();
  boundingRect() returns a rectangle describing the pixels this string
  will cover whereas width() returns the distance to where the next string
  should be drawn.

  \sa boundingRect()
*/
int QFontMetrics::width( const QString &str, int len ) const
{
    if (len < 0)
	len = str.length();
    if (len == 0)
	return 0;

    // this algorithm is similar to the one used for painting
    QString shaped = QComplexText::shapedString( str, 0, len );
    len = shaped.length();

    return d->textWidth( shaped, 0, len );
}


/*!
    Returns the bounding rectangle of the first \a len characters of \a
    str, which is the set of pixels the text would cover if drawn at
    (0,0).

  If \a len is negative (the default), the entire string is used.

  Note that the bounding rectangle may extend to the left of (0,0),
  e.g. for italicized fonts, and that the text output may cover \e all
  pixels in the bounding rectangle.

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
    QString shaped = QComplexText::shapedString( str, 0, len);

    QCharStruct overall;

    d->textExtents( shaped, 0, shaped.length(), &overall );

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
	    XftFontStruct *xftfs =
		(XftFontStruct *) qfs->xfthandle;
	    ww = xftfs->max_advance_width;
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
  Returns the distance from the base line to where an underscore should be
  drawn.
  \sa strikeOutPos(), lineWidth()
*/
int QFontMetrics::underlinePos() const
{
    int pos = ((lineWidth() * 2) + 3) / 6;

    return pos ? pos : 1;
}


/*!
  Returns the distance from the base line to where the strikeout line
  should be drawn.
  \sa underlinePos(), lineWidth()
*/
int QFontMetrics::strikeOutPos() const
{
    int pos = ascent() / 3;

    return pos > 0 ? pos : 1;
}


/*!
  Returns the width of the underline and strikeout lines, adjusted for
  the point size of the font.
  \sa underlinePos(), strikeOutPos()
*/
int QFontMetrics::lineWidth() const
{
    // lazy computation of linewidth
    d->computeLineWidth();

    return d->lineWidth;
}
