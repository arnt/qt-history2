/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont_x11.cpp#190 $
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

#include "qfont.h"
#include "qfontdata_p.h"
#include "qcomplextext_p.h"
#include "qfontinfo.h"
#include "qfontdatabase.h"
#include "qfontmetrics.h"
#include "qpainter.h"

#include <qpaintdevice.h>
#include <qregexp.h>
#include <qdict.h>
#include <qtextcodec.h>
#include <qcleanuphandler.h>

#include "../codecs/qfontcodecs_p.h"

#ifndef QT_NO_XFTFREETYPE
#  include <qintdict.h>
#  include <qpixmap.h>
#  include <qapplication.h> // for settings
#  include <qsettings.h>
#  include <netinet/in.h>
#endif // QT_NO_XFTFREETYPE

#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "qt_x11.h"


// #define QFONTLOADER_DEBUG
// #define QFONTLOADER_DEBUG_VERBOSE


// to get which font encodings are installed:
// xlsfonts | egrep -- "^-.*-.*-.*$" | cut -f 14- -d- | sort | uniq

// This array must be kept in sync with the QFontPrivate::Script enum in
// qfontdata_p.h
static const char *qt_x11encodings[][QFont::NScripts + 1] = {
    { "iso8859-1"        , 0 }, // BasicLatin

    { "iso8859-2"        , 0 }, // LatinExtendedA
    // TODO: Latin Extended-B
    { 0                      }, // LatinExtendedB
    { 0                      }, // IPAExtensions
    { 0                      }, // LatinExtendedAdditional
    { 0                      }, // LatinLigatures

    { 0                      }, // Diacritical

    { "iso8859-7"        , 0 }, // Greek
    { 0                      }, // GreekExtended
    { "iso8859-5",
      "koi8-r",
      "koi8-ru"          , 0 }, // Cyrillic
    { 0                      }, // CyrillicHistoric
    { 0                      }, // CyrillicExtended
    { 0                      }, // Armenian
    { 0                      }, // Georgian
    { 0                      }, // Runic
    { 0                      }, // Ogham

    { "iso8859-8-i"        , 0 }, // Hebrew
    { 0                      }, // HebrewPresentation

    { "iso8859-6.8x"     , 0 }, // // Arabic
    // "iso8859-6" - commented for now since arabic doesn't work at all for iso8859-6

    { 0                      }, // ArabicPresentationA
    { 0                      }, // ArabicPresentationB
    { 0                      }, // Syriac
    { 0                      }, // Thaana

    { 0                      }, // Devanagari
    { 0                      }, // Bengali
    { 0                      }, // Gurmukhi
    { 0                      }, // Gujarati
    { 0                      }, // Oriya
    { "tscii-*"          , 0 }, // Tamil
    { 0                      }, // Telugu
    { 0                      }, // Kannada
    { 0                      }, // Malayalam
    { 0                      }, // Sinhala
    { "tis620*-0",
      "iso8859-11"       , 0 }, // Thai
    { 0                      }, // Lao
    { 0                      }, // Tibetan
    { 0                      }, // Myanmar
    { 0                      }, // Khmer

    { "jisx0208.1983-0",
      "gb2312.1980-0",
      "big5*-*",
      "ksc5601.1987-0",    0 }, // UnifiedHan
    { "jisx0208.1983-0",   0 }, // Hiragana
    { "jisx0208.1983-0"  , 0 }, // Katakana
    { "ksc5601.1987-0"   , 0 }, // Hangul
    { "gb2312.1980-0"    , 0 }, // Bopomofo
    { 0                      }, // Yi

    { 0                      }, // Ethiopic
    { 0                      }, // Cherokee
    { 0                      }, // CanadianAboriginal
    { 0                      }, // Mongolian

    { "big5*-*"          , 0 }, // UnifiedHanX11

    { "iso8859-3"        , 0 }, // LatinExtendedA_3
    { "iso8859-4"        , 0 }, // LatinExtendedA_4
    { "iso8859-14"       , 0 }, // LatinExtendedA_14
    { "iso8859-15"       , 0 }, // LatinExtendedA_15

    { "iso10646-1",
      "unicode-*"        , 0 }, // UNICODE == ISO-10646-1 (for now)

    { "*-*"              , 0 } // UnknownScript
};


static int qt_x11indices[QFont::NScripts + 1] = {
    0, // BasicLatin

    0, // LatinExtendedA
    0, // LatinExtendedB
    0, // IPAExtensions
    0, // LatinExtendedAdditional
    0, // LatinLigatures

    0, // Diacritical

    0, // Greek
    0, // GreekExtended
    0, // Cyrillic
    0, // CyrillicHistoric
    0, // CyrillicExtended
    0, // Armenian
    0, // Georgian
    0, // Runic
    0, // Ogham

    0, // Hebrew
    0, // HebrewPresentation
    0, // Arabic
    0, // ArabicPresentationA
    0, // ArabicPresentationB
    0, // Syriac
    0, // Thaana

    0, // Devanagari
    0, // Bengali
    0, // Gurmukhi
    0, // Gujarati
    0, // Oriya
    0, // Tamil
    0, // Telugu
    0, // Kannada
    0, // Malayalam
    0, // Sinhala
    0, // Thai
    0, // Lao
    0, // Tibetan
    0, // Myanmar
    0, // Khmer

    0, // UnifiedHan
    0, // Hiragana
    0, // Katakana
    0, // Hangul
    0, // Bopomofo
    0, // Yi

    0, // Ethiopic
    0, // Cherokee
    0, // CanadianAboriginal
    0, // Mongolian

    0, // HanHack

    // LatinExtendedA_2 is LatinExtendedA
    0, // LatinExtendedA_3
    0, // LatinExtendedA_4
    0, // LatinExtendedA_14
    0, // LatinExtendedA_15

    0, // Unicode

    0, // NScripts
};



bool qt_has_xft = FALSE;

// Data needed for XftFreeType support
#ifndef QT_NO_XFTFREETYPE
typedef QIntDict<QPixmap> QPixmapDict;
static QPixmapDict *qt_xft_render_sources = 0;
QCleanupHandler<QPixmapDict> cleanup_pixmapdict;
#endif // QT_NO_XFTFREETYPE








