#include <qfixedpoint.h>

#include <math.h>


/*
  see http://www.worldserver.com/turk/computergraphics/FixedSqrt.pdf

*/

#ifndef Q_CC_BOR

QFixedPoint sqrt(QFixedPoint  x)
{
    Q_ASSERT(QFixedPoint::HighBits + QFixedPoint::LowBits == 32);
    Q_ASSERT(QFixedPoint::LowBits/2*2 == QFixedPoint::LowBits);

    if (x <= 0)
        return 0;

    uint root = 0;

    uint rem_hi = 0;
    uint rem_lo = x.value();
    int count  = QFixedPoint::HighBits/2 + QFixedPoint::LowBits;
    do
    {
        rem_hi   = ( rem_hi << 2 ) | ( rem_lo >> 30 );
        rem_lo <<= 2;
        root   <<= 1;
        uint test_div = ( root << 1 ) + 1;

        if ( rem_hi >= test_div )
        {
            rem_hi -= test_div;
            root   += 1;
        }
    } while ( --count );

    return QFixedPoint(root, QFixedPoint::FixedPoint);
}

#if defined (__i386__) && !defined (Q_CC_BOR)
inline double qCos(double a)
{
    double r;
    __asm__ (
        "fcos"
        : "=t" (r) : "0" (a));
    return r;
}

inline double qSin(double a)
{
    double r;
    __asm__ (
        "fsin"
        : "=t" (r) : "0" (a));
    return r;
}
#else
static const double Q_PI   = 3.14159265358979323846;   // pi
static const double Q_2PI  = 6.28318530717958647693;   // 2*pi
static const double Q_PI2  = 1.57079632679489661923;   // pi/2

inline double qSinCos(double a, bool calcCos=false)
{
    if (calcCos)                              // calculate cosine
        a -= Q_PI2;
    if (a >= Q_2PI || a <= -Q_2PI) {          // fix range: -2*pi < a < 2*pi
        int m = (int)(a/Q_2PI);
        a -= Q_2PI*m;
    }
    if (a < 0.0)                              // 0 <= a < 2*pi
        a += Q_2PI;
    int sign = a > Q_PI ? -1 : 1;
    if (a >= Q_PI)
        a = Q_2PI - a;
    if (a >= Q_PI2)
        a = Q_PI - a;
    if (calcCos)
        sign = -sign;
    double a2  = a*a;                           // here: 0 <= a < pi/4
    double a3  = a2*a;                          // make taylor sin sum
    double a5  = a3*a2;
    double a7  = a5*a2;
    double a9  = a7*a2;
    double a11 = a9*a2;
    return (a-a3/6+a5/120-a7/5040+a9/362880-a11/39916800)*sign;
}
#define qSin(a) qSinCos(a, false)
#define qCos(a) qSinCos(a, true)
#endif

QFixedPoint sin(QFixedPoint f)
{
    return qSin(f.toDouble());
}
QFixedPoint cos(QFixedPoint f)
{
    return qCos(f.toDouble());
}

#if defined (__i386__) && !defined (Q_CC_BOR)
inline double qACos(double x)
{
    // #####
    if (qAbs(x) > 1.)
        return 0;

    register double z;
    asm("fmulp":  "=t" (z) : "0" (1+x), "u" (1-x) );
    asm("fsqrt": "=t" (z) : "0" (z) );
    asm("fpatan": "=t" (z) : "0" (1.0), "u" (z) );
    return z;
}
QFixedPoint acos(QFixedPoint f)
{
    return QFixedPoint(qACos(f.toDouble()));
}
#else
QFixedPoint acos(QFixedPoint f)
{
    return QFixedPoint(acos(f.toDouble()));
}
#endif

QFixedPointLong &QFixedPointLong::operator/=(const QFixedPointLong &o) {
    if (o.val == 0) {
        val = Q_INT64_C(0x7FFFFFFFFFFFFFFF);
    } else {
        bool neg = false;
        qint64 ah = val >> 32;
        quint64 al = ((quint64)val) & Q_UINT64_C(0xffffffff);

        qint64 b = o.val;
        if (ah < 0) { ah = -ah; neg = true; }
        if (b < 0) { b = -b; neg = !neg; }

        qint64 res = (((ah << LowBits) / b) << 32)
                      + (((al << LowBits) + (b >> 1)) / b);

        val = (neg ? -res : res);
    }
    return *this;
}

QFixedPointLong &QFixedPointLong::operator*=(const QFixedPointLong &o) {
    bool neg = false;
    qint64 ah = val >> 32;
    quint64 al = ((quint64)val) & Q_UINT64_C(0xffffffff);
    qint64 b = o.val;
    if (ah < 0) { ah = -ah; neg = true; }
    if (b < 0) { b = -b; neg = !neg; }

    qint64 res = (ah * b) + ((al * b + 0x80000000L) >> 32);
    val = neg ? -res : res;
    return *this;
}


QFixedPointLong sqrt(QFixedPointLong x)
{
    Q_ASSERT(QFixedPoint::HighBits + QFixedPoint::LowBits == 64);
    Q_ASSERT(QFixedPoint::LowBits/2*2 == QFixedPoint::LowBits);

    if (x.value() <= 0)
        return 0;

    uint root = 0;

    quint64 rem_hi = 0;
    quint64 rem_lo = x.value();
    int count  = QFixedPoint::HighBits/2 + QFixedPoint::LowBits;
    do
    {
        rem_hi   = ( rem_hi << 2 ) | ( rem_lo >> 30 );
        rem_lo <<= 2;
        root   <<= 1;
        quint64 test_div = ( root << 1 ) + 1;

        if ( rem_hi >= test_div )
        {
            rem_hi -= test_div;
            root   += 1;
        }
    } while ( --count );

    return QFixedPointLong(root, QFixedPointLong::FixedPoint);
}

QFixedPointLong sin(QFixedPointLong f)
{
    return qSin(f.toDouble());
}
QFixedPointLong cos(QFixedPointLong f)
{
    return qCos(f.toDouble());
}

#if defined (__i386__) && !defined (Q_CC_BOR)
QFixedPointLong acos(QFixedPointLong f)
{
    return QFixedPointLong(qACos(f.toDouble()));
}
#else
QFixedPointLong acos(QFixedPointLong f)
{
    return QFixedPointLong(acos(f.toDouble()));
}
#endif

#endif // Q_CC_BOR
