/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.cpp#18 $
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
#include "qpainter.h"
#include "qwidcoll.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfont.cpp#18 $";
#endif


  /*!
  \class QFont qfont.h

  \brief The QFont class specifies a font used for drawing text in QPainter.

  A QFont has a series of attributes that can be set to specify an abstract
  font. When actual drawing of text is done Qt will select a font in the
  underlying window system that
            \link fontmatch.html matches \endlink.
  the abstract font as close as possible. The most important attributes
  are 
  \link setFamily() family \endlink , 
  \link setPointSize() point size \endlink , 
  \link setWeight() weight \endlink
  and 
  \link setItalic() italic \endlink.

  One of the QFont constructors take exactly these attributes as
  arguments:

  \code
  void YourWidget::paintEvent( QPaintEvent * ) {
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
  X. The QFont class has an internal sharing and reference count
  mechanism, it has a lazy loading mechanism and does not match and
  load a font until it \e really has to, it also caches previously loaded
  fonts and under X it caches previously matched font attribute
  combinations.

  Note that the functions returning attribute values in QFont return
  the values previously set \e not the attributes of the actual
  window system font used for drawing. To get information about the
  actual font use QFontInfo.

  To get font size information use the class QFontMetrics.

  */


  /*!
  Copy constructor.
  */
QFont::QFont( const QFont &font )
{
    d = font.d;
    d->ref();
}

  /*!
  Cleans up when a QFont dies.
  */
QFont::~QFont()
{
    if ( d->deref() )
	delete d;
}

  /*
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
  Sets the family name of the font (e.g. "Helvetica" or "times"). Case does not
  matter.
  If the family is not available a default family will be used instead 

  \sa family(), setStyleHint(), QFontInfo and 
            \link fontmatch.html font matching \endlink.

  \todo Use a table of MANY different family names to find a good default
  family.
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
  Sets the point size (e.g. 12 or 18). If the point size is not available
  the closest available will be used 

  Setting of point sizes less than or equal to 0 will be ignored.

  \sa pointSize(), QFontInfo and 
            \link fontmatch.html font matching \endlink.

  */

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
	d->req.pointSize = pointSize * 10;
	d->req.dirty     = TRUE;
    }
}

  /*! \page fontmatch.html

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
  available the \link setStyleHint() style hint \endlink is used to
  select a replacement family. If the style hint has not been set, 
  "helvetica" will be used (this is not optimal and a table of
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
  <a href=mailto:qt@troll.no>mail us</a>
  if you do, preferrably with a list of the fonts you have installed 
  (under X windows, type "xlsfonts").

  <p>

  The following attributes are then matched, in order of priority:

  <p>

  <dl>
  <dt> \link setCharSet() character set \endlink <dd>
  <dt> \link setFixedPitch() pitch \endlink <dd>
  <dt> \link setPointSize() point size \endlink <dd>
  <dt> \link setWeight() weight \endlink <dd>
  <dt> \link setItalic() italic \endlink <dd>
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
  about this please <a href=mailto:qt@troll.no>mail us</a>!

  */


  /*! -------------------------------------------------------------------------
  Sets italic on or off. If the mode selected is not available
  the other will be used.

  \sa italic(), QFontInfo and \link fontmatch.html font matching \endlink.

  */

void QFont::setItalic( bool i )
{
    if ( d->req.italic != i ) {
        detach();
	d->req.italic = i;
	d->req.dirty  = TRUE;
    }
}

  /*!
  Sets the weight (or boldness). The weight must be in the range [0,99] (where
  0 is ultralight and 99 is extremely black), the values of the enum type
  Weight can also be used.

  \e example: \code
  QFont f( "courier" );
  f.setWeight( QFont::Bold );
  \endcode

  If the weight is not available the closest available will be used.
  (Use QFontInfo to check.)
  Setting of weights outside the legal range will be ignored.

  \e Note: The window systems supported by Qt will in practice only support
  QFont::Normal and QFont::Bold, and, if you are lucky, the other values 
  in Weight.

  See also weight(), QFontInfo and \link fontmatch.html font matching \endlink.

  */

