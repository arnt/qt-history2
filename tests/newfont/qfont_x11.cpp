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

// pull in new headers
#include "qfont.h"
#include "qfontdata_p.h"
#include "qfontinfo.h"
#include "qfontdatabase.h"
#include "qfontmetrics.h"

#include "qwidget.h"
#include "qcache.h"
#include "qdict.h"
#include "qtextcodec.h"
#include "qregexp.h"
#include "qpainter.h"
#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "qt_x11.h"
#include "qmap.h"

#include "qfontencodings_p.h"


// to get which font encodings are installed:
// xlsfonts | egrep -- "^-.*-.*-.*$" | cut -f 14- -d- | sort | uniq

// This array must be kept in sync with the QFontPrivate::Script enum in
// qfontdata_p.h
static char *qt_x11encodings[][QFontPrivate::NScripts + 1] = {
    { "iso8859-1"        , 0 }, // BASICLATIN

    { "iso8859-2"        , 0 }, // EXTLATINA2
    { "iso8859-3"        , 0 }, // EXTLATINA3
    { "iso8859-4"        , 0 }, // EXTLATINA4
    { "iso8859-9"        , 0 }, // EXTLATINA9
    { "iso8859-14"       , 0 }, // EXTLATINA14
    { "iso8859-15"       , 0 }, // EXTLATINA15

    // TODO: Latin Extended-B

    { "iso8859-5",
      "koi8-r",
      "koi8-ru"          , 0 }, // CYRILLIC
    { "iso8859-6"        , 0 }, // ARABIC
    { "iso8859-7"        , 0 }, // GREEK
    { "iso8859-8"        , 0 }, // HEBREW

    { "tscii-*"          , 0 }, // TAMIL
    { "tis620-0",
      "iso8859-11"       , 0 }, // THAI

    { "gb2312.1980-0",
      "big5-0",
      "jisx0208.1983-0",
      "ksc5601.1987-0"   , 0 }, // HAN
    { "jisx0208.1983-0"  , 0 }, // HIRAGANA
    { "jisx0208.1983-0"  , 0 }, // KATAKANA
    { "ksc5601.1987-0"   , 0 }, // HANGUL
    { "gb2312.1980-0"    , 0 }, // BOPOMOFO

    { "iso10646-1",
      "unicode-*"        , 0 }, // UNICODE == ISO-10646-1 (for now)

    { "*-*"              , 0 } // UnknownScript
};


static int qt_x11indices[QFontPrivate::NScripts + 1] = {
    0, // BASICLATIN

    0, // EXTLATINA2
    0, // EXTLATINA3
    0, // EXTLATINA4
    0, // EXTLATINA9
    0, // EXTLATINA14
    0, // EXTLATINA15

    0, // CYRILLIC
    0, // ARABIC
    0, // GREEK
    0, // HEBREW

    0, // TAMIL
    0, // THAI

    0, // HAN
    0, // HIRAGANA
    0, // KATAKANA
    0, // HANGUL
    0, // BOPOMOFO

    0, // UNICODE

    0  // UnknownScript
};








// **********************************************************************
// QFontCache
// **********************************************************************
static const int qtReserveCost = 1024*100;
static const int qtFontCacheSize = 1024*1024*4;
static const int qtFontCacheCount = 61;


typedef QDictIterator<QFontStruct> QFontDictIt;
class QFontDict : public QDict<QFontStruct>
{
public:
    QFontDict(int size = 29)
	: QDict<QFontStruct>(size)
    { ; }

    void deleteItem(Item);
};


void QFontDict::deleteItem(Item d)
{
    QFontStruct *qfs = (QFontStruct *) d;

    // don't try to delete negative cache items
    if (qfs == (QFontStruct *) -1) {
	return;
    }

    if (qfs->count == 0 ||
	qfs->deref()) {
    	delete qfs;
    }
}


typedef QCacheIterator<QFontStruct> QFontCacheIt;
class QFontCache : public QCache<QFontStruct>
{
public:
    QFontCache(int maxCost, int size = 29)
	: QCache<QFontStruct>(maxCost, size)
    { ; }

    void deleteItem(Item);
};


void QFontCache::deleteItem(Item d)
{
    QFontStruct *qfs = (QFontStruct *) d;

    // don't try to delete negative cache items
    if (qfs == (QFontStruct *) -1) {
	return;
    }

    if (qfs->count == 0 ||
	qfs->deref()) {
	delete qfs;
    }
}


struct QXFontName
{
    QXFontName( const QCString &n, bool e )
	: name(n), exactMatch(e)
    { ; }

    QCString name;
    bool exactMatch;
};

typedef QDict<QXFontName> QFontNameDict;


// cache of loaded fonts
static QFontCache *fontCache = 0;
 // dict of all loaded fonts
static QFontDict *fontDict = 0;
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
    qDebug("QFS::~QFS: deleted with count %d", count);

    if (fontCache) {
	(void) fontCache->take(name);
    }

    if (fontDict) {
	(void) fontDict->take(name);
    }

    if (handle) {
	XFreeFont(QPaintDevice::x11AppDisplay(), (XFontStruct *) handle);
	handle = 0;
    }
}


bool QFontStruct::deref()
{
    if (count == 0) {
	qDebug("QFS::deref: NEGATIVE REFCOUNT!  WHAT IS WRONG WITH BIDI TT FONTS?");
	// int *foo = 0; *foo = 0;
    }

    return QShared::deref();
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
    "6x13",
    "7x13",
    "8x13",
    "9x15",
    "fixed",
    0
};

QString QFontPrivate::lastResortFont() const
{
    // qDebug("QFontPrivate::lastResortFont");

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
    qFatal( "QFont::lastResortFont: Cannot find any reasonable font" );
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
    QCString buffer = xlfd;
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
	fd->pointSize = (int) ((px * 720 + QPaintDevice::x11AppDpiY() / 2) /
			       QPaintDevice::x11AppDpiY());
    }

    fd->underline     = FALSE;
    fd->strikeOut     = FALSE;
    fd->hintSetByUser = FALSE;
    fd->rawMode       = FALSE;
    fd->dirty         = FALSE;
    return TRUE;
}


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


// **********************************************************************
// QFontPrivate member methods
// **********************************************************************

// Scoring constants
#define exactScore           0xfffe
#define exactNonUnicodeScore 0xffff

#define PitchScore	     0x40
#define SizeScore	     0x20
#define ResolutionScore	     0x10
#define WeightScore	     0x08
#define SlantScore	     0x04
#define WidthScore	     0x02
#define NonUnicodeScore	     0x01

