/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsize.cpp#19 $
**
** Implementation of QSize class
**
** Created : 931028
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define QSIZE_C
#include "qsize.h"
#include "qdstream.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qsize.cpp#19 $");


/*!
  \class QSize qsize.h
  \brief The QSize class defines the size of a two-dimensional object.

  A size is specified by a width and a height.

  The coordinate type is QCOORD (defined in qwindefs.h as \c short).
  The minimum value of QCOORD is QCOORD_MIN (-32768) and the maximum
  value is  QCOORD_MAX (32767).

  \sa QPoint, QRect
*/


/*****************************************************************************
  QSize member functions
 *****************************************************************************/

/*!
  \fn QSize::QSize()
  Constructs a size with undefined width and height.
*/

/*!
  \fn QSize::QSize( int w, int h )
  Constructs a size with width \e w and height \e h.
*/

/*!
  \fn bool QSize::isNull() const
  Returns TRUE if the width is 0 and the height is 0, otherwise FALSE.
*/

/*!
  \fn bool QSize::isEmpty() const
  Returns TRUE if the width is <= 0 or the height is <= 0,
  otherwise FALSE.
*/

/*!
  \fn bool QSize::isValid() const
  Returns TRUE if the width is equal to or greater than 0 and the height is
  equal to or greater than 0, otherwise FALSE.
*/

/*!
  \fn int QSize::width() const
  Returns the width.
  \sa height()
*/

/*!
  \fn int QSize::height() const
  Returns the height.
  \sa width()
*/

/*!
  \fn void QSize::setWidth( int w )
  Sets the width to \e w.
  \sa width(), setHeight()
*/

/*!
  \fn void QSize::setHeight( int h )
  Sets the height to \e h.
  \sa height(), setWidth()
*/

/*!
  Swaps the values of width and height.
*/

void QSize::transpose()
{
    QCOORD tmp = wd;
    wd = ht;
    ht = tmp;
}

/*!
  \fn QCOORD &QSize::rwidth()
  Returns a reference to the width.

  Using a reference makes it possible to directly manipulate the width.

  Example:
  \code
    QSize s( 100, 10 );
    s.rwidth() += 20;		// s becomes (120,10)
  \endcode

  \sa rheight()
*/

/*!
  \fn QCOORD &QSize::rheight()
  Returns a reference to the height.

  Using a reference makes it possible to directly manipulate the height.

  Example:
  \code
    QSize s( 100, 10 );
    s.rheight() += 5;		// s becomes (100,15)
  \endcode

  \sa rwidth()
*/

/*!
  \fn QSize &QSize::operator+=( const QSize &s )

  Adds \e s to the size and returns a reference to this size.

  Example:
  \code
    QSize s(  3, 7 );
    QSize r( -1, 4 );
    s += r;			// s becomes (2,11)
\endcode
*/

/*!
  \fn QSize &QSize::operator-=( const QSize &s )

  Subtracts \e s from the size and returns a reference to this size.

  Example:
  \code
    QSize s(  3, 7 );
    QSize r( -1, 4 );
    s -= r;			// s becomes (4,3)
  \endcode
*/

/*!
  \fn QSize &QSize::operator*=( int c )
  Multiplies both the width and height with \e c and returns a reference to
  the size.
*/

/*!
  \fn QSize &QSize::operator*=( float c )

  Multiplies both the width and height with \e c and returns a reference to
  the size.

  Note that the result is truncated.
*/

/*!
  \fn bool operator==( const QSize &s1, const QSize &s2 )
  \relates QSize
  Returns TRUE if \e s1 and \e s2 are equal, or FALSE if they are different.
*/

/*!
  \fn bool operator!=( const QSize &s1, const QSize &s2 )
  \relates QSize
  Returns TRUE if \e s1 and \e s2 are different, or FALSE if they are equal.
*/

/*!
  \fn QSize operator+( const QSize &s1, const QSize &s2 )
  \relates QSize
  Returns the sum of \e s1 and \e s2; each component is added separately.
*/

/*!
  \fn QSize operator-( const QSize &s1, const QSize &s2 )
  \relates QSize
  Returns \e s2 subtracted from \e s1; each component is
  subtracted separately.
*/

/*!
  \fn QSize operator*( const QSize &s, int c )
  \relates QSize
  Multiplies \e s by \e c and returns the result.
*/

/*!
  \fn QSize operator*( int c, const QSize &s )
  \relates QSize
  Multiplies \e s by \e c and returns the result.
*/

/*!
  \fn QSize operator*( const QSize &s, float c )
  \relates QSize
  Multiplies \e s by \e c and returns the result.
*/

/*!
  \fn QSize operator*( float c, const QSize &s )
  \relates QSize
  Multiplies \e s by \e c and returns the result.
*/


/*!
  Divides both the width and height by \e c and returns a reference to the
  size.
*/

QSize &QSize::operator/=( int c )
{
    if ( c == 0 )
#if defined(CHECK_MATH)
	warning( "QSize: Attempt to divide size by zero" );
#endif
    wd/=(QCOORD)c; ht/=(QCOORD)c; return *this;
}

/*!
  Divides both the width and height by \e c and returns a reference to the
  size.

  Note that the result is truncated.
*/

QSize &QSize::operator/=( float c )
{
    if ( c == 0.0 )
#if defined(CHECK_MATH)
	warning( "QSize: Attempt to divide size by zero" );
#endif
    wd=(QCOORD)(wd/c); ht=(QCOORD)(ht/c); return *this;
}

/*!
  \relates QSize
  Divides \e s by \e c and returns the result.
*/

QSize operator/( const QSize &s, int c )
{
    if ( c == 0 )
#if defined(CHECK_MATH)
	warning( "QSize: Attempt to divide size by zero" );
#endif
    return QSize( s.wd/c, s.ht/c );
}

/*!
  \relates QSize
  Divides \e s by \e c and returns the result.

  Note that the result is truncated.
*/

QSize operator/( const QSize &s, float c )
{
    if ( c == 0.0 )
#if defined(CHECK_MATH)
	warning( "QSize: Attempt to divide size by zero" );
#endif
    return QSize( (QCOORD)(s.wd/c), (QCOORD)(s.ht/c) );
}


/*****************************************************************************
  QSize stream functions
 *****************************************************************************/

/*!
  \relates QSize
  Writes the size to the stream and returns a reference to the stream.

  Serialization format: [width (INT16), height (INT16)].
*/

QDataStream &operator<<( QDataStream &s, const QSize &sz )
{
    return s << (INT16)sz.width() << (INT16)sz.height();
}

/*!
  \relates QSize
  Reads the size from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QSize &sz )
{
    INT16 w, h;
    s >> w;  sz.rwidth() = w;
    s >> h;  sz.rheight() = h;
    return s;
}


