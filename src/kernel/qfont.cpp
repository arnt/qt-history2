/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.cpp#1 $
**
** Implementation of QFont and QFontMetrics classes
**
** Author  : Eirik Eng
** Created : 941207
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qfont.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfont.cpp#1 $";
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
    QFont f( data->name );
    return f;
}


void QFont::setFamily( const char *family )
{
    if ( data->family != family ) {
        data->family = family;
	dirty = TRUE;
    }
}

void QFont::setPointSize( int tenTimesPointSize )
{
#if defined(CHECK_RANGE)
    if ( tenTimesPointSize < 0 ) {
        warning( "QFont::setPointSize: Negative point size (%i)",
                 tenTimesPointSize );
        return;
    }
#endif
    if ( data->pointSize != tenTimesPointSize ) {
        data->pointSize = tenTimesPointSize;
	dirty = TRUE;
    }
}

void QFont::setItalic( bool i )
{
    if ( data->italic != i ) {
        data->italic = i;
	dirty = TRUE;
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
    if ( data->weigt != w ) {
        data->weigt = w;
	dirty = TRUE;
    }
}

void QFont::setFixedPitch( bool b )
{
    if ( data->fixedPitch != b ) {
        data->fixedPitch = b;
	dirty = TRUE;
    }
}

void QFont::setStyleHint( StyleHint h )
{
    if ( data->styleHint != h ) {
        data->styleHint     = h;
        data->hintSetByUser = TRUE;
	dirty = TRUE;
    }
}

void QFont::setCharSet( CharSet c )
{
    if ( data->charSet != c ) {
        data->charSet = c;
	dirty = TRUE;
    }
}

bool QFont::exactMatch()
{
    if ( dirty )
        loadFont();
    return exactMatch;
}

void QFont::setRawMode( bool b )
{
    if ( data->rawMode != b ) {
        data->rawMode = b;
	dirty = TRUE;
    }
}

bool QFont::operator==( const QFont &f ) const
{
    return (f.data == data) || (f.data->name == data->name);
}

void QFont::init()
{

    data = new QFontData;
    CHECK_PTR( data );
    data->styleHint     = AnyStyle;
    data->charSet       = Latin1;
    data->fixedPitch    = FALSE;
    data->dirty         = TRUE;
    data->exactMatch    = FALSE;
    data->hintSetByUser = FALSE;
    data->rawMode       = FALSE;
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
             << bits

}

QDataStream &operator>>( QDataStream &s, QFont &f )
{
    UINT8 bits, styleH, charS, w;

    s >> f.data->family;
    s >> f.data->pointSize;
    s >> styleH >> charS >> w >> bits;

    f.data->styleHint     = styleH;
    f.data->charSet       = charS;
    f.data->weight        = w;
    f.data->italic        = ( bits && 0x1 ) ? TRUE : FALSE;
    f.data->fixedPitch    = ( bits && 0x2 ) ? TRUE : FALSE;
    f.data->hintSetByUser = ( bits && 0x4 ) ? TRUE : FALSE;
    f.data->rawMode       = ( bits && 0x8 ) ? TRUE : FALSE;
    f.data->dirty         = TRUE;

    return s;
}


