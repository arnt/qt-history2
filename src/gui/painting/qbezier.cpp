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

#include "qbezier_p.h"
#include <qdebug.h>
#include <qline.h>
#include <qpolygon.h>

#include <private/qnumeric_p.h>

/*!
  \internal
*/
QBezier QBezier::fromPoints(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4)
{
    QBezier b;
    b.x1 = p1.x();
    b.y1 = p1.y();
    b.x2 = p2.x();
    b.y2 = p2.y();
    b.x3 = p3.x();
    b.y3 = p3.y();
    b.x4 = p4.x();
    b.y4 = p4.y();
    return b;
}

/*!
  \internal
*/
QPolygonF QBezier::toPolygon() const
{
    // flattening is done by splitting the bezier until we can replace the segment by a straight
    // line. We split further until the control points are close enough to the line connecting the
    // boundary points.
    //
    // the Distance of a point p from a line given by the points (a,b) is given by:
    //
    // d = abs( (bx - ax)(ay - py) - (by - ay)(ax - px) ) / line_length
    //
    // We can stop splitting if both control points are close enough to the line.
    // To make the algorithm faster we use the manhattan length of the line.

    QPolygonF polygon;
    polygon.append(QPointF(x1, y1));
    QBezier beziers[32];
    beziers[0] = *this;
    QBezier *b = beziers;
    while (b >= beziers) {
        // check if we can pop the top bezier curve from the stack
        qreal l = qAbs(b->x4 - b->x1) + qAbs(b->y4 - b->y1);
        qreal d;
        if (l > 1.) {
            d = qAbs( (b->x4 - b->x1)*(b->y1 - b->y2) - (b->y4 - b->y1)*(b->x1 - b->x2) )
                + qAbs( (b->x4 - b->x1)*(b->y1 - b->y3) - (b->y4 - b->y1)*(b->x1 - b->x3) );
            d /= l;
        } else {
            d = qAbs(b->x1 - b->x2) + qAbs(b->y1 - b->y2) +
                qAbs(b->x1 - b->x3) + qAbs(b->y1 - b->y3);
        }
        if (d < 1. || b == beziers + 31) {
            // good enough, we pop it off and add the endpoint
            polygon.append(QPointF(b->x4, b->y4));
            --b;
        } else {
            // split, second half of the polygon goes lower into the stack
            b->split(b+1, b);
            ++b;
        }
    }
    return polygon;
}

void QBezier::split(QBezier *firstHalf, QBezier *secondHalf) const
{
    Q_ASSERT(firstHalf);
    Q_ASSERT(secondHalf);

    qreal c = (x2 + x3)/2;
    firstHalf->x2 = (x1 + x2)/2;
    secondHalf->x3 = (x3 + x4)/2;
    firstHalf->x1 = x1;
    secondHalf->x4 = x4;
    firstHalf->x3 = (firstHalf->x2 + c)/2;
    secondHalf->x2 = (secondHalf->x3 + c)/2;
    firstHalf->x4 = secondHalf->x1 = (firstHalf->x3 + secondHalf->x2)/2;

    c = (y2 + y3)/2;
    firstHalf->y2 = (y1 + y2)/2;
    secondHalf->y3 = (y3 + y4)/2;
    firstHalf->y1 = y1;
    secondHalf->y4 = y4;
    firstHalf->y3 = (firstHalf->y2 + c)/2;
    secondHalf->y2 = (secondHalf->y3 + c)/2;
    firstHalf->y4 = secondHalf->y1 = (firstHalf->y3 + secondHalf->y2)/2;
}

int QBezier::shifted(QBezier *curveSegments, int maxSegments, float offset, float threshold) const
{
    Q_ASSERT(curveSegments);
    Q_ASSERT(maxSegments > 0);

    // We offset the control vectors to have a basis for the offset.
    QLineF l1(x1, y1, x2, y2);
    if (l1.isNull()) l1 = QLineF(x1, y1, x3, y3);
    if (l1.isNull()) l1 = QLineF(x1, y1, x4, y4);
    if (l1.isNull()) return 0;

    QLineF l2(x3, y3, x4, y4);
    if (l2.isNull()) l2 = QLineF(x2, y2, x4, y4);
    if (l2.isNull()) l2 = QLineF(x1, y1, x4, y4);
    if (l2.isNull()) return 0;

    QLineF l1n = l1.normalVector().unitVector();
    l1.translate(l1n.dx() * offset, l1n.dy() * offset);

    QLineF l2n = l2.normalVector().unitVector();
    l2.translate(l2n.dx() * offset, l2n.dy() * offset);

    QBezier shifted = QBezier::fromPoints(l1.p1(), l1.p2(), l2.p1(), l2.p2());

    // Locate the center off the offsetted curve.
    QPointF offsetCenter = shifted.midPoint();

    // Calculate where the center of the offsetted curve should be
    QPointF center = midPoint();
    QLineF centerTangent = midTangent();
    QLineF ctn = centerTangent.normalVector().unitVector();
    center += QPointF(ctn.dx() * offset, ctn.dy() * offset);

    // Recurse if the distance between actual and expected is greater than threshold
    if (QLineF(center, offsetCenter).length() > qAbs(threshold) && maxSegments > 1) {
        QBezier firstHalf, secondHalf;
        split(&firstHalf, &secondHalf);
        int num = firstHalf.shifted(curveSegments, maxSegments / 2, offset, threshold);
        num += secondHalf.shifted(curveSegments + num, maxSegments - num, offset, threshold);
        return num;
    } else {
        *curveSegments = shifted;
        return 1;
    }
}
