/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.cpp#24 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes
**
** Author  : Eirik Eng
** Created : 941207
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
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

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfont.cpp#24 $";
#endif


/*----------------------------------------------------------------------------
  \class QFont qfont.h
  \brief The QFont class specifies a font used for drawing text.

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
  void MyWidget::paintEvent( QPaintEvent * ) {
      QPainter p;
      p.begin( this );
					// times, 12pt, normal
      p.setFont( QFont( "times" ) );
      p.drawText( 10, 20, "Text1" );
					// helvetica, 18pt, normal
      p.setFont( QFont( "helvetica", 18 ) );
      p.drawText( 10, 120, "Text2" );
					// courier, 18pt, bold
      p.setFont( QFont( "courier", 24, QFont::Bold ) );
      p.drawText( 10, 220, "Text3" );
					// lucida, 24pt, bold, italic
      p.setFont( QFont( "lucida", 36, QFont::Bold, TRUE ) );
      p.drawText( 10, 320, "Text4" );

      p.end();
  }
  \endcode

  In general font handling and loading are costly operations that put
  a heavy load on the window system, this is especially true for
  X-Windows. The QFont class has an internal sharing and reference count
  mechanism, it has a lazy loading mechanism and does not match and
  load a font until it \e really has to. It also caches previously loaded
  fonts and under X it caches previously matched font attribute
  combinations.

  Note that the functions returning attribute values in QFont return
  the values previously set, \e not the attributes of the actual
  window system font used for drawing. To get information about the
  actual font use QFontInfo.

  To get font size information use the class QFontMetrics.

  QFont objects make use of implicit sharing.

  \sa QApplication::setFont(), QWidget::setFont(), QPainter::setFont()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \internal
  Initializes the internal QFontData structure.
 ----------------------------------------------------------------------------*/

void QFont::init()
{
    d = new QFontData;
    CHECK_PTR( d );
    d->req.styleHint	 = AnyStyle;
    d->req.charSet	 = Latin1;
    d->req.underline	 = FALSE;
    d->req.strikeOut	 = FALSE;
    d->req.fixedPitch	 = FALSE;
    d->req.hintSetByUser = FALSE;
    d->req.rawMode	 = FALSE;
    d->req.dirty	 = TRUE;
    d->act.dirty	 = TRUE;
    d->exactMatch	 = FALSE;
}

/*----------------------------------------------------------------------------
  \internal
  Constructs a font that gets a deep copy of \e data.
 ----------------------------------------------------------------------------*/

QFont::QFont( QFontData *data )			// copies a font
{
    d  = new QFontData;
    CHECK_PTR( d );
    *d = *data;
    d->count = 1;				// now a single reference
}

/*----------------------------------------------------------------------------
  \internal
  Detaches the font object from common font data.
 ----------------------------------------------------------------------------*/

void QFont::detach()
{
    if ( d->count != 1 )
	*this = QFont( d );
}

/*----------------------------------------------------------------------------
  Constructs a font object that refers to the default font.
 ----------------------------------------------------------------------------*/

QFont::QFont()
{
    d = defFont->d;
    d->ref();
}

/*----------------------------------------------------------------------------
  Constructs a font object with the specified \e family, \e pointSize,
  \e weight and \e italic settings.
  \sa setFamily(), setPointSize(), setWeight(), setItalic()
 ----------------------------------------------------------------------------*/

QFont::QFont( const char *family, int pointSize, int weight, bool italic )
{
    init();
    d->req.family    = family;
    d->req.pointSize = pointSize * 10;
    d->req.weight    = weight;
    d->req.italic    = italic;
    d->xfd	     = 0;
}

/*----------------------------------------------------------------------------
  Constructs a font that is a copy of \e font.
 ----------------------------------------------------------------------------*/

QFont::QFont( const QFont &font )
{
    d = font.d;
    d->ref();
}

/*----------------------------------------------------------------------------
  Destroys the font object.
 ----------------------------------------------------------------------------*/

QFont::~QFont()
{
    if ( d->deref() )
	delete d;
}


/*----------------------------------------------------------------------------
  Assigns \e font to this font and returns a reference to this font.
 ----------------------------------------------------------------------------*/

QFont &QFont::operator=( const QFont &font )
{
    font.d->ref();
    if ( d->deref() )
	delete d;
    d = font.d;
    return *this;
}


