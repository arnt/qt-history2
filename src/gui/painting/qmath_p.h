/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMATH_P_H
#define QMATH_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <math.h>

static const double Q_PI   = 3.14159265358979323846;   // pi
static const double Q_2PI  = 6.28318530717958647693;   // 2*pi
static const double Q_PI2  = 1.57079632679489661923;   // pi/2

inline int qCeil(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS
    if (sizeof(qreal) == sizeof(float))
        return int(ceilf(v));
    else
#endif
        return int(ceil(v));
}

inline int qFloor(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS
    if (sizeof(qreal) == sizeof(float))
        return int(floorf(v));
    else
#endif
        return int(floor(v));
}

inline qreal qSin(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS
    if (sizeof(qreal) == sizeof(float))
        return sinf(v);
    else
#endif
        return sin(v);
}

inline qreal qCos(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS
    if (sizeof(qreal) == sizeof(float))
        return cosf(v);
    else
#endif
        return cos(v);
}

#endif // QMATH_P_H
