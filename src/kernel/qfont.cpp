/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.cpp#71 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes
**
** Created : 941207
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qfont.h"
#include "qfontdta.h"
#include "qfontmet.h"
#include "qfontinf.h"
#include "qwidget.h"
#include "qpainter.h"
#include "qdict.h"
#include "qstrlist.h"
#include "qdstream.h"
#include <ctype.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qfont.cpp#71 $");


/*!
  \class QFont qfont.h

  \brief The QFont class specifies a font used for drawing text.

  \ingroup fonts
  \ingroup drawing

  A QFont has a series of attributes that can be set to specify an abstract
  font. When actual drawing of text is done Qt will select a font in the
  underlying window system that \link fontmatch.html matches\endlink
  the abstract font as close as possible. The most important attributes
  are
  \link setFamily() family\endlink,
  \link setPointSize() point size\endlink,
  \link setWeight() weight\endlink and
  \link setItalic() italic\endlink.

  One of the QFont constructors take exactly these attributes as
  arguments:

  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPainter p;
	p.begin( this );
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

	p.end();
    }
  \endcode

  In general font handling and loading are costly operations that put
  a heavy load on the window system, this is especially true for
  X11. The QFont class has an internal sharing and reference
  count mechanism in order to speed up copies so QFonts may be passed
  around as arguments, and it has a lazy loading mechanism and does
  not load a font until it \e really has to. It also caches previously
  loaded fonts and under X it caches previously matched font attribute
  combinations.

  Note that the functions returning attribute values in QFont return
  the values previously set, \e not the attributes of the actual
  window system font used for drawing. To get information about the
  actual font use QFontInfo.

  To get font size information use the class QFontMetrics.

  QFont objects make use of implicit sharing.

  \sa QApplication::setFont(), QWidget::setFont(), QPainter::setFont()
*/


/*!
  \internal
  Initializes the internal QFontData structure.
*/

void QFont::init()
{
    d = new QFontData;
    CHECK_PTR( d );
    d->req.pointSize	 = 0;
    d->req.styleHint	 = AnyStyle;
    d->req.charSet	 = Latin1;
    d->req.weight	 = 0;
    d->req.italic	 = FALSE;
    d->req.underline	 = FALSE;
    d->req.strikeOut	 = FALSE;
    d->req.fixedPitch	 = FALSE;
    d->req.hintSetByUser = FALSE;
    d->req.rawMode	 = FALSE;
    d->req.dirty	 = TRUE;
    d->exactMatch	 = FALSE;
}

/*!
  \internal
  Constructs a font that gets a deep copy of \e data.
*/

QFont::QFont( QFontData *data )
{
    d = new QFontData( *data );
    CHECK_PTR( d );
    d->count = 1;				// now a single reference
}

/*!
  \internal
  Detaches the font object from common font data.
*/

void QFont::detach()
{
    if ( d->count != 1 )
	*this = QFont( d );
}

/*!
  Constructs a font object that refers to the default font.
*/

QFont::QFont()
{
    if ( !defFont )
	defFont = new QFont( TRUE );
    d = defFont->d;
    d->ref();
}


/*!
  Constructs a font object with the specified \e family, \e pointSize,
  \e weight and \e italic settings. If \e pointSize is less than or
  equal to 0 it is set to 1.

  \sa setFamily(), setPointSize(), setWeight(), setItalic()
*/

QFont::QFont( const char *family, int pointSize, int weight, bool italic )
{
    init();
    d->req.family    = family;
    if ( pointSize <= 0 )
	pointSize = 1;
    d->req.pointSize = pointSize * 10;
    d->req.weight    = weight;
    d->req.italic    = italic;
}



/*!
  Constructs a font object with the specified \e family, \e pointSize,
  \e weight, \e italic and \a charSet settings.  If \e pointSize is
  less than or equal to 0 it is set to 1.

  \sa setFamily(), setPointSize(), setWeight(), setItalic()
*/

QFont::QFont( const char *family, int pointSize, int weight, bool italic,
	      CharSet charSet)
{
    init();
    d->req.family    = family;
    if ( pointSize <= 0 )
	pointSize = 1;
    d->req.pointSize = pointSize * 10;
    d->req.weight    = weight;
    d->req.italic    = italic;
    d->req.charSet   = charSet;
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
  Destroys the font object.
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
    font.d->ref();
    if ( d->deref() )
	delete d;
    d = font.d;
    return *this;
}


/*!
  Returns the family name set by setFamily().

  Use QFontInfo to find the family name of the window system font actually
  used.

  Example:
  \code
    QFont     font( "Nairobi" );
    QFontInfo info( font );
    debug( "Font family requested is    : \"%s\"", font.family() );
    debug( "Font family actually used is: \"%s\"", info.family() );
  \endcode
  \sa setFamily(), substitute()
*/

