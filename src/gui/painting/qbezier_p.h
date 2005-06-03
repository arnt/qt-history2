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

#ifndef QBEZIER_P_H
#define QBEZIER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qpoint.h>
#include <qline.h>

class QPolygonF;

class Q_GUI_EXPORT QBezier
{
public:
    QBezier();
    QBezier(qreal p1x, qreal p1y, qreal p2x, qreal p2y,
            qreal p3x, qreal p3y, qreal p4x, qreal p4y);
    QBezier(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4);

    inline QPointF pointAt(qreal t) const;
    QPolygonF toPolygon() const;

    QPointF pt1() const { return QPointF(x1, y1); }
    QPointF pt2() const { return QPointF(x2, y2); }
    QPointF pt3() const { return QPointF(x3, y3); }
    QPointF pt4() const { return QPointF(x4, y4); }

    inline QPointF midPoint() const;
    inline QLineF midTangent() const;

    void split(QBezier *firstHalf, QBezier *secondHalf) const;
    int shifted(QBezier *curveSegments, int maxSegmets, float offset, float threshold) const;

private:
    qreal x1, y1, x2, y2, x3, y3, x4, y4;
};

inline QPointF QBezier::midPoint() const
{
    return QPointF((x1 + x4 + 3*(x2 + x3))/8., (y1 + y4 + 3*(y2 + y3))/8.);
}

inline QLineF QBezier::midTangent() const
{
    QPointF mid = midPoint();
    QLineF dir(QLineF(x1, y1, x2, y2).pointAt(0.5), QLineF(x3, y3, x4, y4).pointAt(0.5));
    return QLineF(mid.x() - dir.dx(), mid.y() - dir.dy(),
                  mid.x() + dir.dx(), mid.y() + dir.dy());
}

inline QPointF QBezier::pointAt(qreal t) const
{
    Q_ASSERT(t >= 0);
    Q_ASSERT(t <= 1);
    qreal a, b, c, d;
    qreal m_t = 1. - t;
    b = m_t * m_t;
    c = t * t;
    d = c * t;
    a = b * m_t;
    b *= 3. * t;
    c *= 3. * m_t;

    return QPointF(a*x1 + b*x2 + c*x3 + d*x4, a*y1 + b*y2 + c*y3 + d*y4);
}

#endif // QBEZIER_P_H
