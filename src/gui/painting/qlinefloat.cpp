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

    \brief The class QLineFloat describes a line using floating points.

    It provides convenience functions for working with lines, such as
    intersections, normal vectors and unit vectors.
*/

/*!
    \enum QLineFloat::IntersectMode

    \value Unbounded This mode does intersection without being bounded
    by the start and end points of the line. This means that
    non-parallel lines will always have an intersection.

    \value Bounded This mode does intersection and takes the start
    and end points into account.
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
    Returnes a normalized version of this line, starting in the same
    point as this line. A normalized line is a line of length = 1.
*/
QLineFloat QLineFloat::unitVector() const
{
    float x = p2.x() - p1.x();
    float y = p2.y() - p1.y();
    float len = sqrt(x*x + y*y);
    return QLineFloat(start(), QPointFloat(x/len, y/len));
}

/*!
    Returns the intersection point between the this line and the line
    \a l. The mode \a mode specifies if the intersection is bounded or
    not and \a intersected is set to true if the lines did intersect.
    The return value is undefined if the lines do not intersect.
*/
QPointFloat QLineFloat::intersect(const QLineFloat &l, IntersectMode mode, bool *intersected) const
{
    // Parallell lines
    if (vx() == l.vx() && vy() == l.vy()) {
        if (intersected)
            *intersected = false;
        return QPointFloat();
    }

    // For special case where one of the lines are vertical
    if (vx() == 0)
        return l.intersect(*this, mode, intersected);

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

    Translates this line along the line \a l according
    to the distance of \l.
*/
