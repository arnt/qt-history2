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

#ifndef QFIXEDPOINT_H
#define QFIXEDPOINT_H

class QFixedPoint;

#if !defined(Q_CORE_EXPORT)
#include <QtCore/qglobal.h>
#endif
/* Fixed point class. emulates IEEE behaviour for infinity, doesn't have NaN */

#ifndef Q_CC_BOR

class Q_CORE_EXPORT QFixedPoint {
public:
    enum {
        HighBits = 24,
        LowBits = 8
    };
    enum fixedpoint_t { FixedPoint };
    enum fixed26d6_t { FixedPoint26D6 };
    inline QFixedPoint(int v, fixedpoint_t) { val = v; }
    inline QFixedPoint(int v, fixed26d6_t) { val = v * 4; }
    QFixedPoint() : val(0) {}
    explicit QFixedPoint(int i) : val(i<<8) {}
    explicit QFixedPoint(unsigned int i) : val(i*256) {}
    explicit QFixedPoint(long i) : val(i*256) {}
    explicit QFixedPoint(double d) { val = (int)(d*256.); }
    QFixedPoint(const QFixedPoint &other) : val(other.val) {}
    QFixedPoint & operator=(const QFixedPoint &other) { val = other.val; return *this; }

    inline int toInt() const { return (((val)+128) & -256)/256; }
    inline double toDouble() const { return ((double)val)/256.; }

    inline bool operator!() const { return !val; }

    inline QFixedPoint &operator+=(const QFixedPoint &other) { val += other.val; return *this; }
    inline QFixedPoint &operator-=(const QFixedPoint &other) { val -= other.val; return *this; }
    inline QFixedPoint operator-() const { return QFixedPoint(-val, FixedPoint); }

    inline QFixedPoint &operator/=(int d) { val /= d; return *this; }
    inline QFixedPoint &operator/=(const QFixedPoint &o) {
        if (o.val == 0) {
            val =0x7FFFFFFFL;
        } else {
            bool neg = false;
            qint64 a = val;
            qint64 b = o.val;
            if (a < 0) { a = -a; neg = true; }
            if (b < 0) { b = -b; neg = !neg; }

            int res = (int)(((a << LowBits) + (b >> 1)) / b);

            val = (neg ? -res : res);
        }
        return *this;
    }
    inline QFixedPoint &operator/=(double d) { QFixedPoint v(d); return operator/=(v); }

    inline QFixedPoint operator/(int d) const { return QFixedPoint((val+(d>>1))/d, FixedPoint); }
    inline QFixedPoint operator/(const QFixedPoint &b) const { QFixedPoint v = *this; return (v /= b); }
    inline QFixedPoint operator/(double d) const { QFixedPoint v(d); return (*this)/v; }

    inline QFixedPoint &operator*=(int i) { val *= i; return *this; }
    inline QFixedPoint &operator*=(const QFixedPoint &o) {
        bool neg = false;
        qint64 a = val;
        qint64 b = o.val;
        if (a < 0) { a = -a; neg = true; }
        if (b < 0) { b = -b; neg = !neg; }

        int res = (int)((a * b + 0x80L) >> 8);
        val = neg ? -res : res;
        return *this;
    }
    inline QFixedPoint &operator*=(double d) { QFixedPoint v(d); return operator*=(v); }

    inline QFixedPoint operator*(int i) const { return QFixedPoint(val*i, FixedPoint); }
    inline QFixedPoint operator*(double f) const { QFixedPoint v(f); return (v *= *this); }
    inline QFixedPoint operator*(const QFixedPoint &o) const { QFixedPoint v = *this; return (v *= o); }

    inline int value() const { return val; }
private:
    int val;
};
Q_DECLARE_TYPEINFO(QFixedPoint, Q_PRIMITIVE_TYPE);

inline QFixedPoint operator+(const QFixedPoint &a, const QFixedPoint &b)
{ return QFixedPoint(a.value() + b.value(), QFixedPoint::FixedPoint); }
inline QFixedPoint operator-(const QFixedPoint &a, const QFixedPoint &b)
{ return QFixedPoint(a.value() - b.value(), QFixedPoint::FixedPoint); }

inline QFixedPoint operator*(int i, const QFixedPoint &d) { return d*i; }
inline QFixedPoint operator*(double d, const QFixedPoint &d2) { return d2*d; }