void QFont::setWeight( int w )
{
#if defined(CHECK_RANGE)
    if ( w < 0 || w > 99 ) {
	warning( "QFont::setWeight: Value out of range (%i)", w );
	return;
    }
#endif
    if ( d->req.weight != w ) {
        detach();
	d->req.weight = w;
	d->req.dirty  = TRUE;
    }
}

  /*!
  Sets underline on or off. If the mode selected is not available
  the other will be used. (Both are always available under X windows.).

  \sa underline(), QFontInfo and 
            \link fontmatch.html font matching \endlink.

  */

void QFont::setUnderline( bool b )
{
    if ( d->req.underline != b ) {
        detach();
	d->req.underline  = b;
	d->act.underline  = b;                  // underline always possible
    }
}

  /*!
  Sets strike out on or off. If the mode selected is not available
  the other will be used (Both are always available under X windows.).

  \sa strikeOut(), QFontInfo and \link fontmatch.html font
  matching \endlink.
*/

void QFont::setStrikeOut( bool b )
{
    if ( d->req.strikeOut != b ) {
        detach();
	d->req.strikeOut  = b;
	d->act.strikeOut  = b;                  // strikeOut always posible
    }
}

  //  --------------------------------------------------------------------
  /*!
  Sets fixed pitch on or off. If the mode selected is not available
  the other will be used. A fixed pitch font is a font that has constant
  character pixel width.
  (see QFontInfo and \link fontmatch.html font matching \endlink).

  \sa fixedPitch(), QFontInfo and 
            \link fontmatch.html font matching \endlink.

  */

void QFont::setFixedPitch( bool b )
{
    if ( d->req.fixedPitch != b ) {
        detach();
	d->req.fixedPitch = b;
	d->req.dirty      = TRUE;
    }
}

  // --------------------------------------------------------------------
  /*!
  Sets the style hint. The style hint is used by the \link
  fontmatch.html font matching \endlink
  algorithm when a selected font family cannot be found and is used to 
  find an appropriate default family.

  The style hint has a default value of AnyStyle which leaves the task of
  finding a good default family to the font matching algorithm.

  In this example (which is a complete program) the pushbutton 
  will display its text label with the
  Bavaria font family if this family is available, if not it will 
  display its text label with the Times font family:
  \code 
  #include <qapp.h>
  #include <qpushbt.h>
  #include <qfont.h>

  int main( int argc, char **argv )
  {
      QApplication app( argc, argv );
      QPushButton push("Push me");

      QFont fnt( "Bavaria", 18 );       \/  Preferrred family is Bavaria
      fnt.setStyleHint( QFont::Times ); \/  Use Times if Bavaria is not available

      push.setFont( fnt );
      push.show();
      return app.exec( &push );
  } 
  \endcode

  \sa styleHint(), QFontInfo and \link fontmatch.html font
  matching \endlink.
  
  */

void QFont::setStyleHint( StyleHint h )
{
    if ( d->req.styleHint != h ) {
        detach();
	d->req.styleHint     = h;
	d->req.hintSetByUser = TRUE;
	d->req.dirty         = TRUE;
    }
}

  /*!
  Sets the character set (e.g. Latin1). If the character set is not available
  another will be used, for most non-trivial applications you will probably
  not want this to happen since it can totally obscure the text shown to the 
  user when the font is used. This is why the \link fontmatch.html font
  matching \endlink algorithm gives high priority to finding the correct
  character set.

  (Currently using the correct font family has higher priority than
  using the correct character set. We are not certain if this should
  be reversed and might do so in the 1.0 release. If you have opinions 
  about this please mail us!)

  To ensure that the character set is correct you can use the QFontInfo
  class, \e example: 
  \code
  QFont     fnt( "times", 14 );              \/ default character set is Latin1
  QFontInfo info( fnt );
  if ( info.charSet() != Latin1 )            \/ Check info \e NOT fnt
      fatal( "Cannot find a Latin 1 Times font" ); 
  \endcode

  \sa charSet(), QFontInfo and 
            \link fontmatch.html font matching \endlink.
  */

