/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsize.cpp#7 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qsize.cpp#7 $";
#endif

/*!
\class QSize qsize.h
\brief The QSize class defines the size of a two-dimensional object.

A size is specified by a width and a height.

The width/height type is QCOORD (defined as <code>short</code>). The minimum
value of QCOORD is -32768 and the maximum value is 32767.

\sa QPoint and QRect.
*/

// --------------------------------------------------------------------------
// QSize member functions
//

/*!
\fn QSize::QSize()
Constructs a size with undefined width and height.
*/

/*!
Constructs a size with width \e w and height \e h.
*/

QSize::QSize( QCOORD w, QCOORD h )
{
    wd=w; ht=h;
}

/*!
\fn bool QSize::isNull() const
Returns TRUE if the width is 0 and the height is 0, otherwise FALSE.
*/

/*!
\fn bool QSize::isEmpty() const
Returns TRUE if the width is less than 0 or the height is less than 0,
otherwise FALSE.
*/

/*!
\fn bool QSize::isValid() const
Returns TRUE if the width is equal to or greater than 0 and the height is
equal to or greater than 0, otherwise FALSE.
*/

/*!
\fn QCOORD QSize::width() const
Returns the width.

\sa height().
*/

/*!
\fn QCOORD QSize::height() const
Returns the height.

\sa width().
*/

/*!
\fn void QSize::setWidth( QCOORD w )
Sets the width to \e w.

\sa setHeight().
*/

/*!
\fn void QSize::setHeight( QCOORD h )
Sets the height to \e h.

\sa setWidth().
*/

/*!
\fn QCOORD &QSize::rwidth()
Returns a reference to the width.

Using a reference makes it possible to directly manipulate the width:
\code
  QSize s( 100, 10 );
  s.rwidth() += 20;		\/ s becomes (120,10)
\endcode

\sa rheight().
*/

/*!
\fn QCOORD &QSize::rheight()
Returns a reference to the height.

Using a reference makes it possible to directly manipulate the height:
\code
  QSize s( 100, 10 );
  s.rheight() += 5;		\/ s becomes (100,15)
\endcode

\sa rwidth().
*/

/*!
Multiplies both the width and height with \e c and returns a reference to
the size.
*/

QSize &QSize::operator*=( int c )
{
    wd*=c; ht*=c; return *this;
}

/*!
Multiplies both the width and height with \e c and returns a reference to
the size.

Notice that the result is truncated.
*/

QSize &QSize::operator*=( float c )
{
    wd=(QCOORD)(wd*c); ht=(QCOORD)(ht*c); return *this;
}

/*!
Divides both the width and height by \e c and returns a reference to the
size.

The division will not be performed if \e c is 0.
*/

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

/*!
Divides both the width and height by \e c and returns a reference to the
size.

The division will not be performed if \e c is 0.

Notice that the result is truncated.
*/

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

/*!
\relates QSize
Returns TRUE if \e s1 and \e s2 are equal, or FALSE if they are different.
*/

bool operator==( const QSize &s1, const QSize &s2 )
{
    return s1.wd == s2.wd && s1.ht == s2.ht;
}

/*!
\relates QSize
Returns TRUE if \e s1 and \e s2 are different, or FALSE if they are equal.
*/

bool operator!=( const QSize &s1, const QSize &s2 )
{
    return s1.wd != s2.wd || s1.ht != s2.ht;
}

/*!
\relates QSize
Multiplies \e s by \e c and returns the result.
*/

QSize operator*( const QSize &s, int c )
{
    return QSize( s.wd*c, s.ht*c );
}

/*!
\relates QSize
Multiplies \e s by \e c and returns the result.
*/

QSize operator*( int c, const QSize &s )
{
    return QSize( s.wd*c, s.ht*c );
}

/*!
\relates QSize
Multiplies \e s by \e c and returns the result.
*/

QSize operator*( const QSize &s, float c )
{
    return QSize( (QCOORD)(s.wd*c), (QCOORD)(s.ht*c) );
}

/*!
\relates QSize
Multiplies \e s by \e c and returns the result.
*/

QSize operator*( float c, const QSize &s )
{
    return QSize( (QCOORD)(s.wd*c), (QCOORD)(s.ht*c) );
}

/*!
\relates QSize
Divides \e s by \e c and returns the result.

This function returns \e s if \e c is 0.
*/

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

/*!
\relates QSize
Divides \e s by \e c and returns the result.

This function returns \e s if \e c is 0.

Notice that the result is truncated.
*/

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

/*!
\relates QSize
Writes the size to the stream.

The output format is two INT16 (first teh width, then the height).
*/

QDataStream &operator<<( QDataStream &s, const QSize &sz )
{
    return s << (INT16)sz.width() << (INT16)sz.height();
}

/*!
\relates QSize
Reads the size from the stream.
*/

QDataStream &operator>>( QDataStream &s, QSize &sz )
{
    INT16 w, h;
    s >> w;  sz.rwidth() = w;
    s >> h;  sz.rheight() = h;
    return s;
}
