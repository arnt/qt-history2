/****************************************************************************
**
** Definition of some Qt private functions.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTSLIDER_P_H
#define QABSTRACTSLIDER_P_H

#ifndef QT_H
#include <private/qwidget_p.h>
#include "qbasictimer.h"
#endif // QT_H

class QAbstractSliderPrivate : public QWidgetPrivate
{
    Q_DECL_PUBLIC( QAbstractSlider );
public:
    QAbstractSliderPrivate();
    ~QAbstractSliderPrivate();

    int minimum, maximum, singleStep, pageStep, value, position;
    uint tracking : 1;
    uint blocktracking :1;
    uint pressed : 1;
    Orientation orientation;

    QBasicTimer repeatActionTimer;
    int repeatActionTime;
    QAbstractSlider::SliderAction repeatAction;

    inline int bound(int val){ return qMax(minimum, qMin(maximum, val)); }
};

#endif
