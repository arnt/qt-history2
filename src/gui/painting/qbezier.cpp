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
#include <qpolygon.h>
#include <qline.h>

// Manhattan length between two QPointF's
#define mlen(a, b) (qAbs(a.x() - b.x()) + qAbs(a.y() - b.y()))

/*!
  \internal
*/
QBezier::QBezier(float p1x_, float p1y_, float p2x_, float p2y_,
                 float p3x_, float p3y_, float p4x_, float p4y_)
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
    ax = -x1 + 3*x2 - 3*x3 + x4;
    bx = 3*x1 - 6*x2 + 3*x3;
    cx = -3*x1 + 3*x2;
    dx = x1;

    ay = -y1 + 3*y2 - 3*y3 + y4;
    by = 3*y1 - 6*y2 + 3*y3;
    cy = -3*y1 + 3*y2;
    dy = y1;
}

struct QBezierLineSegment
{
    QBezierLineSegment() { }
    QBezierLineSegment(float st, float en, const QLineF &line) : t_start(st), t_end(en), l(line) { }
    float t_start;
    float t_end;
    QLineF l;
};

Q_DECLARE_TYPEINFO(QBezierLineSegment, Q_PRIMITIVE_TYPE); // actually MOVABLE, but we don't care here...

/*!
  \internal
*/
QPolygonF QBezier::toPolygon() const
{
    QBezierLineSegment *lines = (QBezierLineSegment *) qMalloc(32 * sizeof(QBezierLineSegment));
    int pos = 0;
    int alloc = 32;
    QPolygonF polygon;
    polygon.reserve(32);

    polygon << QPointF(x1, y1);

    const float distance = 0.5;

    QPointF at13 = pointAt(1.f/3.f);
    QPointF at23 = pointAt(2.f/3.f);

    lines[pos++] = QBezierLineSegment(2.f/3.f, 1.f, QLineF(at23, QPointF(x4, y4))); //push
    lines[pos++] = QBezierLineSegment(1.f/3.f, 2.f/3.f, QLineF(at13, at23)); // push
    lines[pos++] = QBezierLineSegment(0.f, 1.f/3.f, QLineF(QPointF(x1, y1), at13)); // push

    while (pos > 0) {
        QBezierLineSegment s = lines[--pos]; // pop
        float t_half = (s.t_start + s.t_end) / 2.0f;
        QPointF curvePt = pointAt(t_half);
        QPointF linePt = s.l.pointAt(1.f / 2.0f);
        if (mlen(curvePt, linePt) < distance) {
            polygon.append(s.l.end());
        } else {
            if (pos >= alloc - 2) {
                alloc *= 2;
                lines = (QBezierLineSegment *) qRealloc(lines, alloc*sizeof(QBezierLineSegment));
            }
            lines[pos++] = QBezierLineSegment(t_half, s.t_end, QLineF(curvePt, s.l.end())); // push
            lines[pos++] = QBezierLineSegment(s.t_start, t_half, QLineF(s.l.start(), curvePt)); // push
        }
    }
    qFree(lines);
    return polygon;
}
