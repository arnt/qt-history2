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

#include "qpointfloat.h"
#include <qdebug.h>

#ifndef QT_NO_DEBUG
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
    for accuracy. If you only need the accuracy of integers, you may want to
    use QPoint instead of this class.



    Convenience functions are provided for reading and writing the individual
    coordinates used to define the point: x(), y(), setX(), and setY().


    \sa QPointF
*/

/*!
    \fn QPointF::QPointF()

    Constructs a null point.
*/

/*!
    \fn QPointF::QPointF(const QPoint &point)

    Copy constructor. Constructs a point using the values of the \a point
    specified.
*/

/*!
    \fn QPointF::QPointF(float x, float y)

    Constructs a point with coordinates specified by \a x and \a y.
*/

/*!
    \fn bool QPointF::isNull() const

    Returns true if the point is not set up; otherwise returns false.
*/

/*!
    \fn float QPointF::x() const

    Returns the x-coordinate of the point.
*/

/*!
    \fn float QPointF::y() const

    Returns the y-coordinate of the point.
*/

/*!
    \fn void QPointF::setX(float x)

    Sets the x-coordinate of the point to the value specified by \a x.
*/

/*!
    \fn void QPointF::setY(float y)

    Sets the y-coordinate of the point to the value specified by \a y.
*/

/*!
    \fn float QPointF::&rx()

    Returns a reference to the x-coordinate of the point.
*/

/*!
    \fn float QPointF::&ry()

    Returns a reference to the y-coordinate of the point.
*/

/*!
    \fn QPointF QPointF::&operator+=(const QPointF &other)

    Adds the coordinates of this point to the corresponding coordinates of
    the \a other point, and returns a reference to this point with the new
    coordinates. (Vector addition.)

    \sa operator+() operator-=()
*/

/*!
    \fn QPointF QPointF::&operator-=(const QPointF &other)

    Subtracts the coordinates of the \a other point from the
    corresponding coordinates of this point, and returns a reference to this
    point with the new coordinates. (Vector subtraction.)

    \sa operator-() operator+=()
*/

/*!
    \fn QPointF QPointF::&operator*=(float factor)

    Multiplies the coordinates of this point by the given scale \a factor, and
    returns a reference to this point with the new coordinates. (Scalar
    multiplication of a vector.)

    \sa operator*() operator/=()
*/

/*!
    \fn QPointF QPointF::&operator/=(float factor)

    Divides the coordinates of this point by the given scale \a factor, and
    returns a references to this point with the new coordinates. (Scalar
    division of a vector.)

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
    \fn const QPointF operator*(const QPointF &point, int factor)

    \overload

    Multiplies the coordinates of the \a point by the given scale \a factor, and
    returns a point with the new coordinates. (Scalar multiplication of a
    vector.)

    \sa operator/()
*/

/*!
    \fn const QPointF operator*(float factor, const QPointF &point)

    \overload

    Multiplies the coordinates of the \a point by the given scale \a factor, and
    returns a point with the new coordinates. (Scalar multiplication of a
    vector.)

    \sa operator/()
*/

/*!
    \fn const QPointF operator*(const QPointF &point, float factor)

    Multiplies the coordinates of the \a point by the given scale \a factor, and
    returns a point with the new coordinates. (Scalar multiplication of a vector.)

    \sa operator/()
*/

/*!
    \fn const QPointF operator-(const QPointF &point)

    Negates the coordinates of the \a point, and returns a point with the
    new coordinates. (Inversion).
*/

/*!
    \fn const QPointF operator/(const QPointF &point, float factor)

    Divides the coordinates of the \a point by the given scale \a factor, and
    returns a point with the new coordinates. (Scalar division of a vector.)

    \sa operator*()
*/

/*!
    \fn QPoint QPointF::toPoint() const

    Converts the coordinates of this point to integers and returns a QPoint
    with these coordinates.

    \sa QPoint
*/
