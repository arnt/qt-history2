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

#ifndef QABSTRACTSLIDER_P_H
#define QABSTRACTSLIDER_P_H

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

#include "QtCore/qbasictimer.h"
#include "private/qwidget_p.h"
#include "qstyle.h"

QT_BEGIN_NAMESPACE

class QAbstractSliderPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QAbstractSlider)
public:
    QAbstractSliderPrivate();
    ~QAbstractSliderPrivate();

    void setSteps(int single, int page);

    int minimum, maximum, singleStep, pageStep, value, position, pressValue;
    uint tracking : 1;
    uint blocktracking :1;
    uint pressed : 1;
    uint invertedAppearance : 1;
    uint invertedControls : 1;
    Qt::Orientation orientation;

    QBasicTimer repeatActionTimer;
    int repeatActionTime;
    QAbstractSlider::SliderAction repeatAction;
    
#ifdef QT_KEYPAD_NAVIGATION
    int origValue;
#endif

    inline int bound(int val) const { return qMax(minimum, qMin(maximum, val)); }
    inline void setAdjustedSliderPosition(int position)
    {
        Q_Q(QAbstractSlider);
        if (q->style()->styleHint(QStyle::SH_Slider_StopMouseOverSlider, 0, q)) {
            if ((position > pressValue - 2 * pageStep) && (position < pressValue + 2 * pageStep)) {
                repeatAction = QAbstractSlider::SliderNoAction;
                position = pressValue;
            }
        }
        q->setSliderPosition(position);
    }
};

QT_END_NAMESPACE

#endif // QABSTRACTSLIDER_P_H