inline QFixedPoint operator/(int i, const QFixedPoint &d) { return QFixedPoint(i)/d; }
inline QFixedPoint operator/(double d, const QFixedPoint &d2) { return QFixedPoint(d)/d2; }

inline int qRound(QFixedPoint f) { return f.value() > 0 ? (f.value() + 128)>>8 : (f.value() - 128)/256; }
inline int qIntCast(QFixedPoint f) { return f.value()/256; }
inline int floor(QFixedPoint f) { return f.value() > 0 ? f.value() >> 8 : (f.value() - 255)/256; }
inline int ceil(QFixedPoint f) { return f.value() > 0 ? (f.value() + 255) >> 8 : f.value()/256; }

Q_CORE_EXPORT QFixedPoint sqrt(QFixedPoint f);

Q_CORE_EXPORT QFixedPoint sin(QFixedPoint f);
Q_CORE_EXPORT QFixedPoint cos(QFixedPoint f);
Q_CORE_EXPORT QFixedPoint acos(QFixedPoint f);

inline bool operator==(const QFixedPoint &a, const QFixedPoint &b)
{ return a.value() == b.value(); }
inline bool operator!=(const QFixedPoint &a, const QFixedPoint &b)
{ return a.value() != b.value(); }
inline bool operator<(const QFixedPoint &a, const QFixedPoint &b)
{ return a.value() < b.value(); }
inline bool operator>(const QFixedPoint &a, const QFixedPoint &b)
{ return a.value() > b.value(); }
inline bool operator<=(const QFixedPoint &a, const QFixedPoint &b)
{ return a.value() <= b.value(); }
inline bool operator>=(const QFixedPoint &a, const QFixedPoint &b)
{ return a.value() >= b.value(); }

inline bool operator==(const QFixedPoint &a, const double &b)
{ return a.value() == QFixedPoint(b).value(); }
inline bool operator!=(const QFixedPoint &a, const double &b)
{ return a.value() != QFixedPoint(b).value(); }
inline bool operator<(const QFixedPoint &a, const double &b)
{ return a.value() < QFixedPoint(b).value(); }
inline bool operator>(const QFixedPoint &a, const double &b)
{ return a.value() > QFixedPoint(b).value(); }
inline bool operator<=(const QFixedPoint &a, const double &b)
{ return a.value() <= QFixedPoint(b).value(); }
inline bool operator>=(const QFixedPoint &a, const double &b)
{ return a.value() >= QFixedPoint(b).value(); }

inline bool operator==(const double &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() == b.value(); }
inline bool operator!=(const double &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() != b.value(); }
inline bool operator<(const double &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() < b.value(); }
inline bool operator>(const double &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() > b.value(); }
inline bool operator<=(const double &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() <= b.value(); }
inline bool operator>=(const double &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() >= b.value(); }

inline bool operator==(const QFixedPoint &a, const int &b)
{ return a.value() == QFixedPoint(b).value(); }
inline bool operator!=(const QFixedPoint &a, const int &b)
{ return a.value() != QFixedPoint(b).value(); }
inline bool operator<(const QFixedPoint &a, const int &b)
{ return a.value() < QFixedPoint(b).value(); }
inline bool operator>(const QFixedPoint &a, const int &b)
{ return a.value() > QFixedPoint(b).value(); }
inline bool operator<=(const QFixedPoint &a, const int &b)
{ return a.value() <= QFixedPoint(b).value(); }
inline bool operator>=(const QFixedPoint &a, const int &b)
{ return a.value() >= QFixedPoint(b).value(); }

inline bool operator==(const int &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() == b.value(); }
inline bool operator!=(const int &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() != b.value(); }
inline bool operator<(const int &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() < b.value(); }
inline bool operator>(const int &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() > b.value(); }
inline bool operator<=(const int &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() <= b.value(); }
inline bool operator>=(const int &a, const QFixedPoint &b)
{ return QFixedPoint(a).value() >= b.value(); }

