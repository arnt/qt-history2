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
#include "fontenginexft.h"
#include "fontenginebox.h"
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
    FontEngineIface *fe;

    for ( int i = 0; i < QFont::LastPrivateScript; i++ ) {
	fe = fontstruct[i];
	fontstruct[i] = 0;

	if ( fe )
	    fe->deref();
    }
}

/*
  Clears the internal cache of mappings from QFont instances to X11
  (XLFD) font names. Called from QPaintDevice::x11App
  */
void qX11ClearFontNameCache()
{
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
    if ( fs && !XGetFontProperty( fs, XA_FONT, &value ) )
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
    FontEngineIface *fe = x11data.fontstruct[script];
    FontEngineIface::Type type = fe->type();

    // set the scale value for each font correctly...
    if ( scale > 0 &&  type == FontEngineIface::Box )
	((FontEngineBox *) fe)->_size = (int)(((FontEngineBox *) fe)->_size*scale);

    if ((script != QFont::Unicode && script != defaultScript) || !actual.dirty ||
	type == FontEngineIface::Box ) {
	// make sure the pixel size is correct, so that we can draw the missing char
	// boxes in the correct size...
	if (request.pixelSize == -1) {
	    actual.pointSize = request.pointSize;
	    actual.pixelSize = (int)(pixelSize( actual, paintdevice, x11Screen ) +.5);
	}
	return;
    }

    if ( paintdevice &&
	 (QPaintDeviceMetrics( paintdevice ).logicalDpiY() != QPaintDevice::x11AppDpiY( x11Screen )) ) {
	// we have a printer font
	actual = request;
	float _pointSize = pointSize( actual, paintdevice, x11Screen );
	float _pixelSize = pixelSize( actual, paintdevice, x11Screen );
	if ( actual.pointSize == -1 )
	    actual.pointSize = (int)(_pointSize + 0.5);
	else
	    actual.pixelSize = (int) (_pixelSize + 0.5);

	if ( type == FontEngineIface::Xlfd ) {
	    FontEngineXLFD *fexlfd = (FontEngineXLFD *)fe;
	    QFontDef font;
	    if ( fillFontDef(fexlfd->name(), &font, x11Screen ) ) {
		if ( font.pixelSize != 0 )
		    fexlfd->_scale *= _pixelSize/((float) font.pixelSize);
		//qDebug("setting scale to %f requested pixel=%f got %d",
		// fe->scale, _pixelSize, font.pixelSize);
	    }
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
	if ( type == FontEngineIface::Xft ) {
	    FontEngineXft *fexft = (FontEngineXft *)fe;
	    // parse the pattern
	    XftPattern *pattern =
		(XftPattern *) fexft->_pattern;

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
	    FontEngineXLFD *fexlfd = (FontEngineXLFD *)fe;
	    QFontDef def;

	    if ( ! fillFontDef( fexlfd->_fs, &def, x11Screen ) &&
		 ! fillFontDef( fexlfd->name(), &def, x11Screen ) ) {
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

    if ( type != FontEngineXLFD::Xlfd )
	return;

    FontEngineXLFD *fexlfd = (FontEngineXLFD *)fe;

    if ( ! fillFontDef( fexlfd->_fs, &actual, x11Screen ) &&
	 ! fillFontDef( fexlfd->name(), &actual, x11Screen ) ) {
	// zero fontdef
	actual = QFontDef();

	actual.family = QString::fromLatin1(fe->name());
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
    case QFont::Latin:                     ch = 0x00c0; break;
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

    case QFont::Unicode:		   ch = 0xfffd; break;

    case QFont::LatinExtendedA_2:          ch = 0x0102; break;
    case QFont::LatinExtendedA_3:          ch = 0x0108; break;
    case QFont::LatinExtendedA_4:          ch = 0x0100; break;
    case QFont::LatinExtendedA_14:         ch = 0x0174; break;
    case QFont::LatinExtendedA_15:         ch = 0x0152; break;

    default:				   ch = 0x0000; break;
    }

    return QChar(ch);
}


static inline bool canRender( FontEngineIface *fe, const QChar &sample )
{
    if ( !fe ) return FALSE;

    QChar chs[2] = { QChar(0xfffe), sample };
    bool hasChar = !fe->canRender( chs, 1 ) && fe->canRender( chs+1, 1 );

#ifdef QFONTLOADER_DEBUG_VERBOSE
    if (hasChar) {
	qDebug("QFontLoader: unicode font has char 0x%04x", sample.unicode() );
    }
#endif

    return hasChar;
}


static inline FontEngineIface *loadEngine( int styleStrategy, int styleHint,
					   const QString &family,
					   const QString &foundry,
					   int weight, bool italic,
					   bool oblique, int pixelSize,
					   char pitch, bool use_regular,
					   const QCString &encoding,
					   int x11Screen )
{
#ifndef QT_NO_XFTFREETYPE
    if ( encoding == "*-*" ) {
	int slant_value;
	double size_value;

	if ( weight == 0 )
	    weight = XFT_WEIGHT_MEDIUM;
	else if ( weight < (QFont::Light + QFont::Normal) / 2 )
	    weight = XFT_WEIGHT_LIGHT;
	else if ( weight < (QFont::Normal + QFont::DemiBold) / 2 )
	    weight = XFT_WEIGHT_MEDIUM;
	else if ( weight < (QFont::DemiBold + QFont::Bold) / 2 )
	    weight = XFT_WEIGHT_DEMIBOLD;
	else if ( weight < (QFont::Bold + QFont::Black) / 2 )
	    weight = XFT_WEIGHT_BOLD;
	else
	    weight = XFT_WEIGHT_BLACK;

	if ( italic )
	    slant_value = XFT_SLANT_ITALIC;
	else if ( oblique )
	    slant_value = XFT_SLANT_OBLIQUE;
	else
	    slant_value = XFT_SLANT_ROMAN;

	size_value = pixelSize;

#if 0
	// ### FIX ME Lars :)
	if ( size_value > MAXFONTSIZE ) {
	    *scale = (double)size_value/(double)MAXFONTSIZE;
	    size_value = MAXFONTSIZE;
	} else {
	    *scale = 1.;
	}
#endif // 0


	const char *generic_value = 0;
	switch ( styleHint ) {
	case QFont::SansSerif:
	default:
	    generic_value = "sans";
	    break;
	case QFont::Serif:
	    generic_value = "serif";
	    break;
	case QFont::TypeWriter:
	    generic_value = "mono";
	    break;
	}

	XftPattern *pattern = XftPatternCreate();
	if ( !pattern ) return 0;

#ifndef QT_XFT2
	XftPatternAddString (pattern, XFT_ENCODING, "iso10646-1");
#endif
	if ( !foundry.isNull() )
	    XftPatternAddString( pattern, XFT_FOUNDRY, foundry.local8Bit().data() );
	if ( !family.isNull() )
	    XftPatternAddString( pattern, XFT_FAMILY, family.local8Bit().data() );
	XftPatternAddString( pattern, XFT_FAMILY, generic_value );

	XftPatternAddInteger( pattern, XFT_SPACING,
			      ( pitch == 'c' ? XFT_CHARCELL :
				( pitch == 'm' ? XFT_MONO : XFT_PROPORTIONAL ) ) );

	XftPatternAddInteger( pattern, XFT_WEIGHT, weight );
	XftPatternAddInteger( pattern, XFT_SLANT, slant_value );
	XftPatternAddDouble( pattern, XFT_PIXEL_SIZE, size_value );

	if ( !qt_use_antialiasing || styleStrategy & ( QFont::PreferAntialias |
						       QFont::NoAntialias) ) {
	    bool requestAA = ( qt_use_antialiasing &&
			       !( styleStrategy & QFont::NoAntialias ) );
	    XftPatternAddBool( pattern, XFT_ANTIALIAS, requestAA );
	}

	XftResult res;
	XftPattern *result = XftFontMatch( QPaintDevice::x11AppDisplay(),
					   x11Screen, pattern, &res );
	XftPatternDestroy(pattern);

	// We pass a duplicate to XftFontOpenPattern because either xft font
	// will own the pattern after the call or the pattern will be
	// destroyed.
	XftPattern *dup = XftPatternDuplicate( result );
	XftFont *xftfs = XftFontOpenPattern(QPaintDevice::x11AppDisplay(), dup);

	return new FontEngineXft( xftfs, result, 0 );
    }
#endif // QT_NO_XFTFREETYPE

    QCString xlfd = "-";
    xlfd += foundry.isEmpty() ? "*" : foundry.latin1();
    xlfd += "-";
    xlfd += family.isEmpty() ? "*" : family.latin1();

    xlfd += "-";
    if ( weight > 0 && weight <= QFont::Light )
	xlfd += "light";
    else if ( weight <= QFont::Normal )
	xlfd += use_regular ? "regular" : "medium";
    else if ( weight <= QFont::DemiBold )
	xlfd += "demibold";
    else if ( weight <= QFont::Bold )
	xlfd += "bold";
    else if ( weight <= QFont::Black )
	xlfd += "black";
    else
	xlfd += "*";

    xlfd += "-";
    xlfd += ( italic ? "i" : ( oblique ? "o" : "r" ) );

    xlfd += "-*-*-";
    xlfd += QString::number( pixelSize ).latin1();
    xlfd += "-*-*-*-";
    // ### handle cell spaced fonts
    xlfd += pitch;
    xlfd += "-*-" + encoding;

    // qDebug( "xlfd: '%s'", xlfd.data() );

    XFontStruct *xfs;
    if (! (xfs = XLoadQueryFont(QPaintDevice::x11AppDisplay(), xlfd.data() ) ) )
	return 0;

    return new FontEngineXLFD( xfs, xlfd.data(), encoding.data(), 0 );
}


void QFontPrivate::load( QFont::Script script, bool )
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

	if (qfs) {
	    // only deref here... we will let the cache take care of cleaning up
	    // while the application is running
	    qfs->deref();
	}

	// dirty unicode also if the font is dirty
	qfs = x11data.fontstruct[QFont::Unicode];
	x11data.fontstruct[QFont::Unicode] = 0;

	if (qfs) {
	    // only deref here... we will let the cache take care of cleaning up
	    // while the application is running
	    qfs->deref();
	}

	// make sure to recalculate fontinfo
	actual.dirty = TRUE;
    }

    QChar sample = sampleCharacter(script);
    FontEngineIface *fe = 0;
    // ### fix the scale calculations
    double scale = 1.0;

    QString k( key() + QString::number( script ) );
    k += "/scr" + QString::number( x11Screen );
    if ( paintdevice )
	k += "/res" + QString::number(QPaintDeviceMetrics( paintdevice ).logicalDpiY());
    else
	k += "/res" + QString::number( QPaintDevice::x11AppDpiY( x11Screen ) );

    // Look in fontCache for font
    fe = fontCache->find(k);
    if ( ! fe ) {
	// qDebug( "QFontPrivate::load: script %d", script );

	// ### support CSS2 style family lists
	QString fam, fnd;
	QCString encoding;

	QStringList familylist = QStringList::split( ',', request.family );
	familylist += QFont::substitutes( request.family );
	// familylist << ... ; // default fallback font for the specified script
	familylist << QString::null;

	QFont::Script scriptlist[] = { QFont::Unicode, script, QFont::UnknownScript };

	QStringList::ConstIterator it = familylist.begin(),
				  end = familylist.end();
	for ( ; !fe && it != end; ++it ) {
	    // qDebug( "\n\nusing family '%s'", (*it).latin1() );

	    uint which_script = ( script == QFont::Unicode ? 1 : 0 );
	    while ( !fe && which_script < 3 ) {
		QFontDatabase::parseFontName( (*it).simplifyWhiteSpace(), fnd, fam );
		int weight = request.weight;
		bool italic = request.italic;
		bool oblique = FALSE;
		int px = (int) pixelSize( request, paintdevice, x11Screen );
		bool use_regular = FALSE;
		char pitch = request.fixedPitch ? 'm' : 'p';

		if ( QFontDatabase::findFont( scriptlist[which_script],
					      request.styleStrategy,
					      fam, fnd, weight, italic, oblique, px,
					      pitch, use_regular, encoding ) ) {
		    // qDebug( "found a font with script %d encoding %s:",
		    // scriptlist[which_script], encoding.data() );
		    // qDebug( "    '%s' '%s' weight %d italic %d oblique %d size %d",
		    // fam.latin1(), fnd.latin1(), weight, italic, oblique, px );

		    fe = loadEngine( request.styleStrategy, request.styleHint,
				     fam, fnd, weight, italic, oblique, px,
				     pitch, use_regular, encoding, x11Screen );

		    // qDebug ("    engine %p", fe );

		    if ( !canRender( fe, sample ) ) {
			// qDebug( "font doesn't have sample char %04x, continuing",
			// sample.unicode() );
			delete fe;
			fe = 0;
		    }
		} else {
		    // qDebug( "couldn't find font with script %d",
		    // scriptlist[which_script] );
		    // qDebug( "    '%s' '%s' weight %d italic %d oblique %d size %d",
		    // fam.latin1(), fnd.latin1(), weight, italic, oblique, px );
		}

		++which_script;
	    }
	}

	if ( !fe )
	    fe = new FontEngineBox( (int) pixelSize( request, paintdevice, x11Screen ) );
    }

    fe->ref();

    x11data.fontstruct[script] = fe;
    initFontInfo( script, scale );
    // ### fix cost calculation
    fontCache->insert( k, x11data.fontstruct[script], 1 );
    request.dirty = FALSE;
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
    if (! qfs ) {
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
    if (! qfs )
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
    if (! qfs )
	return 0;
    return qfs->descent();
}


/*!
    Returns TRUE if character \a ch is a valid character in the font;
    otherwise returns FALSE.
*/
bool QFontMetrics::inFont(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );
    d->load( script );

    FontEngineIface *fe = d->x11data.fontstruct[script];
    if ( fe->type() == FontEngineIface::Box )
	return FALSE;

    return fe->canRender( &ch, 1 );
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
    SCRIPT_FOR_CHAR( script, ch );
    d->load( script );

    FontEngineIface *fe = d->x11data.fontstruct[script];
    if ( fe->type() == FontEngineIface::Box )
	return 0;

    GlyphIndex glyphs[10];
    int nglyphs = 9;
    fe->stringToCMap( &ch, 1, glyphs, &nglyphs );
    // ### can nglyphs != 1 happen at all? Not currently I think
    QGlyphMetrics gi = fe->boundingBox( glyphs[0] );
    return gi.x;
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
    SCRIPT_FOR_CHAR( script, ch );
    d->load( script );

    FontEngineIface *fe = d->x11data.fontstruct[script];
    if ( fe->type() == FontEngineIface::Box )
	return 0;

    GlyphIndex glyphs[10];
    int nglyphs = 9;
    fe->stringToCMap( &ch, 1, glyphs, &nglyphs );
    // ### can nglyphs != 1 happen at all? Not currently I think
    QGlyphMetrics gi = fe->boundingBox( glyphs[0] );
    return gi.xoff - gi.x - gi.width;
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
    if (! qfs )
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
    if (! qfs )
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
    if (! qfs )
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
    if (! qfs )
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
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );
    d->load( script );

    FontEngineIface *fe = d->x11data.fontstruct[script];
    if ( fe->type() == FontEngineIface::Box )
	return ((FontEngineBox *)fe)->size();

    GlyphIndex glyphs[10];
    int nglyphs = 9;
    fe->stringToCMap( &ch, 1, glyphs, &nglyphs );
    // ### can nglyphs != 1 happen at all? Not currently I think
    QGlyphMetrics gi = fe->boundingBox( glyphs[0] );
    return gi.xoff;
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
    if ( pos < 0 || pos > (int)str.length() )
	return 0;

    QFont::Script script;
    SCRIPT_FOR_CHAR( script, str.unicode()[pos] );
    d->load( script );

    // ### need to optimise this and not hardcode arabic.
    if ( script != QFont::Arabic ) {
	FontEngineIface *fe = d->x11data.fontstruct[script];
	if ( fe->type() == FontEngineIface::Box )
	    return ((FontEngineBox *)fe)->size();

	GlyphIndex glyphs[10];
	int nglyphs = 9;
	fe->stringToCMap( str.unicode()+pos, 1, glyphs, &nglyphs );
	// ### can nglyphs != 1 happen at all? Not currently I think
	QGlyphMetrics gi = fe->boundingBox( glyphs[0] );
	return gi.xoff;
    }

    const TextLayout *layout = TextLayout::instance();
    ScriptItemArray items;
    layout->itemize( items,  str );
    int i = 0;
    while ( i < items.size() && pos >= items[i].position )
	i++;
    i--;
    ShapedItem shaped;
    layout->shape( shaped, QFont( d ), str, items, i );

    return layout->width( shaped, pos-items[i].position, 1 );
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

    const TextLayout *layout = TextLayout::instance();
    ScriptItemArray items;
    layout->itemize( items,  str );
    ShapedItem shaped;
    int width = 0;
    int nchars;
    for ( int i = 0; i < items.size() && (nchars = len-items[i].position) > 0; i++ ) {
	layout->shape( shaped, QFont( d ), str, items, i );
	width += layout->width( shaped, 0, nchars );
    }
    return width;
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


    const TextLayout *layout = TextLayout::instance();
    ScriptItemArray items;
    layout->itemize( items,  str );
    ShapedItem shaped;
    int nchars;
    QGlyphMetrics gm;
    for ( int i = 0; i < items.size() && (nchars = len-items[i].position) > 0; i++ ) {
	layout->shape( shaped, QFont( d ), str, items, i );
	// ### not the best way to ensure positioning has happened.
	(void)layout->width( shaped );
	QFont::Script script = (QFont::Script)shaped.d->analysis.script;
	d->load( script );
	// #### take care of length argument
	FontEngineIface *fe = d->x11data.fontstruct[script];
	QGlyphMetrics m = fe->boundingBox( shaped.d->glyphs, shaped.d->advances, shaped.d->offsets, shaped.d->num_glyphs );
	gm.x = QMIN( gm.x, m.x + gm.xoff );
	gm.y = QMIN( gm.y, m.x + gm.yoff );
	gm.width = QMAX( gm.width, m.width+gm.xoff );
	gm.height = QMAX( gm.height, m.height+gm.yoff );
	gm.xoff += m.xoff;
	gm.yoff += m.yoff;
    }
    return QRect( gm.x, gm.y, gm.width, gm.height );
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
	if (! qfs )
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
