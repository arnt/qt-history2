/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpoint.h"
#include "qdatastream.h"
#include "qdebug.h"


/*!
    \class QPoint qpoint.h
    \brief The QPoint class defines a point in the plane.

    \ingroup multimedia

    A point is specified by an x coordinate and a y coordinate. The coordinates
    are specified using integer numbers. QPointF provides points with floating point
    accuracy.

    The coordinates are accessed by the functions x() and y(); they
    can be set by setX() and setY() or by the reference functions rx()
    and ry().

    Given a point \e p, the following statements are all equivalent:
    \code
        p.setX(p.x() + 1);
        p += QPoint(1, 0);
        p.rx()++;
    \endcode

    A QPoint can also be used as a vector. Addition and subtraction
    of QPoints are defined as for vectors (each component is added
    separately). You can divide or multiply a QPoint by an \c int or a
    \c qreal. The function manhattanLength() gives an inexpensive
    approximation of the length of the QPoint interpreted as a vector.

    Example:
    \code
        //QPoint oldPos is defined somewhere else
        MyWidget::mouseMoveEvent(QMouseEvent *e)
        {
            QPoint vector = e->pos() - oldPos;
            if (vector.manhattanLength() > 3)
            ... //mouse has moved more than 3 pixels since oldPos
        }
    \endcode

    QPoints can be compared for equality or inequality, and they can
    be written to and read from a QStream.

    \sa QPolygon QSize QRect QPointF
*/


/*****************************************************************************
  QPoint member functions
 *****************************************************************************/

/*!
    \fn QPoint::QPoint()

    Constructs a point with coordinates (0, 0) (isNull() returns true).
*/

/*!
    \fn QPoint::QPoint(int xpos, int ypos)

    Constructs a point with x value \a xpos and y value \a ypos.
*/

/*!
    \fn bool QPoint::isNull() const

    Returns true if both the x value and the y value are 0; otherwise
    returns false.
*/

/*!
    \fn int QPoint::x() const

    Returns the x coordinate of the point.

    \sa setX() y()
*/

/*!
    \fn int QPoint::y() const

    Returns the y coordinate of the point.

    \sa setY() x()
*/

/*!
    \fn void QPoint::setX(int x)

    Sets the x coordinate of the point to \a x.

    \sa x() setY()
*/

/*!
    \fn void QPoint::setY(int y)

    Sets the y coordinate of the point to \a y.

    \sa y() setX()
*/


/*!
    \fn int &QPoint::rx()

    Returns a reference to the x coordinate of the point.

    Using a reference makes it possible to directly manipulate x.

    Example:
    \code
        QPoint p(1, 2);
        p.rx()--;         // p becomes (0, 2)
    \endcode

    \sa ry() setX()
*/

/*!
    \fn int &QPoint::ry()

    Returns a reference to the y coordinate of the point.

    Using a reference makes it possible to directly manipulate y.

    Example:
    \code
        QPoint p(1, 2);
        p.ry()++;         // p becomes (1, 3)
    \endcode

    \sa rx() setY()
*/


/*!
    \fn QPoint &QPoint::operator+=(const QPoint &p)

    Adds point \a p to this point and returns a reference to this
    point.

    Example:
    \code
        QPoint p( 3, 7);
        QPoint q(-1, 4);
        p += q;            // p becomes (2,11)
    \endcode
*/

/*!
    \fn QPoint &QPoint::operator-=(const QPoint &p)

    Subtracts point \a p from this point and returns a reference to
    this point.

    Example:
    \code
        QPoint p( 3, 7);
        QPoint q(-1, 4);
        p -= q;            // p becomes (4,3)
    \endcode
*/

/*!
    \fn QPoint &QPoint::operator*=(qreal c)
    \overload

    Multiplies this point's x and y by \a c, and returns a reference
    to this point.

    Example:
    \code
        QPoint p(-1, 4);
        p *= 2.5;          // p becomes (-3,10)
    \endcode

    Note that the result is rounded to the nearest integer as points are held as
    integers.

    \sa QPointF
*/


/*!
    \fn bool operator==(const QPoint &p1, const QPoint &p2)

    \relates QPoint

    Returns true if \a p1 and \a p2 are equal; otherwise returns false.
*/

/*!
    \fn bool operator!=(const QPoint &p1, const QPoint &p2)

    \relates QPoint

    Returns true if \a p1 and \a p2 are not equal; otherwise returns false.
*/

/*!
    \fn const QPoint operator+(const QPoint &p1, const QPoint &p2)

    \relates QPoint

    Returns the sum of \a p1 and \a p2; each component is added separately.
*/

/*!
    \fn const QPoint operator-(const QPoint &p1, const QPoint &p2)

    \relates QPoint

    Returns \a p2 subtracted from \a p1; each component is subtracted
    separately.
*/

/*!
    \fn const QPoint operator*(const QPoint &p, qreal c)
    \overload

    \relates QPoint

    Returns the QPoint formed by multiplying both components of \a p
    by \a c.

    Note that the result is rounded to the nearest integer as points are held as
    integers.

    \sa QPointF
*/

