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


// these are the fonts I have -- Brad
// xlsfonts | egrep -- "^-.*-.*-.*$" | cut -f 14- -d- | sort | uniq
// --
// adobe-fontspecific
// ascii-0
// bitmap.8x16-0
// dec-dectech
// fcd8859-15 <- what exactly is this?
// fontspecific-0
// gb2312.1980-0
// iso10646-1
// iso10646.indian-1
// iso646.1991-irv
// iso8859-1
// iso8859-10
// iso8859-11
// iso8859-13
// iso8859-15
// iso8859-2
// iso8859-3
// iso8859-4
// iso8859-5
// iso8859-6
// iso8859-7
// iso8859-8
// iso8859-9
// jisx0201.1976-0
// jisx0208.1983-0
// johab-1
// johabs-1
// johabsh-1
// koi8-r
// koi8-ru
// ksc5601.1987-0
// ksc5601.1987-1
// microsoft-symbol
// misc-fontspecific
// sunolcursor-1
// sunolglyph-1
// tis620-0
// tis620-2
// tscii-0
// tscii-1
// --
static char *X11EncodingNames[QFont::NCharSets + 1] =
{
    "iso8859-1",
	"iso8859-2",
	"iso8859-3",
	"iso8859-4",
	"iso8859-5",
	"iso8859-6",
	"iso8859-7",
	"iso8859-8",
	"iso8859-9",
	"iso8859-10",

	// Thai - which of these should be default?
	"iso8859-11",
	// "tis620-*", // tis620-* since we have -0 and -2 above

	"iso8859-12",
	"iso8859-13",
	"iso8859-14",
	"iso8859-15",
	"koi8-r",
	"koi8-ru",

	// JIS X 0208
	"jisx0208.1983-0",

	// Chinese (aka Simplified Chinese)
	"gb2312.1980-0",

	// Hong Kong/Taiwan (aka Traditional Chinese)
	"big5-0",

	// Korean (aka KS X 1001 aka KS C 5601)
	"ksc5601.1987-0",

	// Tamil (Tamil Standard Code fo Information Interchange)
	"tscii-*",

	// Unicode == ISO 10646 (for now)
	"iso10646-1",

	// fall back for unknown/unsupported charsets
	"*-*"
	};




// **********************************************************************
// QFontPrivate
// **********************************************************************

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
QCString QFontPrivate::findFont(QFont::CharSet charset, bool *exact) const
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
    QCString bestName = bestFamilyMember(charset, foundry, familyName, &score);
    if ( score < exactScore )
	*exact = FALSE;

    if (charset != QFont::ISO_10646_1) {
	// try substitution
	if (bestName.isNull()) {
	    QString f = QFont::substitute( request.family );

	    if( familyName != f ) {
		familyName = f;
		bestName =
		    bestFamilyMember(charset, foundry, familyName, &score);
	    }

	    // try default family for style
	    if (bestName.isNull()) {
		QString f = defaultFamily();

		if ( familyName != f ) {
		    familyName = f;
		    bestName =
			bestFamilyMember(charset, foundry, familyName, &score);
		}
		
		// try system default family
		if (bestName.isNull()) {
		    f = lastResortFamily();

		    if ( familyName != f ) {
			familyName = f;
			bestName =
			    bestFamilyMember(charset, foundry, familyName, &score);
		    }

		    // try *any* family
		    if (bestName.isNull()) {
			f = "*";
		    
			if (familyName != f) {
			    familyName = f;
			    bestName =
				bestFamilyMember(charset, foundry, familyName, &score);
			}
		    }
		}
	    }
	}

	// no matching fonts found
	if (bestName.isNull())
	    bestName = lastResortFont().latin1();
    }
    
    return bestName;
}


