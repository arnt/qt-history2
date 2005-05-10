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

// Manhattan length between two QPointF's
#define mlen(a, b) (qAbs(a.x() - b.x()) + qAbs(a.y() - b.y()))


QBezier::QBezier()
    : valid(false)
{
}

/*!
  \internal
*/
QBezier::QBezier(qreal p1x_, qreal p1y_, qreal p2x_, qreal p2y_,
                 qreal p3x_, qreal p3y_, qreal p4x_, qreal p4y_)
    : x1(p1x_), y1(p1y_), x2(p2x_), y2(p2y_), x3(p3x_), y3(p3y_), x4(p4x_), y4(p4y_)
{
    init();
}

/*!
  \internal
*/
QBezier::QBezier(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4)
    : x1(p1.x()), y1(p1.y()),
      x2(p2.x()), y2(p2.y()),
      x3(p3.x()), y3(p3.y()),
      x4(p4.x()), y4(p4.y())
{
    init();
}

/*!
  \internal
*/
void QBezier::init()
{
    valid = true;

    ax = -x1 + 3*x2 - 3*x3 + x4;
    bx = 3*x1 - 6*x2 + 3*x3;
    cx = -3*x1 + 3*x2;
    dx = x1;

    ay = -y1 + 3*y2 - 3*y3 + y4;
    by = 3*y1 - 6*y2 + 3*y3;
    cy = -3*y1 + 3*y2;
    dy = y1;

#ifndef QT_NO_DEBUG
    if (qIsNan(x1) || qIsNan(x2) || qIsNan(x3) || qIsNan(x4)
        || qIsNan(y1) || qIsNan(y2) || qIsNan(y3) || qIsNan(y4))
        qWarning("QBezier::init(): one or more of the bezier parameters is nan, results are undefined.");
#endif
}

struct QBezierLineSegment
{
    QBezierLineSegment() { }
    QBezierLineSegment(qreal st, qreal en, const QLineF &line) : t_start(st), t_end(en), l(line) { }
    qreal t_start;
    qreal t_end;
    QLineF l;
};

Q_DECLARE_TYPEINFO(QBezierLineSegment, Q_PRIMITIVE_TYPE); // actually MOVABLE, but we don't care here...

/*!
  \internal
*/
QPolygonF QBezier::toPolygon() const
{
    Q_ASSERT(valid);

    QBezierLineSegment *lines = (QBezierLineSegment *) qMalloc(32 * sizeof(QBezierLineSegment));
    int pos = 0;
    int alloc = 32;
    QPolygonF polygon;
    polygon.reserve(32);

    polygon << QPointF(x1, y1);

    const qreal distance = 0.5;

    QPointF at13 = pointAt(1.f/3.f);
    QPointF at23 = pointAt(2.f/3.f);

    lines[pos++] = QBezierLineSegment(2.f/3.f, 1.f, QLineF(at23, QPointF(x4, y4))); //push
    lines[pos++] = QBezierLineSegment(1.f/3.f, 2.f/3.f, QLineF(at13, at23)); // push
    lines[pos++] = QBezierLineSegment(0.f, 1.f/3.f, QLineF(QPointF(x1, y1), at13)); // push

    while (pos > 0) {
        QBezierLineSegment s = lines[--pos]; // pop
        qreal t_half = (s.t_start + s.t_end) / 2.0f;
        QPointF curvePt = pointAt(t_half);
        QPointF linePt = s.l.pointAt(0.5);
        if (mlen(curvePt, linePt) < distance) {
            polygon.append(s.l.p2());
        } else {
            if (pos >= alloc - 2) {
                alloc *= 2;
                lines = (QBezierLineSegment *) qRealloc(lines, alloc*sizeof(QBezierLineSegment));
            }
            lines[pos++] = QBezierLineSegment(t_half, s.t_end, QLineF(curvePt, s.l.p2())); // push
            lines[pos++] = QBezierLineSegment(s.t_start, t_half, QLineF(s.l.p1(), curvePt)); // push
        }
    }
    qFree(lines);
    return polygon;
}

void QBezier::split(QBezier *firstHalf, QBezier *secondHalf) const
{
    Q_ASSERT(valid);
    Q_ASSERT(firstHalf);
    Q_ASSERT(secondHalf);

    qreal ax8 = ax/8;
    qreal ay8 = ay/8;
    qreal bx4 = bx/4;
    qreal by4 = by/4;
    qreal cx2 = cx/2;
    qreal cy2 = cy/2;

    // Calculate the a, b, c, d values based on f(t/2)
    firstHalf->ax = ax8;
    firstHalf->ay = ay8;
    firstHalf->bx = bx4;
    firstHalf->by = by4;
    firstHalf->cx = cx2;
    firstHalf->cy = cy2;
    firstHalf->dx = dx;
    firstHalf->dy = dy;

    // Get the control points by solving M^-1 * [a b c d], where M is
    // the matrix mapping control points to a, b, c, d, as used in init().
    firstHalf->x1 = firstHalf->dx;
    firstHalf->x2 = firstHalf->cx / 3 + firstHalf->dx;
    firstHalf->x3 = firstHalf->bx / 3 + firstHalf->cx * 2 / 3 + firstHalf->dx;
    firstHalf->x4 = firstHalf->ax + firstHalf->bx + firstHalf->cx + firstHalf->dx;
    firstHalf->y1 = firstHalf->dy;
    firstHalf->y2 = firstHalf->cy / 3 + firstHalf->dy;
    firstHalf->y3 = firstHalf->by / 3 + firstHalf->cy * 2 / 3 + firstHalf->dy;
    firstHalf->y4 = firstHalf->ay + firstHalf->by + firstHalf->cy + firstHalf->dy;

    // Repeat for second half, calculated throught f(1/2 + t/2)
    secondHalf->ax = ax8;
    secondHalf->bx = 3*ax8 + bx4;
    secondHalf->cx = 3*ax8 + 2*bx4 + cx2;
    secondHalf->dx = ax8 + bx4 + cx2 + dx;
    secondHalf->ay = ay8;
    secondHalf->by = 3*ay8 + by4;
    secondHalf->cy = 3*ay8 + 2*by4 + cy2;
    secondHalf->dy = ay8 + by4 + cy2 + dy;

    secondHalf->x1 = secondHalf->dx;
    secondHalf->x2 = secondHalf->cx / 3 + secondHalf->dx;
    secondHalf->x3 = secondHalf->bx / 3 + secondHalf->cx * 2 / 3 + secondHalf->dx;
    secondHalf->x4 = secondHalf->ax + secondHalf->bx + secondHalf->cx + secondHalf->dx;
    secondHalf->y1 = secondHalf->dy;
    secondHalf->y2 = secondHalf->cy / 3 + secondHalf->dy;
    secondHalf->y3 = secondHalf->by / 3 + secondHalf->cy * 2 / 3 + secondHalf->dy;
    secondHalf->y4 = secondHalf->ay + secondHalf->by + secondHalf->cy + secondHalf->dy;

    firstHalf->valid = true;
    secondHalf->valid = true;
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

    QBezier shifted(l1.p1(), l1.p2(), l2.p1(), l2.p2());

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