/*----------------------------------------------------------------------------
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
 ----------------------------------------------------------------------------*/

const char *QFont::family() const
{
    return d->req.family;
}

/*----------------------------------------------------------------------------
  Sets the family name of the font (e.g. "Helvetica" or "times").

  The family name is case insensitive.

  If the family is not available a default family will be used instead.

  \todo Use a table of MANY different family names to find a good default
  family.

  \sa family(), setStyleHint(), QFontInfo,
  \link fontmatch.html font matching\endlink
 ----------------------------------------------------------------------------*/

void QFont::setFamily( const char *family )
{
    if ( d->req.family != family ) {
	detach();
	d->req.family = family;
	d->req.dirty  = TRUE;
    }
}


/*----------------------------------------------------------------------------
  Returns the point size in 1/10ths of a point.
  \sa pointSize()
 ----------------------------------------------------------------------------*/

int QFont::deciPointSize() const
{
    return d->req.pointSize;
}


/*----------------------------------------------------------------------------
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
 ----------------------------------------------------------------------------*/

int QFont::pointSize() const
{
    return d->req.pointSize / 10;
}

/*----------------------------------------------------------------------------
  Sets the point size (e.g. 12 or 18). If the point size is not available
  the closest available will be used.

  Setting of point sizes less than or equal to 0 will be ignored.

  \sa pointSize(), QFontInfo, \link fontmatch.html font matching\endlink
 ----------------------------------------------------------------------------*/

void QFont::setPointSize( int pointSize )
{
    if ( pointSize <= 0 ) {
#if defined(CHECK_RANGE)
	warning( "QFont::setPointSize: Point size <= 0 (%i)",
		 pointSize );
#endif
	return;
    }
    if ( d->req.pointSize != pointSize ) {
	detach();
	d->req.pointSize = (short)(pointSize*10);
	d->req.dirty	 = TRUE;
    }
}



/*----------------------------------------------------------------------------
  Returns the value set by setItalic().

  Use QFontInfo to find the italic value of the window system font actually
  used.
  \sa setItalic()
 ----------------------------------------------------------------------------*/

bool QFont::italic() const
{
    return d->req.italic;
}

/*----------------------------------------------------------------------------
  Sets italic on or off.

  If the mode selected is not available the other will be used.

  \sa italic(), QFontInfo, \link fontmatch.html font matching\endlink
 ----------------------------------------------------------------------------*/

void QFont::setItalic( bool enable )
{
    if ( (bool)d->req.italic != enable ) {
	detach();
	d->req.italic = enable;
	d->req.dirty  = TRUE;
    }
}


/*----------------------------------------------------------------------------
  Returns the weight set by setWeight().

  Use QFontInfo to find the weight of the window system font actually used.
  \sa setWeight(), QFontInfo
 ----------------------------------------------------------------------------*/

int QFont::weight() const
{
    return (int)d->req.weight;
}

/*----------------------------------------------------------------------------
  Sets the weight (or boldness).

  The weight must be in the range [0,99] (where 0 is ultralight and 99 is
  extremely black), the values of the enum type \c Weight can also be used.

  Example:
  \code
    QFont font( "courier" );
    font.setWeight( QFont::Bold );
  \endcode

  If the specified weight is not available the closest available will be used.
  Use QFontInfo to check the actual weight.
  Setting of weights outside the legal range will be ignored.

  \sa weight(), QFontInfo, \link fontmatch.html font matching\endlink
 ----------------------------------------------------------------------------*/

void QFont::setWeight( int weight )
{
#if defined(CHECK_RANGE)
    if ( weight < 0 || weight > 99 ) {
	warning( "QFont::setWeight: Value out of range (%d)", weight );
	return;
    }
#endif
    if ( (int)d->req.weight != weight ) {
	detach();
	d->req.weight = weight;
	d->req.dirty  = TRUE;
    }
}


/*----------------------------------------------------------------------------
  Returns the value set by setUnderline().

  Use QFontInfo to find the underline value of the window system font
  actually used.

  \sa setUnderline(), QFontInfo.
 ----------------------------------------------------------------------------*/

bool QFont::underline() const
{
    return (int) d->req.underline;
}

/*----------------------------------------------------------------------------
  Sets underline on or off.

  If the mode selected is not available the other will be used.

  \sa underline(), QFontInfo, \link fontmatch.html font matching\endlink.
 ----------------------------------------------------------------------------*/