class QXFontName
{
public:
    QXFontName( const QCString &n, bool e )
	: name(n), exactMatch(e)
    { ; }

    QCString name;
    bool exactMatch;
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
    if (fontNameDict) fontNameDict->clear();
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

	if ( fontExists(last) ) {
	    return last;
	}

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

    while (1) {
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

    char slant	= tolower( tokens[Slant][0] );
    fd->italic = (slant == 'o' || slant == 'i');
    char fixed	= tolower( tokens[Spacing][0] );
    fd->fixedPitch = (fixed == 'm' || fixed == 'c');
    fd->weight = getFontWeight( tokens[Weight] );

    int r = atoi(tokens[ResolutionY]);
    int px = atoi(tokens[PixelSize]);
    // not "0" or "*", or required DPI
    if ( r && px && QPaintDevice::x11AppDpiY() && r != QPaintDevice::x11AppDpiY() ) {
	// calculate actual pointsize for display DPI
	fd->pixelSize = px;
	fd->pointSize = (int) ((px * 720) / QPaintDevice::x11AppDpiY());
    } else {
	// calculate pixel size from pointsize/dpi
	fd->pixelSize = (px * fd->pointSize) / 720;
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
	    r.setRect( 0, request.pixelSize * -3 / 4,
		       request.pixelSize * 3 / 4, request.pixelSize * 3 / 4);
	else if ( xgi )
	    r.setRect( -xgi->x, -xgi->y, xgi->width, xgi->height);
    } else
#endif // QT_NO_XFTFREETYPE

	{
	    xcs = getCharStruct(qfs, QString(ch), 0);
	    if ( xcs == (XCharStruct *) -1)
		r.setRect( 0, request.pixelSize * -3 / 4,
			   request.pixelSize * 3 / 4, request.pixelSize * 3 / 4);
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
    int i, w = 0;

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
		    w += request.pixelSize * 3 / 4;
		} else if (xgi)
		    w += xgi->xOff;
	    } else
#endif // QT_NO_XFTFREETYPE
		{
		    xcs = getCharStruct(qfs, str, pos + i);
		    if (xcs == (XCharStruct *) -1) {
			// character isn't in the font, set the script to UnknownScript
			tmp = current = QFont::UnknownScript;
			w += request.pixelSize * 3 / 4;
		    } else if (xcs)
			w += xcs->width;
		}
	}

	chars++;
    }

    return w;
}


