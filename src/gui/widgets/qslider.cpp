/****************************************************************************
**
** Implementation of QSlider class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qslider.h"
#ifndef QT_NO_SLIDER
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif
#include "qapplication.h"
#include "qbitmap.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qtimer.h"
#include "private/qabstractslider_p.h"

static const int thresholdTime = 300;
static const int repeatTime = 100;

class QSliderPrivate : public QAbstractSliderPrivate
{
    Q_DECL_PUBLIC(QSlider);
public:
    uint pressedControl;
    int tickInterval;
    QSlider::TickSetting tickSetting;
    int clickOffset;
    int snapBackPosition;
    void init();
    int pixelPosToRangeValue(int pos) const;
    inline int pick(const QPoint &pt) const;
};

#define d d_func()
#define q q_func()

void QSliderPrivate::init()
{
    pressedControl = QStyle::SC_None;
    tickInterval = 0;
    tickSetting = QSlider::NoMarks;
    q->setFocusPolicy(QWidget::TabFocus);
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Fixed);
    if (orientation == Vertical)
        sp.transpose();
    q->setSizePolicy(sp);
    q->clearWState(WState_OwnSizePolicy);
}


int QSliderPrivate::pixelPosToRangeValue(int pos) const
{
    QRect gr = q->style().querySubControlMetrics(QStyle::CC_Slider, q, QStyle::SC_SliderGroove);
    QRect sr = q->style().querySubControlMetrics(QStyle::CC_Slider, q, QStyle::SC_SliderHandle);
    int sliderMin, sliderMax, sliderLength;

    if (orientation == Horizontal) {
	sliderLength = sr.width();
	sliderMin = gr.x();
	sliderMax = gr.right() - sliderLength + 1;
    } else {
	sliderLength = sr.height();
	sliderMin = gr.y();
	sliderMax = gr.bottom() - sliderLength + 1;
    }
    return QStyle::valueFromPosition(d->minimum, d->maximum, pos - sliderMin, sliderMax - sliderMin);
}

inline int QSliderPrivate::pick(const QPoint &pt) const
{
    return orientation == Horizontal ? pt.x() : pt.y();
}

/*!
    \class QSlider
    \brief The QSlider widget provides a vertical or horizontal slider.

    \ingroup basic
    \mainclass

    The slider is the classic widget for controlling a bounded value.
    It lets the user move a slider handle along a horizontal or vertical
    groove and translates the handle's position into an integer value
    within the legal range.

    QSlider has very few of its own functions; most of the functionality is in
    QAbstractSlider. The most useful functions are setValue() to set
    the slider directly to some value; triggerAction() to simulate
    the effects of clicking (useful for accelerator keys);
    setSingleStep(), setPageStep() to set the steps; and setMinimum()
    and setMaximum() to define the range of the scroll bar.

    QSlider provides methods for controlling tickmarks.
    You can use setTickmarks() to indicate where
    you want the tickmarks to be, setTickInterval() to indicate how
    many of them you want.

    QSlider inherits a comprehensive set of signals:
    \table
    \header \i Signal \i Emitted when
    \row \i \l valueChanged()
    \i the slider's value has changed. The tracking()
       determines whether this signal is emitted during user
       interaction.
    \row \i \l sliderPressed()
    \i the user starts to drag the slider.
    \row \i \l sliderMoved()
    \i the user drags the slider.
    \row \i \l sliderReleased()
    \i the user releases the slider.
    \endtable


    QSlider only provides integer ranges. Note that although
    QSlider handles very large numbers, it becomes difficult for users
    to effectivley use a slider for very large ranges.

    A slider accepts focus on Tab and provides both a mouse wheel and a
    keyboard interface. The keyboard interface is the following:

    \list
        \i Left/Right move a horizontal slider by one single step.
        \i Up/Down move a vertical slider by one single step.
        \i PageUp moves up one page.
        \i PageDown moves down one page.
        \i Home moves to the start (mininum).
        \i End moves to the end (maximum).
    \endlist

    <img src=qslider-m.png> <img src=qslider-w.png>

    \important setRange

    \sa QScrollBar QSpinBox
    \link guibooks.html#fowler GUI Design Handbook: Slider\endlink
*/


