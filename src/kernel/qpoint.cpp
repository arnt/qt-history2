/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpoint.cpp#32 $
**
** Implementation of QPoint class
**
** Created : 931028
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qpoint.h"
#include "qdatastream.h"


/*!
  \class QPoint qpoint.h
  \brief The QPoint class defines a point in the plane.

  \ingroup drawing

  A point is specified by an x coordinate and a y coordinate.

  The coordinate type is QCOORD (defined in qwindowdefs.h as \c int).
  The minimum value of QCOORD is QCOORD_MIN (-2147483648) and the maximum
  value is  QCOORD_MAX (2147483647).

  We have defined many operator functions that make arithmetic on points
  simple and intuitive.

  Example:
  \code
    QPoint p(  1, 2 );
    QPoint q( -8, 5 );
    QPoint r(  9, 7 );
    QPoint x = 2*p + (q-r)*5.5 - (r+p/1.5);
  \endcode

  \sa QSize, QRect
*/


/*****************************************************************************
  QPoint member functions
 *****************************************************************************/

/*!
  \fn QPoint::QPoint()
  Constructs a point with undefined x and y values.
*/

/*!
  \fn QPoint::QPoint( int xpos, int ypos )
  Constructs a point with the x value  \e xpos and y value \e ypos.
*/

/*!
  \fn bool QPoint::isNull() const
  Returns TRUE if both the x value and the y value are 0.
*/

/*!
  \fn int QPoint::x() const
  Returns the x coordinate of the point.
  \sa y()
*/

/*!
  \fn int QPoint::y() const
  Returns the y coordinate of the point.
  \sa x()
*/

/*!
  \fn void QPoint::setX( int x )
  Sets the x coordinate of the point to \e x.
  \sa setY()
*/

/*!
  \fn void QPoint::setY( int y )
  Sets the y coordinate of the point to \e y.
  \sa setX()
*/


/*!
  \fn QCOORD &QPoint::rx()
  Returns a reference to the x coordinate of the point.

  Using a reference makes it possible to directly manipulate x.

  Example:
  \code
    QPoint p( 1, 2 );
    p.rx()--;			// p becomes (0,2)
  \endcode

  \sa ry()
*/

/*!
  \fn QCOORD &QPoint::ry()
  Returns a reference to the y coordinate of the point.

  Using a reference makes it possible to directly manipulate y.

  Example:
  \code
    QPoint p( 1, 2 );
    p.ry()++;			// p becomes (1,3)
  \endcode

  \sa rx()
*/


/*!
  \fn QPoint &QPoint::operator+=( const QPoint &p )
  Adds \e p to the point and returns a reference to this point.

  Example:
  \code
    QPoint p(  3, 7 );
    QPoint q( -1, 4 );
    p += q;			// p becomes (2,11)
  \endcode
*/

/*!
  \fn QPoint &QPoint::operator-=( const QPoint &p )
  Subtracts \e p from the point and returns a reference to this point.

  Example:
  \code
    QPoint p(  3, 7 );
    QPoint q( -1, 4 );
    p -= q;			// p becomes (4,3)
  \endcode
*/

/*!
  \fn QPoint &QPoint::operator*=( int c )
  Multiplies both x and y with \e c, and return a reference to this point.

  Example:
  \code
    QPoint p( -1, 4 );
    p *= 2;			// p becomes (-2,8)
  \endcode
*/

/*!
  \fn QPoint &QPoint::operator*=( double c )
  Multiplies both x and y with \e c, and return a reference to this point.

  Example:
  \code
    QPoint p( -1, 4 );
    p *= 2.5;			// p becomes (-3,10)
  \endcode

  Note that the result is truncated.
*/


/*!
  \fn bool operator==( const QPoint &p1, const QPoint &p2 )
  \relates QPoint
  Returns TRUE if \e p1 and \e p2 are equal, or FALSE if they are different.
*/

/*!
  \fn bool operator!=( const QPoint &p1, const QPoint &p2 )
  \relates QPoint
  Returns TRUE if \e p1 and \e p2 are different, or FALSE if they are equal.
*/

/*!
  \fn QPoint operator+( const QPoint &p1, const QPoint &p2 )
  \relates QPoint
  Returns the sum of \e p1 and \e p2; each component is added separately.
*/

/*!
  \fn QPoint operator-( const QPoint &p1, const QPoint &p2 )
  \relates QPoint
  Returns \e p2 subtracted from \e p1; each component is
  subtracted separately.
*/

/*!
  \fn QPoint operator*( const QPoint &p, int c )
  \relates QPoint
  Multiplies both of \e p's components by \e c and returns the result.
*/

/*!
  \fn QPoint operator*( int c, const QPoint &p )
  \relates QPoint
  Multiplies both of \e p's components by \e c and returns the result.
*/

/*!
  \fn QPoint operator*( const QPoint &p, double c )
  \relates QPoint
  Multiplies both of \e p's components by \e c and returns the
  result.
*/

/*!
  \fn QPoint operator*( double c, const QPoint &p )
  \relates QPoint
  Multiplies both of \e p's components by \e c and returns the
  result.
*/

/*!
  \fn QPoint operator-( const QPoint &p )
  \relates QPoint
  Returns \e p where x and y have opposite signs.
*/

/*!
  \fn QPoint &QPoint::operator/=( int c )

  Divides both x and y by \e c, and return a reference to this point.

  Example:
  \code
    QPoint p( -2, 8 );
    p /= 2;			// p becomes (-1,4)
  \endcode
*/

/*!
  \fn QPoint &QPoint::operator/=( double c )

  Divides both x and y by \e c, and return a reference to this point.

  Example:
  \code
    QPoint p( -3, 10 );
    p /= 2.5;			// p becomes (-1,4)
  \endcode

  Note that the result is truncated.
*/

/*!
  \fn QPoint operator/( const QPoint &p, int c )
  \relates QPoint
  Divides both of \e p's components by \e c and returns the result.
*/

/*!
  \fn QPoint operator/( const QPoint &p, double c )
  \relates QPoint

  Divides both of \e p's components by \e c and returns the result.

  Note that the result is truncated.
*/


void QPoint::warningDivByZero()
{
#if defined(DEBUG)
    warning( "QPoint: Division by zero error" );
#endif
}


/*****************************************************************************
  QPoint stream functions
 *****************************************************************************/

/*!
  \relates QPoint
  Writes a QPoint to the stream and returns a reference to the stream.

  Serialization format: [x (Q_INT32), y (Q_INT32)].
*/

QDataStream &operator<<( QDataStream &s, const QPoint &p )
{
    return s << (Q_INT32)p.x() << (Q_INT32)p.y();
}

/*!
  \relates QPoint
  Reads a QPoint from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QPoint &p )
{
    Q_INT32 x, y;
    s >> x;  p.rx() = x;
    s >> y;  p.ry() = y;
    return s;
}
