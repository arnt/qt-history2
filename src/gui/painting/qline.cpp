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

#include "qline.h"

#include <math.h>

/*!
    \class QLineF

    \brief The QLineF class provides a two-dimensional vector that
    uses floating point coordinates for accuracy.

    A QLineF describes a finite length line on a two-dimensional surface.
    The start and end points of the line are specified using floating point
    coordinates for accuracy.

    Convenience functions are provided for finding the lines's length(),
    the unitVector() along the line, and whether two lines intersect().
    The line can be translated along the length of another line with the
    moveBy() function.

    \sa QPointF QSizeF QRectF
*/

/*!
    \enum QLineF::IntersectType

    \value NoIntersection Indicates that the lines does not intersect,
    e.g. they are parallel.

    \value UnboundedIntersection Indicates that the two lines intersected,
    but not within the range of lines. This will be the case if lines
    are not parallel.

    \img qlinefloat-unbounded.png

    \value BoundedIntersection Indicates that the two lines
    intersected with within the the start and end points of each
    line. This means that non-parallel lines will only intersect if
    the intersection occurs between the start and end points of each
    line.

    \img qlinefloat-bounded.png

*/

/*!
    \fn QLineF::QLineF()

    Constructs a null line.
*/

/*!
    \fn QLineF::QLineF(const QPointF &pt1, const QPointF &pt2)

    Constructs a line object that represents the line between \a pt1 and
    \a pt2.
*/

/*!
    \fn QLineF::QLineF(float x1, float y1, float x2, float y2)

    Constructs a line object that represents the line between (\a x1, \a y1) and
    (\a x2, \a y2).
*/

/*!
    \fn bool QLineF::isNull() const

    Returns true if the line is not set up with valid start and end point;
    otherwise returns false.
*/

/*!
    \fn QPointF QLineF::start() const

    Returns the line's start point.
*/

/*!
    \fn QPointF QLineF::end() const

    Returns the line's end point.
*/

/*!
    \fn float QLineF::startX() const

    Returns the x-coordinate of the line's start point.
*/

/*!
    \fn float QLineF::startY() const

    Returns the y-coordinate of the line's start point.
*/

/*!
    \fn float QLineF::endX() const

    Returns the x-coordinate of the line's end point.
*/

/*!
    \fn float QLineF::endY() const

    Returns the y-coordinate of the line's end point.
*/

/*!
    \fn float QLineF::vx() const

    Returns the horizontal component of the line's vector.
*/

/*!
    \fn float QLineF::vy() const

    Returns the vertical component of the line's vector.
*/

/*!
    \fn QLineF::setLength(float len)
*/

/*!
    \fn QLineF QLineF::normalVector() const

    Returns a line that is perpendicular to this line with the same starting
    point and length.

    \img qlinefloat-normalvector.png
*/

/*!
    \fn void QLineF::operator+=(const QPointF &other)

    Performs a vector addition of this line with the \a other line given.
*/


/*!
    Returns the length of the line.
*/
float QLineF::length() const
{
    float x = p2.x() - p1.x();
    float y = p2.y() - p1.y();
    return sqrt(x*x + y*y);
}

/*!
    Returns a normalized version of this line, starting at the same
    point as this line. A normalized line is a line of unit length
    (length() is equal to 1.0).
*/
QLineF QLineF::unitVector() const
{
    float x = p2.x() - p1.x();
    float y = p2.y() - p1.y();

    float len = sqrt(x*x + y*y);
    QLineF f(start(), QPointF(p1.x() + x/len, p1.y() + y/len));
    Q_ASSERT(QABS(f.length() - 1) < 0.001);
    return f;
}

#define SAME_SIGNS(a, b) ((a) * (b) >= 0)

// Line intersection algorithm, copied from Graphics Gems II
static bool qt_linef_intersect(float x1, float y1, float x2, float y2,
                               float x3, float y3, float x4, float y4)
{
    float a1, a2, b1, b2, c1, c2; /* Coefficients of line eqns. */
    float r1, r2, r3, r4;         /* 'Sign' values */

    a1 = y2 - y1;
    b1 = x1 - x2;
    c1 = x2 * y1 - x1 * y2;

    r3 = a1 * x3 + b1 * y3 + c1;
    r4 = a1 * x4 + b1 * y4 + c1;

    if ( r3 != 0 && r4 != 0 && SAME_SIGNS( r3, r4 ))
        return false;

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = x4 * y3 - x3 * y4;

    r1 = a2 * x1 + b2 * y1 + c2;
    r2 = a2 * x2 + b2 * y2 + c2;

    if ( r1 != 0 && r2 != 0 && SAME_SIGNS( r1, r2 ))
        return false;

    return true;
}

/*!
    Returns wheter this line intersects the line \a l or not. By
    passing in a valid object to \a intersectionPoint, it is possible
    to get the actual intersection point. The intersection point is
    undefined if the lines are parallel.
*/
QLineF::IntersectType QLineF::intersect(const QLineF &l, QPointF *intersectionPoint) const
{
    Q_ASSERT(!isNull());
    Q_ASSERT(!l.isNull());

    // Parallell lines
    if (vx() == l.vx() && vy() == l.vy()) {
        return NoIntersection;
    }

    QPointF isect;
    IntersectType type;

    // For special case where one of the lines are vertical
    if (vx() == 0) {
        float la = l.vy() / l.vx();
        isect = QPointF(p1.x(), la * p1.x() + l.startY() - la*l.startX());
        type = qt_linef_intersect(p1.x(), p1.y(), p2.x(), p2.y(),
                                  l.startX(), l.startY(), l.endX(), l.endY())
               ? BoundedIntersection : UnboundedIntersection;
    } else if (l.vx() == 0) {
        float ta = vy() / vx();
        isect = QPointF(l.startX(), ta * l.startX() + startY() - ta*startX());
        type = qt_linef_intersect(p1.x(), p1.y(), p2.x(), p2.y(),
                                  l.startX(), l.startY(), l.endX(), l.endY())
               ? BoundedIntersection : UnboundedIntersection;
    } else {
        float ta = vy()/vx();
        float la = l.vy()/l.vx();
        float x = ( - l.startY() + la * l.startX() + p1.y() - ta * p1.x() ) / (la - ta);
        isect = QPointF(x, ta*(x - p1.x()) + p1.y());
        type = (x >= p1.x() && x <= p2.x() && x >= l.p1.x() && x <= l.p2.x())
               ? BoundedIntersection : UnboundedIntersection;
    }

    if (intersectionPoint)
        *intersectionPoint = isect;
    return type;
}


    /*!
    \fn void QLineF::moveBy(const QLineF &l)

    Translates this line by the vector specified by the line \a l.

*/
