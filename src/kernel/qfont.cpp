/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.cpp#13 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfont.cpp#13 $";
#endif


  /*!
  \class QFont qfont.h

      Yo! Yo! Set yourself free!

  */

QFont::QFont( const QFont &font )
{
    d = font.d;
    d->ref();
}

QFont::~QFont()
{
    if ( d->deref() )
	delete d;
}

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

  See also: family(), setStyleHint(), QFontInfo and 
            <a href=fontmatch.html>font matching</a>.

  \todo Use a table of MANY different family names to find a good default
  family.
  */

void QFont::setFamily( const char *family )
{
    if ( d->req.family != family ) {
	d->req.family = family;
	d->req.dirty  = TRUE;
    }
}

  /*!
  Sets the point size (e.g. 12 or 18). If the point size is not available
  the closest available will be used 

  Setting of point sizes less than or equal to 0 will be ignored.

  See also: pointSize(), QFontInfo and 
            <a href=fontmatch.html>font matching</a>.

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
	d->req.pointSize = pointSize * 10;
	d->req.dirty     = TRUE;
    }
}

  /*!
  \page fontmatch.html
 
  The <a href=fontmatch.html>font matching mechanism</a>

  \todo Put in a hook that enables the user to write his/her own
  fontmatching mechanism.
  */


  /*! -------------------------------------------------------------------------
  Sets italic on or off. If the mode selected is not available
  the other will be used.

  See also: italic(), QFontInfo and <a href=fontmatch.html>font matching</a>.

  */

void QFont::setItalic( bool i )
{
    if ( d->req.italic != i ) {
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

  See also weight(), QFontInfo and <a href=fontmatch.html>font matching</a>.

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
	d->req.weight = w;
	d->req.dirty  = TRUE;
    }
}

  /*!
  Sets underline on or off. If the mode selected is not available
  the other will be used. (Both are always available under X windows.).

  See also: underline(), QFontInfo and 
            <a href=fontmatch.html>font matching</a>.

  */

void QFont::setUnderline( bool b )
{
    if ( d->req.underline != b ) {
	d->req.underline  = b;
	d->act.underline  = b;                  // underline always possible
    }
}

  /*!
  Sets strike out on or off. If the mode selected is not available
  the other will be used (Both are always available under X windows.).

  See also: strikeOut(), QFontInfo and 
            <a href=fontmatch.html>font matching</a>.

  */

void QFont::setStrikeOut( bool b )
{
    if ( d->req.strikeOut != b ) {
	d->req.strikeOut  = b;
	d->act.strikeOut  = b;                  // strikeOut always posible
    }
}

  //  --------------------------------------------------------------------
  /*!
  Sets fixed pitch on or off. If the mode selected is not available
  the other will be used. A fixed pitch font is a font that has constant
  character pixel width.
  (see QFontInfo and <a href=fontmatch.html>font matching</a>).

  See also: fixedPitch(), QFontInfo and 
            <a href=fontmatch.html>font matching</a>.

  */

void QFont::setFixedPitch( bool b )
{
    if ( d->req.fixedPitch != b ) {
	d->req.fixedPitch = b;
	d->req.dirty      = TRUE;
    }
}

  // --------------------------------------------------------------------
  /*!
  Sets the style hint. The style hint is used by the 
  <a href=fontmatch.html>font matching</a>
  algorithm when a selected font family cannot be found and is used to 
  find an appropriate default family.

  The style hint has a default value of AnyStyle which leaves the task of
  finding a good default family to the
  <a href=fontmatch.html>font matching</a>
  algorithm.

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

  See also: styleHint(), QFontInfo and 
            <a href=fontmatch.html>font matching</a>.
  
  */

void QFont::setStyleHint( StyleHint h )
{
    if ( d->req.styleHint != h ) {
	d->req.styleHint     = h;
	d->req.hintSetByUser = TRUE;
	d->req.dirty         = TRUE;
    }
}

  /*!
  Sets the character set (e.g. Latin1). If the character set is not available
  another will be used, for most non-trivial applications you will probably
  not want this to happen since it can totally obscure the text shown to the 
  user when the font is used. This is why the 
  <a href=fontmatch.html>font matching</a>
  algorithm gives high priority to finding the correct character set.

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

  See also: charSet(), QFontInfo and 
            <a href=fontmatch.html>font matching</a>.
  */