void QFont::setUnderline( bool enable )
{
    if ( (bool)d->req.underline != enable ) {
	detach();
	d->req.underline = enable;
	d->act.underline = enable;		// underline always possible
    }
}


/*----------------------------------------------------------------------------
  Returns the value set by setStrikeOut().

  Use QFontInfo to find the strike out value of the window system font
  actually used.

  \sa setStrikeOut(), QFontInfo.
 ----------------------------------------------------------------------------*/

bool QFont::strikeOut() const
{
    return (int) d->req.strikeOut;
}

/*----------------------------------------------------------------------------
  Sets strike out on or off.

  If the mode selected is not available the other will be used.

  \sa strikeOut(), QFontInfo, \link fontmatch.html font matching\endlink
 ----------------------------------------------------------------------------*/

void QFont::setStrikeOut( bool enable )
{
    if ( (bool)d->req.strikeOut != enable ) {
	detach();
	d->req.strikeOut = enable;
	d->act.strikeOut = enable;		// strikeOut always posible
    }
}


/*----------------------------------------------------------------------------
  Returns the value set by setFixedPitch().

  Use QFontInfo to find the fixed pitch value of the window system font
  actually used.
  \sa setFixedPitch()
 ----------------------------------------------------------------------------*/

bool QFont::fixedPitch() const
{
    return d->req.fixedPitch;
}


/*----------------------------------------------------------------------------
  Sets fixed pitch on or off. If the mode selected is not available
  the other will be used. A fixed pitch font is a font that has constant
  character pixel width.
  \sa fixedPitch(), QFontInfo, \link fontmatch.html font matching\endlink
 ----------------------------------------------------------------------------*/

void QFont::setFixedPitch( bool enable )
{
    if ( (bool)d->req.fixedPitch != enable ) {
	detach();
	d->req.fixedPitch = enable;
	d->req.dirty	  = TRUE;
    }
}


/*----------------------------------------------------------------------------
  Returns the StyleHint set by setStyleHint().
  \sa setStyleHint()
 ----------------------------------------------------------------------------*/

QFont::StyleHint QFont::styleHint() const
{
    return (StyleHint) d->req.styleHint;
}

/*----------------------------------------------------------------------------
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
        font.setStyleHint( QFont::Times );  // use Times if such family

        push.setFont( font );
        push.show();
        return app.exec( &push );
    }
  \endcode

  \sa styleHint(), QFontInfo, \link fontmatch.html font matching\endlink
 ----------------------------------------------------------------------------*/

void QFont::setStyleHint( StyleHint hint )
{
    if ( (bool)d->req.styleHint != hint ) {
	detach();
	d->req.styleHint     = hint;
	d->req.hintSetByUser = TRUE;
	d->req.dirty	     = TRUE;
    }
}


/*----------------------------------------------------------------------------
  Returns the character set by setCharSet().

  Use QFontInfo to find the CharSet of the window system font actually used.
  \sa setCharSet()
 ----------------------------------------------------------------------------*/

QFont::CharSet QFont::charSet() const
{
    return (CharSet) d->req.charSet;
}

/*----------------------------------------------------------------------------
  Sets the character set (e.g. \c Latin1).

  If the character set is not available another will be used, for most
  non-trivial applications you will probably not want this to happen since
  it can totally obscure the text shown to the user when the font is
  used. This is why the \link fontmatch.html font matching\endlink
  algorithm gives high priority to finding the correct character set.

  (Currently using the correct font family has higher priority than
  using the correct character set. We are not certain if this should
  be reversed and might do so in the 1.0 release. If you have opinions
  about this please mail us!)

  To ensure that the character set is correct you can use the QFontInfo
  class.

  Example:
  \code
    QFont     font( "times", 14 );	     // default character set is Latin1
    QFontInfo info( font );
    if ( info.charSet() != Latin1 )	     // Check info, \e NOT font
        fatal( "Cannot find a Latin 1 Times font" );
  \endcode
  \sa charSet(), QFontInfo, \link fontmatch.html font matching\endlink
 ----------------------------------------------------------------------------*/

void QFont::setCharSet( CharSet charset )
{
    if ( (bool)d->req.charSet != charset ) {
	detach();
	d->req.charSet = charset;
	d->req.dirty   = TRUE;
    }
}


