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

#include <QtCore/qpoint.h>

class Q_GUI_EXPORT QLineF {
public:

    enum IntersectType { NoIntersection, BoundedIntersection, UnboundedIntersection };

    inline QLineF();
    inline QLineF(const QPointF &pt1, const QPointF &pt2);
    inline QLineF(qreal x1, qreal y1, qreal x2, qreal y2);

    inline bool isNull() const;

    inline QPointF start() const;
    inline QPointF end() const;

    inline qreal startX() const;
    inline qreal startY() const;

    inline qreal endX() const;
    inline qreal endY() const;

    inline qreal vx() const;
    inline qreal vy() const;

    qreal length() const;
    void setLength(qreal len);

    QLineF unitVector() const;
    QLineF normalVector() const;

    IntersectType intersect(const QLineF &l, QPointF *intersectionPoint) const;

    qreal angle(const QLineF &l) const;

    QPointF pointAt(qreal t) const;
#ifdef QT_USE_FIXED_POINT
    QPointF pointAt(QFixedPointLong t) const;
#endif
    inline void translate(const QLineF &p);
    inline void translate(const QPointF &p);
    inline void translate(qreal dx, qreal dy);

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

inline QLineF::QLineF(qreal x1, qreal y1, qreal x2, qreal y2)
    : p1(x1, y1), p2(x2, y2)
{
}

inline bool QLineF::isNull() const
{
    return p1 == p2;
}

inline qreal QLineF::startX() const
{
    return p1.x();
}

inline qreal QLineF::startY() const
{
    return p1.y();
}

inline qreal QLineF::endX() const
{
    return p2.x();
}

inline qreal QLineF::endY() const
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

inline qreal QLineF::vx() const
{
    return p2.x() - p1.x();
}

inline qreal QLineF::vy() const
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

inline void QLineF::translate(qreal dx, qreal dy)
{
    this->translate(QPointF(dx, dy));
}

inline void QLineF::setLength(qreal len)
{
    if (isNull())
        return;
    QLineF v = unitVector();
    p2 = QPointF(p1.x() + v.vx() * len, p1.y() + v.vy() * len);
}

inline QPointF QLineF::pointAt(qreal t) const
{
    qreal vx = p2.x() - p1.x();
    qreal vy = p2.y() - p1.y();
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
