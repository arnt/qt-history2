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
    inline QLineF(float x1, float y1, float x2, float y2);

    inline bool isNull() const;

    inline QPointF start() const;
    inline QPointF end() const;

    inline float startX() const;
    inline float startY() const;

    inline float endX() const;
    inline float endY() const;

    inline float vx() const;
    inline float vy() const;

    float length() const;
    void setLength(float len);

    QLineF unitVector() const;
    QLineF normalVector() const;

    IntersectType intersect(const QLineF &l, QPointF *intersectionPoint) const;

    float angle(const QLineF &l) const;

    QPointF pointAt(float t) const;

    inline void moveBy(const QLineF &p);

    inline void operator+=(const QPointF &d);
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

inline QLineF::QLineF(float x1, float y1, float x2, float y2)
    : p1(x1, y1), p2(x2, y2)
{
}

inline bool QLineF::isNull() const
{
    return p1 == p2;
}

inline float QLineF::startX() const
{
    return p1.x();
}

inline float QLineF::startY() const
{
    return p1.y();
}

inline float QLineF::endX() const
{
    return p2.x();
}

inline float QLineF::endY() const
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

inline float QLineF::vx() const
{
    return p2.x() - p1.x();
}

inline float QLineF::vy() const
{
    return p2.y() - p1.y();
}

inline QLineF QLineF::normalVector() const
{
    return QLineF(start(), start() + QPointF(vy(), -vx()));
}

inline void QLineF::moveBy(const QLineF &l)
{
    QPointF pf(l.vx(), l.vy());
    *this += pf;
}

inline void QLineF::setLength(float len)
{
    if (isNull())
        return;
    QLineF v = unitVector();
    p2 = QPointF(p1.x() + v.vx() * len, p1.y() + v.vy() * len);
}

inline QPointF QLineF::pointAt(float t) const
{
    float vx = p2.x() - p1.x();
    float vy = p2.y() - p1.y();
    return QPointF(p1.x() + vx * t, p1.y() + vy * t);
}

inline void QLineF::operator+=(const QPointF &d)
{
    p1 += d;
    p2 += d;
}

inline bool QLineF::operator==(const QLineF &d) const
{
    return p1 == d.p1 && p2 == d.p2;
}

#ifndef QT_NO_DEBUG_OUTPUT
Q_GUI_EXPORT QDebug operator<<(QDebug d, const QLineF &p);
#endif

#endif // QLINEFLOAT_H
