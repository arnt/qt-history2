#ifndef __QMATH_P_H__
#define __QMATH_P_H__

#include <math.h>

/*****************************************************************************
  Trigonometric function for QPainter

  We have implemented simple sine and cosine function that are called from
  QPainter::drawPie() and QPainter::drawChord() when drawing the outline of
  pies and chords.
  These functions are slower and less accurate than math.h sin() and cos(),
  but with still around 1/70000th sec. execution time (on a 486DX2-66) and
  8 digits accuracy, it should not be the bottleneck in drawing these shapes.
  The advantage is that you don't have to link in the math library.
 *****************************************************************************/

static const double Q_PI   = 3.14159265358979323846;   // pi
static const double Q_2PI  = 6.28318530717958647693;   // 2*pi
static const double Q_PI2  = 1.57079632679489661923;   // pi/2

#ifdef Q_WS_X11
#if defined(Q_CC_GNU) && defined(Q_OS_AIX)
// AIX 4.2 gcc 2.7.2.3 gets internal error.
inline int qRoundAIX(double d)
{
    return qRound(d);
}
#define qRound qRoundAIX
#endif


#if defined(Q_CC_GNU) && defined(__i386__)

inline double qcos_x86(double a)
{
    double r;
    __asm__ (
        "fcos"
        : "=t" (r) : "0" (a));
    return(r);
}
#define qcos qcos_x86

inline double qsin_x86(double a)
{
    double r;
    __asm__ (
        "fsin"
        : "=t" (r) : "0" (a));
    return(r);
}
#define qsin qsin_x86

#else //GNU_CC && I386

inline double qsincos(double a, bool calcCos=false)
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
#define qsin(a) qsincos(a, false)
#define qcos(a) qsincos(a, true)

#endif
#endif //WS_X11

#ifndef qsin
# define qsin sin
#endif
#ifndef qcos
# define qcos cos
#endif

#endif /* __QMATH_P_H__ */