/*----------------------------------------------------------------------------
  Returns the value set by setRawMode.
  \sa setRawMode()
 ----------------------------------------------------------------------------*/

bool QFont::rawMode() const
{
    return d->req.rawMode;
}

/*----------------------------------------------------------------------------
  Sets raw mode on or off.

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
	    debug( "Sorry, could not find the X specific font" );
    #endif
  \endcode

  \warning Do not use raw mode unless you really need it!
  \sa rawMode()
 ----------------------------------------------------------------------------*/

void QFont::setRawMode( bool enable )
{
    if ( (bool)d->req.rawMode != enable ) {
	detach();
	d->req.rawMode = enable;
	d->req.dirty   = TRUE;
    }
}


/*----------------------------------------------------------------------------
  Returns TRUE if a window system font exactly matching the settings
  of this font is available.
  \sa QFontInfo, \link fontmatch.html font matching\endlink
 ----------------------------------------------------------------------------*/

bool QFont::exactMatch() const
{
    if ( d->req.dirty )
	loadFont();
    return d->exactMatch;
}


/*----------------------------------------------------------------------------
  Returns TRUE if the this font is equal to \e f, or FALSE if they are
  different.

  Two QFonts are equal if their font attributes are equal.
  If \link setRawMode() raw mode\endlink is enabled for both fonts,
  then only the family fields are compared.
  \sa operator!=()
 ----------------------------------------------------------------------------*/

bool QFont::operator==( const QFont &f ) const
{
    return f.d == d ||
	   f.d->req.rawMode == d->req.rawMode &&
	   ( f.d->req.rawMode && f.d->req.family == d->req.family ||
	     f.d->req.pointSize	    == d->req.pointSize	    &&
	     f.d->req.styleHint	    == d->req.styleHint	    &&
	     f.d->req.charSet	    == d->req.charSet	    &&
	     f.d->req.weight	    == d->req.weight	    &&
	     f.d->req.italic	    == d->req.italic	    &&
	     f.d->req.underline	    == d->req.underline	    &&
	     f.d->req.strikeOut	    == d->req.strikeOut	    &&
	     f.d->req.fixedPitch    == d->req.fixedPitch    &&
	     f.d->req.hintSetByUser == d->req.hintSetByUser &&
	     f.d->req.family	    == d->req.family	     );
}

/*----------------------------------------------------------------------------
  Returns TRUE if the this font is different from \e f, or FALSE if they are
  equal.

  Two QFonts are different if their font attributes are different.
  If \link setRawMode() raw mode\endlink is enabled for both fonts,
  then only the family fields are compared.
  \sa operator==()
 ----------------------------------------------------------------------------*/

bool QFont::operator!=( const QFont &f ) const
{
    return !(operator==( f ));
}


/*!
  Returns the system default font.
*/

const QFont &QFont::defaultFont()
{
    return *defFont;
}

/*!
  Sets the system default font.
*/

void  QFont::setDefaultFont( const QFont &f )
{
    *defFont = f;
}


/*****************************************************************************
  QFont substitution management
 *****************************************************************************/

typedef declare(QDictM,char)	     QFontSubst;
typedef declare(QDictIteratorM,char) QFontSubstIt;
static QFontSubst *fontSubst = 0;

static void cleanupFontSubst()			// cleanup substitution dict
{
    delete fontSubst;
}

static void initFontSubst()			// create substitution dict
{
    static const char *initTbl[] = {		// default substitutions
	"arial",	"helvetica",
	"helv",		"helvetica",
	"tms rmn",	"times",
	0,		0
    };

    if ( fontSubst )
	return;
    fontSubst = new QFontSubst( 17,
				FALSE );	// case insensitive
    CHECK_PTR( fontSubst );
    fontSubst->setAutoDelete( TRUE );
    for ( int i=0; initTbl[i] != 0; i += 2 )
	fontSubst->insert( initTbl[i],	strdup(initTbl[i+1]) );
    qAddPostRoutine( cleanupFontSubst );

}


