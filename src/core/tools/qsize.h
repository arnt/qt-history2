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

#ifndef QSIZE_H
#define QSIZE_H

#include "qnamespace.h"

class Q_CORE_EXPORT QSize
{
public:
    QSize();
    QSize(int w, int h);

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;

    int width() const;
    int height() const;
    void setWidth(int w);
    void setHeight(int h);
    void transpose();

    void scale(int w, int h, Qt::AspectRatioMode mode);
    void scale(const QSize &s, Qt::AspectRatioMode mode);

    QSize expandedTo(const QSize &) const;
    QSize boundedTo(const QSize &) const;

    int &rwidth();
    int &rheight();

    QSize &operator+=(const QSize &);
    QSize &operator-=(const QSize &);
    QSize &operator*=(int c);
    QSize &operator*=(double c);
    QSize &operator/=(int c);
    QSize &operator/=(double c);

    friend inline bool operator==(const QSize &, const QSize &);
    friend inline bool operator!=(const QSize &, const QSize &);
    friend inline const QSize operator+(const QSize &, const QSize &);
    friend inline const QSize operator-(const QSize &, const QSize &);
    friend inline const QSize operator*(const QSize &, int);
    friend inline const QSize operator*(int, const QSize &);
    friend inline const QSize operator*(const QSize &, double);
    friend inline const QSize operator*(double, const QSize &);
    friend inline const QSize operator/(const QSize &, int);
    friend inline const QSize operator/(const QSize &, double);

private:
    int wd;
    int ht;
};
Q_DECLARE_TYPEINFO(QSize, Q_MOVABLE_TYPE);

/*****************************************************************************
  QSize stream functions
 *****************************************************************************/

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QSize &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QSize &);


/*****************************************************************************
  QSize inline functions
 *****************************************************************************/

inline QSize::QSize()
{ wd = ht = -1; }

inline QSize::QSize(int w, int h)
{ wd = w; ht = h; }

inline bool QSize::isNull() const
{ return wd==0 && ht==0; }

inline bool QSize::isEmpty() const
{ return wd<1 || ht<1; }

inline bool QSize::isValid() const
{ return wd>=0 && ht>=0; }

inline int QSize::width() const
{ return wd; }

inline int QSize::height() const
{ return ht; }

inline void QSize::setWidth(int w)
{ wd = w; }

inline void QSize::setHeight(int h)
{ ht = h; }

inline void QSize::scale(int w, int h, Qt::AspectRatioMode mode)
{ scale(QSize(w, h), mode); }

inline int &QSize::rwidth()
{ return wd; }

inline int &QSize::rheight()
{ return ht; }

inline QSize &QSize::operator+=(const QSize &s)
{ wd+=s.wd; ht+=s.ht; return *this; }

inline QSize &QSize::operator-=(const QSize &s)
{ wd-=s.wd; ht-=s.ht; return *this; }

inline QSize &QSize::operator*=(int c)
{ wd *= c; ht *= c; return *this; }

inline QSize &QSize::operator*=(double c)
{ wd = qRound(wd*c); ht = qRound(ht*c); return *this; }

inline bool operator==(const QSize &s1, const QSize &s2)
{ return s1.wd == s2.wd && s1.ht == s2.ht; }

inline bool operator!=(const QSize &s1, const QSize &s2)
{ return s1.wd != s2.wd || s1.ht != s2.ht; }

inline const QSize operator+(const QSize & s1, const QSize & s2)
{ return QSize(s1.wd+s2.wd, s1.ht+s2.ht); }

inline const QSize operator-(const QSize &s1, const QSize &s2)
{ return QSize(s1.wd-s2.wd, s1.ht-s2.ht); }

inline const QSize operator*(const QSize &s, int c)
{ return QSize(s.wd*c, s.ht*c); }

inline const QSize operator*(int c, const QSize &s)
{  return QSize(s.wd*c, s.ht*c); }

inline const QSize operator*(const QSize &s, double c)
{ return QSize(qRound(s.wd*c), qRound(s.ht*c)); }

inline const QSize operator*(double c, const QSize &s)
{ return QSize(qRound(s.wd*c), qRound(s.ht*c)); }

inline QSize &QSize::operator/=(int c)
{
    Q_ASSERT(c != 0);
    wd /= c; ht /= c;
    return *this;
}

inline QSize &QSize::operator/=(double c)
{
    Q_ASSERT(c != 0.0);
    wd = qRound(wd/c); ht = qRound(ht/c);
    return *this;
}

inline const QSize operator/(const QSize &s, int c)
{
    Q_ASSERT(c != 0);
    return QSize(s.wd/c, s.ht/c);
}

inline const QSize operator/(const QSize &s, double c)
{
    Q_ASSERT(c != 0.0);
    return QSize(qRound(s.wd/c), qRound(s.ht/c));
}

inline QSize QSize::expandedTo(const QSize & otherSize) const
{
    return QSize(qMax(wd,otherSize.wd), qMax(ht,otherSize.ht));
}

inline QSize QSize::boundedTo(const QSize & otherSize) const
{
    return QSize(qMin(wd,otherSize.wd), qMin(ht,otherSize.ht));
}

#ifndef QT_NO_DEBUG_OUTPUT
Q_CORE_EXPORT QDebug operator<<(QDebug, const QSize &);
#endif


class Q_CORE_EXPORT QSizeF
{
public:
    QSizeF();
    QSizeF(const QSize &sz);
    QSizeF(float w, float h);

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;

    float width() const;
    float height() const;
    void setWidth(float w);
    void setHeight(float h);
    void transpose();

