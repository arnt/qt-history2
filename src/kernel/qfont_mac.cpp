/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont_mac.cpp
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for mac
**
** Created : 001019
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
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
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

// REVISED: arnt

#include "qwidget.h"
#include "qpainter.h"
#include "qfontdata_p.h"
#include "qfontdatabase.h"
#include "qstrlist.h"
#include "qcache.h"
#include "qdict.h"
#include "qtextcodec.h"
#include "qapplication.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qdir.h" // Font Guessing
#include "qmap.h"


QFont::CharSet QFont::defaultCharSet = QFont::AnyCharSet;

/*!
  Internal function that uses locale information to find the preferred
  character set of loaded fonts.

  \internal
  Uses QTextCodec::codecForLocale() to find the character set name.
*/
void QFont::locale_init()
{
    qDebug( "QFont::locale_init" );
}

/*!
  Internal function that initializes the font system.

  \internal
  The font cache and font dict do not alloc the keys. The key is a QString
  which is shared between QFontInternal and QXFontName.
*/

void QFont::initialize()
{
    qDebug( "QFont::initialize" );
}

/*!
  Internal function that cleans up the font system.
*/

void QFont::cleanup()
{
    qDebug( "QFont::cleanup" );
}

/*!
  Internal function that dumps font cache statistics.
*/

void QFont::cacheStatistics()
{
    qDebug( "QFont::cacheStatistics" );
}

/* Clears the internal cache of mappings from QFont instances to X11
    (XLFD) font names. Called from QPaintDevice::x11App
*/

// If d->req.dirty is not TRUE the font must have been loaded
// and we can safely assume that d->fin is a valid pointer:

/*!
  Returns the window system handle to the font, for low-level
  access.  Using this function is \e not portable.
*/

Qt::HANDLE QFont::handle() const
{
    qDebug( "QFont::handle" );
    return 0;
}


/*
  Fills in a font definition (QFontDef) from an XLFD (X Logical Font
  Description). Returns TRUE if the the given xlfd is valid. If the xlfd
  is valid the encoding name (charset registry + "-" + charset encoding)
  is returned in /e encodingName if /e encodingName is non-zero. The
  fileds lbearing and rbearing are not given any values.
 */


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
    qDebug( "QFont::rawName" );
    return QString();
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
void QFont::setRawName( const QString & )
{
    qDebug( "QFont::setRawName" );
}



/*!
  Returns TRUE if the font attributes have been changed and the font has to
  be (re)loaded, or FALSE if no changes have been made.
*/

bool QFont::dirty() const
{
    qDebug( "QFont::dirty" );
    return FALSE;
}


/*!
  Returns the family name that corresponds to the current style hint.
*/

QString QFont::defaultFamily() const
{
    qDebug( "QFont::defaultFamily" );
    return QString::fromLatin1("helvetica");
}


/*!
  Returns a last resort family name for the font matching algorithm.

  \sa lastResortFont()
*/

QString QFont::lastResortFamily() const
{
    qDebug( "QFont::lastResortFamily" );
    return QString::fromLatin1("helvetica");
}