/*----------------------------------------------------------------------------
  Returns the font family name to be used whenever \e familyName is
  specified.  The lookup is case insensitive.

  If there is no substitution for \e familyName, then \e familyName is
  returned.

  Example:
  \code
    QFont::insertSubstitution( "NewYork", "London" );
    QFont::insertSubstitution( "Paris",   "Texas" );

    QFont::substitute( "NewYork" );	// returns "London"
    QFont::substitute( "PARIS" );	// returns "Texas"
    QFont::substitute( "Rome" );	// returns "Rome"

    QFont::removeSubstitution( "newyork" );
    QFont::substitute( "NewYork" );	// returns "NewYork"
  \endcode
  \sa setFamily(), insertSubstitution(), removeSubstitution()
 ----------------------------------------------------------------------------*/

const char *QFont::substitute( const char *familyName )
{
    if ( !fontSubst )
	initFontSubst();
    const char *f = fontSubst->find( familyName );
    return f ? f : familyName;
}

/*----------------------------------------------------------------------------
  Inserts a new font family name substitution in the family substitution
  table.

  If \e familyName already exists in the substitution table, it will
  be replaced with this new substitution.

  \sa removeSubstitution(), listSubstitutions(), substitute()
 ----------------------------------------------------------------------------*/

void QFont::insertSubstitution( const char *familyName,
				const char *replacementName )
{
    if ( !fontSubst )
	initFontSubst();
    fontSubst->replace( familyName, replacementName );
}

/*----------------------------------------------------------------------------
  Removes a font family name substitution from the family substitution
  table.

  \sa insertSubstitution(), listSubstitutions(), substitute()
 ----------------------------------------------------------------------------*/

void QFont::removeSubstitution( const char *familyName )
{
    if ( !fontSubst )
	initFontSubst();
    if ( fontSubst )
	fontSubst->remove( familyName );
}


/*----------------------------------------------------------------------------
  Returns a sorted list of substituted family names in \e list.

  \sa insertSubstitution(), removeSubstitution(), substitute()
 ----------------------------------------------------------------------------*/

void QFont::listSubstitutions( QStrList *list )
{
    if ( !fontSubst )
	initFontSubst();
    list->clear();
    list->setAutoDelete( TRUE );
    QFontSubstIt it( *fontSubst );
    const char *n;
    while ( (n=it.currentKey()) ) {
	list->inSort( strdup(n) );
	++it;
    }
}


/*****************************************************************************
  QFont stream functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \relates QFont
  Writes a font to the stream.
 ----------------------------------------------------------------------------*/

QDataStream &operator<<( QDataStream &s, const QFont &f )
{
    UINT8 bits = 0;
    if ( f.d->req.italic )
	bits |= 0x01;
    if ( f.d->req.underline )
	bits |= 0x02;
    if ( f.d->req.strikeOut )
	bits |= 0x04;
    if ( f.d->req.fixedPitch )
	bits |= 0x08;
    if ( f.d->req.hintSetByUser )
	bits |= 0x10;
    if ( f.d->req.rawMode )
	bits |= 0x20;
    return s << f.d->req.family
	     << (INT16)f.d->req.pointSize
	     << (UINT8)f.d->req.styleHint
	     << (UINT8)f.d->req.charSet
	     << (UINT8)f.d->req.weight
	     << bits;
}

/*----------------------------------------------------------------------------
  \relates QFont
  Reads a font from the stream.
 ----------------------------------------------------------------------------*/

QDataStream &operator>>( QDataStream &s, QFont &f )
{
    if ( f.d->deref() )
	delete f.d;
    f.d = new QFontData;
    CHECK_PTR( f.d );

    INT16 pointSize;
    UINT8 styleHint, charSet, weight, bits;
    s >> f.d->req.family;
    s >> pointSize;
    s >> styleHint >> charSet >> weight >> bits;

    f.d->req.pointSize	   = pointSize;
    f.d->req.styleHint	   = styleHint;
    f.d->req.charSet	   = charSet;
    f.d->req.weight	   = weight;
    f.d->req.italic	   = (bits & 0x01) ? TRUE : FALSE;
    f.d->req.underline	   = (bits & 0x02) ? TRUE : FALSE;
    f.d->req.strikeOut	   = (bits & 0x04) ? TRUE : FALSE;
    f.d->req.fixedPitch	   = (bits & 0x08) ? TRUE : FALSE;
    f.d->req.hintSetByUser = (bits & 0x10) ? TRUE : FALSE;
    f.d->req.rawMode	   = (bits & 0x20) ? TRUE : FALSE;
    f.d->req.dirty	   = TRUE;

    return s;
}


