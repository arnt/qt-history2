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
    QBezier(float p1x, float p1y, float p2x, float p2y,
                 float p3x, float p3y, float p4x, float p4y);
    QBezier(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4);

    inline QPointF pointAt(float t) const;

    QPolygonF toPolygon() const;

private:
    void init();

    float x1, y1, x2, y2, x3, y3, x4, y4;
    float ax, bx, cx, dx, ay, by, cy, dy;
};

inline QPointF QBezier::pointAt(float t) const
{
    Q_ASSERT(t >= 0);
    Q_ASSERT(t <= 1);
    return QPointF(ax*t*t*t + bx*t*t + cx*t + dx,
                   ay*t*t*t + by*t*t + cy*t + dy);
}

#endif // QBEZIER_P_H
