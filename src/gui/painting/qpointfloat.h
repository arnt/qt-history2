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

#ifndef QPOINTFLOAT_H
#define QPOINTFLOAT_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qpoint.h"
#endif // QT_H

class Q_GUI_EXPORT QPointFloat
{
public:
    QPointFloat();
    QPointFloat(const QPoint &p);
    QPointFloat(float xpos, float ypos);

    bool isNull() const;

    float x() const;
    float y() const;
    void setX(float x);
    void setY(float y);

    float &rx();
    float &ry();

    QPointFloat &operator+=(const QPointFloat &p);
    QPointFloat &operator-=(const QPointFloat &p);
    QPointFloat &operator*=(float c);
    QPointFloat &operator/=(float c);

    friend inline bool operator==(const QPointFloat &, const QPointFloat &);
    friend inline bool operator!=(const QPointFloat &, const QPointFloat &);
    friend inline const QPointFloat operator+(const QPointFloat &, const QPointFloat &);
    friend inline const QPointFloat operator-(const QPointFloat &, const QPointFloat &);
    friend inline const QPointFloat operator*(const QPointFloat &, int);
    friend inline const QPointFloat operator*(float, const QPointFloat &);
    friend inline const QPointFloat operator*(const QPointFloat &, float);
    friend inline const QPointFloat operator-(const QPointFloat &);
    friend inline const QPointFloat operator/(const QPointFloat &, float);

    QPoint toPoint() const;

private:
    float xp;
    float yp;
};


/*****************************************************************************
  QPointFloat stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPointFloat &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPointFloat &);
#endif

/*****************************************************************************
  QPointFloat inline functions
 *****************************************************************************/

inline QPointFloat::QPointFloat() : xp(0), yp(0) { }

inline QPointFloat::QPointFloat(float xpos, float ypos) : xp(xpos), yp(ypos) { }

inline QPointFloat::QPointFloat(const QPoint &p) : xp(p.x()), yp(p.y()) { }

inline bool QPointFloat::isNull() const
{
    return xp == 0 && yp == 0;
}

inline float QPointFloat::x() const
{
    return xp;
}

inline float QPointFloat::y() const
{
    return yp;
}

inline void QPointFloat::setX(float x)
{
    xp = x;
}

inline void QPointFloat::setY(float y)
{
    yp = y;
}

inline float &QPointFloat::rx()
{
    return xp;
}

inline float &QPointFloat::ry()
{
    return yp;
}

inline QPointFloat &QPointFloat::operator+=(const QPointFloat &p)
{
    xp+=p.xp;
    yp+=p.yp;
    return *this;
}

inline QPointFloat &QPointFloat::operator-=(const QPointFloat &p)
{
    xp-=p.xp; yp-=p.yp; return *this;
}

inline QPointFloat &QPointFloat::operator*=(float c)
{
    xp*=c; yp*=c; return *this;
}

inline bool operator==(const QPointFloat &p1, const QPointFloat &p2)
{
    return p1.xp == p2.xp && p1.yp == p2.yp;
}

inline bool operator!=(const QPointFloat &p1, const QPointFloat &p2)
{
    return p1.xp != p2.xp || p1.yp != p2.yp;
}

inline const QPointFloat operator+(const QPointFloat &p1, const QPointFloat &p2)
{
    return QPointFloat(p1.xp+p2.xp, p1.yp+p2.yp);
}

inline const QPointFloat operator-(const QPointFloat &p1, const QPointFloat &p2)
{
    return QPointFloat(p1.xp-p2.xp, p1.yp-p2.yp);
}

inline const QPointFloat operator*(const QPointFloat &p, float c)
{
    return QPointFloat(p.xp*c, p.yp*c);
}

inline const QPointFloat operator*(float c, const QPointFloat &p)
{
    return QPointFloat(p.xp*c, p.yp*c);
}

inline const QPointFloat operator-(const QPointFloat &p)
{
    return QPointFloat(-p.xp, -p.yp);
}

inline QPointFloat &QPointFloat::operator/=(float c)
{
    Q_ASSERT(c != 0);
    xp/=c;
    yp/=c;
    return *this;
}

inline const QPointFloat operator/(const QPointFloat &p, float c)
{
    Q_ASSERT(c != 0);
    return QPointFloat(p.xp/c, p.yp/c);
}

inline QPoint QPointFloat::toPoint() const
{
    return QPoint(qRound(xp), qRound(yp));
}

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug d, const QPointFloat &p);
#endif

#endif // QPOINTFLOAT_H