/***************************************************************************** 
  QFontMetrics member functions
 *****************************************************************************/

typedef declare(QListM,QFontMetrics) QFontMetricsList;
static QFontMetricsList *fm_list = 0;

static void insertFontMetrics( QFontMetrics *fm )
{
    if ( !fm_list )
	fm_list = new QFontMetricsList;
    fm_list->append( fm );
}

static void removeFontMetrics( QFontMetrics *fm )
{
    ASSERT( fm_list );
    if ( fm_list->findRef( fm ) >= 0 ) {
	fm_list->remove();
	if ( fm_list->count() == 0 ) {
	    delete fm_list;
	    fm_list = 0;
	}
    }
}


/*----------------------------------------------------------------------------
  Resets all pointers to \e pdev in all font metrics objects in the
  applications.
 ----------------------------------------------------------------------------*/

void QFontMetrics::reset( const QPaintDevice *pdev )
{
    if ( fm_list ) {
	QFontMetrics *fm = fm_list->first();
	while ( fm ) {
	    if ( fm->pdev == pdev )		// points to paint device
		fm->pdev = 0;			// paint device dead
	    fm = fm_list->next();
	}
    }
}


/*----------------------------------------------------------------------------
  \class QFontMetrics qfontmet.h
  \brief The QFontMetrics class provides font metrics information about
  the current font for a paint device.

  QFontMetrics functions calculate size of characters and strings for a given
  font.  A font metric object can be obtained for a
  \link QPaintDevice paint device\endlink.

  \sa QPaintDevice::fontMetrics(), QPainter::fontMetrics(), QFont, QFontInfo
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Constructs a font metrics object for the paint device \e paintDev.
 ----------------------------------------------------------------------------*/

QFontMetrics::QFontMetrics( const QPaintDevice *paintDev )
{
    bool got_font = FALSE;
    if ( paintDev ) {
	pdev = (QPaintDevice *)paintDev;
	pdev->devFlags |= PDF_FONTMET;		// tell pdev there's a ref
	if ( pdev->devType() == PDT_WIDGET ) {
	    f = ((QWidget*)pdev)->font();
	    got_font = TRUE;
	}
    }
    else
	pdev = 0;
    if ( !got_font )
	f = QFont::defaultFont();
    insertFontMetrics( this );			// register this object
}

/*----------------------------------------------------------------------------
  Constructs a font metrics object via a painter.
 ----------------------------------------------------------------------------*/

QFontMetrics::QFontMetrics( const QPainter *p )
{
    pdev = p->device();
    pdev->devFlags |= PDF_FONTMET;		// tell pdev there's a ref
    f = p->font();
    insertFontMetrics( this );			// register this object
}

/*----------------------------------------------------------------------------
  Destroys the font metrics object.
 ----------------------------------------------------------------------------*/

QFontMetrics::~QFontMetrics()
{
    removeFontMetrics( this );
}


/*----------------------------------------------------------------------------
  Returns the pixel width of a \e ch.
 ----------------------------------------------------------------------------*/

int QFontMetrics::width( char ch ) const
{
    char tmp[2];
    tmp[1] = '\0';
    tmp[0] = ch;
    return width( tmp, 1 );
}

/*----------------------------------------------------------------------------
  Returns the bounding rectangle of \e ch.

  Note that the bounding rectangle may extend to the left of (0,0) and that
  the text output may cover \e all pixels in the bounding rectangle.
 ----------------------------------------------------------------------------*/

QRect QFontMetrics::boundingRect( char ch ) const
{
    char tmp[2];
    tmp[1] = '\0';
    tmp[0] = ch;
    return boundingRect( tmp, 1 );
}


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

typedef declare(QListM,QFontInfo) QFontInfoList;
static QFontInfoList *fi_list = 0;

static void insertFontInfo( QFontInfo *fi )
{
    if ( !fi_list )
	fi_list = new QFontInfoList;
    fi_list->append( fi );
}

static void removeFontInfo( QFontInfo *fi )
{
    ASSERT( fi_list );
    if ( fi_list->findRef( fi ) >= 0 ) {
	fi_list->remove();
	if ( fi_list->count() == 0 ) {
	    delete fi_list;
	    fi_list = 0;
	}
    }
}