void QFont::setCharSet( CharSet c )
{
    if ( d->req.charSet != c ) {
        detach();
	d->req.charSet = c;
	d->req.dirty   = TRUE;
    }
}

  /*!
  Returns the family name set by setFamily(), use QFontInfo 
  to find the family name of the window system font actually used.

  \e Example: \code
  QFont     fnt( "Nairobi" );
  QFontInfo info( fnt );
  debug( "Font family requested is    : \"%s\"", fnt.family() );
  debug( "Font family actually used is: \"%s\"", info.family() );
  \endcode

  \sa setFamily() and QFontInfo.
  */

const char *QFont::family() const
{
    return d->req.family;
}

  /*!
  Returns the point size set by setPointSize(), use QFontInfo 
  to find the point size of the window system font actually used.

  \e Example: \code
  QFont     fnt( "helvetica" );
  QFontInfo info( fnt );
  fnt.setPointSize( 53 );
  debug( "Font size requested is    : \"%i\"", fnt.pointSize() );
  debug( "Font size actually used is: \"%i\"", info.pointSize() );
  \endcode

  \sa setPointSize() and QFontInfo.
  */

int QFont::pointSize() const
{
    return d->req.pointSize / 10;
}

  /*!
  Returns the value set by setItalic(), use QFontInfo 
  to find the italic value of the window system font actually used.

  \sa setItalic() and QFontInfo.
  */

bool QFont::italic() const
{
    return d->req.italic;
}
  /*!
  Returns the weight set by setWeight(), use QFontInfo 
  to find the weight of the window system font actually used.

  \sa setWeight() and QFontInfo.
  */

int QFont::weight() const
{
    return (int) d->req.weight;
}
  /*!
  Returns the value set by setUnderline(), use QFontInfo 
  to find the underline value of the window system font actually used.

  \sa setUnderline() and QFontInfo.
  */

bool QFont::underline() const
{
    return (int) d->req.underline;
}
  /*!
  Returns the value set by setStrikeOut(), use QFontInfo 
  to find the strike out value of the window system font actually used.

  \sa setStrikeOut() and QFontInfo.
  */

bool QFont::strikeOut() const
{
    return (int) d->req.strikeOut;
}
  /*!
  Returns the value set by setFixedPitch(), use QFontInfo 
  to find the fixed pitch value of the window system font actually used.

  \sa setFixedPitch() and QFontInfo.
  */

bool QFont::fixedPitch() const
{
    return d->req.fixedPitch;
}
  /*!
  Returns the StyleHint set by setStyleHint().

  \sa setStyleHint().
  */

QFont::StyleHint QFont::styleHint() const
{
    return (StyleHint) d->req.styleHint;
}
  /*!
  Returns the CharSet set by setCharSet(), use QFontInfo 
  to find the CharSet of the window system font actually used.

  \sa setCharSet() and QFontInfo.
  */

QFont::CharSet QFont::charSet() const
{
    return (CharSet) d->req.charSet;
}

  /*!
  Returns the value set by setRawMode.

  \sa setRawMode()
  */

bool QFont::rawMode() const
{
    return d->req.rawMode;
}

  /*!
  Returns TRUE if a window system font exactly matching the settings
  of this font is available.

  \sa QFontInfo and \link fontmatch.html font matching \endlink.
  */

bool QFont::exactMatch() const
{
    if ( d->req.dirty )
	loadFont();
    return d->exactMatch;
}

  /*!
  Sets raw mode on or off. This function only has effect under X windows. If
  raw mode is on Qt will search for an X font with a complete font name 
  matching the family name, all other values set for the QFont will be ignored.
  If the font name matches several fonts, Qt will use the first font returned
  by X. QFontInfo \e cannot be used to fetch information about a QFont using
  raw mode (it will return the values set in the QFont for all parameters,
  including the family name).

  \e Example: \code
  #if defined(_WS_X11_)
      QFont fnt( "-*-fixed-*-*-*-*-*-140-75-75-c-*-iso8859-1" );
      fnt.setRawMode( TRUE );
      if ( !fnt.exactMatch() )
          debug( "Sorry, could not find the X specific font" );
  #endif
  \endcode

  \warning Don't use it if you don't need it!

  \sa rawMode()
  */

