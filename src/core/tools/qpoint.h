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

#ifndef QPOINT_H
#define QPOINT_H

#include "qnamespace.h"

class Q_CORE_EXPORT QPoint
{
public:
    QPoint();
    QPoint(int xpos, int ypos);

    bool isNull() const;

    int x() const;
    int y() const;
    void setX(int x);
    void setY(int y);

    int manhattanLength() const;

    int &rx();
    int &ry();

    QPoint &operator+=(const QPoint &p);
    QPoint &operator-=(const QPoint &p);
    QPoint &operator*=(int c);
    QPoint &operator*=(double c);
    QPoint &operator/=(int c);
    QPoint &operator/=(double c);

    friend inline bool operator==(const QPoint &, const QPoint &);
    friend inline bool operator!=(const QPoint &, const QPoint &);
    friend inline const QPoint operator+(const QPoint &, const QPoint &);
    friend inline const QPoint operator-(const QPoint &, const QPoint &);
    friend inline const QPoint operator*(const QPoint &, int);
    friend inline const QPoint operator*(int, const QPoint &);
    friend inline const QPoint operator*(const QPoint &, double);
    friend inline const QPoint operator*(double, const QPoint &);
    friend inline const QPoint operator-(const QPoint &);
    friend inline const QPoint operator/(const QPoint &, int);
    friend inline const QPoint operator/(const QPoint &, double);

private:

#if defined(Q_OS_MAC)
    int yp;
    int xp;
#else
    int xp;
    int yp;
#endif
};

Q_DECLARE_TYPEINFO(QPoint, Q_MOVABLE_TYPE);

/*****************************************************************************
  QPoint stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QPoint &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QPoint &);
#endif

/*****************************************************************************
  QPoint inline functions
 *****************************************************************************/

inline QPoint::QPoint()
{ xp=0; yp=0; }

inline QPoint::QPoint(int xpos, int ypos)
{ xp = xpos; yp = ypos; }

inline bool QPoint::isNull() const
{ return xp == 0 && yp == 0; }

inline int QPoint::x() const
{ return xp; }

inline int QPoint::y() const
{ return yp; }

inline void QPoint::setX(int x)
{ xp = x; }

inline void QPoint::setY(int y)
{ yp = y; }

inline int &QPoint::rx()
{ return xp; }

inline int &QPoint::ry()
{ return yp; }

inline QPoint &QPoint::operator+=(const QPoint &p)
{ xp+=p.xp; yp+=p.yp; return *this; }

inline QPoint &QPoint::operator-=(const QPoint &p)
{ xp-=p.xp; yp-=p.yp; return *this; }

inline QPoint &QPoint::operator*=(int c)
{ xp *= c; yp *= c; return *this; }

inline QPoint &QPoint::operator*=(double c)
{ xp = qRound(xp*c); yp = qRound(yp*c); return *this; }

inline bool operator==(const QPoint &p1, const QPoint &p2)
{ return p1.xp == p2.xp && p1.yp == p2.yp; }

inline bool operator!=(const QPoint &p1, const QPoint &p2)
{ return p1.xp != p2.xp || p1.yp != p2.yp; }

inline const QPoint operator+(const QPoint &p1, const QPoint &p2)
{ return QPoint(p1.xp+p2.xp, p1.yp+p2.yp); }

inline const QPoint operator-(const QPoint &p1, const QPoint &p2)
{ return QPoint(p1.xp-p2.xp, p1.yp-p2.yp); }

inline const QPoint operator*(const QPoint &p, int c)
{ return QPoint(p.xp*c, p.yp*c); }

inline const QPoint operator*(int c, const QPoint &p)
{ return QPoint(p.xp*c, p.yp*c); }

inline const QPoint operator*(const QPoint &p, double c)
{ return QPoint(qRound(p.xp*c), qRound(p.yp*c)); }

inline const QPoint operator*(double c, const QPoint &p)
{ return QPoint(qRound(p.xp*c), qRound(p.yp*c)); }

inline const QPoint operator-(const QPoint &p)
{ return QPoint(-p.xp, -p.yp); }

inline QPoint &QPoint::operator/=(int c)
{
    Q_ASSERT(c != 0);
    xp /= c;
    yp /= c;
    return *this;
}

inline QPoint &QPoint::operator/=(double c)
{
    Q_ASSERT(c != 0.0);
    xp = qRound(xp/c);
    yp = qRound(yp/c);
    return *this;
}