// Returns an XLFD for the font and sets exact to TRUE if the font found matches
// the font queried
QCString QFontPrivate::findFont(QFontPrivate::Script script, bool *exact) const
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
	    if (! qt_x11encodings[script][++qt_x11indices[script]]) {
		qt_x11indices[script] = 0;
	    }

	    if (qt_x11indices[script] == start_index) {
		done = TRUE;
	    }
	} else {
	    done = TRUE;
	}
    }

    if ( score < exactScore )
	*exact = FALSE;

    // qDebug("QFP::findFont: try 1 %s", (const char *) bestName);

    if (script == QFontPrivate::UNICODE) {
	return bestName;
    } else if (! bestName.isNull()) {
	return bestName;
    }

    // try substitution
    QStringList list = QFont::substitutes( request.family );
    QStringList::Iterator sit = list.begin();

    while (sit != list.end() && bestName.isNull()) {
	if (familyName != *sit) {
	    done = FALSE;
	    qt_x11indices[script] = start_index;

	    while (! done) {
		bestName = bestFamilyMember(script, foundry, *sit, &score);

		if (bestName.isNull()) {
		    if (! qt_x11encodings[script][++qt_x11indices[script]]) {
			qt_x11indices[script] = 0;
		    }

		    if (qt_x11indices[script] == start_index) {
			done = TRUE;
		    }
		} else {
		    done = TRUE;
		}
	    }
	}

	++sit;
    }

    // qDebug("QFP::findFont: try 2 %s", (const char *) bestName);

    if (! bestName.isNull()) return bestName;

    // try default family for style
    QString f = defaultFamily();

    if ( familyName != f ) {
	familyName = f;
	done = FALSE;
	qt_x11indices[script] = start_index;

	while (! done) {
	    bestName = bestFamilyMember(script, foundry, familyName, &score);

	    if (bestName.isNull()) {
		if (! qt_x11encodings[script][++qt_x11indices[script]]) {
		    qt_x11indices[script] = 0;
		}

		if (qt_x11indices[script] == start_index) {
		    done = TRUE;
		}
	    } else {
		done = TRUE;
	    }
	}
    }

    // qDebug("QFP::findFont: try 3 %s", (const char *) bestName);

    if (! bestName.isNull()) return bestName;

    // try system default family
    f = lastResortFamily();

    if ( familyName != f ) {
	familyName = f;
	done = FALSE;
	qt_x11indices[script] = start_index;

	while (! done) {
	    bestName = bestFamilyMember(script, foundry, familyName, &score);

	    if (bestName.isNull()) {
		if (! qt_x11encodings[script][++qt_x11indices[script]]) {
		    qt_x11indices[script] = 0;
		}

		if (qt_x11indices[script] == start_index) {
		    done = TRUE;
		}
	    } else {
		done = TRUE;
	    }
	}
    }

    // qDebug("QFP::findFont: try 4 %s", (const char *) bestName);

    if (! bestName.isNull()) return bestName;

    // try *any* family
    f = "*";

    if (familyName != f) {
	familyName = f;
	done = FALSE;
	qt_x11indices[script] = start_index;

	while (! done) {
	    bestName = bestFamilyMember(script, foundry, familyName, &score);

	    if (bestName.isNull()) {
		if (! qt_x11encodings[script][++qt_x11indices[script]]) {
		    qt_x11indices[script] = 0;
		}

		if (qt_x11indices[script] == start_index) {
		    done = TRUE;
		}
	    } else {
		done = TRUE;
	    }
	}
    }

    // qDebug("QFP::findFont: try 5 %s", (const char *) bestName);

    // no matching fonts found
    if (bestName.isNull()) {
	bestName = lastResortFont().latin1();
    }

    // qDebug("QFP::findFont: done trying %s", (const char *) bestName);

    return bestName;
}


