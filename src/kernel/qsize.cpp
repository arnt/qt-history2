/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsize.cpp#4 $
**
** Implementation of QSize class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define QSIZE_C
#include "qsize.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qsize.cpp#4 $";
#endif


// --------------------------------------------------------------------------
// QSize member functions
//

QSize::QSize( QCOOT w, QCOOT h )
{
    wd=w; ht=h;
}

QSize &QSize::operator*=( int c )
{
    wd*=c; ht*=c; return *this;
}

QSize &QSize::operator*=( float c )
{
    wd=(QCOOT)(wd*c); ht=(QCOOT)(ht*c); return *this;
}

QSize &QSize::operator/=( int c )
{
    if ( c == 0 ) {
#if defined(CHECK_MATH)
	warning( "QSize: Attempt to divide size by zero" );
#endif
	return *this;
    }
    wd/=c; ht/=c; return *this;
}

QSize &QSize::operator/=( float c )
{
    if ( c == 0.0 ) {
#if defined(CHECK_MATH)
	warning( "QSize: Attempt to divide size by zero" );
#endif
	return *this;
    }
    wd=(QCOOT)(wd/c); ht=(QCOOT)(ht/c); return *this;
}

bool operator==( const QSize &s1, const QSize &s2 )
{
    return s1.wd == s2.wd && s1.ht == s2.ht;
}

bool operator!=( const QSize &s1, const QSize &s2 )
{
    return s1.wd != s2.wd || s1.ht != s2.ht;
}

QSize operator*( const QSize &s, int c )
{
    return QSize( s.wd*c, s.ht*c );
}

QSize operator*( int c, const QSize &s )
{
    return QSize( s.wd*c, s.ht*c );
}

QSize operator*( const QSize &s, float c )
{
    return QSize( (QCOOT)(s.wd*c), (QCOOT)(s.ht*c) );
}

QSize operator*( float c, const QSize &s )
{
    return QSize( (QCOOT)(s.wd*c), (QCOOT)(s.ht*c) );
}

QSize operator/( const QSize &s, int c )
{
    if ( c == 0 ) {
#if defined(CHECK_MATH)
	warning( "QSize: Attempt to divide size by zero" );
#endif
	return s;
    }
    return QSize( s.wd/c, s.ht/c );
}

QSize operator/( const QSize &s, float c )
{
    if ( c == 0.0 ) {
#if defined(CHECK_MATH)
	warning( "QSize: Attempt to divide size by zero" );
#endif
	return s;
    }
    return QSize( (QCOOT)(s.wd/c), (QCOOT)(s.ht/c) );
}


// --------------------------------------------------------------------------
// QSize stream functions
//

QDataStream &operator<<( QDataStream &s, const QSize &sz )
{
    return s << (INT16)sz.width() << (INT16)sz.height();
}

QDataStream &operator>>( QDataStream &s, QSize &sz )
{
    INT16 w, h;
    s >> w;  sz.rwidth() = w;
    s >> h;  sz.rheight() = h;
    return s;
}