    void scale(float w, float h, Qt::AspectRatioMode mode);
    void scale(const QSizeF &s, Qt::AspectRatioMode mode);

    QSizeF expandedTo(const QSizeF &) const;
    QSizeF boundedTo(const QSizeF &) const;

    float &rwidth();
    float &rheight();

    QSizeF &operator+=(const QSizeF &);
    QSizeF &operator-=(const QSizeF &);
    QSizeF &operator*=(int c);
    QSizeF &operator*=(float c);
    QSizeF &operator/=(int c);
    QSizeF &operator/=(float c);

    friend inline bool operator==(const QSizeF &, const QSizeF &);
    friend inline bool operator!=(const QSizeF &, const QSizeF &);
    friend inline const QSizeF operator+(const QSizeF &, const QSizeF &);
    friend inline const QSizeF operator-(const QSizeF &, const QSizeF &);
    friend inline const QSizeF operator*(const QSizeF &, int);
    friend inline const QSizeF operator*(int, const QSizeF &);
    friend inline const QSizeF operator*(const QSizeF &, float);
    friend inline const QSizeF operator*(float, const QSizeF &);
    friend inline const QSizeF operator/(const QSizeF &, int);
    friend inline const QSizeF operator/(const QSizeF &, float);

    inline QSize toSize() const;

private:
    float wd;
    float ht;
};
Q_DECLARE_TYPEINFO(QSizeF, Q_MOVABLE_TYPE);


/*****************************************************************************
  QSizeF stream functions
 *****************************************************************************/

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QSizeF &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QSizeF &);


/*****************************************************************************
  QSizeF inline functions
 *****************************************************************************/

inline QSizeF::QSizeF()
{ wd = ht = -1.; }

inline QSizeF::QSizeF(const QSize &sz)
    : wd(sz.width()), ht(sz.height())
{
}

inline QSizeF::QSizeF(float w, float h)
{ wd = w; ht = h; }

inline bool QSizeF::isNull() const
{ return wd == 0 && ht == 0; }

inline bool QSizeF::isEmpty() const
{ return wd <= 0. || ht <= 0.; }

inline bool QSizeF::isValid() const
{ return wd >= 0. && ht >= 0.; }

inline float QSizeF::width() const
{ return wd; }

inline float QSizeF::height() const
{ return ht; }

inline void QSizeF::setWidth(float w)
{ wd = w; }

inline void QSizeF::setHeight(float h)
{ ht = h; }

inline void QSizeF::scale(float w, float h, Qt::AspectRatioMode mode)
{ scale(QSizeF(w, h), mode); }

inline float &QSizeF::rwidth()
{ return wd; }

inline float &QSizeF::rheight()
{ return ht; }

inline QSizeF &QSizeF::operator+=(const QSizeF &s)
{ wd += s.wd; ht += s.ht; return *this; }

inline QSizeF &QSizeF::operator-=(const QSizeF &s)
{ wd -= s.wd; ht -= s.ht; return *this; }

inline QSizeF &QSizeF::operator*=(int c)
{ wd *= c; ht *= c; return *this; }

inline QSizeF &QSizeF::operator*=(float c)
{ wd *= c; ht *= c; return *this; }

inline bool operator==(const QSizeF &s1, const QSizeF &s2)
{ return s1.wd == s2.wd && s1.ht == s2.ht; }

inline bool operator!=(const QSizeF &s1, const QSizeF &s2)
{ return s1.wd != s2.wd || s1.ht != s2.ht; }

inline const QSizeF operator+(const QSizeF & s1, const QSizeF & s2)
{ return QSizeF(s1.wd+s2.wd, s1.ht+s2.ht); }

inline const QSizeF operator-(const QSizeF &s1, const QSizeF &s2)
{ return QSizeF(s1.wd-s2.wd, s1.ht-s2.ht); }

inline const QSizeF operator*(const QSizeF &s, int c)
{ return QSizeF(s.wd*c, s.ht*c); }

inline const QSizeF operator*(int c, const QSizeF &s)
{  return QSizeF(s.wd*c, s.ht*c); }

inline const QSizeF operator*(const QSizeF &s, float c)
{ return QSizeF(s.wd*c, s.ht*c); }

inline const QSizeF operator*(float c, const QSizeF &s)
{ return QSizeF(s.wd*c, s.ht*c); }

inline QSizeF &QSizeF::operator/=(int c)
{
    Q_ASSERT(c != 0);
    wd /= c; ht /= c;
    return *this;
}

inline QSizeF &QSizeF::operator/=(float c)
{
    Q_ASSERT(c != 0.0);
    wd = wd/c; ht = ht/c;
    return *this;
}

inline const QSizeF operator/(const QSizeF &s, int c)
{
    Q_ASSERT(c != 0);
    return QSizeF(s.wd/c, s.ht/c);
}

inline const QSizeF operator/(const QSizeF &s, float c)
{
    Q_ASSERT(c != 0.0);
    return QSizeF(s.wd/c, s.ht/c);
}

inline QSizeF QSizeF::expandedTo(const QSizeF & otherSize) const
{
    return QSizeF(qMax(wd,otherSize.wd), qMax(ht,otherSize.ht));
}

inline QSizeF QSizeF::boundedTo(const QSizeF & otherSize) const
{
    return QSizeF(qMin(wd,otherSize.wd), qMin(ht,otherSize.ht));
}

inline QSize QSizeF::toSize() const
{
    return QSize(qRound(wd), qRound(ht));
}

#ifndef QT_NO_DEBUG_OUTPUT
Q_CORE_EXPORT QDebug operator<<(QDebug, const QSizeF &);
#endif

#endif // QSIZE_H