const char *QFont::family() const
{
    return d->req.family;
}

/*!
  Sets the family name of the font (e.g. "Helvetica" or "times").

  The family name is case insensitive.

  If the family is not available a default family will be used instead.

  \sa family(), setStyleHint(), QFontInfo,
  \link fontmatch.html font matching\endlink
*/

void QFont::setFamily( const char *family )
{
    if ( d->req.family != family ) {
	detach();
	d->req.family = family;
	d->req.dirty  = TRUE;
    }
}


/*!
  Returns the point size in 1/10ths of a point.
  \sa pointSize()
*/

int QFont::deciPointSize() const
{
    return d->req.pointSize;
}


/*!
  Returns the point size set by setPointSize().

  Use QFontInfo to find the point size of the window system font actually used.

  Example of use:
  \code
    QFont     font( "helvetica" );
    QFontInfo info( font );
    font.setPointSize( 53 );
    debug( "Font size requested is    : %d", font.pointSize() );
    debug( "Font size actually used is: %d", info.pointSize() );
  \endcode

  \sa setPointSize()
*/

int QFont::pointSize() const
{
    return d->req.pointSize / 10;
}

/*!
  Sets the point size to \a pointSize. The point size must be greater
  than zero.
  
  Example:
  \code
    QFont font( "courier" );
    font.setPointSize( 18 );
  \endcode

  If the point size is not available the closest available will be used.

  \sa pointSize(), QFontInfo, \link fontmatch.html font matching\endlink
*/

void QFont::setPointSize( int pointSize )
{
    if ( pointSize <= 0 ) {
#if defined(CHECK_RANGE)
	warning( "QFont::setPointSize: Point size <= 0 (%d)", pointSize );
#endif
	return;
    }
    pointSize *= 10;
    if ( d->req.pointSize != pointSize ) {
	detach();
	d->req.pointSize = (short)pointSize;
	d->req.dirty	 = TRUE;
    }
}



/*!
  Returns the value set by setItalic().

  Use QFontInfo to find the italic value of the window system font actually
  used.
  \sa setItalic()
*/

bool QFont::italic() const
{
    return d->req.italic;
}

/*!
  Sets italic on or off.

  If the mode selected is not available the other will be used.

  \sa italic(), QFontInfo, \link fontmatch.html font matching\endlink
*/

void QFont::setItalic( bool enable )
{
    if ( (bool)d->req.italic != enable ) {
	detach();
	d->req.italic = enable;
	d->req.dirty  = TRUE;
    }
}


/*!
  Returns the weight set by setWeight().

  Use QFontInfo to find the weight of the window system font actually used.
  \sa setWeight(), QFontInfo
*/

int QFont::weight() const
{
    return d->req.weight;
}

/*!
  Sets the weight (or boldness).

  The enum \c QFont::Weight contains the predefined font weights:<ul>
  <li> \c QFont::Light (25)
  <li> \c QFont::Normal (50)
  <li> \c QFont::DemiBold (63)
  <li> \c QFont::Bold (75)
  <li> \c QFont::Black (87)
  </ul>

  Strictly speaking you can use all values in the range [0,99] (where
  0 is ultralight and 99 is extremely black), but there is such a
  thing as asking too much of the underlying window system.

  Example:
  \code
    QFont font( "courier" );
    font.setWeight( QFont::Bold );
  \endcode

  If the specified weight is not available the closest available will
  be used. Use QFontInfo to check the actual weight.

  \sa weight(), QFontInfo, \link fontmatch.html font matching\endlink
*/