void QFont::setRawMode( bool b )
{
    if ( d->req.rawMode != b ) {
        detach();
	d->req.rawMode = b;
	d->req.dirty   = TRUE;
    }
}

  /*!
  Returns TRUE if the two QFonts have the same values for all fields,
  i.e rawMode, pointSize, styleHint, charSet, weight, italic, underline,
  strikeOut, fixedPitch and family. If the QFonts both are in rawMode()
  only the family fields are compared.

  \sa operator!=().
  */

bool QFont::operator==( const QFont &f ) const
{
    return f.d == d ||
           f.d->req.rawMode == d->req.rawMode &&
           ( f.d->req.rawMode && f.d->req.family == d->req.family ||
             f.d->req.pointSize     == d->req.pointSize     &&
             f.d->req.styleHint     == d->req.styleHint     &&
             f.d->req.charSet       == d->req.charSet       &&
             f.d->req.weight        == d->req.weight        &&
             f.d->req.italic        == d->req.italic        &&
             f.d->req.underline     == d->req.underline     &&
             f.d->req.strikeOut     == d->req.strikeOut     &&
             f.d->req.fixedPitch    == d->req.fixedPitch    &&
             f.d->req.hintSetByUser == d->req.hintSetByUser &&
             f.d->req.family        == d->req.family         );
}

  /*!
  Returns FALSE if the two QFonts have the same values for all fields,
  i.e rawMode, pointSize, styleHint, charSet, weight, italic, underline,
  strikeOut, fixedPitch and family. If the QFonts both are in rawMode()
  only the family fields are compared.

  \sa operator==().
  */

bool QFont::operator!=( const QFont &f ) const
{
    return !(operator==( f ));
}

void QFont::init()
{
    d = new QFontData;
    CHECK_PTR( d );
    d->req.styleHint	 = AnyStyle;
    d->req.charSet	 = Latin1;
    d->req.underline     = FALSE;
    d->req.strikeOut     = FALSE;
    d->req.fixedPitch	 = FALSE;
    d->req.hintSetByUser = FALSE;
    d->req.rawMode	 = FALSE;
    d->req.dirty	 = TRUE;
    d->act.dirty	 = TRUE;
    d->exactMatch	 = FALSE;
}

  /*!
  Returns the point size in 1/10ths of a point.
  */
int QFont::deciPointSize() const
{
    return d->req.pointSize;
}


void QFont::detach()
{
    if ( d->count != 1 )
        *this = QFont( d );
}

// --------------------------------------------------------------------------
// QFont stream functions
//

#include "qdstream.h"

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
	bits |= 0x0f;
    if ( f.d->req.rawMode )
	bits |= 0x10;

    return s << f.d->req.family
	     << f.d->req.pointSize
	     << (UINT8) f.d->req.styleHint
	     << (UINT8) f.d->req.charSet
	     << (UINT8) f.d->req.weight
	     << bits;
}

QDataStream &operator>>( QDataStream &s, QFont &f )
{
    UINT8 bits, styleH, charS, w;

    if ( f.d->deref() )
        delete f.d;
    f.d = new QFontData;
    CHECK_PTR( f.d );    

    s >> f.d->req.family;
    s >> f.d->req.pointSize;
    s >> styleH >> charS >> w >> bits;

    f.d->req.styleHint	   = styleH;
    f.d->req.charSet	   = charS;
    f.d->req.weight	   = w;
    f.d->req.italic	   = ( bits && 0x01 ) ? TRUE : FALSE;
    f.d->req.underline	   = ( bits && 0x02 ) ? TRUE : FALSE;
    f.d->req.strikeOut	   = ( bits && 0x04 ) ? TRUE : FALSE;
    f.d->req.fixedPitch	   = ( bits && 0x08 ) ? TRUE : FALSE;
    f.d->req.hintSetByUser = ( bits && 0x0f ) ? TRUE : FALSE;
    f.d->req.rawMode	   = ( bits && 0x10 ) ? TRUE : FALSE;
    f.d->req.dirty	   = TRUE;

    return s;
}