/*!
    \enum QSlider::TickSetting

    This enum specifies where the tickmarks are to be drawn relative
    to the slider's groove and the handle the user moves.

    \value NoMarks do not draw any tickmarks.
    \value Both draw tickmarks on both sides of the groove.
    \value Above draw tickmarks above the (horizontal) slider
    \value Below draw tickmarks below the (horizontal) slider
    \value Left draw tickmarks to the left of the (vertical) slider
    \value Right draw tickmarks to the right of the (vertical) slider
*/


/*!
    Constructs a vertical slider.

    The \a parent argument is sent to the QAbstractSlider
    constructor.
*/
QSlider::QSlider(QWidget *parent)
    : QAbstractSlider(*new QSliderPrivate, parent)
{
    d->orientation = Vertical;
    d->init();
}

/*!
    Constructs a slider.

    The \a orientation must be \l Qt::Vertical or \l Qt::Horizontal.

    The \a parent argument is sent on to the QAbstractSlider
    constructor.
*/

QSlider::QSlider(Orientation orientation, QWidget *parent)
    : QAbstractSlider(*new QSliderPrivate, parent)
{
    d->orientation = orientation;
    d->init();
}

#ifdef QT_COMPAT
QSlider::QSlider(QWidget *parent, const char *name)
    : QAbstractSlider(*new QSliderPrivate, parent)
{
    setObjectName(name);
    d->orientation = Vertical;
    d->init();
}
QSlider::QSlider(Orientation orientation, QWidget *parent, const char *name)
    : QAbstractSlider(*new QSliderPrivate, parent)
{
    setObjectName(name);
    d->orientation = orientation;
    d->init();
}

QSlider::QSlider(int minValue, int maxValue, int pageStep, int value, Orientation orientation,
                 QWidget *parent, const char *name)
    : QAbstractSlider(*new QSliderPrivate, parent)
{
    setObjectName(name);
    d->minimum = minValue;
    d->maximum = maxValue;
    d->pageStep = pageStep;
    d->value = value;
    d->orientation = orientation;
    d->init();
}
#endif

/*!
    Destructor.
*/
QSlider::~QSlider()
{
}

/*!
    \reimp
*/
void QSlider::paintEvent(QPaintEvent *)
{
    QPainter p( this );

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
	flags |= QStyle::Style_Enabled;
    if (hasFocus())
	flags |= QStyle::Style_HasFocus;

    QStyle::SCFlags sub = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
    if (d->tickSetting != NoMarks)
	sub |= QStyle::SC_SliderTickmarks;

    style().drawComplexControl(QStyle::CC_Slider, &p, this, rect(), palette(),
                               flags, sub, d->pressedControl);
}

/*!
    \reimp
*/
void QSlider::mousePressEvent(QMouseEvent *ev)
{
    if (d->maximum == d->minimum || (ev->state() & MouseButtonMask)
        || (ev->button() != LeftButton)) {
        ev->ignore();
        return;
    }
    ev->accept();
    d->pressedControl = style().querySubControl(QStyle::CC_Slider, this, ev->pos());
    SliderAction action = SliderNoAction;
    if (d->pressedControl == QStyle::SC_SliderGroove) {
        int pressValue = d->pixelPosToRangeValue(d->pick(ev->pos()));
        if (pressValue > d->value)
            action = SliderPageStepAdd;
        else if (pressValue < d->value)
            action = SliderPageStepSub;

        if (action) {
            triggerAction(action);
            setRepeatAction(action);
        }
    } else if (d->pressedControl == QStyle::SC_SliderHandle) {
        QRect sr = style().querySubControlMetrics(QStyle::CC_Slider, this, QStyle::SC_SliderHandle);
        d->clickOffset = d->pick(ev->pos() - sr.topLeft());
	d->snapBackPosition = d->position;
        update(sr);
    }
}

