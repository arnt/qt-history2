/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsize.cpp#6 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qsize.cpp#6 $";
#endif

/*!
\class QSize qsize.h

\brief This class is used all over Qt to represent sizes of things and
nowhere to represent points - QPoint represents points.

This is a convenience and type-safety class.  It provides convenient
operators for doing things to sizes, and it provides safety against
thinko bugs, since the compiler kan know that you're (I would never do
anything like that) confusing two variables, for instance a point and
a size.
*/

// --------------------------------------------------------------------------
// QSize member functions
//

/*! Creates a new QSize with width \e w and height \e h. */
QSize::QSize( QCOORD w, QCOORD h )
{
    wd=w; ht=h;
}

/*! Multiplies both the width and height with \e c.  There is no
    overflow checking. */

QSize &QSize::operator*=( int c )
{
    wd*=c; ht*=c; return *this;
}

/*! Multiplies both the width and height with \e c.  There is no
    overflow checking. */

QSize &QSize::operator*=( float c )
{
    wd=(QCOORD)(wd*c); ht=(QCOORD)(ht*c); return *this;
}

/*! Divides both the width and height by \e c.  Excep in debug mode, 0
is equal to 1 (a major mathematical discovery). */
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

/*! Divides both the width and height by \e c.  Excep in debug mode, 0
is equal to 1. */
QSize &QSize::operator/=( float c )
{
    if ( c == 0.0 ) {
#if defined(CHECK_MATH)
	warning( "QSize: Attempt to divide size by zero" );
#endif
	return *this;
    }
    wd=(QCOORD)(wd/c); ht=(QCOORD)(ht/c); return *this;
}

/*! Nothing remarkable; just simpler than comparing both components */
bool operator==( const QSize &s1, const QSize &s2 )
{
    return s1.wd == s2.wd && s1.ht == s2.ht;
}

/*! Nothing remarkable; just simpler than comparing both components */
bool operator!=( const QSize &s1, const QSize &s2 )
{
    return s1.wd != s2.wd || s1.ht != s2.ht;
}

/*! Multiplies \e s by \e c and returns the result. */
QSize operator*( const QSize &s, int c )
{
    return QSize( s.wd*c, s.ht*c );
}

/*! Multiplies \e s by \e c and returns the result. */
QSize operator*( int c, const QSize &s )
{
    return QSize( s.wd*c, s.ht*c );
}

/*! Multiplies \e s by \e c and returns the result. */
QSize operator*( const QSize &s, float c )
{
    return QSize( (QCOORD)(s.wd*c), (QCOORD)(s.ht*c) );
}

/*! Multiplies \e s by \e c and returns the result. */
QSize operator*( float c, const QSize &s )
{
    return QSize( (QCOORD)(s.wd*c), (QCOORD)(s.ht*c) );
}

/*!  Divides \e s by \e c and returns the result. Division by zero is
treated like division by one except in debug mode. */
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

/*!  Divides \e s by \e c and returns the result. Division by zero is
treated like division by one except in debug mode. */
QSize operator/( const QSize &s, float c )
{
    if ( c == 0.0 ) {
#if defined(CHECK_MATH)
	warning( "QSize: Attempt to divide size by zero" );
#endif
	return s;
    }
    return QSize( (QCOORD)(s.wd/c), (QCOORD)(s.ht/c) );
}


// --------------------------------------------------------------------------
// QSize stream functions
//

/*! Writes the width, then the height to the stream. */
QDataStream &operator<<( QDataStream &s, const QSize &sz )
{
    return s << (INT16)sz.width() << (INT16)sz.height();
}

/*! Reads the width, then the height from the stream. */
QDataStream &operator>>( QDataStream &s, QSize &sz )
{
    INT16 w, h;
    s >> w;  sz.rwidth() = w;
    s >> h;  sz.rheight() = h;
    return s;
}

