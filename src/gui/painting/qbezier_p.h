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

class QPolygonF;

class Q_GUI_EXPORT QBezier
{
public:
    QBezier(qreal p1x, qreal p1y, qreal p2x, qreal p2y,
                 qreal p3x, qreal p3y, qreal p4x, qreal p4y);
    QBezier(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4);

#ifdef QT_USE_FIXED_POINT
    inline QPointF pointAt(QFixedPointLong t) const;
#else
    inline QPointF pointAt(qreal t) const;
#endif
    QPolygonF toPolygon() const;

private:
    void init();

    qreal x1, y1, x2, y2, x3, y3, x4, y4;
    qreal ax, bx, cx, dx, ay, by, cy, dy;
};

#ifdef QT_USE_FIXED_POINT
inline QPointF QBezier::pointAt(QFixedPointLong _t) const
{
    QFixedPointLong _ax = ax;
    QFixedPointLong _ay = ay;
    QFixedPointLong _bx = bx;
    QFixedPointLong _by = by;
    QFixedPointLong _cx = cx;
    QFixedPointLong _cy = cy;
    return QPointF((_ax*_t*_t*_t + _bx*_t*_t + _cx*_t).toFixed() + dx,
                   (_ay*_t*_t*_t + _by*_t*_t + _cy*_t).toFixed() + dy);
}
#else
inline QPointF QBezier::pointAt(qreal t) const
{
    Q_ASSERT(t >= 0);
    Q_ASSERT(t <= 1);
    return QPointF(ax*t*t*t + bx*t*t + cx*t + dx,
                   ay*t*t*t + by*t*t + cy*t + dy);
}
#endif

#endif // QBEZIER_P_H
