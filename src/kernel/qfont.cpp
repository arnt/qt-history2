/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.cpp#5 $
**
** Implementation of QFont and QFontMetrics classes
**
** Author  : Eirik Eng
** Created : 941207
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qfont.h"
#include "qfontdta.h"
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfont.cpp#5 $";
#endif


QFont::QFont( const QFont &font )
{
    data = font.data;
    data->ref();
}

QFont::~QFont()
{
    if ( data->deref() )
	delete data;
}

QFont &QFont::operator=( const QFont &font )
{
    font.data->ref();
    if ( data->deref() )
	delete data;
    data = font.data;
    return *this;
}


QFont QFont::copy() const
{
    QFont f( data->family );
    return f;
}


void QFont::setFamily( const char *family )
{
    if ( data->family != family ) {
	data->family = family;
	data->dirty  = TRUE;
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
    if ( data->pointSize != pointSize ) {
	data->pointSize = pointSize * 10;
	data->dirty     = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

void QFont::setItalic( bool i )
{
    if ( data->italic != i ) {
	data->italic = i;
	data->dirty  = TRUE;
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
    if ( data->weight != w ) {
	data->weight = w;
	data->dirty  = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

void QFont::setFixedPitch( bool b )
{
    if ( data->fixedPitch != b ) {
	data->fixedPitch = b;
	data->dirty      = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

void QFont::setStyleHint( StyleHint h )
{
    if ( data->styleHint != h ) {
	data->styleHint	    = h;
	data->hintSetByUser = TRUE;
	data->dirty         = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

void QFont::setCharSet( CharSet c )
{
    if ( data->charSet != c ) {
	data->charSet = c;
	data->dirty   = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

const char *QFont::family() const
{
    return data->family;
}

int QFont::pointSize() const
{
    return data->pointSize / 10;
}

bool QFont::italic() const
{
    return data->italic;
}

int QFont::weight() const
{
    return (int) data->weight;
}

bool QFont::fixedPitch() const
{
    return data->fixedPitch;
}

QFont::StyleHint QFont::styleHint() const
{
    return (StyleHint) data->styleHint;
}

QFont::CharSet QFont::charSet() const
{
    return (CharSet) data->charSet;
}

bool QFont::rawMode() const
{
    return data->rawMode;
}

bool QFont::exactMatch() const
{
    if ( data->dirty )
	loadFont();
    return data->exactMatch;
}

void QFont::setRawMode( bool b )
{
    if ( data->rawMode != b ) {
	data->rawMode = b;
	data->dirty = TRUE;
	QPainter::changedFont( this, TRUE );	// tell painter about new font
    }
}

bool QFont::operator==( const QFont &f ) const
{
    return (f.data == data) || (f.data->family == data->family);
}

void QFont::init()
{
    data = new QFontData;
    CHECK_PTR( data );
    data->styleHint	= AnyStyle;
    data->charSet	= Latin1;
    data->fixedPitch	= FALSE;
    data->dirty		= TRUE;
    data->exactMatch	= FALSE;
    data->hintSetByUser = FALSE;
    data->rawMode	= FALSE;
}

int QFont::deciPointSize() const
{
    return data->pointSize;
}

// --------------------------------------------------------------------------
// QFont stream functions
//

#include "qdstream.h"

QDataStream &operator<<( QDataStream &s, const QFont &f )
{
    UINT8 bits = 0;

    if ( f.data->italic )
	bits |= 0x1;
    if ( f.data->fixedPitch )
	bits |= 0x2;
    if ( f.data->hintSetByUser )
	bits |= 0x4;
    if ( f.data->rawMode )
	bits |= 0x8;

    return s << f.data->family
	     << f.data->pointSize
	     << (UINT8) f.data->styleHint
	     << (UINT8) f.data->charSet
	     << (UINT8) f.data->weight
	     << bits;
}

QDataStream &operator>>( QDataStream &s, QFont &f )
{
    UINT8 bits, styleH, charS, w;

    s >> f.data->family;
    s >> f.data->pointSize;
    s >> styleH >> charS >> w >> bits;

    f.data->styleHint	  = styleH;
    f.data->charSet	  = charS;
    f.data->weight	  = w;
    f.data->italic	  = ( bits && 0x1 ) ? TRUE : FALSE;
    f.data->fixedPitch	  = ( bits && 0x2 ) ? TRUE : FALSE;
    f.data->hintSetByUser = ( bits && 0x4 ) ? TRUE : FALSE;
    f.data->rawMode	  = ( bits && 0x8 ) ? TRUE : FALSE;
    f.data->dirty	  = TRUE;
    QPainter::changedFont( &f, TRUE );	// tell painter about new font

    return s;
}


const char *QFontMetrics::family() const
{
    if ( data->dirty || f->data->dirty )
	updateData();
    return data->family;
}

int QFontMetrics::pointSize() const
{
    if ( data->dirty || f->data->dirty )
	updateData();
    return data->pointSize / 10;
}

bool QFontMetrics::italic() const
{
    if ( data->dirty || f->data->dirty )
	updateData();
    return data->italic;
}

int QFontMetrics::weight() const
{
    if ( data->dirty || f->data->dirty )
	updateData();
    return (int) data->weight;
}

bool QFontMetrics::fixedPitch() const
{
    if ( data->dirty || f->data->dirty )
	updateData();
    return data->fixedPitch;
}

QFont::StyleHint QFontMetrics::styleHint() const
{
    if ( data->dirty || f->data->dirty )
	updateData();
    return (QFont::StyleHint) data->styleHint;
}

QFont::CharSet QFontMetrics::charSet() const
{
    if ( data->dirty || f->data->dirty )
	updateData();
    return (QFont::CharSet) data->charSet;
}

bool QFontMetrics::rawMode() const
{
    if ( data->dirty || f->data->dirty )
	updateData();
    return data->rawMode;
}

bool QFontMetrics::exactMatch() const
{
    if ( data->dirty || f->data->dirty )
	updateData();
    return data->exactMatch;
}

int QFontMetrics::width( char ch ) const
{
    char tmp[2];
    tmp[1] = '\0';
    tmp[0] = ch;
    return width( tmp, 1 );
}
