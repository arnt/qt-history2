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

#ifndef QLINEFLOAT_H
#define QLINEFLOAT_H

#include <qglobal.h>

#include <qpointfloat.h>

class Q_GUI_EXPORT QLineFloat {
public:

    enum IntersectMode { Unbounded, Bounded };

    inline QLineFloat();
    inline QLineFloat(const QPointFloat &pt1, const QPointFloat &pt2);
    inline QLineFloat(float x1, float y1, float x2, float y2);

    inline QPointFloat start() const;
    inline QPointFloat end() const;

    inline float startX() const;
    inline float startY() const;

    inline float endX() const;
    inline float endY() const;

    inline float vx() const;
    inline float vy() const;

    float length() const;
    void setLength(float len);

    QLineFloat unitVector() const;

    QLineFloat normalVector() const;

    inline bool intersects(const QLineFloat &l, IntersectMode mode = Unbounded) const;

    QPointFloat intersect(const QLineFloat &l, IntersectMode mode = Unbounded, bool *intersected = 0) const;

    inline void moveBy(const QLineFloat &p);

    inline void operator+=(const QPointFloat &d);

private:
    QPointFloat p1, p2;
};

inline QLineFloat::QLineFloat()
{
}

inline QLineFloat::QLineFloat(const QPointFloat &pt1, const QPointFloat &pt2)
    : p1(pt1), p2(pt2)
{
}

inline QLineFloat::QLineFloat(float x1, float y1, float x2, float y2)
    : p1(x1, y1), p2(x2, y2)
{
}

inline float QLineFloat::startX() const
{
    return p1.x();
}

inline float QLineFloat::startY() const
{
    return p1.y();
}

inline float QLineFloat::endX() const
{
    return p2.x();
}

inline float QLineFloat::endY() const
{
    return p2.y();
}

inline QPointFloat QLineFloat::start() const
{
    return p1;
}

inline QPointFloat QLineFloat::end() const
{
    return p2;
}

inline float QLineFloat::vx() const
{
    return p2.x() - p1.x();
}

inline float QLineFloat::vy() const
{
    return p2.y() - p1.y();
}

inline bool QLineFloat::intersects(const QLineFloat &l, IntersectMode mode) const
{
    bool intersected = false;
    intersect(l, mode, &intersected);
    return intersected;
}

inline QLineFloat QLineFloat::normalVector() const
{
    return QLineFloat(start(), start() + QPointFloat(vy(), -vx()));
}

inline void QLineFloat::moveBy(const QLineFloat &l)
{
    QPointFloat pf(l.vx(), l.vy());
    *this += pf;
}

inline void QLineFloat::setLength(float len)
{
    QLineFloat v = unitVector();
    p2 = QPointFloat(p1.x() + vx() * len, p1.y() + vy() * len);
}

inline void QLineFloat::operator+=(const QPointFloat &d)
{
    p1 += d;
    p2 += d;
}

#endif // QLINEFLOAT_H
