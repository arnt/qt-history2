/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.cpp#8 $
**
** Implementation of QFont and QFontInfo classes
**
** Author  : Eirik Eng
** Created : 941207
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qfont.h"
#include "qfontdta.h"
#include "qfontmet.h"
#include "qfontinf.h"
#include "qpainter.h"
#include "qwidcoll.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfont.cpp#8 $";
#endif


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

void QFont::setFamily( const char *family )
{
    if ( d->req.family != family ) {
	d->req.family = family;
	d->req.dirty  = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

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
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

void QFont::setItalic( bool i )
{
    if ( d->req.italic != i ) {
	d->req.italic = i;
	d->req.dirty  = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

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
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

void QFont::setUnderline( bool b )
{
    if ( d->req.underline != b ) {
	d->req.underline  = b;
	d->act.underline  = b;                  // underline always possible
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

void QFont::setStrikeOut( bool b )
{
    if ( d->req.strikeOut != b ) {
	d->req.strikeOut  = b;
	d->act.strikeOut  = b;                  // strikeOut always posible
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

void QFont::setFixedPitch( bool b )
{
    if ( d->req.fixedPitch != b ) {
	d->req.fixedPitch = b;
	d->req.dirty      = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

void QFont::setStyleHint( StyleHint h )
{
    if ( d->req.styleHint != h ) {
	d->req.styleHint     = h;
	d->req.hintSetByUser = TRUE;
	d->req.dirty         = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

void QFont::setCharSet( CharSet c )
{
    if ( d->req.charSet != c ) {
	d->req.charSet = c;
	d->req.dirty   = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

const char *QFont::family() const
{
    return d->req.family;
}

int QFont::pointSize() const
{
    return d->req.pointSize / 10;
}

bool QFont::italic() const
{
    return d->req.italic;
}

int QFont::weight() const
{
    return (int) d->req.weight;
}

int QFont::underline() const
{
    return (int) d->req.underline;
}

int QFont::strikeOut() const
{
    return (int) d->req.strikeOut;
}

bool QFont::fixedPitch() const
{
    return d->req.fixedPitch;
}

QFont::StyleHint QFont::styleHint() const
{
    return (StyleHint) d->req.styleHint;
}

QFont::CharSet QFont::charSet() const
{
    return (CharSet) d->req.charSet;
}

bool QFont::rawMode() const
{
    return d->req.rawMode;
}

bool QFont::exactMatch() const
{
    if ( d->req.dirty )
	loadFont();
    return d->exactMatch;
}

void QFont::setRawMode( bool b )
{
    if ( d->req.rawMode != b ) {
	d->req.rawMode = b;
	d->req.dirty   = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

bool QFont::operator==( const QFont &f ) const
{
    return f.d == d;
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

int QFont::deciPointSize() const
{
    return d->req.pointSize;
}

bool QFont::isDefaultFont()
{
    return d->isDefaultFont;
}

void QFont::updateSubscribers()
{
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::wmapper()) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// for all widgets that use
	if ( w->fontRef().d == d )		//   this font
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
    QPainter::changedFont( &f, TRUE );	// tell painter about new font

    return s;
}

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

int QFontMetrics::width( char ch ) const
{
    char tmp[2];
    tmp[1] = '\0';
    tmp[0] = ch;
    return width( tmp, 1 );
}

const QFont &QFontMetrics::font() const
{
    return f;
}

void QFontMetrics::setFont( const QFont &font )
{
    f = font;
}
