/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpoint.cpp#17 $
**
** Implementation of QPoint class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define QPOINT_C
#include "qpoint.h"
#include "qdstream.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qpoint.cpp#17 $")


/*----------------------------------------------------------------------------
  \class QPoint qpoint.h
  \brief The QPoint class defines a point in the plane.

  A point is specified by an x coordinate and a y coordinate.

  The coordinate type is QCOORD (defined as <code>short</code>). The minimum
  value of QCOORD is -32768 and the maximum value is 32767.

  We have defined many operator functions that make arithmetic on points
  simple and intuitive:
  \code
    QPoint p(  1, 2 );
    QPoint q( -8, 5 );
    QPoint r(  9, 7 );
    QPoint x = 2*p + (q-r)*5.5 - (r+p/1.5);
  \endcode

  \sa QSize, QRect
 ----------------------------------------------------------------------------*/


/*****************************************************************************
  QPoint member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \fn QPoint::QPoint()
  Constructs a point with undefined x and y values.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QPoint::QPoint( int xpos, int ypos )
  Constructs a point with the x value  \e xpos and y value \e ypos.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QPoint::isNull() const
  Returns TRUE if both the x value and the y value are 0.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QPoint::x() const
  Returns the x coordinate of the point.
  \sa y()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QPoint::y() const
  Returns the y coordinate of the point.
  \sa x()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QPoint::setX( int x )
  Sets the x coordinate of the point to \e x.
  \sa setY()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QPoint::setY( int y )
  Sets the y coordinate of the point to \e y.
  \sa setX()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn QCOORD &QPoint::rx()
  Returns a reference to the x coordinate of the point.

  Using a reference makes it possible to directly manipulate x:
  \code
    QPoint p( 1, 2 );
    p.rx()--;			// p becomes (0,2)
  \endcode

  \sa ry()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QCOORD &QPoint::ry()
  Returns a reference to the y coordinate of the point.

  Using a reference makes it possible to directly manipulate y:
  \code
    QPoint p( 1, 2 );
    p.ry()++;			// p becomes (1,3)
  \endcode

  \sa rx()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn QPoint &QPoint::operator+=( const QPoint &p )
  Adds \e p to the point and returns a reference to this point.

  \code
    QPoint p(  3, 7 );
    QPoint q( -1, 4 );
    p += q;			// p becomes (2,11)
  \endcode
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QPoint &QPoint::operator-=( const QPoint &p )
  Subtracts \e p from the point and returns a reference to this point.

  \code
    QPoint p(  3, 7 );
    QPoint q( -1, 4 );
    p -= q;			// p becomes (4,3)
  \endcode
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QPoint &QPoint::operator*=( int c )
  Multiplies both x and y with \e c, and return a reference to this point.

  \code
    QPoint p( -1, 4 );
    p *= 2;			// p becomes (-2,8)
  \endcode
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QPoint &QPoint::operator*=( double c )
  Multiplies both x and y with \e c, and return a reference to this point.

  \code
    QPoint p( -1, 4 );
    p *= 2.5;			// p becomes (-3,10)
  \endcode

  Notice that the result is truncated.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \relates QPoint
  \fn bool operator==( const QPoint &p1, const QPoint &p2 )
  Returns TRUE if \e p1 and \e p2 are equal, or FALSE if they are different.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \relates QPoint
  \fn bool operator!=( const QPoint &p1, const QPoint &p2 )
  Returns TRUE if \e p1 and \e p2 are different, or FALSE if they are equal.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \relates QPoint
  \fn QPoint operator+( const QPoint &p1, const QPoint &p2 )
  Returns the sum of \e p1 and \e p2; each component is added separately.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \relates QPoint
  \fn QPoint operator-( const QPoint &p1, const QPoint &p2 )
  Returns \e p2 subtracted from \e p1; each component is
  subtracted separately.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \relates QPoint
  \fn QPoint operator*( const QPoint &p, int c )
  Multiplies both of \e p's components by \e c and returns the result.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \relates QPoint
  \fn QPoint operator*( int c, const QPoint &p )
  Multiplies both of \e p's components by \e c and returns the result.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \relates QPoint
  \fn QPoint operator*( const QPoint &p, double c )
  Multiplies both of \e p's components by \e c and returns the
  result.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \relates QPoint
  \fn QPoint operator*( double c, const QPoint &p )
  Multiplies both of \e p's components by \e c and returns the
  result.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \relates QPoint
  \fn QPoint operator-( const QPoint &p )
  Returns \e p where x and y have opposite signs.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Divides both x and y by \e c, and return a reference to this point.

  The division will not be performed if \e c is 0.

  \code
    QPoint p( -2, 8 );
    p /= 2;			// p becomes (-1,4)
  \endcode
 ----------------------------------------------------------------------------*/

QPoint &QPoint::operator/=( int c )
{
    if ( c == 0 )
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
    xp/=(QCOORD)c; yp/=(QCOORD)c;
    return *this;
}

/*----------------------------------------------------------------------------
  Divides both x and y by \e c, and return a reference to this point.

  The division will not be performed if \e c is 0.

  \code
    QPoint p( -3, 10 );
    p /= 2.5;			// p becomes (-1,4)
  \endcode

  Notice that the result is truncated.
 ----------------------------------------------------------------------------*/

QPoint &QPoint::operator/=( double c )
{
    if ( c == 0.0 )
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
    xp=(QCOORD)(xp/c); yp=(QCOORD)(yp/c);
    return *this;
}

/*----------------------------------------------------------------------------
  \relates QPoint
  Divides both of \e p's components by \e c and returns the result.

  This function returns \e p if \e c is 0.
 ----------------------------------------------------------------------------*/

QPoint operator/( const QPoint &p, int c )
{
    if ( c == 0 )
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
    return QPoint( p.xp/c, p.yp/c );
}

/*----------------------------------------------------------------------------
  \relates QPoint
  Divides both of \e p's components by \e c and returns the result.

  This function returns \e p if \e c is 0.

  Notice that the result is truncated.
 ----------------------------------------------------------------------------*/

QPoint operator/( const QPoint &p, double c )
{
    if ( c == 0.0 )
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
    return QPoint( (QCOORD)(p.xp/c), (QCOORD)(p.yp/c) );
}


/*****************************************************************************
  QPoint stream functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \relates QPoint
  Writes a QPoint to the stream and returns a reference to the stream.

  First \e p.x() and then p.y() are written, both as signed 16-bit
  numbers.
 ----------------------------------------------------------------------------*/

QDataStream &operator<<( QDataStream &s, const QPoint &p )
{
    return s << (INT16)p.x() << (INT16)p.y();
}

/*----------------------------------------------------------------------------
  \relates QPoint
  Reads a QPoint from the stream and returns a reference to the stream.
 ----------------------------------------------------------------------------*/

QDataStream &operator>>( QDataStream &s, QPoint &p )
{
    INT16 x, y;
    s >> x;  p.rx() = x;
    s >> y;  p.ry() = y;
    return s;
}