class Q_CORE_EXPORT QFixedPointLong {
public:
    enum {
        HighBits = 32,
        LowBits = 32
    };
    enum fixedpoint_t { FixedPoint };
    inline QFixedPointLong(qint64 v, fixedpoint_t) { val = v; }
    QFixedPointLong() : val(0) {}
    QFixedPointLong(int i) : val(i) { val <<= 32; }
    QFixedPointLong(qint64 i) : val(i) { val <<= 32; }
    QFixedPointLong(double d) { val = (qint64)(d*(double(Q_INT64_C(1)<<32))); }
    QFixedPointLong(const QFixedPointLong &other) : val(other.val) {}
    QFixedPointLong(const QFixedPoint &other) { val = other.value(); val <<= 24; }
    QFixedPointLong & operator=(const QFixedPointLong &other) { val = other.val; return *this; }
    QFixedPointLong & operator=(const QFixedPoint &other) { val = other.value(); val <<= 24; return *this; }

    inline double toDouble() const { return (val >> 32) + ((val & 0xffffffff)/(double(Q_INT64_C(1)<<32))); }
    inline QFixedPoint toFixed() const { return QFixedPoint((int)(val>>24), QFixedPoint::FixedPoint); }
    inline bool operator!() const { return !val; }

    inline QFixedPointLong &operator+=(const QFixedPointLong &other) { val += other.val; return *this; }
    inline QFixedPointLong &operator-=(const QFixedPointLong &other) { val -= other.val; return *this; }
    inline QFixedPointLong operator-() const { return QFixedPointLong(-val, FixedPoint); }

    inline QFixedPointLong &operator/=(int d) { val /= d; return *this; }
    QFixedPointLong &operator/=(const QFixedPointLong &o);

    inline QFixedPointLong &operator/=(double d) { QFixedPointLong v(d); return operator/=(v); }

    inline QFixedPointLong operator/(int d) const { return QFixedPointLong((val+(d>>1))/d, FixedPoint); }
    inline QFixedPointLong operator/(const QFixedPointLong &b) const { QFixedPointLong v = *this; return (v /= b); }
    inline QFixedPointLong operator/(double d) const { QFixedPointLong v(d); return (*this)/v; }

    inline QFixedPointLong &operator*=(int i) { val *= i; return *this; }
    QFixedPointLong &operator*=(const QFixedPointLong &o);
    inline QFixedPointLong &operator*=(double d) { QFixedPointLong v(d); return operator*=(v); }

    inline QFixedPointLong operator*(int i) const { return QFixedPointLong(val*i, FixedPoint); }
    inline QFixedPointLong operator*(double f) const { QFixedPointLong v(f); return (v *= *this); }
    inline QFixedPointLong operator*(const QFixedPointLong &o) const { QFixedPointLong v = *this; return (v *= o); }

    inline qint64 value() const { return val; }
private:
    qint64 val;
};
Q_DECLARE_TYPEINFO(QFixedPointLong, Q_PRIMITIVE_TYPE);

inline QFixedPointLong operator+(const QFixedPointLong &a, const QFixedPointLong &b)
{ return QFixedPointLong(a.value() + b.value(), QFixedPointLong::FixedPoint); }
inline QFixedPointLong operator-(const QFixedPointLong &a, const QFixedPointLong &b)
{ return QFixedPointLong(a.value() - b.value(), QFixedPointLong::FixedPoint); }

inline QFixedPointLong operator*(int i, const QFixedPointLong &d) { return d*i; }
inline QFixedPointLong operator*(double d, const QFixedPointLong &d2) { return d2*d; }

inline QFixedPointLong operator/(int i, const QFixedPointLong &d) { return QFixedPointLong(i)/d; }
inline QFixedPointLong operator/(double d, const QFixedPointLong &d2) { return QFixedPointLong(d)/d2; }

inline int qRound(QFixedPointLong f)
{ return (int)((f.value() > 0 ? (f.value() + 0x80000000) : (f.value() - 0x80000000))>>32); }
inline int qIntCast(QFixedPointLong f) { return (int)(f.value()>>32); }
inline int floor(QFixedPointLong f) { return (int)((f.value() > 0 ? f.value() : (f.value() - 0x80000000))>>32); }
inline int ceil(QFixedPointLong f) { return (int)((f.value() > 0 ? (f.value() + 0xffffffff) : f.value())>>32); }

Q_CORE_EXPORT QFixedPointLong sqrt(QFixedPointLong f);

Q_CORE_EXPORT QFixedPointLong sin(QFixedPointLong f);
Q_CORE_EXPORT QFixedPointLong cos(QFixedPointLong f);
Q_CORE_EXPORT QFixedPointLong acos(QFixedPointLong f);

#endif // Q_CC_BOR

#endif // QFIXEDPOINT_H