inline const QPoint operator/(const QPoint &p, int c)
{
    Q_ASSERT(c != 0);
    return QPoint(p.xp/c, p.yp/c);
}

inline const QPoint operator/(const QPoint &p, double c)
{
    Q_ASSERT(c != 0.0);
    return QPoint(qRound(p.xp/c), qRound(p.yp/c));
}

#ifndef QT_NO_DEBUG_OUTPUT
Q_CORE_EXPORT QDebug operator<<(QDebug, const QPoint &);
#endif





class Q_CORE_EXPORT QPointF
{
public:
    QPointF();
    QPointF(const QPoint &p);
    QPointF(float xpos, float ypos);

    bool isNull() const;

    float x() const;
    float y() const;
    void setX(float x);
    void setY(float y);

    float &rx();
    float &ry();

    QPointF &operator+=(const QPointF &p);
    QPointF &operator-=(const QPointF &p);
    QPointF &operator*=(float c);
    QPointF &operator/=(float c);

    friend inline bool operator==(const QPointF &, const QPointF &);
    friend inline bool operator!=(const QPointF &, const QPointF &);
    friend inline const QPointF operator+(const QPointF &, const QPointF &);
    friend inline const QPointF operator-(const QPointF &, const QPointF &);
    friend inline const QPointF operator*(float, const QPointF &);
    friend inline const QPointF operator*(const QPointF &, float);
    friend inline const QPointF operator-(const QPointF &);
    friend inline const QPointF operator/(const QPointF &, float);

    QPoint toPoint() const;

private:
    friend class QMatrix;

    float xp;
    float yp;
};

Q_DECLARE_TYPEINFO(QPointF, Q_MOVABLE_TYPE);

/*****************************************************************************
  QPointF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QPointF &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QPointF &);
#endif

/*****************************************************************************
  QPointF inline functions
 *****************************************************************************/

inline QPointF::QPointF() : xp(0), yp(0) { }

inline QPointF::QPointF(float xpos, float ypos) : xp(xpos), yp(ypos) { }

inline QPointF::QPointF(const QPoint &p) : xp(p.x()), yp(p.y()) { }

inline bool QPointF::isNull() const
{
    return xp == 0 && yp == 0;
}

inline float QPointF::x() const
{
    return xp;
}

inline float QPointF::y() const
{
    return yp;
}

inline void QPointF::setX(float x)
{
    xp = x;
}

inline void QPointF::setY(float y)
{
    yp = y;
}

inline float &QPointF::rx()
{
    return xp;
}

inline float &QPointF::ry()
{
    return yp;
}

inline QPointF &QPointF::operator+=(const QPointF &p)
{
    xp+=p.xp;
    yp+=p.yp;
    return *this;
}

inline QPointF &QPointF::operator-=(const QPointF &p)
{
    xp-=p.xp; yp-=p.yp; return *this;
}

inline QPointF &QPointF::operator*=(float c)
{
    xp*=c; yp*=c; return *this;
}

inline bool operator==(const QPointF &p1, const QPointF &p2)
{
    return p1.xp == p2.xp && p1.yp == p2.yp;
}

inline bool operator!=(const QPointF &p1, const QPointF &p2)
{
    return p1.xp != p2.xp || p1.yp != p2.yp;
}

inline const QPointF operator+(const QPointF &p1, const QPointF &p2)
{
    return QPointF(p1.xp+p2.xp, p1.yp+p2.yp);
}

inline const QPointF operator-(const QPointF &p1, const QPointF &p2)
{
    return QPointF(p1.xp-p2.xp, p1.yp-p2.yp);
}

inline const QPointF operator*(const QPointF &p, float c)
{
    return QPointF(p.xp*c, p.yp*c);
}

inline const QPointF operator*(float c, const QPointF &p)
{
    return QPointF(p.xp*c, p.yp*c);
}

inline const QPointF operator-(const QPointF &p)
{
    return QPointF(-p.xp, -p.yp);
}

inline QPointF &QPointF::operator/=(float c)
{
    Q_ASSERT(c != 0);
    xp/=c;
    yp/=c;
    return *this;
}

inline const QPointF operator/(const QPointF &p, float c)
{
    Q_ASSERT(c != 0);
    return QPointF(p.xp/c, p.yp/c);
}

inline QPoint QPointF::toPoint() const
{
    return QPoint(qRound(xp), qRound(yp));
}

#ifndef QT_NO_DEBUG_OUTPUT
Q_CORE_EXPORT QDebug operator<<(QDebug d, const QPointF &p);
#endif


#endif // QPOINT_H