/*----------------------------------------------------------------------------
  Resets all pointers to \e pdev in all font info objects in the
  applications.
 ----------------------------------------------------------------------------*/

void QFontInfo::reset( const QPaintDevice *pdev )
{
    if ( fi_list ) {
	QFontInfo *fi = fi_list->first();
	while ( fi ) {
	    if ( fi->pdev == pdev )		// points to paint device
		fi->pdev = 0;			// paint device dead
	    fi = fi_list->next();
	}
    }
}


/*----------------------------------------------------------------------------
  \class QFontInfo qfontinf.h
  \brief The QFontInfo class provides information about the current
  font for a paint device.

  The QFont class might not always map exactly to the specified font for
  a paint device.

  The QFontInfo class provides information of the actual font that
  matches a QFont specification.

  \sa QPaintDevice::fontInfo(), QPainter::fontInfo(), QFont, QFontMetrics
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Constructs a font info object for the paint device \e paintDev.
 ----------------------------------------------------------------------------*/

QFontInfo::QFontInfo( const QPaintDevice *paintDev )
{
    bool got_font = FALSE;
    if ( paintDev ) {
	pdev = (QPaintDevice *)paintDev;
	pdev->devFlags |= PDF_FONTINF;		// tell pdev there's a ref
	if ( pdev->devType() == PDT_WIDGET ) {
	    f = ((QWidget*)pdev)->font();
	    got_font = TRUE;
	}
    }
    else
	pdev = 0;
    if ( !got_font )
	f = QFont::defaultFont();
    insertFontInfo( this );			// register this object
}

/*----------------------------------------------------------------------------
  Constructs a font info object via a painter.
 ----------------------------------------------------------------------------*/

QFontInfo::QFontInfo( const QPainter *p )
{
    pdev = p->device();
    pdev->devFlags |= PDF_FONTINF;		// tell pdev there's a ref
    f = p->font();
    insertFontInfo( this );			// register this object
}

/*----------------------------------------------------------------------------
  Destroys the font info object.
 ----------------------------------------------------------------------------*/

QFontInfo::~QFontInfo()
{
    removeFontInfo( this );
}


// !!!hanord Trenger vi dette lengre???
#define UPDATE_DATA	     \
    if ( f.d->req.dirty )    \
	f.loadFont();	     \
    if ( f.d->act.dirty )    \
	f.updateFontInfo();

/*----------------------------------------------------------------------------
  Returns the family name of the matched window system font.
  \sa QFont::family()
 ----------------------------------------------------------------------------*/

const char *QFontInfo::family() const
{
    UPDATE_DATA
    return f.d->act.family;
}

/*----------------------------------------------------------------------------
  Returns the point size of the matched window system font.
  \sa QFont::pointSize()
 ----------------------------------------------------------------------------*/

int QFontInfo::pointSize() const
{
    UPDATE_DATA
    return f.d->act.pointSize / 10;
}

/*----------------------------------------------------------------------------
  Returns the italic value of the matched window system font.
  \sa QFont::italic()
 ----------------------------------------------------------------------------*/

bool QFontInfo::italic() const
{
    UPDATE_DATA
    return f.d->act.italic;
}

/*----------------------------------------------------------------------------
  Returns the weight of the matched window system font.
  \sa QFont::weight()
 ----------------------------------------------------------------------------*/

int QFontInfo::weight() const
{
    UPDATE_DATA
    return (int) f.d->act.weight;
}

/*----------------------------------------------------------------------------
  Returns the underline value of the matched window system font.

  <strong>Implementation note:</strong>
  This is always TRUE for X Windows.

  \sa QFont::underline()
 ----------------------------------------------------------------------------*/

bool QFontInfo::underline() const
{
    UPDATE_DATA
    return (int) f.d->act.underline;
}

/*----------------------------------------------------------------------------
  Returns the strike out value of the matched window system font.

  <strong>Implementation note:</strong>
  This is always TRUE for X Windows.

  \sa QFont::strikeOut()
 ----------------------------------------------------------------------------*/

bool QFontInfo::strikeOut() const
{
    UPDATE_DATA
    return (int) f.d->act.strikeOut;
}

/*----------------------------------------------------------------------------
  Returns the fixed pitch value of the matched window system font.
  \sa QFont::fixedPitch()
 ----------------------------------------------------------------------------*/

