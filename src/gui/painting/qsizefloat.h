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


class Q_GUI_EXPORT QSizeF
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

    void scale(float w, float h, Qt::ScaleMode mode);
    void scale(const QSizeF &s, Qt::ScaleMode mode);

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


/*****************************************************************************
  QSizeF stream functions
 *****************************************************************************/

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QSizeF &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QSizeF &);


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

inline void QSizeF::scale(float w, float h, Qt::ScaleMode mode)
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

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QSizeF &);
#endif

#endif // QSIZE_H
