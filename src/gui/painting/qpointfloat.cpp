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
QDebug operator<<(QDebug d, const QPointFloat &p)
{
    d << "QPointFloat(" << p.x() << ", " << p.y() << ")";
    return d;
}
#endif

/*!
    \class QPointFloat
    \brief The QPointFloat class provides a point object that uses floating
    point coordinates for accuracy.

    A QPointFloat describes a point on a two-dimensional surface. The
    coordinates of the point are specified using floating point numbers
    for accuracy. If you only need the accuracy of integers, you may want to
    use QPoint instead of this class.



    Convenience functions are provided for reading and writing the individual
    coordinates used to define the point: x(), y(), setX(), and setY().


    \sa QPointFloat
*/

/*!
    \fn QPointFloat::QPointFloat()

    Constructs a null point.
*/

/*!
    \fn QPointFloat::QPointFloat(const QPoint &point)

    Copy constructor. Constructs a point using the values of the \a point
    specified.
*/

/*!
    \fn QPointFloat::QPointFloat(float x, float y)

    Constructs a point with coordinates specified by \a x and \a y.
*/

/*!
    \fn bool QPointFloat::isNull() const

    Returns true if the point is not set up; otherwise returns false.
*/

/*!
    \fn float QPointFloat::x() const

    Returns the x-coordinate of the point.
*/

/*!
    \fn float QPointFloat::y() const

    Returns the y-coordinate of the point.
*/

/*!
    \fn void QPointFloat::setX(float x)

    Sets the x-coordinate of the point to the value specified by \a x.
*/

/*!
    \fn void QPointFloat::setY(float y)

    Sets the y-coordinate of the point to the value specified by \a y.
*/

/*!
    \fn float QPointFloat::&rx()

    Returns a reference to the x-coordinate of the point.
*/

/*!
    \fn float QPointFloat::&ry()

    Returns a reference to the y-coordinate of the point.
*/

/*!
    \fn QPointFloat QPointFloat::&operator+=(const QPointFloat &other)

    Adds the coordinates of this point to the corresponding coordinates of
    the \a other point, and returns a reference to this point with the new
    coordinates. (Vector addition.)

    \sa operator+() operator-=()
*/

/*!
    \fn QPointFloat QPointFloat::&operator-=(const QPointFloat &other)

    Subtracts the coordinates of the \a other point from the
    corresponding coordinates of this point, and returns a reference to this
    point with the new coordinates. (Vector subtraction.)

    \sa operator-() operator+=()
*/

/*!
    \fn QPointFloat QPointFloat::&operator*=(float factor)

    Multiplies the coordinates of this point by the given scale \a factor, and
    returns a reference to this point with the new coordinates. (Scalar
    multiplication of a vector.)

    \sa operator*() operator/=()
*/

/*!
    \fn QPointFloat QPointFloat::&operator/=(float factor)

    Divides the coordinates of this point by the given scale \a factor, and
    returns a references to this point with the new coordinates. (Scalar
    division of a vector.)

    \sa operator*()
*/

/*!
    \fn bool QPointFloat::operator==(const QPointFloat &, const QPointFloat &)

    Returns true if the \a point is the same as the \a other point given.

    \sa operator!=()
*/

/*!
    \fn bool QPointFloat::operator!=(const QPointFloat &point, const QPointFloat &other)

    Returns true if the \a point is not the same as the \a other point given.

    \sa operator==()
*/

/*!
    \fn const QPointFloat QPointFloat::operator+(const QPointFloat &point, const QPointFloat &other)

    Adds the coordinates of the \a point to the corresponding coordinates of
    the \a other point, and returns a point with the new coordinates. (Vector
    addition.)

    \sa operator-()
*/

/*!
    \fn const QPointFloat QPointFloat::operator-(const QPointFloat &point, const QPointFloat &other)

    Subtracts the coordinates of the \a other point from the
    corresponding coordinates of the given \a point, and returns a point with
    the new coordinates. (Vector subtraction.)

    \sa operator+()
*/

/*!
    \fn const QPointFloat QPointFloat::operator*(const QPointFloat &point, int factor)

    \overload

    Multiplies the coordinates of the \a point by the given scale \a factor, and
    returns a point with the new coordinates. (Scalar multiplication of a
    vector.)

    \sa operator/()
*/

/*!
    \fn const QPointFloat QPointFloat::operator*(float factor, const QPointFloat &point)

    \overload

    Multiplies the coordinates of the \a point by the given scale \a factor, and
    returns a point with the new coordinates. (Scalar multiplication of a
    vector.)

    \sa operator/()
*/

/*!
    \fn const QPointFloat QPointFloat::operator*(const QPointFloat &point, float factor)

    Multiplies the coordinates of the \a point by the given scale \a factor, and
    returns a point with the new coordinates. (Scalar multiplication of a vector.)

    \sa operator/()
*/

/*!
    \fn const QPointFloat QPointFloat::operator-(const QPointFloat &point)

    Negates the coordinates of the \a point, and returns a point with the
    new coordinates. (Inversion).
*/

/*!
    \fn const QPointFloat QPointFloat::operator/(const QPointFloat &point, float factor)

    Divides the coordinates of the \a point by the given scale \a factor, and
    returns a point with the new coordinates. (Scalar division of a vector.)

    \sa operator*()
*/

/*!
    \fn QPoint QPointFloat::toPoint() const

    Converts the coordinates of this point to integers and returns a QPoint
    with these coordinates.

    \sa QPoint
*/
