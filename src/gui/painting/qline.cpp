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
#include "qdebug.h"
#include <private/qnumeric_p.h>

#include <math.h>

#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676655900576
#endif

/*!
    \class QLineF

    \brief The QLineF class provides a two-dimensional vector that
    uses floating point coordinates for accuracy.

    A QLineF describes a finite length line on a two-dimensional surface.
    The start and end points of the line are specified using floating point
    coordinates for accuracy.

    Use isNull() to determine whether the QLineF represents a valid line
    or a null line.

    The positions of the line's end points can be found with the startX(),
    startY(), endX(), and endY() functions. The horizontal and vertical
    components of the line are returned by the vx() and vy() functions.

    Convenience functions are provided for finding the lines's length(),
    the unitVector() along the line, whether two lines intersect(), and
    the angle() between two lines. The line's length can be changed using
    setLength().

    Vector addition of two lines is supported through the use of the
    \l{operator+=()}{+= operator}.

    The line can be translated along the length of another line with the
    moveBy() function, and can be traversed using a parameter with the
    pointAt() function.

    \sa QPointF QSizeF QRectF
*/

/*!
    \enum QLineF::IntersectType

    \value NoIntersection Indicates that the lines do not intersect;
    i.e. they are parallel.

    \value UnboundedIntersection The two lines intersect,
    but not within the range defined by their lengths. This will be
    the case if the lines are not parallel.

    \img qlinef-unbounded.png

    \value BoundedIntersection The two lines intersect with each other
    within the start and end points of each line.

    \img qlinef-bounded.png

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
    \fn QLineF::QLineF(qReal x1, qReal y1, qReal x2, qReal y2)

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
    \fn qReal QLineF::startX() const

    Returns the x-coordinate of the line's start point.
*/

/*!
    \fn qReal QLineF::startY() const

    Returns the y-coordinate of the line's start point.
*/

/*!
    \fn qReal QLineF::endX() const

    Returns the x-coordinate of the line's end point.
*/

/*!
    \fn qReal QLineF::endY() const

    Returns the y-coordinate of the line's end point.
*/

/*!
    \fn qReal QLineF::vx() const

    Returns the horizontal component of the line's vector.
*/

/*!
    \fn qReal QLineF::vy() const

    Returns the vertical component of the line's vector.
*/

/*!
    \fn QLineF::setLength(qReal length)

    Sets the \a length of the line.

    \sa length()
*/

/*!
    \fn QLineF QLineF::normalVector() const

    Returns a line that is perpendicular to this line with the same starting
    point and length.

    \img qlinef-normalvector.png
*/

/*!
    \fn void QLineF::operator+=(const QPointF &point)

    Translates this line with the \a point given.
*/

/*!
    \fn bool QLineF::operator==(const QLineF &other) const

    Returns true if \a other is the same line as this line.

    A line is identical if the two points are the same and their
    order is the same.
*/

/*!
  \fn qReal QLineF::pointAt(qReal t) const

  Returns the point at the parameterized position \a t, where
  the start and end point are defined to be at positions t=0 and t=1.
*/

/*!
    Returns the length of the line.

    \sa setLength()
*/
qReal QLineF::length() const
{
    qReal x = p2.x() - p1.x();
    qReal y = p2.y() - p1.y();
    return sqrt(x*x + y*y);
}

/*!
    Returns a normalized version of this line, starting at the same
    point as this line. A normalized line is a line of unit length
    (length() is equal to 1.0).
*/
QLineF QLineF::unitVector() const
{
    qReal x = p2.x() - p1.x();
    qReal y = p2.y() - p1.y();

    qReal len = sqrt(x*x + y*y);
    QLineF f(start(), QPointF(p1.x() + x/len, p1.y() + y/len));
    Q_ASSERT(qAbs(f.length() - 1) < 0.001);
    return f;
}

#define SAME_SIGNS(a, b) ((a) * (b) >= 0)

// Line intersection algorithm, copied from Graphics Gems II
static bool qt_linef_intersect(qReal x1, qReal y1, qReal x2, qReal y2,
                               qReal x3, qReal y3, qReal x4, qReal y4)
{
    qReal a1, a2, b1, b2, c1, c2; /* Coefficients of line eqns. */
    qReal r1, r2, r3, r4;         /* 'Sign' values */

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
    \fn QLineF::IntersectType QLineF::intersect(const QLineF &other, QPointF *intersectionPoint) const

    Returns a value indicating whether or not this line intersects the
    \a other line. By passing a valid object as \a intersectionPoint, it
    is possible to get the actual intersection point. The intersection
    point is undefined if the lines are parallel.
*/

QLineF::IntersectType QLineF::intersect(const QLineF &l, QPointF *intersectionPoint) const
{
    if (isNull() || l.isNull()
        || !qIsFinite(p1.x()) || !qIsFinite(p1.y()) || !qIsFinite(p2.x()) || !qIsFinite(p2.y())
        || !qIsFinite(l.p1.x()) || !qIsFinite(l.p1.y()) || !qIsFinite(l.p2.x()) || !qIsFinite(l.p2.y()))
        return NoIntersection;

    QPointF isect;
    IntersectType type = qt_linef_intersect(p1.x(), p1.y(), p2.x(), p2.y(),
                                            l.startX(), l.startY(), l.endX(), l.endY())
                         ? BoundedIntersection : UnboundedIntersection;

    // For special case where one of the lines are vertical
    if (vx() == 0 && l.vx() == 0) {
        type = NoIntersection;
    } else if (vx() == 0) {
        qReal la = l.vy() / l.vx();
        isect = QPointF(p1.x(), la * p1.x() + l.startY() - la*l.startX());
    } else if (l.vx() == 0) {
        qReal ta = vy() / vx();
        isect = QPointF(l.startX(), ta * l.startX() + startY() - ta*startX());
    } else {
        qReal ta = vy()/vx();
        qReal la = l.vy()/l.vx();
        if (ta == la) // no intersection
            return NoIntersection;

        qReal x = ( - l.startY() + la * l.startX() + p1.y() - ta * p1.x() ) / (la - ta);
        isect = QPointF(x, ta*(x - p1.x()) + p1.y());
    }
    if (intersectionPoint)
        *intersectionPoint = isect;
    return type;
}


/*!
    \fn void QLineF::translate(const QLineF &line)

    Translates this line by the vector specified by the \a line given.
*/

/*!
    \fn void QLineF::translate(const QPointF &point)

    Translates this line with the \a point given.
*/

/*!
  \fn qReal QLineF::angle(const QLineF &line) const

  Returns the smallest angle between the given \a line and this line, not
  taking into account whether the lines intersect or not. The angle is
  specified in degrees.
*/
qReal QLineF::angle(const QLineF &l) const
{
    if (isNull() || l.isNull())
        return 0;
    qReal rad = acos( (vx()*l.vx() + vy()*l.vy()) / (length()*l.length()) );
    return rad * 360 / M_2PI;
}


#ifndef QT_NO_DEBUG_OUTPUT
QDebug operator<<(QDebug d, const QLineF &p)
{
    d << "QLineF(" << p.start() << "," << p.end() << ")";
    return d;
}
#endif