void QFont::setCharSet( CharSet c )
{
    if ( d->req.charSet != c ) {
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

  See also: setFamily() and QFontInfo.
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

  See also: setPointSize() and QFontInfo.
  */

int QFont::pointSize() const
{
    return d->req.pointSize / 10;
}

  /*!
  Returns the value set by setItalic(), use QFontInfo 
  to find the italic value of the window system font actually used.

  See also: setItalic() and QFontInfo.
  */

bool QFont::italic() const
{
    return d->req.italic;
}
  /*!
  Returns the weight set by setWeight(), use QFontInfo 
  to find the weight of the window system font actually used.

  See also: setWeight() and QFontInfo.
  */

int QFont::weight() const
{
    return (int) d->req.weight;
}
  /*!
  Returns the value set by setUnderline(), use QFontInfo 
  to find the underline value of the window system font actually used.

  See also: setUnderline() and QFontInfo.
  */

bool QFont::underline() const
{
    return (int) d->req.underline;
}
  /*!
  Returns the value set by setStrikeOut(), use QFontInfo 
  to find the strike out value of the window system font actually used.

  See also: setStrikeOut() and QFontInfo.
  */

bool QFont::strikeOut() const
{
    return (int) d->req.strikeOut;
}
  /*!
  Returns the value set by setFixedPitch(), use QFontInfo 
  to find the fixed pitch value of the window system font actually used.

  See also: setFixedPitch() and QFontInfo.
  */

bool QFont::fixedPitch() const
{
    return d->req.fixedPitch;
}
  /*!
  Returns the StyleHint set by setStyleHint().

  See also: setStyleHint().
  */

QFont::StyleHint QFont::styleHint() const
{
    return (StyleHint) d->req.styleHint;
}
  /*!
  Returns the CharSet set by setCharSet(), use QFontInfo 
  to find the CharSet of the window system font actually used.

  See also: setCharSet() and QFontInfo.
  */

QFont::CharSet QFont::charSet() const
{
    return (CharSet) d->req.charSet;
}

  /*!
  Returns the value set by setRawMode.

  See also: setRawMode()
  */

bool QFont::rawMode() const
{
    return d->req.rawMode;
}

  /*!
  Returns TRUE if a window system font exactly matching the settings
  of this font is available.

  See also: QFontInfo and <a href=fontmatch.html>font matching</a>.
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

  See also: setRawMode()
  */

void QFont::setRawMode( bool b )
{
    if ( d->req.rawMode != b ) {
	d->req.rawMode = b;
	d->req.dirty   = TRUE;
    }
}

  /*!
  Returns TRUE if the two QFonts have the same values for all fields,
  i.e rawMode, pointSize, styleHint, charSet, weight, italic, underline,
  strikeOut, fixedPitch and family. If the QFonts both are in rawMode()
  only the family fields are compared.

  See also: operator!=().
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

  See also: operator==().
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

  /*!
  Returns TRUE if the font is the global default font.
  */
bool QFont::isDefaultFont()
{
    return d->isDefaultFont;
}

  /*!
  \internal
  Update all widgets that are using ths font.

  \todo Not currently in use, do we need this anymore?
  */
void QFont::updateSubscribers()
{
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::wmapper()) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// for all widgets that use
	if ( w->fontRef().d == d )		// this font
	    w->setFont( *this );		// update the font
	++it;
    }    
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

int QFontInfo::pointSize() const
{
    UPDATE_DATA
    return f.d->act.pointSize / 10;
}

bool QFontInfo::italic() const
{
    UPDATE_DATA
    return f.d->act.italic;
}

int QFontInfo::weight() const
{
    UPDATE_DATA
    return (int) f.d->act.weight;
}

int QFontInfo::underline() const
{
    UPDATE_DATA
    return (int) f.d->act.underline;
}

int QFontInfo::strikeOut() const
{
    UPDATE_DATA
    return (int) f.d->act.strikeOut;
}

bool QFontInfo::fixedPitch() const
{
    UPDATE_DATA
    return f.d->act.fixedPitch;
}

QFont::StyleHint QFontInfo::styleHint() const
{
    UPDATE_DATA
    return (QFont::StyleHint) f.d->act.styleHint;
}

QFont::CharSet QFontInfo::charSet() const
{
    UPDATE_DATA
    return (QFont::CharSet) f.d->act.charSet;
}

bool QFontInfo::rawMode() const
{
    UPDATE_DATA
    return f.d->act.rawMode;
}

bool QFontInfo::exactMatch() const
{
    UPDATE_DATA
    return f.d->exactMatch;
}

// ----------------------------------------------------------------------------
// QFontMetrics member functions
//

/*!
\class QFontMetrics qfontmet.h

    Yo! Yo! Set yourself free!

*/

int QFontMetrics::width( char ch ) const
{
    char tmp[2];
    tmp[1] = '\0';
    tmp[0] = ch;
    return width( tmp, 1 );
}

QRect QFontMetrics::boundingRect( char ch ) const
{
    char tmp[2];
    tmp[1] = '\0';
    tmp[0] = ch;
    return boundingRect( tmp, 1 );
}

const QFont &QFontMetrics::font() const
{
    return f;
}

void QFontMetrics::setFont( const QFont &font )
{
    f = font;
}