/*!
    \reimp
*/
void QSlider::mouseMoveEvent(QMouseEvent *ev)
{
    if (d->pressedControl != QStyle::SC_SliderHandle || !(ev->state() & LeftButton)) {
        ev->ignore();
        return;
    }
    int newPosition = d->pixelPosToRangeValue(d->pick(ev->pos()) - d->clickOffset);
    int m = style().pixelMetric(QStyle::PM_MaximumDragDistance, this);
    if (m >= 0) {
        QRect r = rect();
        r.addCoords(-m, -m, m, m);
        if (!r.contains(ev->pos()))
            newPosition = d->snapBackPosition;
    }
    setSliderPosition(newPosition);
}


/*!
    \reimp
*/
void QSlider::mouseReleaseEvent(QMouseEvent *ev)
{
    if (d->pressedControl == QStyle::SC_None || ev->stateAfter() & MouseButtonMask) {
        ev->ignore();
        return;
    }
    ev->accept();
    QStyle::SubControl oldPressed = (QStyle::SubControl)d->pressedControl;
    d->pressedControl = QStyle::SC_None;
    setRepeatAction(SliderNoAction);
    if (oldPressed == QStyle::SC_SliderHandle)
        setSliderDown(false);
    update(style().querySubControlMetrics(QStyle::CC_Slider, this, oldPressed));
}



/*!
    \reimp
*/
void QSlider::keyPressEvent(QKeyEvent *ev)
{
    SliderAction action = SliderNoAction;
    switch (ev->key()) {
        case Key_Left:
            if (d->orientation == Horizontal)
                action = SliderSingleStepSub;
            break;
        case Key_Right:
            if (d->orientation == Horizontal)
                action = SliderSingleStepAdd;
            break;
        case Key_Up:
            if (d->orientation == Vertical)
                action = SliderSingleStepSub;
            break;
        case Key_Down:
            if (d->orientation == Vertical)
                action = SliderSingleStepAdd;
            break;
        case Key_PageUp:
            action = SliderPageStepSub;
            break;
        case Key_PageDown:
            action = SliderPageStepAdd;
            break;
        case Key_Home:
            action = SliderToMinimum;
            break;
        case Key_End:
            action = SliderToMaximum;
            break;
        default:
            ev->ignore();
            break;
    }
    if (action)
	triggerAction(action);
}

/*!
    \reimp
*/
QSize QSlider::sizeHint() const
{
    ensurePolished();
    const int SliderLength = 84, TickSpace = 5;
    int thick = style().pixelMetric(QStyle::PM_SliderThickness, this);
    if (d->tickSetting & Above)
	thick += TickSpace;
    if (d->tickSetting & Below )
	thick += TickSpace;
    int w = thick, h = SliderLength;
    if (d->orientation == Horizontal) {
	w = SliderLength;
	h = thick;
    }
    return style().sizeFromContents(QStyle::CT_Slider, this,
                                    QSize(w, h)).expandedTo(QApplication::globalStrut());
}

/*!
    \reimp
*/
QSize QSlider::minimumSizeHint() const
{
    QSize s = sizeHint();
    int length = style().pixelMetric(QStyle::PM_SliderLength, this);
    if (d->orientation == Horizontal)
	s.setWidth(length);
    else
	s.setHeight(length);
    return s;
}

/*!
    \property QSlider::tickmarks
    \brief the tickmark settings for this slider

    The valid values are in \l{QSlider::TickSetting}. The default is
    \c NoMarks.

    \sa tickInterval
*/

void QSlider::setTickmarks(TickSetting ts)
{
    d->tickSetting = ts;
    update();
}

QSlider::TickSetting QSlider::tickmarks() const
{
    return d->tickSetting;
}

/*!
    \property QSlider::tickInterval
    \brief the interval between tickmarks

    This is a value interval, not a pixel interval. If it is 0, the
    slider will choose between lineStep() and pageStep(). The initial
    value of tickInterval is 0.

    \sa QRangeControl::lineStep(), QRangeControl::pageStep()
*/

void QSlider::setTickInterval(int ts)
{
    d->tickInterval = qMax(0, ts);
    update();
}

int QSlider::tickInterval() const
{
    return d->tickInterval;
}


#endif
