/****************************************************************************
**
** Definition of QSize class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSIZE_H
#define QSIZE_H

#ifndef QT_H
#include "qpoint.h" // ### change to qwindowdefs.h?
#endif // QT_H

class Q_GUI_EXPORT QSize
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

    void scale(int w, int h, Qt::ScaleMode mode);
    void scale(const QSize &s, Qt::ScaleMode mode);

    QSize expandedTo(const QSize &) const;
    QSize boundedTo(const QSize &) const;

    QCOORD &rwidth();
    QCOORD &rheight();

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
    QCOORD wd;
    QCOORD ht;
};


/*****************************************************************************
  QSize stream functions
 *****************************************************************************/

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QSize &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QSize &);


/*****************************************************************************
  QSize inline functions
 *****************************************************************************/

inline QSize::QSize()
{ wd = ht = -1; }

inline QSize::QSize(int w, int h)
{ wd=(QCOORD)w; ht=(QCOORD)h; }

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
{ wd=(QCOORD)w; }

inline void QSize::setHeight(int h)
{ ht=(QCOORD)h; }

inline QCOORD &QSize::rwidth()
{ return wd; }

inline QCOORD &QSize::rheight()
{ return ht; }

inline QSize &QSize::operator+=(const QSize &s)
{ wd+=s.wd; ht+=s.ht; return *this; }

inline QSize &QSize::operator-=(const QSize &s)
{ wd-=s.wd; ht-=s.ht; return *this; }

inline QSize &QSize::operator*=(int c)
{ wd*=(QCOORD)c; ht*=(QCOORD)c; return *this; }

inline QSize &QSize::operator*=(double c)
{ wd=(QCOORD)(wd*c); ht=(QCOORD)(ht*c); return *this; }

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
{ return QSize((QCOORD)(s.wd*c), (QCOORD)(s.ht*c)); }

inline const QSize operator*(double c, const QSize &s)
{ return QSize((QCOORD)(s.wd*c), (QCOORD)(s.ht*c)); }

inline QSize &QSize::operator/=(int c)
{
    Q_ASSERT(c != 0);
    wd/=(QCOORD)c; ht/=(QCOORD)c;
    return *this;
}

inline QSize &QSize::operator/=(double c)
{
    Q_ASSERT(c != 0.0);
    wd=(QCOORD)(wd/c); ht=(QCOORD)(ht/c);
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
    return QSize((QCOORD)(s.wd/c), (QCOORD)(s.ht/c));
}

inline QSize QSize::expandedTo(const QSize & otherSize) const
{
    return QSize(qMax(wd,otherSize.wd), qMax(ht,otherSize.ht));
}

inline QSize QSize::boundedTo(const QSize & otherSize) const
{
    return QSize(qMin(wd,otherSize.wd), qMin(ht,otherSize.ht));
}

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QSize &);
#endif

#endif // QSIZE_H