static const char * const tryFonts[] = {
    "6x13",
    "7x13",
    "8x13",
    "9x15",
    "fixed",
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

QString QFont::lastResortFont() const
{
    qDebug( "QFont::lastResortFont" );
    static QString last;
    if ( !last.isNull() )			// already found
	return last;
    int i = 0;
    const char* f;
    while ( (f = tryFonts[i]) ) {
	last = QString::fromLatin1(f);
	if ( TRUE ) {
	    return last;
	}
	i++;
    }
#if defined(CHECK_NULL)
    qFatal( "QFont::lastResortFont: Cannot find any reasonable font" );
#endif
    return last;
}



#ifndef QT_NO_CODECS
#endif //QT_NO_CODECS


/*!
  Initializes the font information in the font's QFontInternal data.
  This function is called from load() for a new font.
*/

void QFont::initFontInfo() const
{
    qDebug( "QFont::lastResortFont" );
}


/*!
  Loads the font.
*/

void QFont::load() const
{
    qDebug( "QFont::load" );
}


/*****************************************************************************
  QFont_Private member functions
 *****************************************************************************/

#define exactScore	   0xfffe
#define exactNonUnicodeScore  0xffff

#define CharSetScore	 0x80
#define PitchScore	 0x40
#define SizeScore	 0x20
#define ResolutionScore	 0x10
#define WeightScore	 0x08
#define SlantScore	 0x04
#define WidthScore	 0x02
#define NonUnicodeScore	 0x01

//
// Returns a score describing how well a font name matches the contents
// of a font.
//


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

const QFontDef *QFontMetrics::spec() const
{
    qDebug( "QFontMetrics::spec" );
    return 0;
}

#ifdef _WS_X11_
void *QFontMetrics::fontStruct() const
{
    qDebug( "QFontMetrics::fontStruct" );
    return 0;
}

void *QFontMetrics::fontSet() const
{
    qDebug( "QFontMetrics::fontSet" );
    return 0;
}

const QTextCodec *QFontMetrics::mapper() const
{
    qDebug( "QFontMetrics::mapper" );
    return 0;
}
#endif

#undef  FS
#define FS (painter ? (XFontStruct*)fontStruct() : fin->fontStruct())
#undef  SET
#define SET ((XFontSet)fontSet())

// How to calculate metrics from ink and logical rectangles.
#define LBEARING(i,l) (i.x+l.x)
#define RBEARING(i,l) (i.width-l.width)
#define ASCENT(i,l) (-i.y)
#define DESCENT(i,l) (i.height+i.y-1)


#ifdef _WS_X11_
int QFontMetrics::printerAdjusted(int val) const
{
    qDebug( "QFontMetrics::printerAdjusted" );
    return val;
}
#endif

/*!
  Returns the maximum ascent of the font.

  The ascent is the distance from the base line to the uppermost line
  where pixels may be drawn.

  \sa descent()
*/

int QFontMetrics::ascent() const
{
    return 1;
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
    qDebug( "QFontMetrics::descent" );
    return 1;
}


/*!
  Returns TRUE if \a ch is a valid character in the font.
*/
bool QFontMetrics::inFont(QChar) const
{
    return TRUE; // ###### XFontSet range?
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
int QFontMetrics::leftBearing(QChar) const
{
    qDebug( "QFontMetrics::leftBearing" );
    return 1;
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
int QFontMetrics::rightBearing(QChar) const
{
    qDebug( "QFontMetrics::rightBearing" );
    return 1;
}

/*!
  Returns the minimum left bearing of the font.

  This is the smallest leftBearing(char) of all characters in the font.

  Note that this function can be very slow if the font is big.

  \sa minRightBearing(), leftBearing(char)
*/
int QFontMetrics::minLeftBearing() const
{
    qDebug( "QFontMetrics::minLeftBearing" );
    return 1;
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
    qDebug( "QFontMetrics::minRightBearing" );
    return 1;
}


/*!
  Returns the height of the font.

  This is always equal to ascent()+descent()+1 (the 1 is for the base line).

  \sa leading(), lineSpacing()
*/

int QFontMetrics::height() const
{
    qDebug( "QFontMetrics::height" );
    return 1;
}

/*!
  Returns the leading of the font.

  This is the natural inter-line spacing.

  \sa height(), lineSpacing()
*/

int QFontMetrics::leading() const
{
    qDebug( "QFontMetrics::leading" );
    return 1;
}


/*!
  Returns the distance from one base line to the next.

  This value is always equal to leading()+height().

  \sa height(), leading()
*/

int QFontMetrics::lineSpacing() const
{
    qDebug( "QFontMetrics::lineSpacing" );
    return 2;
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

int QFontMetrics::width( QChar ) const
{
    qDebug( "QFontMetrics::width" );
    return 12;
}

/*!
  Returns the width in pixels of the first \e len characters of \e str.

  If \e len is negative (the default value is), the whole string is used.

  Note that this value is \e not equal to boundingRect().width();
  boundingRect() returns a rectangle describing the pixels this string
  will cover whereas width() returns the distance to where the next string
  should be drawn.

  \sa boundingRect()
*/

int QFontMetrics::width( const QString &, int len ) const
{
    qDebug( "QFontMetrics::width" );
    return len*12;
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

QRect QFontMetrics::boundingRect( const QString &, int len ) const
{
    qDebug( "QPainter::boundingRect" );
    return QRect( 0, -1, len*12, 13 );
}


/*!
  Returns the width of the widest character in the font.
*/

int QFontMetrics::maxWidth() const
{
    qDebug( "QFontMetrics::maxWidth" );
    return 12;
}


/*!
  Returns the distance from the base line to where an underscore should be
  drawn.
  \sa strikeOutPos(), lineWidth()
*/

int QFontMetrics::underlinePos() const
{
    qDebug( "QFontMetrics::underlinePos" );
    return 6;
}


/*!
  Returns the distance from the base line to where the strike-out line
  should be drawn.
  \sa underlinePos(), lineWidth()
*/

int QFontMetrics::strikeOutPos() const
{
    qDebug( "QFontMetrics::strikeOutPos" );
    return 6;
}


/*!
  Returns the width of the underline and strike-out lines, adjusted for
  the point size of the font.
  \sa underlinePos(), strikeOutPos()
*/

int QFontMetrics::lineWidth() const
{
    qDebug( "QFontMetrics::lineWidth" );
    return 1;
}


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

const QFontDef *QFontInfo::spec() const
{
    qDebug( "QFontInfo::spec" );
    return 0;
}


/*****************************************************************************
  QFontData member functions
 *****************************************************************************/

const QTextCodec* QFontData::mapper() const
{
    qDebug( "QFontData::mapper" );
    return 0;
}

void* QFontData::fontSet() const
{
    qDebug( "QFontData::fontSet" );
    return 0;
}


/*****************************************************************************
  Internal X font functions
 *****************************************************************************/

//
// Splits an X font name into fields separated by '-'
//

//
// Computes the line width (underline,strikeout) for the X font
// and fills in the X resolution of the font.
//

//
// Converts a weight string to a value
//

int qFontGetWeight( const QCString &, bool )
{
    return QFont::Normal;
}

/*!
  Returns the logical pixel height of characters in the font if shown on
  the screen.
*/
int QFont::pixelSize() const
{
    qDebug( "QFont::Normal" );
    return 12;
}

/*!
  Sets the logical pixel height of characters in the font if shown on
  the screen.
*/
void QFont::setPixelSizeFloat( float )
{
    qDebug( "QFont::setPixelSizeFloat" );
}