QCString QFontPrivate::bestFamilyMember(QFontPrivate::Script script,
					const QString& foundry,
					const QString& family,
					int *score ) const
{
    const int prettyGoodScore = SizeScore | WeightScore | SlantScore | WidthScore;

    int testScore = 0;
    QCString testResult;
    int bestScore = 0;
    QCString result;

    if ( !foundry.isEmpty() ) {
	QString pattern
	    = "-" + foundry + "-" + family + "-*-*-*-*-*-*-*-*-*-*-" +
	    (qt_x11encodings[script])[(qt_x11indices[script])];
	result = bestMatch(pattern.latin1(), &bestScore);
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
	    testResult = bestMatch( pattern.latin1(), &testScore );
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

QCString QFontPrivate::bestMatch( const char *pattern, int *score ) const
{
    QFontMatchData best;
    QFontMatchData bestScalable;

    QCString	matchBuffer( 256 );	// X font name always <= 255 chars
    char **	xFontNames;
    int		count;
    int		sc;
    float	pointDiff;	// difference in % from requested point size
    int		weightDiff;	// difference from requested weight
    bool	scalable       = FALSE;
    bool	smoothScalable = FALSE;
    int		i;

    xFontNames = getXFontNames( pattern, &count );

    for( i = 0; i < count; i++ ) {
	sc = fontMatchScore( xFontNames[i], matchBuffer,
			     &pointDiff, &weightDiff,
			     &scalable, &smoothScalable );

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
		pSize = request.pointSize; // deciPointSize();
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
				   bool	 *scalable     , bool *smoothScalable ) const
{
    char *tokens[NFontFields];
    bool   exactMatch = TRUE;
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
    if ( request.fixedPitch ) {
	if ( pitch == 'm' || pitch == 'c' )
	    score |= PitchScore;
	else
	    exactMatch = FALSE;
    } else {
	if ( pitch != 'p' )
	    exactMatch = FALSE;
    }

    float diff;
    if ( *scalable ) {
	diff = 9.0;	// choose scalable font over >= 0.9 point difference
	score |= SizeScore;
	exactMatch = FALSE;
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

	if ( percentDiff < 20 ) {
	    score |= SizeScore;

	    if ( pSize != request.pointSize ) {
		exactMatch = FALSE;
	    }
	} else {
	    exactMatch = FALSE;
	}
    }

    if ( pointSizeDiff )
	*pointSizeDiff = diff;

    int weightVal = getFontWeight(tokens[Weight], TRUE);
    if ( weightVal == (int) request.weight )
	score |= WeightScore;
    else
	exactMatch = FALSE;

    *weightDiff = QABS( weightVal - (int) request.weight );
    char slant = tolower( tokens[Slant][0] );

    if ( request.italic ) {
	if ( slant == 'o' || slant == 'i' )
	    score |= SlantScore;
	else
	    exactMatch = FALSE;
    } else {
	if ( slant == 'r' )
	    score |= SlantScore;
	else
	    exactMatch = FALSE;
    }

    if ( qstricmp( tokens[Width], "normal" ) == 0 )
	score |= WidthScore;
    else
	exactMatch = FALSE;

    return exactMatch ? (exactScore | (score&NonUnicodeScore)) : score;
}


// Computes the line width (underline,strikeout) for the X font
// and fills in the X resolution of the font.
void QFontPrivate::computeLineWidth(QFontPrivate::Script script)
{
    int nlw;

    // X font name always <= 255 chars
    QCString buffer(256);
    char *tokens[NFontFields];
    qstrcpy( buffer.data(), x11data.fontstruct[script]->name);

    if (! parseXFontName(buffer, tokens)) {
	// name did not conform to X LFD
 	return;
    }

    int weight = getFontWeight(tokens[Weight]);
    int pSize  = atoi(tokens[PointSize]) / 10;
    int ry = atoi(tokens[ResolutionY]);

    if ( ry != QPaintDevice::x11AppDpiY() )
	pSize = ((2 * pSize * ry) + QPaintDevice::x11AppDpiY()) /
		(QPaintDevice::x11AppDpiY() * 2);

    /*
      QCString tmp = tokens[ResolutionX];
      bool ok;
      xres = tmp.toInt( &ok );

      if ( !ok || xres == 0 )
      xres = QPaintDevice::x11AppDpiX();
    */

    // ad hoc algorithm
    int score = pSize*weight;
    nlw = ( score ) / 700;

    // looks better with thicker line for small pointsizes
    if ( nlw < 2 && score >= 1050 ) nlw = 2;
    if ( nlw == 0 ) nlw = 1;

    if (nlw > lineWidth) lineWidth = nlw;
}


// fill the actual fontdef with data from the loaded font
void QFontPrivate::initFontInfo(QFontPrivate::Script script)
{
    if (! actual.dirty) return;

    actual.lbearing = SHRT_MIN;
    actual.rbearing = SHRT_MIN;
    computeLineWidth(script);

    if (exactMatch) {
	actual = request;
	actual.dirty = FALSE;
	return;
    }

    if (! fillFontDef(x11data.fontstruct[script]->name, &actual, 0)) {
	// zero fontdef
	actual.pointSize = 0;
	actual.styleHint = QFont::AnyStyle;
	actual.weight = QFont::Normal;
	actual.italic = FALSE;
	actual.underline = FALSE;
	actual.strikeOut = FALSE;
	actual.fixedPitch = FALSE;
	actual.hintSetByUser = FALSE;
	actual.lbearing = SHRT_MIN;
	actual.rbearing = SHRT_MIN;

	actual.family = QString::fromLatin1(x11data.fontstruct[script]->name);
	actual.rawMode = TRUE;
	exactMatch = FALSE;
	return;
    }

    actual.underline = request.underline;
    actual.strikeOut = request.strikeOut;
}


// Loads the font for the specified script
#define MAXINDEX(f) \
    ((((f)->max_byte1 - (f)->min_byte1) * \
      ((f)->max_char_or_byte2 - (f)->min_char_or_byte2 + 1)) + \
     (f)->max_char_or_byte2 - (f)->min_char_or_byte2)

void QFontPrivate::load(QFontPrivate::Script script, bool tryUnicode)
{
    // Make sure fontCache is initialized
    if (! fontCache) {

#if defined(CHECK_STATE)
	qFatal( "QFont: Must construct a QApplication before a QFont" );
#endif

	return;
    }

    if (x11data.fontstruct[script] && ! request.dirty) {
	return;
    }

    if (request.dirty) {
	if (x11data.fontstruct[script] &&
	    x11data.fontstruct[script] != (QFontStruct *) -1) {
	    // only deref here... we will let the cache take care of cleaning up
	    // while the application is running
	    if (x11data.fontstruct[script] !=
		x11data.fontstruct[QFontPrivate::UNICODE]) {
		x11data.fontstruct[script]->deref();
	    }

	    x11data.fontstruct[script] = 0;
	}

	// dirty unicode also if the font is dirty
	if (x11data.fontstruct[QFontPrivate::UNICODE] &&
	    x11data.fontstruct[QFontPrivate::UNICODE] != (QFontStruct *) -1) {
	    // only deref here... we will let the cache take care of cleaning up
	    // while the application is running
	    x11data.fontstruct[QFontPrivate::UNICODE]->deref();
	    x11data.fontstruct[QFontPrivate::UNICODE] = 0;
	}
    }

    // look for a unicode font first, and see if it has the script that we want...
    if (tryUnicode) {
	if (! x11data.fontstruct[QFontPrivate::UNICODE]) {
	    // qDebug("QFontPrivate::load: trying to load unicode font");
	    load(QFontPrivate::UNICODE, FALSE);
	}

	if (x11data.fontstruct[QFontPrivate::UNICODE] != (QFontStruct *) -1) {
	    bool hasChar = FALSE;
	    QChar ch;

	    XFontStruct *f =
		(XFontStruct *) x11data.fontstruct[QFontPrivate::UNICODE]->handle;

	    if (f->per_char) {
		XCharStruct *xcs = f->per_char + 0xfffe;

		if (xcs && xcs->width + xcs->ascent + xcs->descent == 0) {
		    uchar row, cell;

		    switch (script) {
		    case QFontPrivate::BASICLATIN:  row  = 0x00; cell = 0x30; break;
		    case QFontPrivate::EXTLATINA2:  row  = 0x01; cell = 0x02; break;
		    case QFontPrivate::EXTLATINA3:  row  = 0x01; cell = 0x08; break;
		    case QFontPrivate::EXTLATINA4:  row  = 0x01; cell = 0x00; break;
		    case QFontPrivate::EXTLATINA9:  row  = 0x01; cell = 0x00; break;
		    case QFontPrivate::EXTLATINA14: row  = 0x01; cell = 0x74; break;
		    case QFontPrivate::EXTLATINA15: row  = 0x01; cell = 0x52; break;

		    case QFontPrivate::CYRILLIC:    row  = 0x04; cell = 0x10; break;
		    case QFontPrivate::ARABIC:      row  = 0x06; cell = 0x30; break;
		    case QFontPrivate::GREEK:       row  = 0x03; cell = 0x90; break;
		    case QFontPrivate::HEBREW:      row  = 0x05; cell = 0xd0; break;

		    case QFontPrivate::TAMIL:       row  = 0x0b; cell = 0x80; break;
		    case QFontPrivate::THAI:        row  = 0x0e; cell = 0x00; break;

		    case QFontPrivate::HAN:         row  = 0x4e; cell = 0x00; break;
		    case QFontPrivate::HIRAGANA:    row  = 0x30; cell = 0x50; break;
		    case QFontPrivate::KATAKANA:    row  = 0x30; cell = 0xb0; break;
		    case QFontPrivate::HANGUL:      row  = 0xac; cell = 0x00; break;
		    case QFontPrivate::BOPOMOFO:    row  = 0x31; cell = 0x10; break;

		    default:                 row  = cell = 0;
		    }

		    if (row + cell != 0) {
			ch.cell() = cell;
			ch.row()  = row;
			xcs = f->per_char + ch.unicode();

			hasChar = (xcs && xcs->width + xcs->ascent + xcs->descent != 0);
		    }
		}
	    }

	    if (hasChar) {
		// qDebug("QFontPrivate::load: %p unicode font has char "
		// "0x%04d for %d %s",
		// this, ch.unicode(), script, qt_x11encodings[script]);

		x11data.fontstruct[script] = x11data.fontstruct[QFontPrivate::UNICODE];
		// x11data.fontstruct[script]->ref();

		// x11data.fontName[script] = x11data.fontName[QFontPrivate::UNICODE];

		// x11data.codec[script] = 0;
		request.dirty = FALSE;

		return;
		// } else {
		// qDebug("QFontPrivate::load: unicode font doesn't have char 0x%04d",
		// ch.unicode());
	    }
	    // } else {
	    // qDebug("QFontPrivate::load: unicode font tried, but doesn't exist");
	}
	// } else {
	// qDebug("QFontPrivate::load: not trying Unicode font");
    }

    // Look for font name in fontNameDict based on QFont::key()
    QString k(key() + (qt_x11encodings[script])[(qt_x11indices[script])]);
    QXFontName *qxfn = fontNameDict->find(k);

    if (! qxfn) {
	// if we don't find the name in the dict, we need to find a font name
	// qDebug("QFont::load: getting font name");

	QString name;
	bool match;

	if (request.rawMode) {
	    name = QFont::substitute(request.family);
	    match = fontExists(name);

	    if (! match) {
		name = lastResortFont();
	    }
	} else {
	    name = findFont(script, &match);
	}

	if (name.isNull()) {
	    // no font name... this can only happen with Unicode
	    // qDebug("QFontPrivate::load: no font name - this must be unicode (%d %s)",
	    // script, qt_x11encodings[script]);

	    // qDebug("negative caching here!");

	    name = k + "NU";
	    // x11data.fontstruct[script] = (QFontStruct *) -1;
	    // return;
	}

	// qDebug("QFontPrivate::load: putting '%s' into name dict", name.latin1());
	// Put font name into fontNameDict
	qxfn = new QXFontName(name.latin1(), match);
	Q_CHECK_PTR(qxfn);
	fontNameDict->insert(k, qxfn);
    }

    // qDebug("QFont::load: using name '%s'", (const char *) qxfn->name);
    exactMatch = qxfn->exactMatch;

    QCString n(qxfn->name);

    // qDebug("QFontPrivate::load: name is '%s'", (const char *) n);

    // Look in fontCache for font
    QFontStruct *qfs = fontCache->find(n.data());

    if (! qfs) {
	// Look for font in fontDict (which means that the font is out of the cache,
	// but still exists because something is holding a reference to it)... this
	// is not the way I want to do this, but unfortunately QCache doesn't grok
	// QShared subclasses (the cache will delete items even if something is still
	// holding references)
	qfs = fontDict->find(n.data());
    }

    if (qfs) {
	// Found font in either cache or dict...
	if (qfs != (QFontStruct *) -1)
	    qfs->ref();
	x11data.fontstruct[script] = qfs;

	return;
    }

    // font was never loaded, we need to do that now
    XFontStruct *f = 0;

    // qDebug("QFontPrivate::load: %p loading font for %d %s\n\t%s", this,
    // script, qt_x11encodings[script], (const char *) n);
    // qDebug("QFP::load: %s", (const char *) n);

    if (! (f = XLoadQueryFont(QPaintDevice::x11AppDisplay(),
			      (const char *) n))) {
	if (! script == QFontPrivate::UNICODE) {
	    // qDebug("QFontPrivate::load: load failed, trying last resort");
	    exactMatch = FALSE;

	    if (! (f = XLoadQueryFont(QPaintDevice::x11AppDisplay(),
				      lastResortFont().latin1()))) {
		qFatal("QFontPrivate::load: Internal error");
	    }
	} else {
	    // Didn't get unicode font, set to sentinel and return
	    // qDebug("no unicode font, doing negative caching");
	    x11data.fontstruct[script] = (QFontStruct *) -1;

	    fontCache->insert((const char *) n, x11data.fontstruct[script], 1);

	    return;
	}
    }

    // Adjust cost of this item in the fontCache (and adjust the maxcost of
    // the cache if necessary)
    int chars = MAXINDEX(f);
    if ( chars > 2000 ) {
	// If the user is using large fonts, we assume they have
	// turned on the Xserver option deferGlyphs, and that they
	// have more memory available to the server.
	chars = 2000;
    }

    int size = (f->max_bounds.ascent + f->max_bounds.descent) *
	       f->max_bounds.width * chars / 8;

    // If we get a cache overflow, we make room for this font only
    if (size > fontCache->maxCost() + qtReserveCost)
	fontCache->setMaxCost(size + qtReserveCost);

    qfs = new QFontStruct((Qt::HANDLE) f, n);
    x11data.fontstruct[script] = qfs;

#warning "TODO: codec matching should be done better"
    // get unicode -> font encoding codec
    if (script < QFontPrivate::UNICODE) {
	qfs->codec =
	    QTextCodec::codecForName((qt_x11encodings[script])[(qt_x11indices[script])]);
    }

    // if (qfs->codec) {
    // qDebug("QFP::load: got codec %s for script %d %s",
    // qfs->codec->name(), script,
    // qt_x11encodings[script][(qt_x11indices[script])]);
    // }

    request.dirty = FALSE;
    initFontInfo(script);

    // Insert font into the font cache and font dict
    bool inserted;

    inserted = fontCache->insert(qfs->name, qfs, size);

#if defined(CHECK_STATE)
    if (! inserted)
	qFatal("QFont::load: font cache overflow error");
#endif

    fontDict->insert(qfs->name, qfs);
}








// **********************************************************************
// QFont static methods
// **********************************************************************

QFontPrivate::Script QFontPrivate::defaultScript = QFontPrivate::AnyScript;

/*!
  Internal function that initializes the font system.

  \internal
  The font cache and font dict do not alloc the keys. The key is a QString
  which is shared between QFontPrivate and QXFontName.
*/
void QFont::initialize()
{
    if (QFontPrivate::NScripts != NSCRIPTSEGCSHACK) {
	qFatal("The QFontPrivate::Script enum has changed, but the x11data.fontstruct\n"
	       "member array size wasn't updated");
    }
    
    QCString oldlctime = setlocale(LC_TIME, 0);
    QCString lctime = setlocale(LC_TIME, "");

    // qDebug("QFont::initialize: set LC_TIME locale (%s %s)",
    // (const char *) oldlctime, (const char *) lctime);

    time_t ttmp = time(NULL);
    struct tm *tt = 0;
    char samp[64];
    QString sample;

    // qDebug("QFont::initialize: getting text sample for locale with strftime");
    if (ttmp != -1 && (tt = localtime(&ttmp)) != 0 &&
	strftime(samp, 64, "%A%B", tt) > 0) {
	// qDebug("QFont::initialize: sample '%s' retrieved", samp);

	QTextCodec *codec = QTextCodec::codecForLocale();
	if (codec) {
	    // qDebug("QFont::initialize: using codec '%s'", codec->name());
	    sample = codec->toUnicode(samp);
	    // qDebug("QFont::initialize: first char of sample: U+%02x%02x",
	    // sample[0].row(), sample[0].cell());
	}
    }

    if (! sample.isNull() && ! sample.isEmpty()) {
	QFontPrivate::Script cs = QFontPrivate::AnyScript, tmp;
	const QChar *uc = sample.unicode();

	for (uint i = 0; i < sample.length(); i++) {
	    tmp = QFontPrivate::scriptForChar(*uc++);
	    if (tmp != cs && tmp != QFontPrivate::AnyScript) {
		cs = tmp;
		break;
	    }
	}

	if (cs != QFontPrivate::AnyScript) {
	    // qDebug("QFont::initialize: got text sample successfully (%d %s)",
	    // cs, qt_x11encodings[cs]);

	    QFontPrivate::defaultScript = cs;
	}
    }

    // qDebug("QFont::initialize: restoring LC_TIME locale");
    setlocale(LC_TIME, (const char *) oldlctime);

    // qDebug("QFont::initialize: creating font cache");
    if (! fontCache) {
	fontCache = new QFontCache(qtFontCacheSize, qtFontCacheCount);
	Q_CHECK_PTR(fontCache);

	fontDict  = new QFontDict(qtFontCacheCount);
	Q_CHECK_PTR(fontDict);

	fontNameDict = new QFontNameDict(qtFontCacheCount);
	Q_CHECK_PTR(fontNameDict);
	fontNameDict->setAutoDelete(TRUE);
    }

    // qDebug("QFont::initialize: creating font codecs");

#ifndef QT_NO_CODECS
    (void) new QFontJis0208Codec;
    (void) new QFontKsc5601Codec;
    (void) new QFontGB2312Codec;
    (void) new QFontBig5Codec;
#endif

}


/*!
  Internal function that cleans up the font system.
*/
void QFont::cleanup()
{
    // cleanout negative cache items before deleting cache
    QFontCacheIt cit(*fontCache);
    QFontDictIt dit(*fontDict);
    QString ckey;
    QFontStruct *qfs;
    while ((qfs = cit.current())) {
	ckey = cit.currentKey();
	++cit;

	if (qfs == (QFontStruct *) -1) {
	    fontCache->take(ckey);
	}
    }

    while ((qfs = dit.current())) {
	ckey = dit.currentKey();
	++dit;

	if (qfs == (QFontStruct *) -1) {
	    fontDict->take(ckey);
	}
    }

    // delete cache, dict and namedict
    fontCache->setAutoDelete(TRUE);
    delete fontCache;
    fontCache = 0;


    fontDict->setAutoDelete(TRUE);
    delete fontDict;
    fontDict = 0;

    delete fontNameDict;
    fontNameDict = 0;
}


/*!
  Internal function that dumps font cache statistics.
*/
void QFont::cacheStatistics()
{
#if defined(DEBUG)
    fontCache->statistics();

    qDebug("QFont::cacheStatistics: not fully implemented");

    /*
      QFontCacheIt it(*fontCache);
      QFontPrivate *fin;
      qDebug( "{" );
      while ( (fin = it.current()) ) {
      ++it;
      qDebug( "   [%s]", fin->name() );
      }
      qDebug( "}" );
    */
#endif
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
    // qDebug("QFont::handle: d %p", d);

    d->load(QFontPrivate::defaultScript);

#ifndef QT_NO_COMPAT
    // qDebug("QFont::handle: checking charset for !QT_NO_COMPAT");
    // qDebug("QFont::handle: d->charset is %d '%s'", d->charset,
    // qt_x11encodings[d->charset]);

    // if the charset has been set, then we return the handle for that font
    // if (d->charset != QFont::AnyScript) {
    // qDebug("font charset is %d '%s'", d->charset, qt_x11encodings[d->charset]);

    // if (d->x11data.fontstruct[d->charset]) {
    // qDebug("QFont::handle: charset has been set, returning that font id");
    // return ((XFontStruct *) d->x11data.fontstruct[d->charset]->handle)->fid;
    // } else {
    // qDebug("QFont::handle: charset has been set, but font isn't loaded");
    // return 0;
    // }
    // } else {
    // qDebug("QFont::handle: charset not set");
    // }
#endif

    // qDebug("QFont::handle: looking for fontstruct");
    // find the first font id and return that
    for (int i = 0; i < QFontPrivate::NScripts; i++) {
	if (d->x11data.fontstruct[i]) {
	    // qDebug("QFont::handle: returning font id for script %d '%s'",
	    // i, qt_x11encodings[i]);
	    d->load((QFontPrivate::Script) i);
	    return ((XFontStruct *) d->x11data.fontstruct[i]->handle)->fid;
	    // } else {
	    // qDebug("QFont::handle: no font id for script %d '%s'",
	    // i, qt_x11encodings[i]);
	}
    }

    // no font ids in the font, so we return an invalid handle
    qDebug("QFont::handle: no font ids in font, returning invalid handle");
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
    /*    if ( DIRTY_FONT )
	load();

    return QString::fromLatin1(d->priv->name());
    */

    return QString::null;
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
#if defined(CHECK_STATE)
	qWarning("QFont::setRawMode(): Invalid XLFD: \"%s\"", name.latin1());
#endif

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
    return ((d->request.pointSize * QPaintDevice::x11AppDpiY()) + 360) / 720;
}


/*!
  Sets the logical pixel height of characters in the font if shown on
  the screen.
*/
void QFont::setPixelSizeFloat( float pixelSize )
{
    setPointSizeFloat(pixelSize * 72.0 / QPaintDevice::x11AppDpiY());
}






#define printerAdjusted(a) (a)

// **********************************************************************
// QFontMetrics member methods
// **********************************************************************

// int QFontMetrics::printerAdjusted(int val) const
// {
    /*
      if ( painter && painter->device() &&
      painter->device()->devType() == QInternal::Printer) {
      painter->cfont.handle();

      // ### printer font metrics hack
      if ( painter->device() &&
      painter->device()->devType() == QInternal::Printer &&
      painter->cfont.d->printerHackFont ) {
      painter->cfont.d->printerHackFont->handle();
      val *= painter->cfont.pointSize() / 64;
      }
      }
    */
//
// return val;
// }


/*!
  Returns the maximum ascent of the font.

  The ascent is the distance from the base line to the uppermost line
  where pixels may be drawn.

  \sa descent()
*/
int QFontMetrics::ascent() const
{
    XFontStruct *f;
    int a = 0, aa;
    for (int i = 0; i < QFontPrivate::NScripts - 1; i++) {
	if (! d->x11data.fontstruct[i]) continue;

	f = (XFontStruct *) d->x11data.fontstruct[i]->handle;
	aa = f->max_bounds.ascent;
	if (aa > a)
	    a = aa;
    }

    return printerAdjusted(a);
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
    XFontStruct *f;
    int c = 0, cc;
    for (int i = 0; i < QFontPrivate::NScripts - 1; i++) {
	if (! d->x11data.fontstruct[i]) continue;

	f = (XFontStruct *) d->x11data.fontstruct[i]->handle;
	cc = f->max_bounds.descent - 1;
	if (cc > c)
	    c = cc;
    }

    return printerAdjusted(c);
}


/*!
  Returns TRUE if \a ch is a valid character in the font.
*/
bool QFontMetrics::inFont(QChar ch) const
{
    QFontPrivate::Script script = QFontPrivate::scriptForChar(ch);

    if (script == QFontPrivate::UnknownScript)
	return FALSE;

    // if (! d->x11data.fontstruct[script]) {
    d->load(script);

    if (! d->x11data.fontstruct[script])
	return FALSE;
    // }

    XFontStruct *f = (XFontStruct *) d->x11data.fontstruct[script]->handle;

    if (f->max_byte1) {
        return (ch.cell() >= f->min_char_or_byte2 &&
		ch.cell() <= f->max_char_or_byte2 &&
		ch.row() >= f->min_byte1 &&
		ch.row() <= f->max_byte1);
    } else if (ch.row()) {
        uint ch16 = ch.unicode();
        return (ch16 >= f->min_char_or_byte2 &&
		ch16 <= f->max_char_or_byte2);
    }

    return (ch.cell() >= f->min_char_or_byte2 &&
	    ch.cell() <= f->max_char_or_byte2);
}


static
XCharStruct* charStr(const QTextCodec* codec, XFontStruct *f, QChar ch)
{
    // Optimized - inFont() is merged in here.

    if ( !f->per_char ) {
	// qDebug("charStr: returning max bounds");
	return &f->max_bounds;
    }

    if (codec) {
	/*
	  int l = 1;
	  QCString c = mapper->fromUnicode(ch,l);
	  // #### What if c.length()>1 ?
	  if (c.length() > 1)
	  ch = QChar((unsigned
	*/

	// qDebug("charStr: using codec before: 0x%04x", ch.unicode());
	ch = QChar(codec->characterFromUnicode(ch));
	// qDebug("charStr: using codec after: 0x%04x", ch.unicode());
    }

    if ( f->max_byte1 ) {
	// qDebug("charStr: multi row font");

	if ( !(ch.cell() >= f->min_char_or_byte2
	       && ch.cell() <= f->max_char_or_byte2
	       && ch.row() >= f->min_byte1
	       && ch.row() <= f->max_byte1) ) {
	    // qDebug("charStr: char not in font, using default");
	    ch = QChar((ushort)f->default_char);
	    // qDebug("charStr: after: 0x%04x", ch.unicode());
	}
	return f->per_char +
	    ((ch.row() - f->min_byte1)
	     * (f->max_char_or_byte2 - f->min_char_or_byte2 + 1)
	     + ch.cell() - f->min_char_or_byte2);
    } else if ( ch.row() ) {
	// qDebug("charStr: single row and char has row range");

	uint ch16 = ch.unicode();
	if ( !(ch16 >= f->min_char_or_byte2
	       && ch16 <= f->max_char_or_byte2) )
	    ch16 = f->default_char;
	return f->per_char + ch16;
    }

    // qDebug("charStr: single row font, single row char");

    if ( !( ch.cell() >= f->min_char_or_byte2
	    && ch.cell() <= f->max_char_or_byte2) )
	ch = QChar((uchar)f->default_char);
    return f->per_char + ch.cell() - f->min_char_or_byte2;
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
    QFontPrivate::Script script = QFontPrivate::scriptForChar(ch);

    if (script == QFontPrivate::UnknownScript) return 0;

    // if (! d->x11data.fontstruct[script]) {
    d->load(script);

    if (! d->x11data.fontstruct[script])
	return 0;
    // }

    XCharStruct *xcs =
	charStr(d->x11data.fontstruct[script]->codec,
		((XFontStruct *) d->x11data.fontstruct[script]->handle), ch);
    return printerAdjusted(xcs ? xcs->width : 0);
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
    QFontPrivate::Script script = QFontPrivate::scriptForChar(ch);

    if (script == QFontPrivate::UnknownScript) return 0;

    if (! d->x11data.fontstruct[script]) {
	d->load(script);

	if (! d->x11data.fontstruct[script])
	    return 0;
    }

    XCharStruct *xcs =
	charStr(d->x11data.fontstruct[script]->codec,
		((XFontStruct *) d->x11data.fontstruct[script]->handle), ch);
    return printerAdjusted(xcs ? xcs->width : 0);
}


/*!
  Returns the minimum left bearing of the font.

  This is the smallest leftBearing(char) of all characters in the font.

  Note that this function can be very slow if the font is big.

  \sa minRightBearing(), leftBearing(char)
*/
int QFontMetrics::minLeftBearing() const
{
    XFontStruct *f;
    int mlb = SHRT_MAX;

    for(int i = 0; i < QFontPrivate::NScripts - 1; i++) {
	if (! d->x11data.fontstruct[i]) continue;

	f = (XFontStruct *) d->x11data.fontstruct[i]->handle;
	if (f->min_bounds.lbearing < mlb)
	    mlb = f->min_bounds.lbearing;
    }

    return printerAdjusted(mlb);
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
	XFontStruct *f;
	int mrb = SHRT_MAX;
	int mx = 0;

	for (int i = 0; i < QFontPrivate::NScripts - 1; i++) {
	    if (! d->x11data.fontstruct[i]) {
		// qDebug("QFontMetrics::minRightBearing: no fontstruct for %d %s",
		// i, qt_x11encodings[i]);

		continue;
	    }

	    f = (XFontStruct *) d->x11data.fontstruct[i]->handle;

	    if ( f->per_char ) {
		XCharStruct *cs = f->per_char;
		int nc = MAXINDEX(f) + 1;
		mx = cs->width - cs->rbearing;

		for (int c = 1; c < nc; c++) {
		    int nmx = cs[c].width - cs[c].rbearing;
		    if (nmx < mx) mx = nmx;
		}
	    } else {
		mx = f->max_bounds.width - f->max_bounds.rbearing;
	    }

	    if (mx < mrb)
		mrb = mx;

	    // qDebug("QFontMetrics::minRightBearing: %d %d for %d %s",
	    // mx, mrb, i, qt_x11encodings[i]);
	}

	d->actual.rbearing = mx;
    }

    // qDebug("QFontMetrics::minRightBearing: %d", d->actual.rbearing);

    return printerAdjusted(d->actual.rbearing);
}


/*!
  Returns the height of the font.

  This is always equal to ascent()+descent()+1 (the 1 is for the base line).

  \sa leading(), lineSpacing()
*/
int QFontMetrics::height() const
{
    XFontStruct *f;
    int h = 0, hh;
    for (int i = 0; i < QFontPrivate::NScripts - 1; i++) {
	if (! d->x11data.fontstruct[i]) continue;

	f = (XFontStruct *) d->x11data.fontstruct[i]->handle;
	hh = printerAdjusted(f->max_bounds.ascent + f->max_bounds.descent);
	if (hh > h)
	    h = hh;
    }

    return h;
}


/*!
  Returns the leading of the font.

  This is the natural inter-line spacing.

  \sa height(), lineSpacing()
*/
int QFontMetrics::leading() const
{
    XFontStruct *f;
    int l = 0, ll;
    for (int i = 0; i < QFontPrivate::NScripts - 1; i++) {
	if (! d->x11data.fontstruct[i]) continue;

	f = (XFontStruct *) d->x11data.fontstruct[i]->handle;
	ll = printerAdjusted(f->ascent + f->descent - f->max_bounds.ascent -
			     f->max_bounds.descent);
	if (ll > l)
	    l = ll;
    }

    return l;
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

  \sa boundingRect()
*/
int QFontMetrics::width(QChar ch) const
{
    QFontPrivate::Script script = QFontPrivate::scriptForChar(ch);

    if (script == QFontPrivate::UnknownScript)
	return 0;

    d->load(script);
    if (! d->x11data.fontstruct[script]) return 0;

    XCharStruct *xcs =
	charStr(d->x11data.fontstruct[script]->codec,
		((XFontStruct *) d->x11data.fontstruct[script]->handle), ch);
    return printerAdjusted(xcs ? xcs->width : 0);
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
    if ( len < 0 ) len = str.length();

    // this algorithm is similar to the one used for painting
    const QChar *uc = str.unicode();
    QFontPrivate::Script currs = QFontPrivate::UnknownScript, tmp;
    XFontStruct *f;
    QCString mapped;
    int currw = 0;
    int lasts = -1;
    int i;

    for (i = 0; i < len; i++) {
	tmp = QFontPrivate::scriptForChar(*uc++);

	if (tmp != currs && tmp != QFontPrivate::UnknownScript) {
	    currs = tmp;

	    if (lasts >= 0) {
		d->load(currs);
		if (! d->x11data.fontstruct[currs]) continue;

		f = (XFontStruct *) d->x11data.fontstruct[currs]->handle;

		if (d->x11data.fontstruct[currs]->codec) {
		    mapped = d->x11data.fontstruct[currs]->codec->
			     fromUnicode(str.mid(lasts, i - lasts));
		}

		if (mapped.isNull()) {
		    // we are dealing with unicode text and a unicode font - YAY
		    if (f->max_byte1) {
			currw +=
			    XTextWidth16(f, (XChar2b *) (str.unicode() + lasts),
					 i - lasts);
		    }
		    // STOP: we want to use unicode, but don't have a multi-byte
		    // font?  something is seriously wrong... assume we have text
		    // we know nothing about
		} else {
		    if (f->max_byte1) {
			currw += XTextWidth16(f, (XChar2b *) mapped.data(),
					      mapped.length() / 2);
		    } else {
			currw += XTextWidth(f, mapped.data(), mapped.length());
		    }

		    mapped.resize(0);
		}
	    }

	    lasts = i;
	}
    }

    if (lasts >= 0) {
	d->load(currs);

	if (d->x11data.fontstruct[currs]) {
	    f = (XFontStruct *) d->x11data.fontstruct[currs]->handle;

	    if (d->x11data.fontstruct[currs]->codec) {
		mapped = d->x11data.fontstruct[currs]->codec->
			 fromUnicode(str.mid(lasts, i - lasts));
	    }

	    if (mapped.isNull()) {
		if (f->max_byte1) {
		    // we are dealing with unicode text and a unicode font - YAY
		    currw +=
			XTextWidth16(f, (XChar2b *) (str.unicode() + lasts), i - lasts);
		}
		// STOP: we want to use unicode, but don't have a multi-byte
		// font?  something is seriously wrong... assume we have text
		// we know nothing about
	    } else {
		if (f->max_byte1) {
		    currw +=
			XTextWidth16(f, (XChar2b *) mapped.data(), mapped.length() / 2);
		} else {
		    currw +=
			XTextWidth(f, mapped.data(), mapped.length());
		}
	    }
	}
    }

    return currw;
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
    if (len < 0) len = str.length();

    // Values are printerAdjusted during calculations.

    // this algorithm is similar to width(const QString &, int)
    const QChar *uc = str.unicode();
    QFontPrivate::Script currs = QFontPrivate::UnknownScript, tmp;
    QCString mapped;
    XFontStruct *f;
    XCharStruct overall;
    XCharStruct immediate;
    int lasts = -1;
    int i;
    int unused;

    // zero overall
    overall.lbearing = 0;
    overall.rbearing = 0;
    overall.ascent = 0;
    overall.descent = 0;
    overall.width = 0;

    for (i = 0; i < len; i++) {
	tmp = QFontPrivate::scriptForChar(*uc++);

	if (tmp != currs && tmp != QFontPrivate::UnknownScript) {
	    currs = tmp;

	    if (lasts >= 0) {
		d->load(currs);
		if (! d->x11data.fontstruct[currs]) continue;

		f = (XFontStruct *) d->x11data.fontstruct[currs]->handle;

		if (d->x11data.fontstruct[currs]->codec) {
		    mapped = d->x11data.fontstruct[currs]->codec->
			     fromUnicode(str.mid(lasts, i - lasts));
		}

		if (mapped.isNull()) {
		    // we are dealing with unicode text and a unicode font - YAY
		    if (f->max_byte1) {
			XTextExtents16(f, (XChar2b *) (str.unicode() + lasts),
				       i - lasts, &unused, &unused, &unused,
				       &immediate);

			overall.lbearing = QMAX(overall.lbearing, immediate.lbearing);
			overall.rbearing = QMAX(overall.rbearing, immediate.rbearing);
			overall.ascent = QMAX(overall.ascent, immediate.ascent);
			overall.descent = QMAX(overall.descent, immediate.descent);
			overall.width += immediate.width;
		    }
		    // STOP: we want to use unicode, but don't have a multi-byte
		    // font?  something is seriously wrong... assume we have text
		    // we know nothing about
		} else {
		    if (f->max_byte1) {
			XTextExtents16(f, (XChar2b *) mapped.data(),
				       mapped.length() / 2, &unused, &unused, &unused,
				       &immediate);

			overall.lbearing = QMAX(overall.lbearing, immediate.lbearing);
			overall.rbearing = QMAX(overall.rbearing, immediate.rbearing);
			overall.ascent = QMAX(overall.ascent, immediate.ascent);
			overall.descent = QMAX(overall.descent, immediate.descent);
			overall.width += immediate.width;
		    } else {
			XTextExtents(f, mapped.data(), mapped.length(), &unused,
				     &unused, &unused, &immediate);

			overall.lbearing = QMAX(overall.lbearing, immediate.lbearing);
			overall.rbearing = QMAX(overall.rbearing, immediate.rbearing);
			overall.ascent = QMAX(overall.ascent, immediate.ascent);
			overall.descent = QMAX(overall.descent, immediate.descent);
			overall.width += immediate.width;
		    }

		    mapped.resize(0);
		}
	    }

	    lasts = i;
	}
    }

    if (lasts >= 0) {
	d->load(currs);

	if (d->x11data.fontstruct[currs]) {
	    f = (XFontStruct *) d->x11data.fontstruct[currs]->handle;

	    if (d->x11data.fontstruct[currs]->codec) {
		mapped = d->x11data.fontstruct[currs]->codec->
			 fromUnicode(str.mid(lasts, i - lasts));
	    }

	    if (mapped.isNull()) {
		// we are dealing with unicode text and a unicode font - YAY
		if (f->max_byte1) {
		    XTextExtents16(f, (XChar2b *) (str.unicode() + lasts),
				   i - lasts, &unused, &unused, &unused,
				   &immediate);

		    overall.lbearing = QMAX(overall.lbearing, immediate.lbearing);
		    overall.rbearing = QMAX(overall.rbearing, immediate.rbearing);
		    overall.ascent = QMAX(overall.ascent, immediate.ascent);
		    overall.descent = QMAX(overall.descent, immediate.descent);
		    overall.width += immediate.width;
		}
		// STOP: we want to use unicode, but don't have a multi-byte
		// font?  something is seriously wrong... assume we have text
		// we know nothing about
	    } else {
		if (f->max_byte1) {
		    XTextExtents16(f, (XChar2b *) mapped.data(),
				   mapped.length() / 2, &unused, &unused, &unused,
				   &immediate);

		    overall.lbearing = QMAX(overall.lbearing, immediate.lbearing);
		    overall.rbearing = QMAX(overall.rbearing, immediate.rbearing);
		    overall.ascent = QMAX(overall.ascent, immediate.ascent);
		    overall.descent = QMAX(overall.descent, immediate.descent);
		    overall.width += immediate.width;
		} else {
		    XTextExtents(f, mapped.data(), mapped.length(), &unused,
				 &unused, &unused, &immediate);

		    overall.lbearing = QMAX(overall.lbearing, immediate.lbearing);
		    overall.rbearing = QMAX(overall.rbearing, immediate.rbearing);
		    overall.ascent = QMAX(overall.ascent, immediate.ascent);
		    overall.descent = QMAX(overall.descent, immediate.descent);
		    overall.width += immediate.width;
		}

		mapped.resize(0);
	    }
	}
    }

    overall.lbearing = printerAdjusted(overall.lbearing);
    overall.rbearing = printerAdjusted(overall.rbearing);
    overall.ascent = printerAdjusted(overall.ascent);
    overall.descent = printerAdjusted(overall.descent);
    overall.width = printerAdjusted(overall.width);

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
	width  = overall.rbearing - startX;
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

    // qDebug("QFontMetrics::boundingRect 2: %d %d %d %d",
    // startX, -ascent, width, descent + ascent);

    return QRect(startX, -ascent, width, descent + ascent);
}


/*!
  Returns the width of the widest character in the font.
*/
int QFontMetrics::maxWidth() const
{
    XFontStruct *f;
    int w = 0, ww;
    for (int i = 0; i < QFontPrivate::NScripts - 1; i++) {
	if (! d->x11data.fontstruct[i]) continue;

	f = (XFontStruct *) d->x11data.fontstruct[i]->handle;
	ww = printerAdjusted(f->max_bounds.width);
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

    if (pos)
	return pos;
    return 1;
}


/*!
  Returns the distance from the base line to where the strike-out line
  should be drawn.
  \sa underlinePos(), lineWidth()
*/
int QFontMetrics::strikeOutPos() const
{
    int pos = ascent() / 3;

    if (pos)
	return printerAdjusted(pos);
    return 1;
}


/*!
  Returns the width of the underline and strike-out lines, adjusted for
  the point size of the font.
  \sa underlinePos(), strikeOutPos()
*/
int QFontMetrics::lineWidth() const
{
    if ( painter ) {
	painter->cfont.d->load(QFontPrivate::defaultScript);
	return printerAdjusted(painter->cfont.d->lineWidth);
    }

    return d->lineWidth;
}