QCString QFontPrivate::bestFamilyMember(QFont::CharSet charset, const QString& foundry,
					const QString& family, int *score ) const
{
    const int prettyGoodScore = SizeScore | WeightScore | SlantScore | WidthScore;

    int testScore = 0;
    QCString testResult;
    int bestScore = 0;
    QCString result;

    if ( !foundry.isEmpty() ) {
	QString pattern
	    = "-" + foundry + "-" + family + "-*-*-*-*-*-*-*-*-*-*-" +
	    X11EncodingNames[charset];
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
			      X11EncodingNames[charset];
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


// fill the actual fontdef with data from the loaded font
void QFontPrivate::initFontInfo(QFont::CharSet charset)
{
    if (! actual.dirty) return;

    actual.lbearing = SHRT_MIN;
    actual.rbearing = SHRT_MIN;
    computeLineWidth(charset);

    if (exactMatch) {
	actual = request;
	actual.dirty = FALSE;
	return;
    }

    if (! fillFontDef(x11data.fontName[charset], &actual, 0)) {
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

	actual.family = QString::fromLatin1(x11data.fontName[charset]);
	actual.rawMode = TRUE;
	exactMatch = FALSE;
	return;
    }

    actual.underline = request.underline;
    actual.strikeOut = request.strikeOut;
}


// Computes the line width (underline,strikeout) for the X font
// and fills in the X resolution of the font.
void QFontPrivate::computeLineWidth(QFont::CharSet charset)
{
    int nlw;

    // X font name always <= 255 chars
    QCString buffer(256);
    char *tokens[NFontFields];
    qstrcpy( buffer.data(), x11data.fontName[charset]);

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

    /*


      inline bool inFont(XFontStruct *f, QChar ch)
      {
      // we should get the character set for the character and
      // check for a loaded font?

      if ( f->max_byte1 ) {
      return (ch.cell() >= f->min_char_or_byte2 &&
      ch.cell() <= f->max_char_or_byte2 &&
      ch.row() >= f->min_byte1 &&
      ch.row() <= f->max_byte1);
      } else if ( ch.row() ) {
      uint ch16 = ch.unicode();
      return (ch16 >= f->min_char_or_byte2 &&
      ch16 <= f->max_char_or_byte2);
      }

      return (ch.cell() >= f->min_char_or_byte2 &&
      ch.cell() <= f->max_char_or_byte2);
      }

      bool parseXFontName(CharSet cs, char **tokens)
      {
      if ( fontName[cs].isEmpty() || fontName[cs][0] != '-' ) {
      tokens[0] = 0;
      return FALSE;
      }

      int i;
      char *f = fontName[cs].data() + 1;
      for (i = 0; i < NFontFields && f && f[0]; i++) {
      tokens[i] = f;
      f = strchr( f, '-' );

      if (f) *f++ = '\0';
      }

      if (i < NFontFields) {
      for (int j=i ; j < NFontFields; j++)
      tokens[j] = 0;

      return FALSE;
      }

      return TRUE;
      }

      // old QFont_Private

      int fontMatchScore(const char *fontName, QCString &buffer,
      float *pointSizeDiff, int *weightDiff,
      bool *scalable, bool *smoothScalable);
      QCString bestMatch(const char *pattern, int *score);
      QCString bestFamilyMember(const QString& foundry,
      const QString& family, int *score);
      QCString findFont(bool *exact);
    */







/*****************************************************************************
  QFontInternal contains X11 font data: an XLFD font name ("-*-*-..") and
  an X font struct.

  Two global dictionaries and a cache hold QFontInternal objects, which
  are shared between all QFonts.
  This mechanism makes font loading much faster than using XLoadQueryFont.
 ***************************************************************************** /
    class QFontPrivateX
    {
    public:
    ~QFontPrivateX();
    bool	    dirty() const;
    const char	   *name()  const;
    int		    xResolution() const;
    XFontStruct	   *fontStruct() const;
    const QFontDef *spec()  const;
    int		    lineWidth() const;
    const QTextCodec *mapper() const;
    void	    reset();
    private:
    QFontPrivateX( const QString & );
    void computeLineWidth();

    QCString	    n;
    XFontStruct	   *f;
    QFontDef	    s;
    int		    lw;
    int		    xres;
    QTextCodec	   *cmapper;
    friend void QFont::load() const;
    friend void QFont::initFontInfo() const;
    };

    inline QFontPrivate::QFontPrivate( const QString &name )
    : n(name.ascii()), f(0), cmapper(0)
    {
    s.dirty = TRUE;
    }

    inline bool QFontPrivate::dirty() const
    {
    return f == 0;
    }

    inline const char *QFontPrivate::name() const
    {
    return n;
    }

    inline XFontStruct *QFontPrivate::fontStruct() const
    {
    return f;
    }

    inline const QFontDef *QFontPrivate::spec() const
    {
    return &s;
    }

    inline int QFontPrivate::lineWidth() const
    {
    return lw;
    }

    inline int QFontPrivate::xResolution() const
    {
    return xres;
    }

    inline const QTextCodec *QFontPrivate::mapper() const
    {
    return cmapper;
    }

    inline void QFontPrivate::reset()
    {
    if ( f ) {
    XFreeFont( QPaintDevice::x11AppDisplay(), f );
    f = 0;
    }
    }

    inline QFontPrivate::~QFontPrivate()
    {
    reset();
    }
*/


// QFontCache
/*
  static const int reserveCost   = 1024*100;
  static const int fontCacheSize = 1024*1024*4;

  typedef QCacheIterator<QFontPrivate> QFontCacheIt;
  typedef QDict<QFontPrivate>	      QFontDict;
  typedef QDictIterator<QFontPrivate>  QFontDictIt;

  class QFontCache : public QCache<QFontPrivate>
  {
  public:
  QFontCache( int maxCost, int size=17 )
  : QCache<QFontPrivate>(maxCost,size) {}
  void deleteItem( Item );
  };

  void QFontCache::deleteItem( Item d )
  {
  QFontPrivate *fin = (QFontPrivate *)d;
  //    fin->reset();
  }

  struct QXFontName
  {
  QXFontName( const QCString &n, bool e ) : name(n), exactMatch(e) {}
  QCString name;
  bool    exactMatch;
  };

  typedef QDict<QXFontName> QFontNameDict;

  static QFontCache    *fontCache	     = 0;	// cache of loaded fonts
  static QFontDict     *fontDict	     = 0;	// dict of all loaded fonts
  static QFontNameDict *fontNameDict   = 0;	// dict of matched font names default character set

  Clears the internal cache of mappings from QFont instances to X11
  (XLFD) font names. Called from QPaintDevice::x11App
  * /
  void qX11ClearFontNameCache()
  {
  if ( fontNameDict )
  fontNameDict->clear();
  }
*/


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

/*!
  Internal function that initializes the font system.

  \internal
  The font cache and font dict do not alloc the keys. The key is a QString
  which is shared between QFontPrivate and QXFontName.
*/

QFont::CharSet QFontPrivate::defaultCharSet = QFont::AnyCharSet;

void QFont::initialize()
{
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
	CharSet cs = AnyCharSet, tmp;
	const QChar *uc = sample.unicode();

	for (uint i = 0; i < sample.length(); i++) {
	    tmp = QFont::encodingForChar(*uc++);
	    if (tmp != cs && tmp != AnyCharSet) {
		cs = tmp;
		break;
	    }
	}

	if (cs != AnyCharSet) {
	    qDebug("QFont::initialize: got text sample successfully (%d %s)",
		   cs, X11EncodingNames[cs]);

	    QFontPrivate::defaultCharSet = cs;
	}
    }

    // qDebug("QFont::initialize: restoring LC_TIME locale");
    setlocale(LC_TIME, (const char *) oldlctime);

    qDebug("QFont::initialize: need to do something abount the font cache");

    /*
      if ( fontCache )
      return;
      fontCache = new QFontCache( fontCacheSize, 29 );
      CHECK_PTR( fontCache );
      fontDict  = new QFontDict( 29 );
      CHECK_PTR( fontDict );
      fontNameDict = new QFontNameDict( 29 );
      CHECK_PTR( fontNameDict );
      fontNameDict->setAutoDelete( TRUE );
    */
    
    qDebug("QFont::initialize: creating font codecs");
    
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
    qDebug("QFont::cleanup: sorry, function not implemented");

    /*
      delete fontCache;
      fontCache = 0;
      if ( fontDict )
      fontDict->setAutoDelete( TRUE );
      delete fontDict;
      fontDict = 0;
      delete fontNameDict;
      fontNameDict = 0;
    */
}


/*!
  Internal function that dumps font cache statistics.
*/
void QFont::cacheStatistics()
{
    qDebug("QFont:cacheStatistics: sorry, function not implemented");

    /*
      #if defined(DEBUG)
      fontCache->statistics();
      QFontCacheIt it(*fontCache);
      QFontPrivate *fin;
      qDebug( "{" );
      while ( (fin = it.current()) ) {
      ++it;
      qDebug( "   [%s]", fin->name() );
      }
      qDebug( "}" );
      #endif
    */
}


/*!
  Returns the window system handle to the font, for low-level
  access.  Using this function is \e not portable.
*/
Qt::HANDLE QFont::handle() const
{
    // qDebug("QFont::handle: d %p", d);

    if (d->request.dirty) {
	// qDebug("QFont::handle: font dirty");

	if (QFontPrivate::defaultCharSet != QFont::AnyCharSet) {
	    // qDebug("QFont::handle: loading font for charset %d %s",
	    // QFontPrivate::defaultCharSet,
	    // X11EncodingNames[QFontPrivate::defaultCharSet]);
	    d->load(QFontPrivate::defaultCharSet);
	    // qDebug("QFont::handle: font loaded %p %lx",
	    // d, d->x11data.fs[QFontPrivate::defaultCharSet]);
	}
	// } else {
	// qDebug("QFont::handle: font not dirty, what do i do here?");
	// qDebug("QFont::handle: normal font code would hit the fontcache");
    }

#ifndef QT_NO_COMPAT
    // qDebug("QFont::handle: checking charset for !QT_NO_COMPAT");
    // qDebug("QFont::handle: d->charset is %d '%s'", d->charset,
    // X11EncodingNames[d->charset]);

    // if the charset has been set, then we return the handle for that font
    if (d->charset != QFont::AnyCharSet) {
	// qDebug("font charset is %d '%s'", d->charset, X11EncodingNames[d->charset]);

	if (d->x11data.fs[d->charset]) {
	    // qDebug("QFont::handle: charset has been set, returning that font id");
	    return ((XFontStruct *) d->x11data.fs[d->charset])->fid;
	} else {
	    // qDebug("QFont::handle: charset has been set, but font isn't loaded");
	    return 0;
	}
    } else {
	// qDebug("QFont::handle: charset not set");
    }
#endif

    // qDebug("QFont::handle: looking for fontstruct");
    // find the first font id and return that
    for (int i = 0; i < NCharSets; i++) {
	if (d->x11data.fs[i]) {
	    // qDebug("QFont::handle: returning font id for charset %d '%s'",
	    // i, X11EncodingNames[i]);
	    return ((XFontStruct *) d->x11data.fs[i])->fid;
	    // } else {
	    // qDebug("QFont::handle: no font id for charset %d '%s'",
	    // i, X11EncodingNames[i]);
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











/*!
  Returns the family name that corresponds to the current style hint.
*/
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


/*!
  Returns a last resort family name for the font matching algorithm.

  \sa lastResortFont()
*/
QString QFontPrivate::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}


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
    0 };


/*!
  Returns a last resort raw font name for the font matching algorithm.
  This is used if even the last resort family is not available.  It
  returns \e something, almost no matter what.

  The current implementation tries a wide variety of common fonts,
  returning the first one it finds.  The implementation may change at
  any time.

  \sa lastResortFamily()
*/
QString QFontPrivate::lastResortFont() const
{
    qDebug("QFontPrivate::lastResortFont");
    
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


#define MAXINDEX(f) \
    ((((f)->max_byte1 - (f)->min_byte1) * \
      ((f)->max_char_or_byte2 - (f)->min_char_or_byte2 + 1)) + \
     (f)->max_char_or_byte2 - (f)->min_char_or_byte2)

/*!
  Loads the font.
*/

void QFontPrivate::load(QFont::CharSet charset, bool tryUnicode)
{
    if (x11data.fs[charset] && ! request.dirty) {
	// qDebug("QFontPrivate::load: charset %d %s already loaded",
	// charset, X11EncodingNames[charset]);
	return;
    }

    if (request.dirty) {
#warning "BIG NO NO - readd the cache to avoid risking fonts not being unloaded"
	for (int i = 0; i < QFont::NCharSets; i++)
	    x11data.fs[i] = 0;
    }

    // qDebug("QFont::load: skipping font cache lookup");
    /*
      if ( !fontCache ) {				// not initialized
      #if defined(CHECK_STATE)
      qFatal( "QFont: Must construct a QApplication before a QFont" );
      #endif

      return;
      }

      QString k = key();
      QXFontName *fn = fontNameDict->find( k );

      if ( !fn ) {
    */

    // qDebug("QFont::load: getting font name");

    // look for a unicode font first, and see if it has the charset that we want...
    if (tryUnicode) {
	if (! x11data.fs[QFont::ISO_10646_1]) {
	    // qDebug("QFontPrivate::load: trying to load unicode font");
	    load(QFont::ISO_10646_1, FALSE);
	    // qDebug("QFontPrivate::load: back from unicode tryload");
	}
	
	if ((signed) x11data.fs[QFont::ISO_10646_1] != -1) {
	    bool hasChar = FALSE;
	    QChar ch;

	    XFontStruct *f = (XFontStruct *) x11data.fs[QFont::ISO_10646_1];
	    if (f->per_char) {
		XCharStruct *cs = f->per_char + 0xfffe;

		if (cs->width + cs->ascent + cs->descent == 0) {
		    uchar row, cell;

		    switch (charset) {
		    case QFont::ISO_8859_1:
			// default:
			row  = 0x00;
			cell = 0x30;
			break;

		    case QFont::ISO_8859_2:
			row  = 0x01;
			cell = 0x00;
			break;

			// case QFont::ISO_8859_3:
			// case QFont::ISO_8859_4:

		    case QFont::ISO_8859_5:
		    case QFont::KOI8_R:
		    case QFont::KOI8_U:
			row  = 0x04;
			cell = 0x10;
			break;

		    case QFont::ISO_8859_6:
			row  = 0x06;
			cell = 0x30;
			break;

		    case QFont::ISO_8859_7:
			row  = 0x03;
			cell = 0x90;
			break;

		    case QFont::ISO_8859_8:
			row  = 0x05;
			cell = 0xd0;
			break;

			// case QFont::ISO_8859_9:
			// case QFont::ISO_8859_10:

		    case QFont::ISO_8859_11:
			row  = 0x0e;
			cell = 0x00;

			// case QFont::ISO_8859_12:
			// case QFont::ISO_8859_13:
			// case QFont::ISO_8859_14:
			// case QFont::ISO_8859_15:

		    case QFont::JISX0208:
			row  = 0x30;
			cell = 0x50;
			break;

			// case QFont::GB2312:
			// break;
			
			// case QFont::BIG5:
			// break;

		    case QFont::KSC5601:
			row  = 0xac;
			cell = 0x00;
			break;

		    case QFont::TSCII:
			row  = 0x0b;
			cell = 0x80;
			break;

			// case QFont::ISO_10646_1:
			// break;

		    default:
			row = cell = 0;
		    }

		    if (row + cell != 0) {
			ch.cell() = cell;
			ch.row()  = row;
			cs = f->per_char + ch.unicode();

			hasChar = (cs->width + cs->ascent + cs->descent != 0);
		    }
		}
	    }

	    if (hasChar) {
		// qDebug("QFontPrivate::load: unicode font has char 0x%04d for %d %s",
		// ch.unicode(), charset, X11EncodingNames[charset]);
		
		x11data.fs[charset] = x11data.fs[QFont::ISO_10646_1];
		x11data.fontName[charset] = x11data.fontName[QFont::ISO_10646_1];
		x11data.codec[charset] = 0;
		request.dirty = FALSE;
		
		return;
	    } else {
		// qDebug("QFontPrivate::load: unicode font doesn't have char 0x%04d",
		// ch.unicode());
	    }
	} else {
	    // qDebug("QFontPrivate::load: unicode font tried, but doesn't exist");
	}
    } else {
	// qDebug("QFontPrivate::load: not trying Unicode font");
    }

    QString name;
    bool match;

    if (request.rawMode) {
	name = QFont::substitute(request.family);
	match = fontExists(name);
	
	if (! match) {
	    name = lastResortFont();
	}
    } else {
	name = findFont(charset, &match);
    }

    // qDebug("QFont::load: using name '%s'", name.latin1());

    // qDebug("QFont::load: skipping font cache insertion");
    /*
      fn = new QXFontName( name.ascii(), match );
      CHECK_PTR( fn );
      fontNameDict->insert( k, fn );
      }
    */

    // d->exactMatch = fn->exactMatch;
    exactMatch = match;

    QCString n(name.latin1());
    x11data.fontName[charset] = n;

    // qDebug("QFont::load: skipping another font cache insertion");
    /*
      d->priv = fontCache->find( n.data() );
      if ( !d->priv ) {				// font not loaded
      d->priv = fontDict->find( n.data() );
      if ( !d->priv ) {			// font was never loaded
      d->priv = new QFontPrivate( n );
      CHECK_PTR( d->priv );
      fontDict->insert( d->priv->name(), d->priv );
      }
      }
    */

    // font not loaded
    XFontStruct *f = 0;
    // qDebug("QFont::load: loading font for %d %s", charset, X11EncodingNames[charset]);
    // qDebug("%s", (const char *) n);
    
    if (! (f = XLoadQueryFont(QPaintDevice::x11AppDisplay(),
			      (const char *) n))) {
	if (! charset == QFont::ISO_10646_1) {
	    // qDebug("QFont::load: load failed, trying last resort");
	    exactMatch = FALSE;
	
	    if (! (f = XLoadQueryFont(QPaintDevice::x11AppDisplay(),
				      lastResortFont().latin1()))) {
		qFatal("QFont::load: Internal error");
	    }
	} else {
	    ((signed) x11data.fs[charset]) = -1;
	    return;
	}
    }
    
    // qDebug("QFont::load: skipping cache cost adjustment");
    /*
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
      if ( size > fontCache->maxCost() + reserveCost )
      fontCache->setMaxCost( size + reserveCost );
      #if defined(CHECK_STATE)
      if ( !fontCache->insert(d->priv->name(), d->priv, size) )
      qFatal( "QFont::load: Cache overflow error" );
      #endif
    */
    
    request.dirty = FALSE;
    x11data.fs[charset] = (Qt::HANDLE) f;
    initFontInfo(charset);
	
    // get unicode -> font encoding codec
    switch (charset) {
    case QFont::ISO_8859_1:
    case QFont::ISO_8859_2:
    case QFont::ISO_8859_3:
    case QFont::ISO_8859_4:
    case QFont::ISO_8859_5:
    case QFont::ISO_8859_6:
    case QFont::ISO_8859_7:
    case QFont::ISO_8859_8:
    case QFont::ISO_8859_9:
    case QFont::ISO_8859_10:
    case QFont::ISO_8859_11:
    case QFont::ISO_8859_12:
    case QFont::ISO_8859_13:
    case QFont::ISO_8859_14:
    case QFont::ISO_8859_15:
	{
	    QString codecname = "ISO 8859-" + QString::number(charset + 1);
	    x11data.codec[charset] = QTextCodec::codecForName(codecname);
	    break;
	}
	
    case QFont::KOI8_R:
	// case QFont::KOI8_U: // ### this too?
	x11data.codec[charset] = QTextCodec::codecForName("KOI8-R");
	break;
    
    case QFont::JISX0208:
	x11data.codec[charset] = QTextCodec::codecForName("QFont::JISX0208");
	break;
	
    case QFont::KSC5601:
	x11data.codec[charset] = QTextCodec::codecForName("QFont::KSC5601");
	break;
	
    case QFont::ISO_10646_1:
	break;
	
    default:
	qDebug("will look for codec soon %d", charset);
    }

    // qDebug("QFont::load: no printer hack font just yet");
    // delete d->printerHackFont;
    // d->printerHackFont = 0;
}


// void *QFontMetrics::fontStruct() const
// {
// qDebug("QFontMetrics::fontStruct:  sorry, function not implemented");

    /*
      if ( painter ) {
      painter->cfont.handle();

      // ### printer font metrics hack
      if ( painter->device() && 0 &&
      painter->device()->devType() == QInternal::Printer &&
      painter->cfont.pointSize() < 48 ) {

      if ( painter->cfont.d->printerHackFont == 0 ) {
      painter->cfont.d->printerHackFont
      = new QFont( painter->cfont );
      painter->cfont.d->printerHackFont->setPointSize( 64 );
      }

      painter->cfont.d->printerHackFont->handle();
      return painter->cfont.d->printerHackFont->d->priv->fontStruct();

      }

      return painter->cfont.d->priv->fontStruct();

      } else {

      return d->priv->fontStruct();

      }
    */

// return 0;
// }


const QTextCodec *QFontMetrics::mapper() const
{
    qDebug("QFontMetrics::mapper:  sorry, function not implemented");

    /*
      if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->priv->mapper();
    } else {
	return d->priv->mapper();
    }
    */

    return 0;
}


int QFontMetrics::printerAdjusted(int val) const
{
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

    return val;
}


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
    for (int i = 0; i < QFont::NCharSets - 1; i++) {
	if ((f = (XFontStruct *) d->x11data.fs[i]) == 0) continue;
	
	aa = f->max_bounds.ascent;
	if (aa > a) a = aa;
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
    for (int i = 0; i < QFont::NCharSets - 1; i++) {
	if ((f = (XFontStruct *) d->x11data.fs[i]) == 0) continue;
    
	cc = f->max_bounds.descent - 1;
	if (cc > c) c = cc;
    }
    
    return printerAdjusted(c);
}


/*!
  Returns TRUE if \a ch is a valid character in the font.
*/
bool QFontMetrics::inFont(QChar ch) const
{
    qDebug("QFontMetrics::inFont:  sorry, function not implemented");
    
    /*
      XFontStruct *f = ((XFontStruct *) fontStruct());
      if ( f && !mapper() ) {
      return ::inFont( f, ch );
      } else if ( mapper() ) {
      return mapper()->canEncode(ch);
      }
      return TRUE;
    */
    
    return FALSE;
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
    QFont::CharSet charset = QFont::encodingForChar(ch);
    
    if (charset == QFont::Unknown) return 0;
    
    if (! d->x11data.fs[charset]) {
	d->load(charset);
	
	if (! d->x11data.fs[charset])
	    return 0;
    }
    
    XCharStruct *xcs = charStr(d->x11data.codec[charset],
			       ((XFontStruct *) d->x11data.fs[charset]),
			       ch);
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
    QFont::CharSet cs = QFont::encodingForChar(ch);
    
    if (cs == QFont::Unknown) return 0;
    
    if (! d->x11data.fs[cs]) {
	d->load(cs);
	
	if (! d->x11data.fs[cs])
	    return 0;
    }
    
    return printerAdjusted(charStr(0, ((XFontStruct *) d->x11data.fs[cs]), ch)->width);
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
    
    for(int i = 0; i < QFont::NCharSets - 1; i++) {
	if ((f = (XFontStruct *) d->x11data.fs[i]) == 0 ) continue;

	if (f->min_bounds.lbearing < mlb) mlb = f->min_bounds.lbearing;
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
    // Safely cast away const, as we cache rbearing there.
    if ( d->actual.rbearing == SHRT_MIN ) {
	XFontStruct *f;
	int mrb = SHRT_MAX;
	int mx = 0;

	for (int i = 0; i < QFont::NCharSets - 1; i++) {
	    if ((f = (XFontStruct *) d->x11data.fs[i]) == 0) continue;
	    
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

	    if (mx < mrb) mrb = mx;
	}

	d->actual.rbearing = mx;
    }

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
    for (int i = 0; i < QFont::NCharSets - 1; i++) {
	if ((f = (XFontStruct *) d->x11data.fs[i]) == 0) continue;

	hh = printerAdjusted(f->max_bounds.ascent + f->max_bounds.descent);
	if (hh > h) h = hh;
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
    for (int i = 0; i < QFont::NCharSets - 1; i++) {
	if ((f = (XFontStruct *) d->x11data.fs[i]) == 0) continue;

	ll = f->ascent + f->descent - f->max_bounds.ascent - f->max_bounds.descent;
	if (ll > l) l = ll;
    }

    if ( l > 0 )
	return printerAdjusted(l);
    return 0;
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
int QFontMetrics::width( QChar ch ) const
{
    QFont::CharSet charset = QFont::encodingForChar(ch);

    if (charset == QFont::Unknown) return 0;
    
    if (! d->x11data.fs[charset]) {
	d->load(charset);
	
	if (! d->x11data.fs[charset])
	    return 0;
    }

    XCharStruct *xcs = charStr(d->x11data.codec[charset],
			       ((XFontStruct *) d->x11data.fs[charset]),
			       ch);
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
    QFont::CharSet currs = QFont::Unknown, tmp;
    int currw = 0;
    int lasts = -1;
    int i;
    
    for (i = 0; i < len; i++) {
	tmp = QFont::encodingForChar(*uc++);
	
	if (tmp != currs && tmp != QFont::Unknown) {
	    currs = tmp;
	    
	    if (lasts >= 0) {
		currw +=
		    XTextWidth16((XFontStruct *) d->x11data.fs[currs],
				 (XChar2b *) (str.unicode() + lasts),
				 i - lasts);
	    }
	    
	    lasts = i;
	}
    }
    
    if (lasts >= 0) {
	currw +=
	    XTextWidth16((XFontStruct *) d->x11data.fs[currs],
			 (XChar2b *) (str.unicode() + lasts),
			 i - lasts);
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
    qDebug("QFontMetrics::boundingRect: sorry, function not implemented");

    // Values are printerAdjusted during calculations.
    /*
      if ( len < 0 )
      len = str.length();
      XFontStruct *f = ((XFontStruct *) fontStruct());
      int direction;
      int ascent;
      int descent;
      XCharStruct overall;

      bool underline;
      bool strikeOut;
      if ( painter ) {
      underline = painter->cfont.underline();
      strikeOut = painter->cfont.strikeOut();
      } else {
      underline = underlineFlag();
      strikeOut = strikeOutFlag();
      }

      const QTextCodec *m = mapper();
      if ( m ) {
      XTextExtents( f, m->fromUnicode(str,len), len, &direction, &ascent,
      &descent, &overall );
      } else {
      XTextExtents16( f, (XChar2b*)str.unicode(), len, &direction, &ascent,
      &descent, &overall );
      }

      overall.lbearing = printerAdjusted(overall.lbearing);
      overall.rbearing = printerAdjusted(overall.rbearing);
      overall.ascent = printerAdjusted(overall.ascent);
      overall.descent = printerAdjusted(overall.descent);
      overall.width = printerAdjusted(overall.width);

      int startX = overall.lbearing;
      int width  = overall.rbearing - startX;
      ascent     = overall.ascent;
      descent    = overall.descent;

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

      return QRect( startX, -ascent, width, descent + ascent );
    */

    return QRect();
}


/*!
  Returns the width of the widest character in the font.
*/
int QFontMetrics::maxWidth() const
{
    XFontStruct *f;
    int w = 0, ww;
    for (int i = 0; i < QFont::NCharSets - 1; i++) {
	if ((f = (XFontStruct *) d->x11data.fs[i]) == 0) continue;

	ww = printerAdjusted(f->max_bounds.width);
	if (ww > w) w = ww;
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
    int pos = (lineWidth()*2 + 3)/6;
    if ( pos ) {
	return pos;
    } else {
	return 1;
    }
}


/*!
  Returns the distance from the base line to where the strike-out line
  should be drawn.
  \sa underlinePos(), lineWidth()
*/
int QFontMetrics::strikeOutPos() const
{
    qDebug("QFontMetrics::strikeOutPos: not implemented");

    /*
      XFontStruct *f = ((XFontStruct *) fontStruct());
      if ( f ) {
      int pos = f->max_bounds.ascent/3;
      if ( pos ) {
      return printerAdjusted(pos);
      } else {
      return 1;
      }
      }
      return ascent()/3;
    */

    return 0;
}


/*!
  Returns the width of the underline and strike-out lines, adjusted for
  the point size of the font.
  \sa underlinePos(), strikeOutPos()
*/
int QFontMetrics::lineWidth() const
{
    if ( painter ) {
	painter->cfont.handle();
	return printerAdjusted(painter->cfont.d->lineWidth);
    }

    return d->lineWidth;
}
