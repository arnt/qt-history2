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

#include "qlinefloat.h"

#include <math.h>

/*!
    \class QLineFloat

    \brief The QLineFloat class provides a two-dimensional vector that
    uses floating point coordinates for accuracy.

    A QLineFloat describes a finite length line on a two-dimensional surface.
    The start and end points of the line are specified using floating point
    coordinates for accuracy.

    Convenience functions are provided for finding the lines's length(),
    the unitVector() along the line, and whether two lines intersect().
    The line can be translated along the length of another line with the
    moveBy() function.

    \sa QPointFloat
*/

/*!
    \enum QLineFloat::IntersectMode

    \value Unbounded In this mode, checks for intersection ignore the start and
                     end points of each line. This means that non-parallel
                     lines will always have an intersection.

    \img qlinefloat-unbounded.png

    \value Bounded   In this mode, checks for intersection take into account
                     the start and end points of each line. This means that
                     non-parallel lines will only intersect if the intersection
                     occurs between the start and end points of each line.

    \img qlinefloat-bounded.png

*/

/*!
    Returns the length of the line.
*/
float QLineFloat::length() const
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
QLineFloat QLineFloat::unitVector() const
{
    float x = p2.x() - p1.x();
    float y = p2.y() - p1.y();

    float len = sqrt(x*x + y*y);
    QLineFloat f(start(), QPointFloat(p1.x() + x/len, p1.y() + y/len));
    Q_ASSERT(QABS(f.length() - 1) < 0.001);
    return f;
}

/*!
    Returns the intersection point between the this line and the line
    \a l. The mode \a mode specifies if the intersection is bounded or
    not and \a intersected is set to true if the lines did intersect.
    The return value is undefined if the lines do not intersect.
*/
QPointFloat QLineFloat::intersect(const QLineFloat &l, IntersectMode mode, bool *intersected) const
{
    Q_ASSERT(!isNull());
    Q_ASSERT(!l.isNull());
    // Parallell lines
    if (vx() == l.vx() && vy() == l.vy()) {
        if (intersected)
            *intersected = false;
        return QPointFloat();
    }

    // For special case where one of the lines are vertical
    if (vx() == 0) {
        float la = l.vy() / l.vx();
        QPointFloat isect(p1.x(), la * p1.x() + l.startY() - la*l.startX());
        return isect;
    } else if (l.vx() == 0) {
        float ta = vy() / vx();
        QPointFloat isect(l.startX(), ta * l.startX() + startY() - ta*startX());
        return isect;
    }

    float ta = vy()/vx();
    float la = l.vy()/l.vx();

    float x = ( - l.startY() + la * l.startX() + p1.y() - ta * p1.x() ) / (la - ta);

    QPointFloat isect = QPointFloat(x, ta*(x - p1.x()) + p1.y());

    if (mode == Unbounded) {
        if (intersected)
            *intersected = true;
    } else {
        if (intersected)
            *intersected = (x >= p1.x() && x <= p2.x() && x >= l.p1.x() && x <= l.p2.x());
    }
    return isect;
}


/*!
    \fn void QLineFloat::moveBy(const QLineFloat &l)

    Translates this line by the vector specified by the line \a l.

*/