void QFont::setWeight( int weight )
{
    if ( weight < 0 || weight > 99 ) {
#if defined(CHECK_RANGE)
	warning( "QFont::setWeight: Value out of range (%d)", weight );
#endif
	return;
    }
    if ( (int)d->req.weight != weight ) {
	detach();
	d->req.weight = weight;
	d->req.dirty  = TRUE;
    }
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
  actually used.

  \sa setUnderline(), QFontInfo::underline()
*/

bool QFont::underline() const
{
    return d->req.underline;
}

/*!
  Sets underline on or off.

  If the mode selected is not available the other will be used.

  \sa underline(), QFontInfo, \link fontmatch.html font matching.\endlink
*/

void QFont::setUnderline( bool enable )
{
    if ( (bool)d->req.underline != enable ) {
	detach();
	d->req.underline = enable;
	d->req.dirty = TRUE;
    }
}


/*!
  Returns the value set by setStrikeOut().

  Use QFontInfo to find the strike out value of the window system font
  actually used.

  \sa setStrikeOut(), QFontInfo::strikeOut().
*/

bool QFont::strikeOut() const
{
    return d->req.strikeOut;
}

/*!
  Sets strike out on or off.

  If the mode selected is not available the other will be used.

  \sa strikeOut(), QFontInfo, \link fontmatch.html font matching\endlink
*/

void QFont::setStrikeOut( bool enable )
{
    if ( (bool)d->req.strikeOut != enable ) {
	detach();
	d->req.strikeOut = enable;
	d->req.dirty = TRUE;
    }
}


/*!
  Returns the value set by setFixedPitch().

  Use QFontInfo to find the fixed pitch value of the window system font
  actually used.
  \sa setFixedPitch(), QFontInfo::fixedPitch()
*/

bool QFont::fixedPitch() const
{
    return d->req.fixedPitch;
}


/*!
  Sets fixed pitch on or off.
  
  A fixed pitch font is a font that has constant character pixel width.  
  If the mode selected is not available the other will be used.

  \sa fixedPitch(), QFontInfo, \link fontmatch.html font matching\endlink
*/

void QFont::setFixedPitch( bool enable )
{
    if ( (bool)d->req.fixedPitch != enable ) {
	detach();
	d->req.fixedPitch = enable;
	d->req.dirty	  = TRUE;
    }
}


/*!
  Returns the StyleHint set by setStyleHint().

  \sa setStyleHint(), QFontInfo::styleHint()
*/

QFont::StyleHint QFont::styleHint() const
{
    return (StyleHint)d->req.styleHint;
}

/*!
  Sets the style hint.

  The style hint is used by the \link fontmatch.html font matching\endlink
  algorithm when a selected font family cannot be found and is used to
  find an appropriate default family.

  The style hint has a default value of \c AnyStyle which leaves the task of
  finding a good default family to the font matching algorithm.

  In this example (which is a complete program) the push button
  will display its text label with the Bavaria font family if this family
  is available, if not it will display its text label with the Times font
  family:
  \code
    #include <qapp.h>
    #include <qpushbt.h>
    #include <qfont.h>

    int main( int argc, char **argv )
    {
	QApplication app( argc, argv );
	QPushButton  push("Push me");

	QFont font( "Bavaria", 18 );	    // preferrred family is Bavaria
	font.setStyleHint( QFont::Times );  // use Times if Bavaria isn't here

	push.setFont( font );
	return app.exec( &push );
    }
  \endcode

  \sa styleHint(), QFontInfo, \link fontmatch.html font matching\endlink
*/

void QFont::setStyleHint( StyleHint hint )
{
    if ( (StyleHint)d->req.styleHint != hint ) {
	detach();
	d->req.styleHint     = hint;
	d->req.hintSetByUser = TRUE;
	d->req.dirty	     = TRUE;
    }
}


/*!
  Returns the character set by setCharSet().

  Use QFontInfo to find the CharSet of the window system font actually used.
  \sa setCharSet()
*/

QFont::CharSet QFont::charSet() const
{
    return (CharSet) d->req.charSet;
}

/*!
  Sets the character set (e.g. \c Latin1).

  If the character set is not available another will be used, for most
  non-trivial applications you will probably not want this to happen since
  it can totally obscure the text shown to the user when the font is
  used. This is why the \link fontmatch.html font matching\endlink
  algorithm gives high priority to finding the correct character set.

  To ensure that the character set is correct you can use the QFontInfo
  class.

  Example:
  \code
    QFont     font( "times", 14 );	     // default character set is Latin1
    QFontInfo info( font );
    if ( info.charSet() != Latin1 )	     // check actual font
	fatal( "Cannot find a Latin 1 Times font" );
  \endcode

  \sa charSet(), QFontInfo, \link fontmatch.html font matching\endlink
*/

void QFont::setCharSet( CharSet charset )
{
    if ( (CharSet)d->req.charSet != charset ) {
	detach();
	d->req.charSet = charset;
	d->req.dirty   = TRUE;
    }
}


/*!
  Returns the value set by setRawMode().
  \sa setRawMode()
*/

bool QFont::rawMode() const
{
    return d->req.rawMode;
}

/*!
  Turns raw mode on if \e enable is TRUE, or turns it off if \e enable is
  FALSE.

  Calling this function only has effect under X windows. If raw mode is
  enabled, Qt will search for an X font with a complete font name matching
  the family name, ignoring all other values set for the QFont.
  If the font name matches several fonts, Qt will use the first font returned
  by X. QFontInfo \e cannot be used to fetch information about a QFont using
  raw mode (it will return the values set in the QFont for all parameters,
  including the family name).

  Example:
  \code
    #if defined(_WS_X11_)
	QFont font( "-*-fixed-*-*-*-*-*-140-75-75-c-*-iso8859-1" );
	font.setRawMode( TRUE );
	if ( !font.exactMatch() )
	    fatal( "Sorry, could not find the X specific font" );
    #endif
  \endcode

  \warning Do not use raw mode unless you really need it!
  \sa rawMode()
*/

void QFont::setRawMode( bool enable )
{
    if ( (bool)d->req.rawMode != enable ) {
	detach();
	d->req.rawMode = enable;
	d->req.dirty   = TRUE;
    }
}


/*!
  Returns TRUE if a window system font exactly matching the settings
  of this font is available.
  \sa QFontInfo, \link fontmatch.html font matching\endlink
*/

bool QFont::exactMatch() const
{
    if ( d->req.dirty )
	load();
    return d->exactMatch;
}


/*!
  Returns TRUE if the this font is equal to \e f, or FALSE if they are
  different.

  Two QFonts are equal if their font attributes are equal.
  If \link setRawMode() raw mode\endlink is enabled for both fonts,
  then only the family fields are compared.

  \sa operator!=()
*/

bool QFont::operator==( const QFont &f ) const
{
    return f.d == d || f.key() == key();
}

/*!
  Returns TRUE if the this font is different from \e f, or FALSE if they are
  equal.

  Two QFonts are different if their font attributes are different.
  If \link setRawMode() raw mode\endlink is enabled for both fonts,
  then only the family fields are compared.

  \sa operator==()
*/

bool QFont::operator!=( const QFont &f ) const
{
    return !(operator==( f ));
}


/*!
  Returns the system default font.
*/

const QFont &QFont::defaultFont()
{
    if ( !defFont )
	defFont = new QFont( TRUE );
    return *defFont;
}

/*!
  Sets the system default font.
*/

void  QFont::setDefaultFont( const QFont &f )
{
    if ( !defFont )
	defFont = new QFont( TRUE );
    *defFont = f;
}


/*****************************************************************************
  QFont substitution management
 *****************************************************************************/

typedef Q_DECLARE(QDictM,char)	       QFontSubst;
typedef Q_DECLARE(QDictIteratorM,char) QFontSubstIt;
static QFontSubst *fontSubst = 0;

static void cleanupFontSubst()			// cleanup substitution dict
{
    delete fontSubst;
}

static void initFontSubst()			// create substitution dict
{
    static const char *initTbl[] = {		// default substitutions
#if defined(_WS_X11_)
	"arial",	"helvetica",
	"helv",		"helvetica",
	"tms rmn",	"times",
#endif
	0,		0
    };

    if ( fontSubst )
	return;
    fontSubst = new QFontSubst( 17,
				FALSE );	// case insensitive
    CHECK_PTR( fontSubst );
    fontSubst->setAutoDelete( TRUE );
    for ( int i=0; initTbl[i] != 0; i += 2 )
	fontSubst->insert( initTbl[i],	qstrdup(initTbl[i+1]) );
    qAddPostRoutine( cleanupFontSubst );
}


/*!
  Returns the font family name to be used whenever \e familyName is
  specified, and not found by the \link fontmatch.html font matching
  algorithm \endlink.  The lookup is case insensitive.

  If there is no substitution for \e familyName, then \e familyName is
  returned.

  Example:
  \code
    QFont::insertSubstitution( "NewYork", "London" );
    QFont::insertSubstitution( "Paris",	  "Texas" );

    QFont::substitute( "NewYork" );	// returns "London"
    QFont::substitute( "PARIS" );	// returns "Texas"
    QFont::substitute( "Rome" );	// returns "Rome"

    QFont::removeSubstitution( "newyork" );
    QFont::substitute( "NewYork" );	// returns "NewYork"
  \endcode

  \sa setFamily(), insertSubstitution(), removeSubstitution()
*/

const char *QFont::substitute( const char *familyName )
{
    if ( !fontSubst )
	initFontSubst();
    const char *f = fontSubst->find( familyName );
    return f ? f : familyName;
}

/*!
  Inserts a new font family name substitution in the family substitution
  table.

  If \e familyName already exists in the substitution table, it will
  be replaced with this new substitution.

  \sa removeSubstitution(), listSubstitutions(), substitute()
*/

void QFont::insertSubstitution( const char *familyName,
				const char *replacementName )
{
    if ( !fontSubst )
	initFontSubst();
    fontSubst->replace( familyName, replacementName );
}

/*!
  Removes a font family name substitution from the family substitution
  table.

  \sa insertSubstitution(), listSubstitutions(), substitute()
*/

void QFont::removeSubstitution( const char *familyName )
{
    if ( !fontSubst )
	initFontSubst();
    if ( fontSubst )
	fontSubst->remove( familyName );
}


/*!
  Returns a sorted list of substituted family names in \e list.

  \sa insertSubstitution(), removeSubstitution(), substitute()
*/

void QFont::listSubstitutions( QStrList *list )
{
    if ( !fontSubst )
	initFontSubst();
    list->clear();
    list->setAutoDelete( TRUE );
    QFontSubstIt it( *fontSubst );
    const char *n;
    while ( (n=it.currentKey()) ) {
	list->inSort( qstrdup(n) );
	++it;
    }
}


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

/*
  Internal function. Sets boolean font settings (except dirty)
  from an unsigned 8-bit number. Used for serialization etc.
*/

static void set_font_bits( Q_UINT8 bits, QFontDef *f )
{
    f->italic	     = (bits & 0x01) != 0;
    f->underline     = (bits & 0x02) != 0;
    f->strikeOut     = (bits & 0x04) != 0;
    f->fixedPitch    = (bits & 0x08) != 0;
    f->hintSetByUser = (bits & 0x10) != 0;
    f->rawMode	     = (bits & 0x20) != 0;
}


static void hex2( uchar n, char *s )
{
    uchar b = (n >> 4) & 0x0f;
    *s++ = b + (b < 10 ? '0' : ('a'-10));
    b = n & 0x0f;
    *s++ = b + (b < 10 ? '0' : ('a'-10));
    *s = '\0';
}

static void hex4( ushort n, char *s )
{
    hex2( (n >> 8) & 0xff, s );
    hex2( n & 0xff, s+2 );
}


/*!
  Returns the font's key, which is a textual representation of the font
  settings. It is typically used to insert and find fonts in a
  dictionary or a cache.
  \sa QDict, QCache
*/

QString QFont::key() const
{
    if ( d->req.rawMode )
	return d->req.family.lower();
    int	    len = d->req.family.length();
    QString s( len+13 );
    UINT8   bits = get_font_bits( d->req );
    char   *p = s.data();
    CHECK_PTR( p );
    hex4( d->req.pointSize, p );
    p += 4;
    if ( len ) {
	strcpy( p, d->req.family.data() );
	while ( *p ) {
	    *p = tolower(*p);
	    p++;
	}
    }
    hex2( bits, p );
    hex2( d->req.weight, p+2 );
    hex2( d->req.hintSetByUser ? (int)d->req.styleHint : (int)QFont::AnyStyle,
	  p+4 );
    hex2( d->req.charSet, p+6 );
    return s;
}


/*****************************************************************************
  QFont stream functions
 *****************************************************************************/

/*!
  \relates QFont
  Writes a font to the stream.
*/

QDataStream &operator<<( QDataStream &s, const QFont &f )
{
    return s << f.d->req.family
	     << (INT16)f.d->req.pointSize
	     << (UINT8)f.d->req.styleHint
	     << (UINT8)f.d->req.charSet
	     << (UINT8)f.d->req.weight
	     << get_font_bits(f.d->req);
}

/*!
  \relates QFont
  Reads a font from the stream.
*/

QDataStream &operator>>( QDataStream &s, QFont &f )
{
    if ( f.d->deref() )
	delete f.d;
    f.d = new QFontData;
    CHECK_PTR( f.d );

    Q_INT16 pointSize;
    Q_UINT8 styleHint, charSet, weight, bits;
    s >> f.d->req.family;
    s >> pointSize;
    s >> styleHint >> charSet >> weight >> bits;

    f.d->req.pointSize	   = pointSize;
    f.d->req.styleHint	   = styleHint;
    f.d->req.charSet	   = charSet;
    f.d->req.weight	   = weight;
    f.d->req.dirty	   = TRUE;
    set_font_bits( bits, &(f.d->req) );

    return s;
}


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

typedef Q_DECLARE(QListM,QFontMetrics) QFontMetricsList;
static QFontMetricsList *fm_list = 0;

static void insertFontMetrics( QFontMetrics *fm )
{
    if ( !fm_list ) {
	fm_list = new QFontMetricsList;
	CHECK_PTR( fm_list );
    }
    fm_list->append( fm );
}

static void removeFontMetrics( QFontMetrics *fm )
{
    if ( !fm_list ) {
#if defined(CHECK_NULL)
	warning( "QFontMetrics::~QFontMetrics: Internal error" );
#endif
	return;
    }
    if ( fm_list->removeRef(fm) && fm_list->isEmpty() ) {
	delete fm_list;
	fm_list = 0;
    }
}


/*!
  Resets all pointers to \e w in all font metrics objects in the
  application.
*/

void QFontMetrics::reset( const QWidget *w )
{
    if ( fm_list ) {
	QFontMetrics *fm = fm_list->first();
	while ( fm ) {
	    if ( fm->type() == Widget && fm->u.w == w )
		fm->u.w = 0;			// detach from widget
	    fm = fm_list->next();
	}
    }
}

/*!
  Resets all pointers to \e p in all font metrics objects in the
  application.
*/

void QFontMetrics::reset( const QPainter *p )
{
    if ( fm_list ) {
	QFontMetrics *fm = fm_list->first();
	while ( fm ) {
	    if ( fm->type() == Painter && fm->u.p == p )
		fm->u.p = 0;			// detach from painter
	    fm = fm_list->next();
	}
    }
}


/*!
  \class QFontMetrics qfontmet.h
  \brief The QFontMetrics class provides font metrics information about fonts.

  \ingroup fonts

  QFontMetrics functions calculate size of characters and strings for a given
  font.

  There are three ways you can create a QFontMetrics object:
  <ol>
  <li> The QFontMetrics constructor with a QFont
  creates a font metrics object for a screen-compatible font,
  i.e. the font must not be a printer font.
  <li> QWidget::fontMetrics() returns the font metrics for a
  widget's current font. The font metrics object is automatically
  updated if somebody sets a new widget font.
  <em>Please read the note below.</em>
  <li> QPainter::fontMetrics() returns the font metrics for a
  painter's current font. The font metrics object is automatically
  updated if somebody sets a new painter font.
  <em>Please read the note below.</em>
  </ol>

  Example:
  \code
    QFont font("times",24);
    QFontMetrics fm(font);
    int w = fm.width("What's the width of this text");
    int h = fm.height();
  \endcode

  <strong>In Qt 2.0 the font metrics object will no longer be
  automatically updated when the widget or painter gets a new font.</strong>
  Make sure you program does not depend on this feature.

  <strong>About efficiency:</strong> We recommend that you use
  the QFontMetrics constructor if you can. This is more efficient
  than getting the font metrics from a widget or a painter. The
  QWidget::fontMetrics() and QPainter::fontMetrics() will become faster
  in Qt 2.0, when we have removed the automatic update policy.

  \sa QFont, QFontInfo
*/

#if QT_VERSION == 200
#error "Fix doc above. No longer automatic font metrics update"
#endif

/*!
  Constructs a font metrics object for \a font.

  The font must be screen-compatible, i.e. a font you use when drawing
  text in \link QWidget widgets\endlink or \link QPixmap pixmaps\endlink.
  If \a font is a printer font, you'll probably get wrong results.

  Use the QPainter::fontMetrics() to get the font metrics when painting.
  This is a little slower than using this constructor, but it always
  gives correct results.
*/

QFontMetrics::QFontMetrics( const QFont &font )
{
    t.flags = FontInternal;
    font.handle();
    if ( font.underline() )
	setUnderlineFlag();
    if ( font.strikeOut() )
	setStrikeOutFlag();
    u.f = font.d->fin;
}

/*!
  \internal
  Constructs a font metrics object for a widget.
*/

QFontMetrics::QFontMetrics( const QWidget *widget )
{
#if defined(CHECK_NULL)
    ASSERT( widget != 0 );
#endif
    t.flags = Widget;
    u.w = (QWidget *)widget;
    u.w->setWFlags( WExportFontMetrics );
    insertFontMetrics( this );
}

/*!
  \internal
  Constructs a font metrics object for a painter.
*/

QFontMetrics::QFontMetrics( const QPainter *painter )
{
#if defined(CHECK_NULL)
    ASSERT( painter != 0 );
#endif
    t.flags = Painter;
    u.p = (QPainter *)painter;
#if defined(CHECK_STATE)
    if ( !u.p->isActive() )
	warning( "QFontMetrics: Get font metrics between QPainter::begin() "
		 "and QPainter::end()" );
#endif
    u.p->setf( QPainter::FontMet );
    insertFontMetrics( this );
}

/*!
  Constructs a copy of \e fm.
*/

QFontMetrics::QFontMetrics( const QFontMetrics &fm )
    : t(fm.t), u(fm.u)
{
    if ( type() != FontInternal )
	insertFontMetrics( this );
}

/*!
  Destroys the font metrics object.
*/

QFontMetrics::~QFontMetrics()
{
    if ( type() != FontInternal )
	removeFontMetrics( this );
}


/*!
  Font metrics assignment.
*/

QFontMetrics &QFontMetrics::operator=( const QFontMetrics &fm )
{
    if ( type() != FontInternal )
	removeFontMetrics( this );
    t.flags = fm.t.flags;
    u = fm.u;
    if ( type() != FontInternal )
	insertFontMetrics( this );
    return *this;
}


/*!
  Returns the pixel width of a \e ch.
  \sa boundingRect()
*/

int QFontMetrics::width( char ch ) const
{
    char tmp[2];
    tmp[1] = '\0';
    tmp[0] = ch;
    return width( tmp, 1 );
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

QRect QFontMetrics::boundingRect( char ch ) const
{
    char tmp[2];
    tmp[1] = '\0';
    tmp[0] = ch;
    return boundingRect( tmp, 1 );
}


/*!
  Returns the font currently set for the widget or painter.
*/

const QFont &QFontMetrics::font() const
{
    switch ( type() ) {
    case Widget:
	return u.w->font();
    case Painter:
	return u.p->font();
    default:
#if defined(CHECK_STATE)
	warning( "QFontMetrics::font: You can only get the font "
		 "when you get the QFontMetrics from either "
		 "QWidget::fontMetrics() or QPainter::fontMetrics()\n"
		 "\tAvoid this function. It will be removed in a "
		 "future version of Qt" );
#endif
    }
    static QFont f;
    return f;
}


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

typedef Q_DECLARE(QListM,QFontInfo) QFontInfoList;
static QFontInfoList *fi_list = 0;

static void insertFontInfo( QFontInfo *fi )
{
    if ( !fi_list ) {
	fi_list = new QFontInfoList;
	CHECK_PTR( fi_list );
    }
    fi_list->append( fi );
}

static void removeFontInfo( QFontInfo *fi )
{
    if ( !fi_list ) {
#if defined(CHECK_NULL)
	warning( "QFontInfo::~QFontInfo: Internal error" );
#endif
	return;
    }
    if ( fi_list->removeRef(fi) && fi_list->isEmpty() ) {
	delete fi_list;
	fi_list = 0;
    }
}


/*!
  Resets all pointers to \e w in all font info objects in the
  application.
*/

void QFontInfo::reset( const QWidget *w )
{
    if ( fi_list ) {
	QFontInfo *fi = fi_list->first();
	while ( fi ) {
	    if ( fi->type() == Widget && fi->u.w == w )
		fi->u.w = 0;			// detach from widget
	    fi = fi_list->next();
	}
    }
}

/*!
  Resets all pointers to \e p in all font metrics objects in the
  application.
*/

void QFontInfo::reset( const QPainter *p )
{
    if ( fi_list ) {
	QFontInfo *fi = fi_list->first();
	while ( fi ) {
	    if ( fi->type() == Painter && fi->u.p == p )
		fi->u.p = 0;			// detach from painter
	    fi = fi_list->next();
	}
    }
}


/*!
  \class QFontInfo qfontinf.h

  \brief The QFontInfo class provides general information about fonts.

  \ingroup fonts

  The QFont class might not always map exactly to the specified font for
  a paint device. The QFontInfo class provides information of the actual
  font that matched a QFont specification.

  There are three ways you can create a QFontInfo object:
  <ol>
  <li> The QFontInfo constructor with a QFont
  creates a font info object for a screen-compatible font,
  i.e. the font must not be a printer font.
  <li> QWidget::fontInfo() returns the font info for a
  widget's current font. The font info object is automatically
  updated if somebody sets a new widget font.
  <em>Please read the note below.</em>
  <li> QPainter::fontInfo() returns the font info for a
  painter's current font. The font info object is automatically
  updated if somebody sets a new painter font.
  <em>Please read the note below.</em>
  </ol>

  Example:
  \code
    QFont font("reykjavik",24);
    QFontInfo fi(font);
    if ( strcmp(font.family(),fi.family()) == 0 ) {
	; // got this font
    } else {
	; // got fi.family() instead
    }
  \endcode

  <strong>In Qt 2.0 the font info object will no longer be
  automatically updated when the widget or painter gets a new font.</strong>
  Make sure you program does not depend on this feature.

  <strong>About efficiency:</strong> We recommend that you use
  the QFontInfo constructor if you can. This is more efficient
  than getting the font info from a widget or a painter. The
  QWidget::fontInfo() and QPainter::fontInfo() will become faster
  in Qt 2.0, when we have removed the automatic update policy.

  \sa QFont, QFontMetrics
*/

#if QT_VERSION == 200
#error "Fix doc above. No longer automatic font info update"
#error "Get rid of FontInternalExactMatch"
#endif

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
    t.flags = FontInternal;
    font.handle();
    if ( font.underline() )
	setUnderlineFlag();
    if ( font.strikeOut() )
	setStrikeOutFlag();
    if ( font.exactMatch() )
	setExactMatchFlag();
    u.f = font.d->fin;
}

/*!
  \internal
  Constructs a font info object for a widget.
*/

QFontInfo::QFontInfo( const QWidget *widget )
{
#if defined(CHECK_NULL)
    ASSERT( widget != 0 );
#endif
    t.flags = Widget;
    u.w = (QWidget *)widget;
    u.w->setWFlags( WExportFontInfo );
    insertFontInfo( this );
}

/*!
  \internal
  Constructs a font info object for a painter.
*/

QFontInfo::QFontInfo( const QPainter *painter )
{
#if defined(CHECK_NULL)
    ASSERT( painter != 0 );
#endif
    t.flags = Painter;
    u.p = (QPainter *)painter;
#if defined(CHECK_STATE)
    if ( !u.p->isActive() )
	warning( "QFontInfo: Get font info between QPainter::begin() "
		 "and QPainter::end()" );
#endif
    u.p->setf( QPainter::FontInf );
    insertFontInfo( this );			// register this object
}

/*!
  Constructs a copy of \e fi.
*/

QFontInfo::QFontInfo( const QFontInfo &fi )
    : t(fi.t), u(fi.u)
{
    if ( type() != FontInternal )
	insertFontInfo( this );
}

/*!
  Destroys the font info object.
*/

QFontInfo::~QFontInfo()
{
    if ( type() != FontInternal )
	removeFontInfo( this );
}


/*!
  Font info assignment.
*/

QFontInfo &QFontInfo::operator=( const QFontInfo &fi )
{
    if ( type() != FontInternal )
	removeFontInfo( this );
    t.flags = fi.t.flags;
    u = fi.u;
    if ( type() != FontInternal )
	insertFontInfo( this );
    return *this;
}


/*!
  Returns the family name of the matched window system font.
  \sa QFont::family()
*/

const char *QFontInfo::family() const
{
    return spec()->family;
}

/*!
  Returns the point size of the matched window system font.
  \sa QFont::pointSize()
*/

int QFontInfo::pointSize() const
{
    return spec()->pointSize / 10;
}

/*!
  Returns the italic value of the matched window system font.
  \sa QFont::italic()
*/

bool QFontInfo::italic() const
{
    return spec()->italic;
}

/*!
  Returns the weight of the matched window system font.

  \sa QFont::weight(), bold()
*/

int QFontInfo::weight() const
{
    return (int)spec()->weight;
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

  \internal Here we read the underline flag directly from the QFont.  This
  is ok for X11 and for Windows because we always get what we want.
*/

bool QFontInfo::underline() const
{
    if ( type() == FontInternal ) {
	return underlineFlag();
    } else if ( type() == Widget && u.w ) {
	return u.w->font().underline();
    } else if ( type() == Painter && u.p ) {
	return u.p->font().underline();
    }
#if defined(CHECK_NULL)
    warning( "QFontInfo::underline: Invalid font info" );
#endif
    return FALSE;
}

/*!
  Returns the strike out value of the matched window system font.
  \sa QFont::strikeOut()

  \internal Here we read the strikeOut flag directly from the QFont.  This
  is ok for X11 and for Windows because we always get what we want.
*/

bool QFontInfo::strikeOut() const
{
    if ( type() == FontInternal ) {
	return strikeOutFlag();
    } else if ( type() == Widget && u.w ) {
	return u.w->font().strikeOut();
    } else if ( type() == Painter && u.p ) {
	return u.p->font().strikeOut();
    }
#if defined(CHECK_NULL)
    warning( "QFontInfo::strikeOut: Invalid font info" );
#endif
    return FALSE;
}

/*!
  Returns the fixed pitch value of the matched window system font.
  A fixed pitch font is a font that has constant character pixel width.  
  \sa QFont::fixedPitch()
*/

bool QFontInfo::fixedPitch() const
{
    return spec()->fixedPitch;
}

/*!
  Returns the style of the matched window system font.

  Currently only returns the hint set in QFont.
  \sa QFont::styleHint()
*/

QFont::StyleHint QFontInfo::styleHint() const
{
    return (QFont::StyleHint)spec()->styleHint;
}

/*!
  Returns the character set of the matched window system font.
  \sa QFont::charSet()
*/

QFont::CharSet QFontInfo::charSet() const
{
    return (QFont::CharSet)spec()->charSet;
}

/*!
  Returns TRUE if the font is a raw mode font.

  If it is a raw mode font, all other functions in QFontInfo will return the
  same values set in the QFont, regardless of the font actually used.

  \warning The default font is a raw-mode font.

  \sa QFont::rawMode()
*/

bool QFontInfo::rawMode() const
{
    return spec()->rawMode;
}

/*!
  Returns TRUE if the matched window system font is exactly the one specified
  by the font.
  \sa QFont::exactMatch()
*/

bool QFontInfo::exactMatch() const
{
    bool m;
    switch ( type() ) {
    case FontInternal:
	m = exactMatchFlag();
	break;
    case Widget:
	m = u.w->font().exactMatch();
	break;
    case Painter:
	m = u.p->font().exactMatch();
	break;
    default:
	m = FALSE;
    }
    return m;
}


/*!
  Returns the font currently set for the widget or painter.
*/

const QFont &QFontInfo::font() const
{
    switch ( type() ) {
    case Widget:
	return u.w->font();
    case Painter:
	return u.p->font();
    default:
#if defined(CHECK_STATE)
	warning( "QFontInfo::font: You can only get the font "
		 "when you get the QFontInfo from either "
		 "QWidget::fontInfo() or QPainter::fontInfo()\n"
		 "\tAvoid this function. It will be removed in a "
		 "future version of Qt" );
#endif
    }
    static QFont f;
    return f;
}
