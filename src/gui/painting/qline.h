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

#ifndef QLINE_H
#define QLINE_H

#include <qpoint.h>

class Q_GUI_EXPORT QLineF {
public:

    enum IntersectType { NoIntersection, BoundedIntersection, UnboundedIntersection };

    inline QLineF();
    inline QLineF(const QPointF &pt1, const QPointF &pt2);
    inline QLineF(qReal x1, qReal y1, qReal x2, qReal y2);

    inline bool isNull() const;

    inline QPointF start() const;
    inline QPointF end() const;

    inline qReal startX() const;
    inline qReal startY() const;

    inline qReal endX() const;
    inline qReal endY() const;

    inline qReal vx() const;
    inline qReal vy() const;

    qReal length() const;
    void setLength(qReal len);

    QLineF unitVector() const;
    QLineF normalVector() const;

    IntersectType intersect(const QLineF &l, QPointF *intersectionPoint) const;

    qReal angle(const QLineF &l) const;

    QPointF pointAt(qReal t) const;
#ifdef QT_USE_FIXED_POINT
    QPointF pointAt(QFixedPointLong t) const;
#endif
    inline void translate(const QLineF &p);
    inline void translate(const QPointF &p);

    inline void operator+=(const QPointF &d);
    inline void operator-=(const QPointF &d);
    inline bool operator==(const QLineF &d) const;

private:
    QPointF p1, p2;
};
Q_DECLARE_TYPEINFO(QLineF, Q_MOVABLE_TYPE);

inline QLineF::QLineF()
{
}

inline QLineF::QLineF(const QPointF &pt1, const QPointF &pt2)
    : p1(pt1), p2(pt2)
{
}

inline QLineF::QLineF(qReal x1, qReal y1, qReal x2, qReal y2)
    : p1(x1, y1), p2(x2, y2)
{
}

inline bool QLineF::isNull() const
{
    return p1 == p2;
}

inline qReal QLineF::startX() const
{
    return p1.x();
}

inline qReal QLineF::startY() const
{
    return p1.y();
}

inline qReal QLineF::endX() const
{
    return p2.x();
}

inline qReal QLineF::endY() const
{
    return p2.y();
}

inline QPointF QLineF::start() const
{
    return p1;
}

inline QPointF QLineF::end() const
{
    return p2;
}

inline qReal QLineF::vx() const
{
    return p2.x() - p1.x();
}

inline qReal QLineF::vy() const
{
    return p2.y() - p1.y();
}

inline QLineF QLineF::normalVector() const
{
    return QLineF(start(), start() + QPointF(vy(), -vx()));
}

inline void QLineF::translate(const QLineF &l)
{
    QPointF point(l.vx(), l.vy());
    p1 += point;
    p2 += point;
}

inline void QLineF::translate(const QPointF &point)
{
    p1 += point;
    p2 += point;
}

inline void QLineF::setLength(qReal len)
{
    if (isNull())
        return;
    QLineF v = unitVector();
    p2 = QPointF(p1.x() + v.vx() * len, p1.y() + v.vy() * len);
}

inline QPointF QLineF::pointAt(qReal t) const
{
    qReal vx = p2.x() - p1.x();
    qReal vy = p2.y() - p1.y();
    return QPointF(p1.x() + vx * t, p1.y() + vy * t);
}

#ifdef QT_USE_FIXED_POINT
inline QPointF QLineF::pointAt(QFixedPointLong t) const
{
    QFixedPointLong vx = p2.x() - p1.x();
    QFixedPointLong vy = p2.y() - p1.y();
    return QPointF((p1.x() + vx * t).toFixed(), (p1.y() + vy * t).toFixed());
}
#endif

inline void QLineF::operator+=(const QPointF &point)
{
    p1 += point;
    p2 += point;
}

inline void QLineF::operator-=(const QPointF &point)
{
    p1 -= point;
    p2 -= point;
}

inline bool QLineF::operator==(const QLineF &d) const
{
    return p1 == d.p1 && p2 == d.p2;
}

#ifndef QT_NO_DEBUG_OUTPUT
Q_GUI_EXPORT QDebug operator<<(QDebug d, const QLineF &p);
#endif

#endif // QLINE_H
