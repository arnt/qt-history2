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

#ifndef QSIZEFLOAT_H
#define QSIZEFLOAT_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qsize.h"
#endif // QT_H


class Q_GUI_EXPORT QSizeFloat
{
public:
    QSizeFloat();
    QSizeFloat(const QSize &sz);
    QSizeFloat(float w, float h);

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;

    float width() const;
    float height() const;
    void setWidth(float w);
    void setHeight(float h);
    void transpose();

    void scale(float w, float h, Qt::ScaleMode mode);
    void scale(const QSizeFloat &s, Qt::ScaleMode mode);

    QSizeFloat expandedTo(const QSizeFloat &) const;
    QSizeFloat boundedTo(const QSizeFloat &) const;

    float &rwidth();
    float &rheight();

    QSizeFloat &operator+=(const QSizeFloat &);
    QSizeFloat &operator-=(const QSizeFloat &);
    QSizeFloat &operator*=(int c);
    QSizeFloat &operator*=(float c);
    QSizeFloat &operator/=(int c);
    QSizeFloat &operator/=(float c);

    friend inline bool operator==(const QSizeFloat &, const QSizeFloat &);
    friend inline bool operator!=(const QSizeFloat &, const QSizeFloat &);
    friend inline const QSizeFloat operator+(const QSizeFloat &, const QSizeFloat &);
    friend inline const QSizeFloat operator-(const QSizeFloat &, const QSizeFloat &);
    friend inline const QSizeFloat operator*(const QSizeFloat &, int);
    friend inline const QSizeFloat operator*(int, const QSizeFloat &);
    friend inline const QSizeFloat operator*(const QSizeFloat &, float);
    friend inline const QSizeFloat operator*(float, const QSizeFloat &);
    friend inline const QSizeFloat operator/(const QSizeFloat &, int);
    friend inline const QSizeFloat operator/(const QSizeFloat &, float);

    inline QSize toSize() const;

private:
    float wd;
    float ht;
};


/*****************************************************************************
  QSizeFloat stream functions
 *****************************************************************************/

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QSizeFloat &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QSizeFloat &);


/*****************************************************************************
  QSizeFloat inline functions
 *****************************************************************************/

inline QSizeFloat::QSizeFloat()
{ wd = ht = -1.; }

inline QSizeFloat::QSizeFloat(const QSize &sz)
    : wd(sz.width()), ht(sz.height())
{
}

inline QSizeFloat::QSizeFloat(float w, float h)
{ wd = w; ht = h; }

inline bool QSizeFloat::isNull() const
{ return wd == 0 && ht == 0; }

inline bool QSizeFloat::isEmpty() const
{ return wd <= 0. || ht <= 0.; }

inline bool QSizeFloat::isValid() const
{ return wd >= 0. && ht >= 0.; }

inline float QSizeFloat::width() const
{ return wd; }

inline float QSizeFloat::height() const
{ return ht; }

inline void QSizeFloat::setWidth(float w)
{ wd = w; }

inline void QSizeFloat::setHeight(float h)
{ ht = h; }

inline void QSizeFloat::scale(float w, float h, Qt::ScaleMode mode)
{ scale(QSizeFloat(w, h), mode); }

inline float &QSizeFloat::rwidth()
{ return wd; }

inline float &QSizeFloat::rheight()
{ return ht; }

inline QSizeFloat &QSizeFloat::operator+=(const QSizeFloat &s)
{ wd += s.wd; ht += s.ht; return *this; }

inline QSizeFloat &QSizeFloat::operator-=(const QSizeFloat &s)
{ wd -= s.wd; ht -= s.ht; return *this; }

inline QSizeFloat &QSizeFloat::operator*=(int c)
{ wd *= c; ht *= c; return *this; }

inline QSizeFloat &QSizeFloat::operator*=(float c)
{ wd *= c; ht *= c; return *this; }

inline bool operator==(const QSizeFloat &s1, const QSizeFloat &s2)
{ return s1.wd == s2.wd && s1.ht == s2.ht; }

inline bool operator!=(const QSizeFloat &s1, const QSizeFloat &s2)
{ return s1.wd != s2.wd || s1.ht != s2.ht; }

inline const QSizeFloat operator+(const QSizeFloat & s1, const QSizeFloat & s2)
{ return QSizeFloat(s1.wd+s2.wd, s1.ht+s2.ht); }

inline const QSizeFloat operator-(const QSizeFloat &s1, const QSizeFloat &s2)
{ return QSizeFloat(s1.wd-s2.wd, s1.ht-s2.ht); }

inline const QSizeFloat operator*(const QSizeFloat &s, int c)
{ return QSizeFloat(s.wd*c, s.ht*c); }

inline const QSizeFloat operator*(int c, const QSizeFloat &s)
{  return QSizeFloat(s.wd*c, s.ht*c); }

inline const QSizeFloat operator*(const QSizeFloat &s, float c)
{ return QSizeFloat(s.wd*c, s.ht*c); }

inline const QSizeFloat operator*(float c, const QSizeFloat &s)
{ return QSizeFloat(s.wd*c, s.ht*c); }

inline QSizeFloat &QSizeFloat::operator/=(int c)
{
    Q_ASSERT(c != 0);
    wd /= c; ht /= c;
    return *this;
}

inline QSizeFloat &QSizeFloat::operator/=(float c)
{
    Q_ASSERT(c != 0.0);
    wd = wd/c; ht = ht/c;
    return *this;
}

inline const QSizeFloat operator/(const QSizeFloat &s, int c)
{
    Q_ASSERT(c != 0);
    return QSizeFloat(s.wd/c, s.ht/c);
}

inline const QSizeFloat operator/(const QSizeFloat &s, float c)
{
    Q_ASSERT(c != 0.0);
    return QSizeFloat(s.wd/c, s.ht/c);
}

inline QSizeFloat QSizeFloat::expandedTo(const QSizeFloat & otherSize) const
{
    return QSizeFloat(qMax(wd,otherSize.wd), qMax(ht,otherSize.ht));
}

inline QSizeFloat QSizeFloat::boundedTo(const QSizeFloat & otherSize) const
{
    return QSizeFloat(qMin(wd,otherSize.wd), qMin(ht,otherSize.ht));
}

inline QSize QSizeFloat::toSize() const
{
    return QSize(qRound(wd), qRound(ht));
}

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QSizeFloat &);
#endif

#endif // QSIZE_H