bool QFontInfo::fixedPitch() const
{
    UPDATE_DATA
    return f.d->act.fixedPitch;
}

/*----------------------------------------------------------------------------
  Returns the style of the matched window system font.

  Currently only returns the hint set in QFont.
  \sa QFont::styleHint()
 ----------------------------------------------------------------------------*/

QFont::StyleHint QFontInfo::styleHint() const
{
    UPDATE_DATA
    return (QFont::StyleHint) f.d->act.styleHint;
}

/*----------------------------------------------------------------------------
  Returns the character set of the matched window system font.
  \sa QFont::charSet()
 ----------------------------------------------------------------------------*/

QFont::CharSet QFontInfo::charSet() const
{
    UPDATE_DATA
    return (QFont::CharSet) f.d->act.charSet;
}

/*----------------------------------------------------------------------------
  Returns TRUE if the font is a raw mode font.

  If it is a raw mode font, all other functions in QFontInfo will return the
  same values set in the QFont, regardless of the font actually used.

  \sa QFont::rawMode()
 ----------------------------------------------------------------------------*/

bool QFontInfo::rawMode() const
{
    UPDATE_DATA
    return f.d->act.rawMode;
}

/*----------------------------------------------------------------------------
  Returns TRUE if the matched window system font is exactly the one specified
  by the font.
  \sa QFont::exactMatch()
 ----------------------------------------------------------------------------*/

bool QFontInfo::exactMatch() const
{
    UPDATE_DATA
    return f.d->exactMatch;
}


/*----------------------------------------------------------------------------
  \page fontmatch.html

  <h1> Qt Font Matching </h1>

  <p>

  The QFont class is an abstract specification of an ideal font. This
  font must be matched to fit an actual window system font when drawing
  is to be done. This is not a trivial operation since there are no
  fonts that we can be sure will be available on all window systems at all
  times, or even on a single window system at all times. Qt queries the
  window system for available fonts and uses a font matching algorithm to
  decide which one of the available fonts closely matches the QFont settings.

  <p>

  In this version Qt has a single built-in font matching mechanism. In
  general it is difficult to make a matching algorithm that fits the
  needs of all types of applications and we will put in the possibility
  for users to supply their own font matching algorithm before the 1.0
  release. We believe that the algorithm selected will fit the needs of
  all but the most highly specialized feinschmecker word processing
  applications.

  <h3>
  The current matching algorithm works as follows:
  </h3>


  <p>

  First an available font family is found. If the requested is not
  available the \link QFont::setStyleHint() style hint \endlink is
  used to select a replacement family. If the style hint has not been
  set, "helvetica" will be used (this is not optimal and a table of
  common font types will be built into Qt before the 1.0 release).

  <p>

  If the replacement family is not found, "helvetica" will be searched for,
  if it is not found Qt will search for a last
  resort font, i.e. a specific font to match to, ignoring the
  attribute settings. Qt searches through a built-in list of very
  common fonts. If one of these is not found, Qt chickens out,
  gives you an error message and aborts (of course this only happens
  if you are using fonts and Qt \e has to load a font). We have not been
  able to find a case where this happens. Please
  <a href=mailto:qt-bugs@troll.no>mail us</a>
  if you do, preferrably with a list of the fonts you have installed
  (under X windows, type "xlsfonts").

  <p>

  The following attributes are then matched, in order of priority:

  <p>

  <dl>
  <dt> \link QFont::setCharSet() character set \endlink <dd>
  <dt> \link QFont::setFixedPitch() pitch \endlink <dd>
  <dt> \link QFont::setPointSize() point size \endlink <dd>
  <dt> \link QFont::setWeight() weight \endlink <dd>
  <dt> \link QFont::setItalic() italic \endlink <dd>
  </dl>

  <p>

  If, for example, a font with the correct character set is found, but
  with all other attributes in the list unmatched, it will be chosen before
  a font with the wrong character set but with all other attributes correct.

  <p>

  The point size is defined to match if it is within 20% of the requested
  point size. Of course, when several fonts match and only point size
  differs the closest point size to the one requested will be chosen.

  <p>

  Currently using the correct font family has higher priority than
  using the correct character set. We are not certain if this should
  be reversed and might do so in the 1.0 release. If you have opinions
  about this please <a href=mailto:qt-bugs@troll.no>mail us</a>!
 ----------------------------------------------------------------------------*/
