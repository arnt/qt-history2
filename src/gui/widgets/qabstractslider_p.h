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

#ifndef QABSTRACTSLIDER_P_H
#define QABSTRACTSLIDER_P_H

#ifndef QT_H
#include <private/qwidget_p.h>
#include "qbasictimer.h"
#endif // QT_H

class QAbstractSliderPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QAbstractSlider)
public:
    QAbstractSliderPrivate();
    ~QAbstractSliderPrivate();

    void setSteps(int single, int page);

    int minimum, maximum, singleStep, pageStep, value, position;
    uint tracking : 1;
    uint blocktracking :1;
    uint pressed : 1;
    uint invertedAppearance : 1;
    uint invertedControls : 1;
    Qt::Orientation orientation;

    QBasicTimer repeatActionTimer;
    int repeatActionTime;
    QAbstractSlider::SliderAction repeatAction;

    inline int bound(int val) const { return qMax(minimum, qMin(maximum, val)); }
};

#endif