/*!
    \fn const QPoint operator*(qreal c, const QPoint &p)
    \overload

    \relates QPoint

    Returns the QPoint formed by multiplying both components of \a p
    by \a c.

    Note that the result is rounded to the nearest integer as points are held as
    integers.

    \sa QPointF
*/

/*!
    \fn const QPoint operator-(const QPoint &p)
    \overload

    \relates QPoint

    Returns the QPoint formed by changing the sign of both components
    of \a p, equivalent to \c{QPoint(0,0) - p}.
*/

/*!
    \fn QPoint &QPoint::operator/=(qreal c)
    \overload

    Divides both x and y by \a c, and returns a reference to this
    point.

    Example:
    \code
        QPoint p(-3, 10);
        p /= 2.5;           // p becomes (-1,4)
    \endcode

    Note that the result is rounded to the nearest integer as points are held as
    integers.

    \sa QPointF
*/

/*!
    \fn const QPoint operator/(const QPoint &p, qreal c)
    \overload

    \relates QPoint

    Returns the QPoint formed by dividing both components of \a p
    by \a c.

    Note that the result is rounded to the nearest integer as points are held as
    integers.

    \sa QPointF
*/

/*****************************************************************************
  QPoint stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QPoint

    Writes point \a p to the stream \a s and returns a reference to
    the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QPoint &p)
{
    if (s.version() == 1)
        s << (qint16)p.x() << (qint16)p.y();
    else
        s << (qint32)p.x() << (qint32)p.y();
    return s;
}

/*!
    \relates QPoint

    Reads a QPoint from the stream \a s into point \a p and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QPoint &p)
{
    if (s.version() == 1) {
        qint16 x, y;
        s >> x;  p.rx() = x;
        s >> y;  p.ry() = y;
    }
    else {
        qint32 x, y;
        s >> x;  p.rx() = x;
        s >> y;  p.ry() = y;
    }
    return s;
}

#endif // QT_NO_DATASTREAM
/*!
    Returns the sum of the absolute values of x() and y(),
    traditionally known as the "Manhattan length" of the vector from
    the origin to the point. The tradition arises because such
    distances apply to travelers who can only travel on a rectangular
    grid, like the streets of Manhattan.

    This is a useful, and quick to calculate, approximation to the
    true length: sqrt(pow(x(),2)+pow(y(),2)).
*/
int QPoint::manhattanLength() const
{
    return qAbs(x())+qAbs(y());
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QPoint &p) {
    dbg.nospace() << "QPoint(" << p.x() << ',' << p.y() << ')';
    return dbg.space();
}

QDebug operator<<(QDebug d, const QPointF &p)
{
    d << "QPointF(" << p.x() << ", " << p.y() << ")";
    return d;
}
#endif

/*!
    \class QPointF
    \brief The QPointF class provides a point object that uses floating
    point coordinates for accuracy.

    A QPointF describes a point on a two-dimensional surface. The
    coordinates of the point are specified using floating point numbers
    for accuracy. QPoint provides points with integer accuracy.

    Convenience functions are provided for reading and writing the individual
    coordinates used to define the point: x(), y(), setX(), and setY().
    QPointF also provides support for the standard arithmetic operators,
    treating each point as a vector from the origin.

    \sa QPolygonF QSizeF QRectF QPoint
*/

/*!
    \fn QPointF::QPointF()

    Constructs a null point.

    \sa isNull()
*/

/*!
    \fn QPointF::QPointF(const QPoint &point)

    Copy constructor. Constructs a point using the values of the \a point
    specified.
*/

/*!
    \fn QPointF::QPointF(qreal x, qreal y)

    Constructs a point with coordinates specified by \a x and \a y.
*/

/*!
    \fn bool QPointF::isNull() const

    Returns true if the point is null; otherwise returns false.

    A point is considered to be null if both the x- and y-coordinates are equal to zero.
*/

/*!
    \fn qreal QPointF::x() const

    Returns the x-coordinate of the point.

    \sa setX() y()
*/

/*!
    \fn qreal QPointF::y() const

    Returns the y-coordinate of the point.

    \sa setX() y()
*/

/*!
    \fn void QPointF::setX(qreal x)

    Sets the x-coordinate of the point to the value specified by \a x.

    \sa setY() x() rx()
*/

/*!
    \fn void QPointF::setY(qreal y)

    Sets the y-coordinate of the point to the value specified by \a y.

    \sa setX() y() ry()
*/

/*!
    \fn qreal& QPointF::rx()

    Returns a reference to the x-coordinate of the point.

    \sa ry() setX()
*/

/*!
    \fn qreal& QPointF::ry()

    Returns a reference to the y-coordinate of the point.

    \sa rx() setY()
*/

/*!
    \fn QPointF& QPointF::operator+=(const QPointF &other)

    Adds the coordinates of this point to the corresponding coordinates of
    the \a other point, and returns a reference to this point with the new
    coordinates. This is equivalent to vector addition.

    \sa operator+() operator-=()
*/

