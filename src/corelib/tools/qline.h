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

/*******************************************************************************
 * class QLine
 *******************************************************************************/

class Q_CORE_EXPORT QLine
{
public:
    inline QLine();
    inline QLine(const QPoint &pt1, const QPoint &pt2);
    inline QLine(int x1, int y1, int x2, int y2);

    inline bool isNull() const;

    inline QPoint p1() const;
    inline QPoint p2() const;

    inline int x1() const;
    inline int y1() const;

    inline int x2() const;
    inline int y2() const;

    inline int dx() const;
    inline int dy() const;

    inline void translate(const QPoint &p);
    inline void translate(int dx, int dy);

    inline bool operator==(const QLine &d) const;
    inline bool operator!=(const QLine &d) const { return !(*this == d); }

private:
    QPoint pt1, pt2;
};
Q_DECLARE_TYPEINFO(QLine, Q_MOVABLE_TYPE);

/*******************************************************************************
 * class QLine inline members
 *******************************************************************************/

inline QLine::QLine() { }

inline QLine::QLine(const QPoint &pt1_, const QPoint &pt2_) : pt1(pt1_), pt2(pt2_) { }

inline QLine::QLine(int x1, int y1, int x2, int y2) : pt1(QPoint(x1, y1)), pt2(QPoint(x2, y2)) { }

inline bool QLine::isNull() const
{
    return pt1 == pt2;
}

inline int QLine::x1() const
{
    return pt1.x();
}

inline int QLine::y1() const
{
    return pt1.y();
}

inline int QLine::x2() const
{
    return pt2.x();
}

inline int QLine::y2() const
{
    return pt2.y();
}

inline QPoint QLine::p1() const
{
    return pt1;
}

inline QPoint QLine::p2() const
{
    return pt2;
}

inline int QLine::dx() const
{
    return pt2.x() - pt1.x();
}

inline int QLine::dy() const
{
    return pt2.y() - pt1.y();
}

inline void QLine::translate(const QPoint &point)
{
    pt1 += point;
    pt2 += point;
}

inline void QLine::translate(int dx, int dy)
{
    this->translate(QPoint(dx, dy));
}

inline bool QLine::operator==(const QLine &d) const
{
    return pt1 == d.pt1 && pt2 == d.pt2;
}

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug d, const QLine &p);
#endif

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QLine &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QLine &);
#endif

/*******************************************************************************
 * class QLineF
 *******************************************************************************/
class Q_CORE_EXPORT QLineF {
public:

    enum IntersectType { NoIntersection, BoundedIntersection, UnboundedIntersection };

    inline QLineF();
    inline QLineF(const QPointF &pt1, const QPointF &pt2);
    inline QLineF(qreal x1, qreal y1, qreal x2, qreal y2);
    inline QLineF(const QLine &line) : pt1(line.p1()), pt2(line.p2()) { }

    inline bool isNull() const;

    inline QPointF p1() const;
    inline QPointF p2() const;

    inline qreal x1() const;
    inline qreal y1() const;

    inline qreal x2() const;
    inline qreal y2() const;

    inline qreal dx() const;
    inline qreal dy() const;

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
    inline void translate(const QPointF &p);
    inline void translate(qreal dx, qreal dy);

    inline bool operator==(const QLineF &d) const;
    inline bool operator!=(const QLineF &d) const { return !(*this == d); }

private:
    QPointF pt1, pt2;
};
Q_DECLARE_TYPEINFO(QLineF, Q_MOVABLE_TYPE);

/*******************************************************************************
 * class QLineF inline members
 *******************************************************************************/

inline QLineF::QLineF()
{
}

inline QLineF::QLineF(const QPointF &pt1, const QPointF &pt2)
    : pt1(pt1), pt2(pt2)
{
}

inline QLineF::QLineF(qreal x1, qreal y1, qreal x2, qreal y2)
    : pt1(x1, y1), pt2(x2, y2)
{
}

inline bool QLineF::isNull() const
{
    return pt1 == pt2;
}

inline qreal QLineF::x1() const
{
    return pt1.x();
}

inline qreal QLineF::y1() const
{
    return pt1.y();
}

inline qreal QLineF::x2() const
{
    return pt2.x();
}

inline qreal QLineF::y2() const
{
    return pt2.y();
}

inline QPointF QLineF::p1() const
{
    return pt1;
}

inline QPointF QLineF::p2() const
{
    return pt2;
}

inline qreal QLineF::dx() const
{
    return pt2.x() - pt1.x();
}

inline qreal QLineF::dy() const
{
    return pt2.y() - pt1.y();
}

inline QLineF QLineF::normalVector() const
{
    return QLineF(p1(), p1() + QPointF(dy(), -dx()));
}

inline void QLineF::translate(const QPointF &point)
{
    pt1 += point;
    pt2 += point;
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
    pt2 = QPointF(pt1.x() + v.dx() * len, pt1.y() + v.dy() * len);
}

inline QPointF QLineF::pointAt(qreal t) const
{
    qreal vx = pt2.x() - pt1.x();
    qreal vy = pt2.y() - pt1.y();
    return QPointF(pt1.x() + vx * t, pt1.y() + vy * t);
}

#ifdef QT_USE_FIXED_POINT
inline QPointF QLineF::pointAt(QFixedPointLong t) const
{
    QFixedPointLong vx = pt2.x() - pt1.x();
    QFixedPointLong vy = pt2.y() - pt1.y();
    return QPointF((pt1.x() + vx * t).toFixed(), (pt1.y() + vy * t).toFixed());
}
#endif

inline bool QLineF::operator==(const QLineF &d) const
{
    return pt1 == d.pt1 && pt2 == d.pt2;
}

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug d, const QLineF &p);
#endif

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QLineF &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QLineF &);
#endif

#endif // QLINE_H