// returns the width of the string in pixels (replaces XTextWidth)... this function
// also sets up a TextRun cache, which is used by QFontPrivate::drawText below
int QFontPrivate::textWidth( const QString &str, int pos, int len,
			     QFontPrivate::TextRun *cache )
{
    const QChar *chars = str.unicode() + pos, *last = 0;
    QFont::Script current = QFont::NoScript, tmp;
    int i, w = 0, pw = 0, lastlen = 0;

    // for non-spacing marks
    QPointArray markpos;
    int nmarks = 0;

    QFontStruct *qfs = 0;
    XCharStruct *xcs = 0;

#ifndef QT_NO_XFTFREETYPE
    XGlyphInfo *xgi = 0;
#endif // QT_NO_XFTFREETYPE

    for (i = 0; i < len; i++) {
	if (chars->combiningClass() == 0 || pos + i == 0) {
	    tmp = scriptForChar(*chars);

	    if (tmp != current) {
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

		    w += request.pixelSize * 3 / 4;
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

			w += request.pixelSize * 3 / 4;
		    } else if (xcs)
			w += xcs->width;
		}

	    chars++;
	    lastlen++;
	} else {
	    // start of a new set of marks
	    if (last && lastlen) {
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

		QPoint p = markpos[n];

		// advance one character
		chars++;
		lastlen++;

		if (last && lastlen) {
		    if (qfs && qfs != (QFontStruct *) -1 && qfs->codec)
			cache->mapped =
			    qfs->codec->fromUnicode(str, pos + i - lastlen, lastlen);
		    // set the position for the mark
		    cache->setParams(pw + p.x(), p.y(), w, last, lastlen, tmp);

		    // advance to the next mark/character
		    cache->next = new QFontPrivate::TextRun();
		    cache = cache->next;
		    pw = w;
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


// return the text extents in overall, replaces XTextExtents
void QFontPrivate::textExtents( const QString &str, int pos, int len,
				XCharStruct *overall )
{
    const QChar *chars = str.unicode() + pos;
    QFont::Script current = QFont::NoScript, tmp;
    int i;

    // for non-spacing marks
    QPointArray markpos;
    int nmarks = 0;

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

		    int size = (request.pixelSize * 3 / 4);
		    overall->ascent = QMAX(overall->ascent, size);
		    overall->descent = QMAX(overall->descent, 0);
		    overall->lbearing = QMIN(overall->lbearing, 0);
		    overall->width += size;
		    overall->rbearing = QMAX(overall->rbearing, overall->width);
		} else {
		    overall->ascent = QMAX(overall->ascent, xgi->y);
		    overall->descent = QMAX(overall->descent, xgi->height - xgi->y);
		    overall->lbearing = QMIN(overall->lbearing, xgi->x);
		    overall->rbearing = QMAX(overall->rbearing,
					     overall->width + (xgi->width - xgi->x));
		    overall->width += xgi->xOff;
		}
	    } else
#endif // QT_NO_XFTFREETYPE
		{
		    xcs = getCharStruct(qfs, str, pos + i);
		    if (xcs == (XCharStruct *) -1) {
			// character isn't in the font, set the script to UnknownScript
			tmp = current = QFont::UnknownScript;

			int size = (request.pixelSize * 3 / 4);
			overall->ascent = QMAX(overall->ascent, size);
			overall->descent = QMAX(overall->descent, 0);
			overall->lbearing = QMIN(overall->lbearing, 0);
			overall->width += size;
			overall->rbearing = QMAX(overall->rbearing, overall->width);
		    } else if (xcs) {
			overall->ascent = QMAX(overall->ascent, xcs->ascent);
			overall->descent = QMAX(overall->descent, xcs->descent);
			overall->lbearing = QMIN(overall->lbearing,
						 overall->width + xcs->lbearing);
			overall->rbearing = QMAX(overall->rbearing,
						 overall->width + xcs->rbearing);
			overall->width += xcs->width;
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

		QPoint p = markpos[n];

		// yes, really do both, this makes sure that marks that rise
		// above the box expand it up, and marks below the box expand it down
		overall->ascent = QMAX(overall->ascent, p.y());
		overall->descent = QMAX(overall->descent, p.y());

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
			     const QFontPrivate::TextRun *cache )
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

    	if ( cache->script < QFont::NScripts && qfs && qfs != (QFontStruct *) -1 ) {
	    xfs = (XFontStruct *) qfs->handle;

#ifndef QT_NO_XFTFREETYPE
	    xftfs = (XftFontStruct *) qfs->xfthandle;
#endif // QT_NO_XFTFREETYPE

	}

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

	    // byteswap!
	    unsigned short *tstr;
	    unsigned short *tshort, c;
	    int i;

	    tshort = new unsigned short[cache->length];
	    tstr = (unsigned short *) cache->string;
	    for (i = 0; i < cache->length; i++) {
		c = tstr[i];
		tshort[i] = htons( c );
	    }

	    if (bgmode != Qt::TransparentMode) {
		XRenderColor col;
		col.red = bgcolor.red();
		col.green = bgcolor.green();
		col.blue = bgcolor.blue();
		col.alpha = 0xffff;
		XRenderFillRectangle(dpy, PictOpSrc, rendhd, &col,
				     x + cache->xoff,  y - xftfs->ascent,
				     cache->x2off - cache->xoff,
				     xftfs->ascent + xftfs->descent);
	    }

	    XftRenderString16(dpy, pm->x11RenderHandle(), xftfs, rendhd, 0, 0,
			      x + cache->xoff, y + cache->yoff, tshort, cache->length);

	    delete [] tshort;
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
		    else
			chars = (XChar2b *) cache->string;

		    if (bgmode != Qt::TransparentMode)
			XDrawImageString16(dpy, hd, gc, x + cache->xoff, y + cache->yoff,
					   chars, cache->length );
		    else
			XDrawString16(dpy, hd, gc, x + cache->xoff, y + cache->yoff,
				      chars, cache->length );
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
		int inc = request.pixelSize * 3 / 4;

		for (int k = 0; k < l; k++) {
		    rects[k].x = x + cache->xoff + (k * inc);
		    rects[k].y = y - inc + 2;
		    rects[k].width = rects[k].height = inc - 3;
		}

		XDrawRectangles(dpy, hd, gc, rects, l);
		delete [] rects;
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
XftPattern *QFontPrivate::findXftFont(const QChar &sample) const
{
    // if the family name contains a '-' (ie. "Adobe-Courier"), then we split
    // at the '-', and use the string as the foundry, and the string to the right as
    // the family
    QString familyName = request.family;
    QString foundryName;
    if ( familyName.contains('-') ) {
	int i = familyName.find('-');
	foundryName = familyName.left( i );
	familyName = familyName.right( familyName.length() - i - 1 );
    }

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

    if (match)
	return match;

    // try font substitutions
    QStringList list = QFont::substitutes(request.family);
    QStringList::Iterator sit = list.begin();

    while (sit != list.end() && ! match) {
	familyName = *sit++;

	if (request.family != familyName) {
	    if ( familyName.contains('-') ) {
		int i = familyName.find('-');
		foundryName = familyName.left( i );
		familyName = familyName.right( familyName.length() - i - 1 );
	    }

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

    if (match)
	return match;

    match = bestXftPattern(QString::null, QString::null);

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

    qDebug("last resort: %p %s", match, familyName.latin1());

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

    size_value = request.pointSize / 10;
    mono_value = request.fixedPitch;

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
	mono_value = 1;
	break;
    }

    XftResult res;
    XftPattern *pattern = 0, *result = 0;

    if (! foundryName.isNull())
	pattern = XftPatternBuild(0, XFT_ENCODING, XftTypeString, "iso10646-1",
				  XFT_FOUNDRY, XftTypeString, foundryName.latin1(),
				  XFT_FAMILY, XftTypeString, familyName.latin1(),
				  XFT_FAMILY, XftTypeString, generic_value.data(),
				  XFT_WEIGHT, XftTypeInteger, weight_value,
				  XFT_SLANT, XftTypeInteger, slant_value,
				  XFT_SIZE, XftTypeDouble, size_value,
				  XFT_SPACING, XftTypeInteger, mono_value,
				  0);
    else if (! familyName.isNull())
	pattern = XftPatternBuild(0, XFT_ENCODING, XftTypeString, "iso10646-1",
				  XFT_FAMILY, XftTypeString, familyName.latin1(),
				  XFT_FAMILY, XftTypeString, generic_value.data(),
				  XFT_WEIGHT, XftTypeInteger, weight_value,
				  XFT_SLANT, XftTypeInteger, slant_value,
				  XFT_SIZE, XftTypeDouble, size_value,
				  XFT_SPACING, XftTypeInteger, mono_value,
				  0);
    else
	pattern = XftPatternBuild(0, XFT_ENCODING, XftTypeString, "iso10646-1",
				  XFT_FAMILY, XftTypeString, generic_value.data(),
				  XFT_WEIGHT, XftTypeInteger, weight_value,
				  XFT_SLANT, XftTypeInteger, slant_value,
				  XFT_SIZE, XftTypeDouble, size_value,
				  XFT_SPACING, XftTypeInteger, mono_value,
				  0);

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
#define ScaledBitmapScore 0x40
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

    // if the family name contains a '-' (ie. "Adobe-Courier"), then we split
    // at the '-', and use the string as the foundry, and the string to the right as
    // the family
    QString foundry;
    if ( familyName.contains('-') ) {
	int i = familyName.find('-');
	foundry = familyName.left( i );
	familyName = familyName.right( familyName.length() - i - 1 );
    }

    int score;

    QCString bestName;
    bool done = FALSE;
    int start_index = qt_x11indices[script];

    while (! done) {
	bestName = bestFamilyMember(script, foundry, familyName, &score);

	if (bestName.isNull()) {
	    if (! qt_x11encodings[script][++qt_x11indices[script]])
		qt_x11indices[script] = 0;

	    if (qt_x11indices[script] == start_index)
		done = TRUE;
	} else
	    done = TRUE;
    }

    if ( score < exactScore )
	*exact = FALSE;

    if (! bestName.isNull())
	return bestName;

    // try substitution
    QStringList list = QFont::substitutes( request.family );
    QStringList::Iterator sit = list.begin();

    while (sit != list.end() && bestName.isNull()) {
	familyName = *sit++;

	if (request.family != familyName) {
	    done = FALSE;
	    qt_x11indices[script] = start_index;

	    if ( familyName.contains('-') ) {
		int i = familyName.find('-');
		foundry = familyName.left( i );
		familyName = familyName.right( familyName.length() - i - 1 );
	    }

	    while (! done) {
		bestName = bestFamilyMember(script, foundry, familyName, &score);

		if (bestName.isNull()) {
		    if (! qt_x11encodings[script][++qt_x11indices[script]])
			qt_x11indices[script] = 0;

		    if (qt_x11indices[script] == start_index)
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
	qt_x11indices[script] = start_index;

	while (! done) {
	    bestName = bestFamilyMember(script, foundry, familyName, &score);

	    if (bestName.isNull()) {
		if (! qt_x11encodings[script][++qt_x11indices[script]])
		    qt_x11indices[script] = 0;

		if (qt_x11indices[script] == start_index)
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
	qt_x11indices[script] = start_index;

	while (! done) {
	    bestName = bestFamilyMember(script, foundry, familyName, &score);

	    if (bestName.isNull()) {
		if (! qt_x11encodings[script][++qt_x11indices[script]])
		    qt_x11indices[script] = 0;

		if (qt_x11indices[script] == start_index)
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
	qt_x11indices[script] = start_index;

	while (! done) {
	    bestName = bestFamilyMember(script, foundry, familyName, &score);

	    if (bestName.isNull()) {
		if (! qt_x11encodings[script][++qt_x11indices[script]])
		    qt_x11indices[script] = 0;

		if (qt_x11indices[script] == start_index)
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
	    = "-" + foundry + "-" + family + "-*-*-*-*-*-*-*-*-*-*-" +
	    (qt_x11encodings[script])[(qt_x11indices[script])];
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
	    QString pattern = "-*-" + fam + "-*-*-*-*-*-*-*-*-*-*-" +
			      (qt_x11encodings[script])[(qt_x11indices[script])];
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
		bestScalable.score	= sc;
		bestScalable.name	= xFontNames[i];
		bestScalable.pointDiff  = pointDiff;
		bestScalable.weightDiff = weightDiff;
		bestScalable.smooth	= smoothScalable;
	    }
	} else {
	    if ( sc > best.score ||
		 sc == best.score && pointDiff < best.pointDiff ||
		 sc == best.score && pointDiff == best.pointDiff &&
		 weightDiff < best.weightDiff ) {
		best.score	= sc;
		best.name	= xFontNames[i];
		best.pointDiff	= pointDiff;
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
		resx  = QPaintDevice::x11AppDpiX();
		resy  = QPaintDevice::x11AppDpiY();
		pSize = request.pointSize;
	    } else {
		resx = atoi(tokens[ResolutionX]);
		resy = atoi(tokens[ResolutionY]);
		pSize = ( (2 * request.pointSize * QPaintDevice::x11AppDpiY()) + resy )
			/ (resy * 2);
	    }

	    bestName.sprintf( "-%s-%s-%s-%s-%s-%s-*-%i-%i-%i-%s-*-%s-%s",
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
	    best.name  = bestName.data();
	    best.score = bestScalable.score;
	}
    }

    *score   = best.score;
    bestName = best.name;

    XFreeFontNames( xFontNames );

    return bestName;
}


// Returns a score describing how well a font name matches the contents
// of a font.
int QFontPrivate::fontMatchScore( const char *fontName, QCString &buffer,
				   float *pointSizeDiff, int  *weightDiff,
				   bool	 *scalable     , bool *smoothScalable,
				  QFont::Script script ) const
{
    char *tokens[NFontFields];
    bool   exactmatch = TRUE;
    int	   score      = NonUnicodeScore;
    *scalable	      = FALSE;
    *smoothScalable   = FALSE;
    *weightDiff	      = 0;
    *pointSizeDiff    = 0;

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

    char pitch = tolower( tokens[Spacing][0] );
    if ( script >= QFont::UnifiedHan && script <= QFont::Yi ||
	 script == QFont::UnifiedHanX11 ) {
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
	// scaled bitmap fonts look just ugly. Never give them a size score.
	diff = 9.0;	// choose scalable font over >= 0.9 point difference
	if ( *smoothScalable )
	    score |= SizeScore;
	else
	    exactmatch = FALSE;
    } else {
	int pSize;
	float percentDiff;
	pSize = ( 2*atoi( tokens[PointSize] )*atoi(tokens[ResolutionY]) +
		  QPaintDevice::x11AppDpiY())
		/ (QPaintDevice::x11AppDpiY() * 2); // adjust actual pointsize

	if ( request.pointSize != 0 ) {
	    diff = (float)QABS(pSize - request.pointSize);
	    percentDiff = diff/request.pointSize*100.0F;
	} else {
	    diff = (float)pSize;
	    percentDiff = 100;
	}

	if ( percentDiff < 10 ) {
	    score |= SizeScore;

	    if ( pSize != request.pointSize ) {
		exactmatch = FALSE;
	    }
	} else {
	    exactmatch = FALSE;
	}
    }

    if ( pointSizeDiff )
	*pointSizeDiff = diff;

    int weightVal = getFontWeight(tokens[Weight], TRUE);
    if ( weightVal == (int) request.weight )
	score |= WeightScore;
    else
	exactmatch = FALSE;

    *weightDiff = QABS( weightVal - (int) request.weight );
    char slant = tolower( tokens[Slant][0] );

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


// Computes the line width (underline,strikeout) for the X font
// and fills in the X resolution of the font.
void QFontPrivate::computeLineWidth()
{
    int nlw;
    int weight = request.weight;
    int pSize  = request.pointSize / 10;

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
    if (script != defaultScript || ! actual.dirty) return;

    actual.lbearing = SHRT_MIN;
    actual.rbearing = SHRT_MIN;

    if (exactMatch) {
	actual = request;
	actual.dirty = FALSE;

	return;
    }

    if (! fillFontDef(x11data.fontstruct[script]->name, &actual, 0)) {
	// zero fontdef
	actual = QFontDef();

	actual.family = QString::fromLatin1(x11data.fontstruct[script]->name);
	actual.rawMode = TRUE;
	exactMatch = FALSE;
    }

    actual.underline = request.underline;
    actual.strikeOut = request.strikeOut;
    actual.dirty = FALSE;
}


QFont::Script QFontPrivate::hanHack( const QChar &c )
{
    QFontStruct *f;

    load( QFont::UnifiedHan, TRUE );
    if ( (f = x11data.fontstruct[QFont::UnifiedHan]) != (QFontStruct *) -1 ) {
	Q_ASSERT( f != 0 );
	if ( !f->codec || f->codec->canEncode( c ) )
	    return QFont::UnifiedHan;
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
    load( QFont::UnifiedHanX11, FALSE );
    if ( (f = x11data.fontstruct[QFont::UnifiedHanX11]) != (QFontStruct *) -1 ) {
	Q_ASSERT( f != 0 );
	if ( !f->codec || f->codec->canEncode( c ) )
	    return QFont::UnifiedHanX11;
    }
    return QFont::UnifiedHan;
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
    case QFont::BasicLatin:
	row = 0x00; cell = 0x30; break;
    case QFont::LatinExtendedA:
	row = 0x01; cell = 0x02; break;
    case QFont::LatinExtendedB:
	row = 0x01; cell = 0x80; break;
    case QFont::IPAExtensions:
	row = 0x02; cell = 0x50; break;
    case QFont::LatinExtendedAdditional:
	row = 0x1e; cell = 0x00; break;
    case QFont::LatinLigatures:
	row = 0xfb; cell = 0x00; break;
    case QFont::Diacritical:
	row = 0x03; cell = 0x00; break;
    case QFont::Greek:
	row = 0x03; cell = 0x90; break;
    case QFont::GreekExtended:
	row = 0x1f; cell = 0x00; break;
    case QFont::Cyrillic:
	row = 0x04; cell = 0x10; break;
    case QFont::CyrillicHistoric:
	row = 0x04; cell = 0x60; break;
    case QFont::CyrillicExtended:
	row = 0x04; cell = 0xa0; break;
    case QFont::Armenian:
	row = 0x05; cell = 0x40; break;
    case QFont::Georgian:
	row = 0x10; cell = 0xa0; break;
    case QFont::Runic:
	row = 0x16; cell = 0xa0; break;
    case QFont::Ogham:
	row = 0x16; cell = 0x80; break;
    case QFont::Hebrew:
	row = 0x05; cell = 0xd0; break;
    case QFont::HebrewPresentation:
	row = 0xfb; cell = 0x20; break;
    case QFont::Arabic:
	row = 0x06; cell = 0x30; break;
    case QFont::ArabicPresentationA:
	row = 0xfb; cell = 0x50; break;
    case QFont::ArabicPresentationB:
	row = 0xfe; cell = 0x70; break;
    case QFont::Syriac:
	row = 0x07; cell = 0x10; break;
    case QFont::Thaana:
	row = 0x07; cell = 0x80; break;
    case QFont::Devanagari:
	row = 0x09; cell = 0x10; break;
    case QFont::Bengali:
	row = 0x09; cell = 0x90; break;
    case QFont::Gurmukhi:
	row = 0xa0; cell = 0x10; break;
    case QFont::Gujarati:
	row = 0x0a; cell = 0x90; break;
    case QFont::Oriya:
	row = 0x0b; cell = 0x10; break;
    case QFont::Tamil:
	row = 0x0b; cell = 0x90; break;
    case QFont::Telugu:
	row = 0x0c; cell = 0x10; break;
    case QFont::Kannada:
	row = 0x0c; cell = 0x90; break;
    case QFont::Malayalam:
	row = 0x0d; cell = 0x10; break;
    case QFont::Sinhala:
	row = 0x0d; cell = 0x90; break;
    case QFont::Thai:
	row = 0x0e; cell = 0x10; break;
    case QFont::Lao:
	row = 0xe0; cell = 0x81; break;
    case QFont::Tibetan:
	row = 0x0f; cell = 0x00; break;
    case QFont::Myanmar:
	row = 0x10; cell = 0x00; break;
    case QFont::Khmer:
	row = 0x17; cell = 0x80; break;
    case QFont::UnifiedHan:
	row = 0x4e; cell = 0x00; break;
    case QFont::Hiragana:
	row = 0x30; cell = 0x50; break;
    case QFont::Katakana:
	row = 0x30; cell = 0xb0; break;
    case QFont::Hangul:
	row = 0xac; cell = 0x00; break;
    case QFont::Bopomofo:
	row = 0x31; cell = 0x10; break;
    case QFont::Yi:
	row = 0xa0; cell = 0x00; break;
    case QFont::Ethiopic:
	row = 0x12; cell = 0x00; break;
    case QFont::Cherokee:
	row = 0x13; cell = 0xa0; break;
    case QFont::CanadianAboriginal:
	row = 0x14; cell = 0x10; break;
    case QFont::LatinExtendedA_3:
	row = 0x01; cell = 0x08; break;
    case QFont::LatinExtendedA_4:
	row = 0x01; cell = 0x00; break;
    case QFont::LatinExtendedA_14:
	row = 0x01; cell = 0x74; break;
    case QFont::LatinExtendedA_15:
	row = 0x01; cell = 0x52; break;
    default:
	row = cell = 0;
    }

    ch.cell() = cell;
    ch.row()  = row;
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
		   qt_x11encodings[script][qt_x11indices[script]]);
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

    if (script > QFont::Unicode)
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
    if (tryUnicode && loadUnicode(script, sample))
	return;

    QFontStruct *qfs = 0;
    QCString fontname;
    QTextCodec *codec = 0;
    XFontStruct *xfs = 0;
    bool use_core = TRUE;

#ifndef QT_NO_XFTFREETYPE
    XftFontStruct *xftfs = 0;
    XftPattern *xftmatch = 0;
#endif // QT_NO_XFTFREETYPE

    // Look for font name in fontNameDict based on QFont::key()
    QString k(key() + qt_x11encodings[script][qt_x11indices[script]]);
    QXFontName *qxfn = fontNameDict->find(k);

    if (! qxfn) {
	// if we don't find the name in the dict, we need to find a font name

#ifdef QFONTLOADER_DEBUG_VERBOSE
	qDebug("QFontLoader: getting font name for family %s", request.family.latin1());
#endif

	QString name;
	bool match;

#ifndef QT_NO_XFTFREETYPE
	if (qt_has_xft) {
	    xftmatch = findXftFont(sample);

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
	    //qDebug("QFontLoader: no font name - this must be unicode (%d %s)",
	    //script, qt_x11encodings[script][qt_x11indices[script]]);

	    name = k + "NU";
	}

#ifdef QFONTLOADER_DEBUG_VERBOSE
	qDebug("QFontLoader: putting '%s' (%d) into name dict", name.latin1(),
	       name.length());
#endif

	// Put font name into fontNameDict
	qxfn = new QXFontName(name.latin1(), match);
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
	}

	request.dirty = FALSE;

	return;
    }

#ifndef QT_NO_XFTFREETYPE
    // with XftFreeType support - we always load a font using Unicode, so we never
    // need a codec

    if (xftmatch) {

#ifdef QFONTLOADER_DEBUG
	qDebug("QFontLoader: loading xft font '%s'", fontname.data());
#endif

	xftfs = XftFreeTypeOpen(QPaintDevice::x11AppDisplay(),
				xftmatch);

	XftPatternDestroy(xftmatch);
    }
#endif // QT_NO_XFTFREETYPE

    if (use_core) {
	// if we have no way to map this script, we give up
	if (! qt_x11encodings[script][qt_x11indices[script]]) {

#ifdef QFONTLOADER_DEBUG_VERBOSE
	    qDebug("QFontLoader: no nothing about script %d, giving up", script);
#endif

	    x11data.fontstruct[script] = (QFontStruct *) -1;
	    return;
	}

	// get unicode -> font encoding codec
	if (script < QFont::Unicode) {
	    codec =
		QTextCodec::codecForName(qt_x11encodings[script][qt_x11indices[script]]);

#ifdef QFONTLOADER_DEBUG
	    if (codec) {
		qDebug("QFontLoader: got codec %s for script %d %s",
		       codec->name(), script,
		       qt_x11encodings[script][qt_x11indices[script]]);
	    }
#endif

	    // if we don't have a codec for the font, don't even bother loading it
	    if (! codec) {
#ifdef QFONTLOADER_DEBUG
		qDebug("QFontLoader: no codec for script %d %s",
		       script,
		       qt_x11encodings[script][qt_x11indices[script]]);
#endif

		x11data.fontstruct[script] = (QFontStruct *) -1;
		fontCache->insert(fontname.data(), x11data.fontstruct[script], 1);
		return;
	    }
	}

	// font was never loaded, we need to do that now
#ifdef QFONTLOADER_DEBUG
	qDebug("QFontLoader: %p loading font for %d %s\n\t%s", this,
	       script, qt_x11encodings[script][qt_x11indices[script]],
	       fontname.data());
#endif


	if (! (xfs = XLoadQueryFont(QPaintDevice::x11AppDisplay(),
				    fontname.data()))) {
	    if (! script == QFont::Unicode) {

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
		xfs->max_bounds.width * cost / 8);
    }
#ifndef QT_NO_XFTFREETYPE
    else if (xftfs) {
	cost = request.pointSize * request.weight;
	cost *= (xftfs->max_char - xftfs->min_char) / 8;
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
// QFont static methods
// **********************************************************************

QFont::Script QFontPrivate::defaultScript = QFont::AnyScript;
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

	QTextCodec *codec = QTextCodec::codecForLocale();
	if (codec) {
	    sample = codec->toUnicode(samp);
	}
    }

    if (! sample.isNull() && ! sample.isEmpty()) {
	QFont::Script cs = QFont::AnyScript, tmp;
	const QChar *uc = sample.unicode();
	QFontPrivate *priv = new QFontPrivate;

	for (uint i = 0; i < sample.length(); i++) {
	    tmp = priv->scriptForChar(*uc++);
	    if (tmp != cs && tmp != QFont::AnyScript) {
		cs = tmp;
		break;
	    }
	}
	delete priv;

	if (cs != QFont::AnyScript) {
	    QFontPrivate::defaultScript = cs;
	}
    }

    setlocale(LC_TIME, oldlctime.data());

#ifndef QT_NO_CODECS
#ifndef QT_NO_BIG_CODECS
    (void) new QFontJis0208Codec;
    (void) new QFontKsc5601Codec;
    (void) new QFontGB2312Codec;
    (void) new QFontBig5Codec;
#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS

    // create font cache and name dict
    QFontPrivate::fontCache = new QFontCache();
    Q_CHECK_PTR(QFontPrivate::fontCache);
    cleanup_fontcache.add(QFontPrivate::fontCache);

    fontNameDict = new QFontNameDict(QFontPrivate::fontCache->size());
    Q_CHECK_PTR(fontNameDict);
    fontNameDict->setAutoDelete(TRUE);
    cleanup_fontnamedict.add(fontNameDict);

#ifndef QT_NO_XFTFREETYPE
    qt_has_xft = FALSE;

    if (qt_use_xrender &&
	qApp->settings()->readBoolEntry("/qt/useXft") &&
	XftInit(0) && XftInitFtLibrary()) {
	qt_has_xft = TRUE;
	qt_xft_render_sources = new QPixmapDict();
	cleanup_pixmapdict.add(qt_xft_render_sources);
    }
#endif // QT_NO_XFTFREETYPE

}


/*!
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
  On Windows, this is usually just the family name of a true type
  font. Under X, it is a rather complex XLFD (X Logical Font
  Description). Using the return value of this function is usually \e
  not \e portable.

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
  particular useful under X, where system font settings ( for example
  X resources) are usually available as XLFD (X Logical Font
  Description) only. You can pass an XLFD as \a name to this function.

  In Qt 2.0 and later, a font set with setRawName() is still a
  full-featured QFont. It can be queried (for example with italic())
  or modified (for example with setItalic() ) and is therefore also
  suitable as a basis font for rendering rich text.

  If Qt's internal font database cannot resolve the raw name, the font
  becomes a raw font with \a name as family.

  Note that the present implementation does not handle handle
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


/*!
  Returns TRUE if the font attributes have been changed and the font has to
  be (re)loaded, or FALSE if no changes have been made.
*/
bool QFont::dirty() const
{
    return d->request.dirty;
}


/*!
  Returns the logical pixel height of characters in the font if shown on
  the screen.
*/
int QFont::pixelSize() const
{
    // 360 == .5 for correct rounding
    return (d->request.pointSize * QPaintDevice::x11AppDpiY()) / 720;
}


/*!
  Sets the logical pixel height of characters in the font if shown on
  the screen.
*/
void QFont::setPixelSizeFloat( float pixelSize )
{
    setPointSizeFloat(pixelSize * 72.0 / QPaintDevice::x11AppDpiY());
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
    if (! qfs || qfs == (QFontStruct *) -1) {
	return d->request.pixelSize * 3 / 4;
    }

#ifndef QT_NO_XFTFREETYPE
    XftFontStruct *xftfs = (XftFontStruct *) qfs->xfthandle;
    if (xftfs)
	return xftfs->ascent;
#endif // QT_NO_XFTFREETYPE

    XFontStruct *f = (XFontStruct *) qfs->handle;
    return f->max_bounds.ascent;
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
    if (! qfs || qfs == (QFontStruct *) -1) {
	return 0;
    }

#ifndef QT_NO_XFTFREETYPE
    XftFontStruct *xftfs = (XftFontStruct *) qfs->xfthandle;
    if (xftfs)
	return xftfs->descent;
#endif // QT_NO_XFTFREETYPE

    XFontStruct *f = (XFontStruct *) qfs->handle;
    return f->max_bounds.descent - 1;
}


/*!
  Returns TRUE if \a ch is a valid character in the font.
*/
bool QFontMetrics::inFont(QChar ch) const
{
    return d->inFont( ch );
}


/*!
  Returns the left bearing of character \a ch in the font.

  The left bearing is the rightward distance of the left-most pixel
  of the character from the logical origin of the character.
  This value is negative if the pixels of the character extend
  to the left of the logical origin.

  See width(QChar) for a graphical description of this metric.

  \sa rightBearing(QChar), minLeftBearing(), width()
*/
int QFontMetrics::leftBearing(QChar ch) const
{
    QFont::Script script = d->scriptForChar(ch);

    if (script == QFont::UnknownScript) {
	return 0;
    }

    d->load(script);

    QFontStruct *qfs = d->x11data.fontstruct[script];
    XCharStruct *xcs = getCharStruct(qfs, QString(ch), 0);
    if (! xcs || xcs == (XCharStruct *) -1)
	return 0;
    return xcs->lbearing;
}


/*!
  Returns the right bearing of character \a ch in the font.

  The right bearing is the leftward distance of the right-most pixel
  of the character from the logical origin of a subsequent character.
  This value is negative if the pixels of the character extend
  to the right of the width() of the character.

  See width() for a graphical description of this metric.

  \sa leftBearing(char), minRightBearing(), width()
*/
int QFontMetrics::rightBearing(QChar ch) const
{
    QFont::Script script = d->scriptForChar(ch);

    if (script == QFont::UnknownScript) {
	return 0;
    }

    d->load(script);

    QFontStruct *qfs = d->x11data.fontstruct[script];
    XCharStruct *xcs = getCharStruct(qfs, QString(ch), 0);
    if (! xcs || xcs == (XCharStruct *) -1)
	return 0;
    return xcs->width - xcs->rbearing;
}


/*!
  Returns the minimum left bearing of the font.

  This is the smallest leftBearing(char) of all characters in the font.

  Note that this function can be very slow if the font is big.

  \sa minRightBearing(), leftBearing(char)
*/
int QFontMetrics::minLeftBearing() const
{
    if ( d->actual.lbearing == SHRT_MIN ) {
	d->load(QFontPrivate::defaultScript);

	QFontStruct *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
	if (! qfs || qfs == (QFontStruct *) -1) {
	    return 0;
	}

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

	    d->actual.lbearing = mx;
	} else {
	    d->actual.lbearing = f->min_bounds.lbearing;
	}
    }

    return d->actual.lbearing;
}


/*!
  Returns the minimum right bearing of the font.

  This is the smallest rightBearing(char) of all characters in the
  font.

  Note that this function can be very slow if the font is big.

  \sa minLeftBearing(), rightBearing(char)
*/
int QFontMetrics::minRightBearing() const
{
    if ( d->actual.rbearing == SHRT_MIN ) {
	d->load(QFontPrivate::defaultScript);

	QFontStruct *qfs = d->x11data.fontstruct[QFontPrivate::defaultScript];
	if (! qfs || qfs == (QFontStruct *) -1) {
	    return 0;
	}

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

	    d->actual.rbearing = mx;
	} else {
	    d->actual.rbearing = f->max_bounds.width - f->max_bounds.rbearing;
	}
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
    if (! qfs || qfs == (QFontStruct *) -1) {
	return (d->request.pixelSize * 3 / 4) + 1;
    }

#ifndef QT_NO_XFTFREETYPE
    XftFontStruct *xftfs = (XftFontStruct *) qfs->xfthandle;
    if (xftfs)
	return xftfs->ascent + xftfs->descent;
#endif // QT_NO_XFTFREETYPE

    XFontStruct *f = (XFontStruct *) qfs->handle;
    return f->max_bounds.ascent + f->max_bounds.descent;
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
    if (! qfs || qfs == (QFontStruct *) -1) {
	return 0;
    }

    XFontStruct *f = (XFontStruct *) qfs->handle;
    int l;

#ifndef QT_NO_XFTFREETYPE
    XftFontStruct *xftfs = (XftFontStruct *) qfs->xfthandle;
    if (xftfs)
	l = xftfs->height - (xftfs->ascent + xftfs->descent);
    else
#endif // QT_NO_XFTFREETYPE
	l = f->ascent + f->descent - f->max_bounds.ascent - f->max_bounds.descent;

    return (l > 0) ? l : 0;
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


/*! \overload int QFontMetrics::width( char c ) const

  \obsolete

  Provided to aid porting from Qt 1.x.
*/


/*!
  <img src="bearings.png" align=right> Returns the logical width of a
  \e ch in pixels.  This is a distance appropriate for drawing a
  subsequent character after \e ch.

  Some of the metrics are described in the image to the right.  The
  tall dark rectangle covers the logical width() of a character.  The
  shorter pale rectangles cover leftBearing() and rightBearing() of
  the characters.  Notice that the bearings of "f" in this particular
  font are both negative, while the bearings of "o" are both positive.

  \warning This function will produce incorrect results for arabic characters
  or non spacing marks in the middle of a string , as
  the glyph shaping  and positioning of marks  that happens when processing
  strings can not be taken into account.
  Use \a charWidth() instead if you aren't looking for the width of isolated
  characters.

  \sa boundingRect(), charWidth()
*/
int QFontMetrics::width(QChar ch) const
{
    if ( ch.combiningClass() > 0 ) return 0;

    QFont::Script script = d->scriptForChar(ch);

    if (script == QFont::UnknownScript) {
	return d->request.pixelSize * 3 / 4;
    }

    d->load(script);

    int w = 0;
    QFontStruct *qfs = d->x11data.fontstruct[script];

#ifndef QT_NO_XFTFREETYPE
    XGlyphInfo *xgi = getGlyphInfo(qfs, ch);
    if (xgi != (XGlyphInfo *) -2) {
	if (xgi == (XGlyphInfo *) -1)
	    w = d->request.pixelSize * 3 / 4;
	else if (! xgi)
	    w = 0;
	else
	    w = xgi->xOff;
    } else
#endif // QT_NO_XFTFREETYPE
	{
	    XCharStruct *xcs = getCharStruct(qfs, QString(ch), 0);
	    if (xcs == (XCharStruct *) -1)
		w = d->request.pixelSize * 3 / 4;
	    else if (! xcs)
		w = 0;
	    else
		w = xcs->width;
	}

    return w;
}


/*!
  Returns the width of the character at position \e pos in the string \e str.

  The whole string is needed, as the glyph drawn may change depending on the
  context (the letter before and after the current one) for some languages
  (eg. Arabic).

  Also takes non spacing marks and ligatures into account.
*/
int QFontMetrics::charWidth( const QString &str, int pos ) const
{
    QChar ch = str[pos];
    if ( ch.combiningClass() > 0 ) return 0;

    QFont::Script script = d->scriptForChar(ch);

    if (script == QFont::UnknownScript) {
	return d->request.pixelSize * 3 / 4;
    }

    d->load(script);

    int w = 0;
    QFontStruct *qfs = d->x11data.fontstruct[script];

#ifndef QT_NO_XFTFREETYPE
    XGlyphInfo *xgi = getGlyphInfo(qfs, str, pos);
    if (xgi != (XGlyphInfo *) -2) {
	if (xgi == (XGlyphInfo *) -1)
	    w = d->request.pixelSize * 3 / 4;
	else if (! xgi)
	    w = 0;
	else
	    w = xgi->xOff;
    } else
#endif // QT_NO_XFTFREETYPE
	{
	    XCharStruct *xcs = getCharStruct(qfs, str, pos);
	    if (xcs == (XCharStruct *) -1)
		w = d->request.pixelSize * 3 / 4;
	    else if (! xcs)
		w = 0;
	    else
		w = xcs->width;
	}

    return w;
}


/*!
  Returns the width in pixels of the first \e len characters of \e str.
  If \e len is negative (the default value is), the whole string is used.

  Note that this value is \e not equal to boundingRect().width();
  boundingRect() returns a rectangle describing the pixels this string
  will cover whereas width() r eturns the distance to where the next string
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
  Returns the bounding rectangle of the first \e len characters of \e str,
  which is the set of pixels the text would cover if drawn at (0,0).

  If \e len is negative (default value), the whole string is used.

  Note that the bounding rectangle may extend to the left of (0,0),
  e.g. for italicized fonts, and that the text output may cover \e all
  pixels in the bounding rectangle.

  Newline characters are processed as regular characters, \e not as
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

    XCharStruct overall;

    // zero overall
    overall.lbearing = 0x4000;
    overall.rbearing = -0x4000;
    overall.ascent = -0x4000;
    overall.descent = -0x4000;
    overall.width = 0;

    d->textExtents( shaped, 0, shaped.length(), &overall );

    bool underline;
    bool strikeOut;
    int startX = overall.lbearing;
    int width = overall.rbearing - startX;
    int ascent = overall.ascent;
    int descent = overall.descent;

    if ( painter ) {
	underline = painter->cfont.underline();
	strikeOut = painter->cfont.strikeOut();
    } else {
	underline = underlineFlag();
	strikeOut = strikeOutFlag();
    }

    if ( !underline && !strikeOut ) {
	width = overall.rbearing - startX;
    } else {
	if ( startX > 0 )
	    startX = 0;

	if ( overall.rbearing < overall.width )
	    width =  overall.width - startX;
	else
	    width =  overall.rbearing - startX;

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

    for (int i = 0; i < QFont::NScripts - 1; i++) {
	if (! d->x11data.fontstruct[i]) {
	    continue;
	}

	d->load((QFont::Script) i);

	qfs = d->x11data.fontstruct[i];
	if (! qfs || qfs == (QFontStruct *) -1) {
	    continue;
	}

#ifndef QT_NO_XFTFREETYPE
	if (d->x11data.fontstruct[i]->xfthandle) {
	    XftFontStruct *xftfs =
		(XftFontStruct *) d->x11data.fontstruct[i]->xfthandle;
	    ww = xftfs->max_advance_width;
	} else
#endif // QT_NO_XFTFREETYPE
	    {
		XFontStruct *f =
		    (XFontStruct *) d->x11data.fontstruct[i]->handle;
		ww = f->max_bounds.width;
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
  Returns the distance from the base line to where the strike-out line
  should be drawn.
  \sa underlinePos(), lineWidth()
*/
int QFontMetrics::strikeOutPos() const
{
    int pos = ascent() / 3;

    return pos ? pos : 1;
}


/*!
  Returns the width of the underline and strike-out lines, adjusted for
  the point size of the font.
  \sa underlinePos(), strikeOutPos()
*/
int QFontMetrics::lineWidth() const
{
    // lazy computation of linewidth
    d->computeLineWidth();

    return d->lineWidth;
}