// ----------------------------------------------------------------------------
// QFontInfo member functions
//

  /*!
  \class QFontInfo qfontinf.h

    \brief The QFontInfo class is used to fetch information about the
    window system font actually used to match a QFont

  */

const QFont &QFontInfo::font() const
{
    return f;
}

void QFontInfo::setFont( const QFont &font )
{
    f = font;
}

#define UPDATE_DATA          \
    if ( f.d->req.dirty )    \
        f.loadFont();        \
    if ( f.d->act.dirty )    \
        f.updateFontInfo();

  /*!
  Returns the family name of the matched window system font.
  */

const char *QFontInfo::family() const
{
    UPDATE_DATA
    return f.d->act.family;
}

  /*!
  Returns the point size of the matched window system font.
  */

int QFontInfo::pointSize() const
{
    UPDATE_DATA
    return f.d->act.pointSize / 10;
}

  /*!
  Returns the italic value of the matched window system font.
  */


bool QFontInfo::italic() const
{
    UPDATE_DATA
    return f.d->act.italic;
}

  /*!
  Returns the weight of the matched window system font.
  */


int QFontInfo::weight() const
{
    UPDATE_DATA
    return (int) f.d->act.weight;
}

  /*!
  Returns the underline value of the matched window system font.
  (this is always equal to the one requested under X windows)
  */


int QFontInfo::underline() const
{
    UPDATE_DATA
    return (int) f.d->act.underline;
}


  /*!
  Returns the strike out value of the matched window system font.
  (this is always equal to the one requested under X windows)
  */


int QFontInfo::strikeOut() const
{
    UPDATE_DATA
    return (int) f.d->act.strikeOut;
}

  /*!
  Returns the fixed pitch value of the matched window system font.
  */


bool QFontInfo::fixedPitch() const
{
    UPDATE_DATA
    return f.d->act.fixedPitch;
}

  /*!
  Returns the style of the matched window system font.
  Currently only returns the hint set in QFont.
  */

QFont::StyleHint QFontInfo::styleHint() const
{
    UPDATE_DATA
    return (QFont::StyleHint) f.d->act.styleHint;
}

  /*!
  Returns the character set of the matched window system font.
  */

QFont::CharSet QFontInfo::charSet() const
{
    UPDATE_DATA
    return (QFont::CharSet) f.d->act.charSet;
}


  /*!
  Returns TRUE if the QFont connected to the QFontInfo is a raw mode
  font. If it is all other functions in QFontInfo will return the
  same values set in the QFont, regardless of the font actually used.
  */

bool QFontInfo::rawMode() const
{
    UPDATE_DATA
    return f.d->act.rawMode;
}


  /*!
  Returns TRUE if the matched window system font exactly matches all
  attributes set in the QFont connected to the QFontInfo.
  */

bool QFontInfo::exactMatch() const
{
    UPDATE_DATA
    return f.d->exactMatch;
}

// ----------------------------------------------------------------------------
// QFontMetrics member functions
//

/*! \class QFontMetrics qfontmet.h

  \brief QFontMetrics gives size information for a font.

*/

  /*! Returns the pixel width of a \e ch.  */

int QFontMetrics::width( char ch ) const
{
    char tmp[2];
    tmp[1] = '\0';
    tmp[0] = ch;
    return width( tmp, 1 );
}

/*! Returns the bounding rectangle of \e ch.  Note that the bounding
  rectangle may extend to the left of (0,0) and that the text output
  may cover \e all pixels in the bounding rectangle. */

QRect QFontMetrics::boundingRect( char ch ) const
{
    char tmp[2];
    tmp[1] = '\0';
    tmp[0] = ch;
    return boundingRect( tmp, 1 );
}

/*! Returns the font this object operates on. */

const QFont &QFontMetrics::font() const
{
    return f;
}

/*! Sets the font this object operates on. */

void QFontMetrics::setFont( const QFont &font )
{
    f = font;
}