/*!
    \fn QPointF& QPointF::operator-=(const QPointF &other)

    Subtracts the coordinates of the \a other point from the
    corresponding coordinates of this point, and returns a reference to this
    point with the new coordinates. This is equivalent to vector subtraction.

    \sa operator-() operator+=()
*/

/*!
    \fn QPointF& QPointF::operator*=(qreal factor)

    Multiplies the coordinates of this point by the given scale \a factor, and
    returns a reference to this point with the new coordinates. This is
    equivalent to scalar multiplication of a vector.

    \sa operator*() operator/=()
*/

/*!
    \fn QPointF& QPointF::operator/=(qreal factor)

    Divides the coordinates of this point by the given scale \a factor, and
    returns a references to this point with the new coordinates. This is
    equivalent to scalar division of a vector.

    \sa operator*()
*/

/*!
    \fn bool operator==(const QPointF &, const QPointF &)

    Returns true if the \a point is the same as the \a other point given.

    \sa operator!=()
*/

/*!
    \fn bool operator!=(const QPointF &point, const QPointF &other)

    Returns true if the \a point is not the same as the \a other point given.

    \sa operator==()
*/

/*!
    \fn const QPointF operator+(const QPointF &point, const QPointF &other)

    Adds the coordinates of the \a point to the corresponding coordinates of
    the \a other point, and returns a point with the new coordinates. (Vector
    addition.)

    \sa operator-()
*/

/*!
    \fn const QPointF operator-(const QPointF &point, const QPointF &other)

    Subtracts the coordinates of the \a other point from the
    corresponding coordinates of the given \a point, and returns a point with
    the new coordinates. (Vector subtraction.)

    \sa operator+()
*/

/*!
    \fn const QPointF operator*(const QPointF &point, qreal factor)

    \overload

    Multiplies the coordinates of the \a point by the given scale \a factor, and
    returns a point with the new coordinates. (Scalar multiplication of a
    vector.)

    \sa operator/()
*/

/*!
    \fn const QPointF operator*(qreal factor, const QPointF &point)

    \overload

    Multiplies the coordinates of the \a point by the given scale \a factor, and
    returns a point with the new coordinates. (Scalar multiplication of a
    vector.)

    \sa operator/()
*/

/*!
    \fn const QPointF operator-(const QPointF &point)

    Negates the coordinates of the \a point, and returns a point with the
    new coordinates. (Inversion).
*/

/*!
    \fn const QPointF operator/(const QPointF &point, qreal factor)

    Divides the coordinates of the \a point by the given scale \a factor, and
    returns a point with the new coordinates. (Scalar division of a vector.)

    \sa operator*()
*/

/*!
    \fn QPoint QPointF::toPoint() const

    Rounds the coordinates of this point to the nearest integer and returns a QPoint
    with these rounded coordinates.
*/

/*!
    \fn bool operator==(const QPointF &point1, const QPointF &point2)
    \relates QPointF

    Returns true if \a point1 is equal to \a point2; otherwise returns false.

    Two points are equal to each other if both x-coordinates and both
    y-coordinates are the same.

*/

/*!
    \fn bool operator!=(const QPointF &point1, const QPointF &point2);
    \relates QPointF

    Returns true if \a point1 is not equal to \a point2; otherwise returns false.
*/
/*!
    \fn const QPointF operator+(const QPointF &point1, const QPointF &point2)
    \relates QPointF

    Returns the sum of \a point1 and \a point2. Each component is added separately.
*/
/*!
    \fn const QPointF operator-(const QPointF &point1, const QPointF &point2)
    \relates QPointF

    Returns the difference of \a point2 subtracted from \a point1. Each component is subtracted separately.
*/
/*!
    \fn const QPointF operator*(qreal c, const QPointF &point)
    \relates QPointF

    Returns the QPointF formed by multiplying both components of \a point by \a c
*/
/*!
    \fn const QPointF operator*(const QPointF &point, qreal c);
    \relates QPointF

    Returns the QPointF formed by multiplying both components of \a point by \a c
*/
/*!
    \fn const QPointF operator-(const QPointF &point);
    \relates QPointF

    Returns the QPointF formed by changed the sign of both components of \a point. This is equivalent to \code QPoint(0, 0) - p \endcode.
*/
/*!
    \fn const QPointF operator/(const QPointF &point, qreal c);
    \relates QPointF

    Returns the QPointF formed by dividing both components of \a point by \a c.
*/

#ifndef QT_NO_DATASTREAM
/*!
    \relates QPointF

    Writes point \a p to the stream \a s and returns a reference to
    the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QPointF &p)
{
    s << p.x() << p.y();
    return s;
}

/*!
    \relates QPointF

    Reads a QPoint from the stream \a s into point \a p and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QPointF &p)
{
    double x, y;
    s >> x;
    s >> y;
    p.setX(x);
    p.setY(y);
    return s;
}
#endif // QT_NO_DATASTREAM
