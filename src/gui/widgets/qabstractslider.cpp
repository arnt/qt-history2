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

#include <qapplication.h>
#include "qabstractslider.h"
#include "qevent.h"
#include "qabstractslider_p.h"
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#include <limits.h>

/*!
    \class QAbstractSlider
    \brief The QAbstractSlider class provides an integer value within a range.

    \ingroup abstractwidgets

    The class is designed as a common super class for widgets like
    QScrollBar, QSlider and QDial.

    Here are the main properties of the class:

    \list 1

    \i \l value: The bounded integer that QAbstractSlider maintains.

    \i \l minimum: The lowest possible value.

    \i \l maximum: The highest possible value.

    \i \l singleStep: The smaller of two natural steps that an
    abstract sliders provides and typically corresponds to the user
    pressing an arrow key.

    \i \l pageStep: The larger of two natural steps that an abstract
    slider provides and typically corresponds to the user pressing
    PageUp or PageDown.

    \i \l tracking: Whether slider tracking is enabled.
    
    \i \l sliderPosition: The current position of the slider. If \l
    tracking is enabled (the default), this is identical to \l value.

    \endlist

    Unity (1) may be viewed as a third step size. setValue() lets you
    set the current value to any integer in the allowed range, not
    just minimum() + \e n * singleStep() for integer values of \e n.
    Some widgets may allow the user to set any value at all; others
    may just provide multiples of singleStep() or pageStep().

    QAbstractSlider emits a comprehensive set of signals:

    \table
    \header \i Signal \i Emitted when
    \row \i \l valueChanged()
         \i the value has changed. The \l tracking
            determines whether this signal is emitted during user
            interaction.
    \row \i \l sliderPressed()
         \i the user starts to drag the slider.
    \row \i \l sliderMoved()
         \i the user drags the slider.
    \row \i \l sliderReleased()
         \i the user releases the slider.
    \row \i \l actionTriggered()
         \i a slider action was triggerd.
    \row \i \l rangeChanged()
         \i a the range has changed.
    \endtable

    QAbstractSlider provides a virtual sliderChange() function that is
    well suited for updating the on-screen representation of
    sliders. By calling triggerAction(), subclasses trigger slider
    actions. Two helper functions QStyle::sliderPositionFromValue() and
    QStyle::sliderValueFromPosition() help subclasses and styles to map
    screen coordinates to logical range values.

*/

/*!
    \enum QAbstractSlider::SliderAction

    \value SliderNoAction
    \value SliderSingleStepAdd
    \value SliderSingleStepSub
    \value SliderPageStepAdd
    \value SliderPageStepSub
    \value SliderToMinimum
    \value SliderToMaximum
    \value SliderMove

*/

/*!
    \fn void QAbstractSlider::valueChanged(int value)

    This signal is emitted when the slider value has changed, with the
    new slider \a value as argument.
*/

/*!
    \fn void QAbstractSlider::sliderPressed()

    This signal is emitted when the user presses the slider with the
    mouse.
*/

/*!
    \fn void QAbstractSlider::sliderMoved(int value)

    This signal is emitted when the slider is dragged by the user, with
    the new slider \a value as an argument.

    This signal is emitted even when tracking is turned off.

    \sa setTracking(), valueChanged()
*/

/*!
    \fn void QAbstractSlider::sliderReleased()

    This signal is emitted when the user releases the slider with the
    mouse.
*/

/*!
    \fn void QAbstractSlider::rangeChanged(int min, int max)

    This signal is emitted when the slider range has changed, with \a
    min being the new minimum, and \a max being the new maximum.

    \sa minimum, maximum
*/

/*!
    \fn void QAbstractSlider::actionTriggered(int action)

    This signal is emitted when the slider action \a action is
    triggered. Actions are \c SliderSingleStepAdd, \c
    SliderSingleStepSub, \c SliderPageStepAdd, \c SliderPageStepSub,
    \c SliderToMinimum, \c SliderToMaximum, and \c SliderMove.

    When the signal is emitted, the \l sliderPosition has been
    adjusted according to the action, but the \l value has not yet
    been propagated (meaning the valueChanged() signal was not yet
    emitted), and the visual display has not been updated. In slots
    connected to this signal you can thus safely adjust any action by
    calling setSliderPosition() yourself, based on both the action and
    the slider's value.

    \sa triggerAction()
*/

/*!
    \enum QAbstractSlider::SliderChange

    \value SliderRangeChange
    \value SliderOrientationChange
    \value SliderStepsChange
    \value SliderValueChange
*/

QAbstractSliderPrivate::QAbstractSliderPrivate()
    : minimum(0), maximum(99), singleStep(1), pageStep(10),
      value(0), position(0), tracking(true), blocktracking(false), pressed(false),
      invertedAppearance(false), invertedControls(false),
      orientation(Qt::Horizontal), repeatAction(QAbstractSlider::SliderNoAction)
{
}

QAbstractSliderPrivate::~QAbstractSliderPrivate()
{
}

/*!
    Sets the slider's minimum to \a min and its maximum to \a max.

    If \a max is smaller than \a min, \a min becomes the only legal
    value.

    \sa minimum maximum
*/
void QAbstractSlider::setRange(int min, int max)
{
    Q_D(QAbstractSlider);
    d->minimum = min;
    d->maximum = qMax(min, max);
    sliderChange(SliderRangeChange);
    emit rangeChanged(d->minimum, d->maximum);
    setValue(d->value); // re-bound
}


void QAbstractSliderPrivate::setSteps(int single, int page)
{
    Q_Q(QAbstractSlider);
    singleStep = qAbs(single);
    pageStep = qAbs(page);
    q->sliderChange(QAbstractSlider::SliderStepsChange);
}


/*!
    Constructs an abstract slider.

    The \a parent arguments is sent to the QWidget constructor.

    The \l minimum defaults to 0, the \l maximum to 99, with a \l
    singleStep size of 1 and a \l pageStep size of 10, and an initial
    \l value of 0.
*/
QAbstractSlider::QAbstractSlider(QWidget *parent)
    :QWidget(*new QAbstractSliderPrivate, parent, 0)
{
}

/*! \internal */
QAbstractSlider::QAbstractSlider(QAbstractSliderPrivate &dd, QWidget *parent)
    :QWidget(dd, parent, 0)
{
}

/*!
    Destroys the slider.
*/
QAbstractSlider::~QAbstractSlider()
{
}


/*!
    \property QAbstractSlider::orientation
    \brief the orientation of the slider

    The orientation must be \l Qt::Vertical (the default) or \l
    Qt::Horizontal.
*/

void QAbstractSlider::setOrientation(Qt::Orientation orientation)
{
    Q_D(QAbstractSlider);
    d->orientation = orientation;
    if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy(sp);
        setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }
    update();
    updateGeometry();
}

Qt::Orientation QAbstractSlider::orientation() const
{
    Q_D(const QAbstractSlider);
    return d->orientation;
}


/*!
    \property QAbstractSlider::minimum
    \brief the sliders's minimum value

    When setting this property, the \l maximum is adjusted if
    necessary to ensure that the range remains valid. Also the
    slider's current value is adjusted to be within the new range.

*/

void QAbstractSlider::setMinimum(int min)
{
    Q_D(QAbstractSlider);
    setRange(min, qMax(d->maximum, min));
}

int QAbstractSlider::minimum() const
{
    Q_D(const QAbstractSlider);
    return d->minimum;
}


/*!
    \property QAbstractSlider::maximum
    \brief the slider's maximum value

    When setting this property, the \l minimum is adjusted if
    necessary to ensure that the range remains valid.  Also the
    slider's current value is adjusted to be within the new range.


*/

void QAbstractSlider::setMaximum(int max)
{
    Q_D(QAbstractSlider);
    setRange(qMin(d->minimum, max), max);
}

int QAbstractSlider::maximum() const
{
    Q_D(const QAbstractSlider);
    return d->maximum;
}



/*!
    \property QAbstractSlider::singleStep
    \brief the single step.

    The smaller of two natural steps that an
    abstract sliders provides and typically corresponds to the user
    pressing an arrow key.

    \sa pageStep
*/

void QAbstractSlider::setSingleStep(int step)
{
    Q_D(QAbstractSlider);
    d->setSteps(step, d->pageStep);
}

int QAbstractSlider::singleStep() const
{
    Q_D(const QAbstractSlider);
    return d->singleStep;
}


/*!
    \property QAbstractSlider::pageStep
    \brief the page step.

    The larger of two natural steps that an abstract slider provides
    and typically corresponds to the user pressing PageUp or PageDown.

    \sa singleStep
*/

void QAbstractSlider::setPageStep(int step)
{
    Q_D(QAbstractSlider);
    d->setSteps(d->singleStep, step);
}

int QAbstractSlider::pageStep() const
{
    Q_D(const QAbstractSlider);
    return d->pageStep;
}

/*!
    \property QAbstractSlider::tracking
    \brief whether slider tracking is enabled

    If tracking is enabled (the default), the slider emits the
    valueChanged() signal while the slider is being dragged. If
    tracking is disabled, the slider emits the valueChanged() signal
    only when the user releases the slider.

    \sa sliderDown
*/
void QAbstractSlider::setTracking(bool enable)
{
    Q_D(QAbstractSlider);
    d->tracking = enable;
}

bool QAbstractSlider::hasTracking() const
{
    Q_D(const QAbstractSlider);
    return d->tracking;
}


/*!
    \property QAbstractSlider::sliderDown
    \brief whether the slider is pressed down.

    The property is set by subclasses in order to let the abstract
    slider know whether or not \l tracking has any effect.

    Changing the slider down property emits the sliderPressed() and
    sliderReleased() signals.

*/
void QAbstractSlider::setSliderDown(bool down)
{
    Q_D(QAbstractSlider);
    d->pressed = down;
    if (!down && d->position != d->value)
        triggerAction(SliderMove);
}

bool QAbstractSlider::isSliderDown() const
{
    Q_D(const QAbstractSlider);
    return d->pressed;
}


/*!
    \property QAbstractSlider::sliderPosition
    \brief the current slider position

    If \l tracking is enabled (the default), this is identical to \l value.
*/
void QAbstractSlider::setSliderPosition(int position)
{
    Q_D(QAbstractSlider);
    position = d->bound(position);
    if (position == d->position)
        return;
    d->position = position;
    if (!d->blocktracking)
        repaint();
    emit sliderMoved(position);
    if (d->tracking && !d->blocktracking)
        triggerAction(SliderMove);
}

int QAbstractSlider::sliderPosition() const
{
    Q_D(const QAbstractSlider);
    return d->position;
}


/*!
    \property QAbstractSlider::value
    \brief the slider's current value

    The slider forces the value to be within the legal range: \l
    minimum <= \c value <= \l maximum.

    Changing the value also changes the \l sliderPosition.
*/


int QAbstractSlider::value() const
{
    Q_D(const QAbstractSlider);
    return d->value;
}

void QAbstractSlider::setValue(int value)
{
    Q_D(QAbstractSlider);
    value = d->bound(value);
    if (d->value == value)
        return;
    d->value = value;
    if (d->position != value)
        emit sliderMoved((d->position = value));
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::ValueChanged);
#endif
    sliderChange(SliderValueChange);
    emit valueChanged(value);
}

/*!
    \property QAbstractSlider::invertedAppearance
    \brief whether or not a slider shows its values inverted.

    If this property is false (the default), the minimum and maximum will
    be shown in its classic position for the inherited widget. If the
    value is true, the minimum and maximum appear at their opposite location.
*/

bool QAbstractSlider::invertedAppearance() const
{
    Q_D(const QAbstractSlider);
    return d->invertedAppearance;
}

void QAbstractSlider::setInvertedAppearance(bool invert)
{
    Q_D(QAbstractSlider);
    d->invertedAppearance = invert;
    update();
}


/*!
    \property QAbstractSlider::invertedControls
    \brief whether or not the slider inverts its wheel and key events.

    If this property is false, scrolling the mouse wheel "up" and using keys
    like page up will increase the slider's value towards its maximum. Otherwise
    pressing page up will move value towards the slider's minimum.
*/


bool QAbstractSlider::invertedControls() const
{
    Q_D(const QAbstractSlider);
    return d->invertedControls;
}

void QAbstractSlider::setInvertedControls(bool invert)
{
    Q_D(QAbstractSlider);
    d->invertedControls = invert;
}

/*!  Triggers a slider \a action.  Possible actions are \c
  SliderSingleStepAdd, \c SliderSingleStepSub, \c SliderPageStepAdd,
  \c SliderPageStepSub, \c SliderToMinimum, \c SliderToMaximum, and \c
  SliderMove.

  \sa actionTriggered()
 */
void QAbstractSlider::triggerAction(SliderAction action)
{
    Q_D(QAbstractSlider);
    d->blocktracking = true;
    switch (action) {
    case SliderSingleStepAdd:
        setSliderPosition(d->value + d->singleStep);
        break;
    case SliderSingleStepSub:
        setSliderPosition(d->value - d->singleStep);
        break;
    case SliderPageStepAdd:
        setSliderPosition(d->value + d->pageStep);
        break;
    case SliderPageStepSub:
        setSliderPosition(d->value - d->pageStep);
        break;
    case SliderToMinimum:
        setSliderPosition(d->minimum);
        break;
    case SliderToMaximum:
        setSliderPosition(d->maximum);
        break;
    case SliderMove:
    case SliderNoAction:
        break;
    };
    emit actionTriggered(action);
    d->blocktracking = false;
    setValue(d->position);
}

/*!  Sets action \a action to be triggered repetitively in intervals
of \a repeatTime, after an initial delay of \a thresholdTime.

\sa triggerAction() repeatAction()
 */
void QAbstractSlider::setRepeatAction(SliderAction action, int thresholdTime, int repeatTime)
{
    Q_D(QAbstractSlider);
    if ((d->repeatAction = action) == SliderNoAction) {
        d->repeatActionTimer.stop();
    } else {
        d->repeatActionTime = repeatTime;
        d->repeatActionTimer.start(thresholdTime, this);
    }
}

/*!
  Returns the current repeat action.
  \sa setRepeatAction()
 */
QAbstractSlider::SliderAction QAbstractSlider::repeatAction() const
{
    Q_D(const QAbstractSlider);
    return d->repeatAction;
}

/*!\reimp
 */
void QAbstractSlider::timerEvent(QTimerEvent *e)
{
    Q_D(QAbstractSlider);
    if (e->timerId() == d->repeatActionTimer.timerId()) {
        if (d->repeatActionTime) { // was threshold time, use repeat time next time
            d->repeatActionTimer.start(d->repeatActionTime, this);
            d->repeatActionTime = 0;
        }
        triggerAction(d->repeatAction);
    }
}

/*!
    Reimplement this virtual function to track slider changes such as
    \c SliderRangeChange, \c SliderOrientationChange, \c
    SliderStepsChange, or \c SliderValueChange. The default
    implementation only updates the display and ignores the \a change
    parameter.
 */
void QAbstractSlider::sliderChange(SliderChange)
{
    update();
}


/*!
    \reimp
*/
void QAbstractSlider::wheelEvent(QWheelEvent * e)
{
    Q_D(QAbstractSlider);
    if (e->orientation() != d->orientation && !rect().contains(e->pos()))
        return;

    static qreal offset = 0;
    static QAbstractSlider *offset_owner = 0;
    if (offset_owner != this){
        offset_owner = this;
        offset = 0;
    }

    int step = qMin(QApplication::wheelScrollLines() * d->singleStep, d->pageStep);
    if ((e->modifiers() & Qt::ControlModifier) || (e->modifiers() & Qt::ShiftModifier))
        step = d->pageStep;
    offset += e->delta() * step / 120;
    if (d->invertedControls)
        offset = -offset;
    if (qAbs(offset) < 1)
        return;
    setValue(d->value + int(offset));
    offset -= int(offset);
    e->accept();
}


/*!
    \reimp
*/
void QAbstractSlider::keyPressEvent(QKeyEvent *ev)
{
    Q_D(QAbstractSlider);
    SliderAction action = SliderNoAction;
    switch (ev->key()) {

        // It seems we need to use invertedAppearance for Left and right, otherwise, things look weird.
        case Qt::Key_Left:
            action = !d->invertedAppearance ? SliderSingleStepSub : SliderSingleStepAdd;
            break;
        case Qt::Key_Right:
            action = !d->invertedAppearance ? SliderSingleStepAdd : SliderSingleStepSub;
            break;
        case Qt::Key_Up:
            action = d->invertedControls ? SliderSingleStepSub : SliderSingleStepAdd;
            break;
        case Qt::Key_Down:
            action = d->invertedControls ? SliderSingleStepAdd : SliderSingleStepSub;
            break;
        case Qt::Key_PageUp:
            action = d->invertedControls ? SliderPageStepSub : SliderPageStepAdd;
            break;
        case Qt::Key_PageDown:
            action = d->invertedControls ? SliderPageStepAdd : SliderPageStepSub;
            break;
        case Qt::Key_Home:
            action = SliderToMinimum;
            break;
        case Qt::Key_End:
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
void QAbstractSlider::changeEvent(QEvent *ev)
{
    Q_D(QAbstractSlider);
    switch (ev->type()) {
    case QEvent::EnabledChange:
        if (!isEnabled()) {
            d->repeatActionTimer.stop();
            setSliderDown(false);
        }
        // fall through...
    default:
        QWidget::changeEvent(ev);
    }
}

/*! \fn int QAbstractSlider::minValue() const

    Use minimum() instead.
*/

/*! \fn int QAbstractSlider::maxValue() const

    Use maximum() instead.
*/

/*! \fn int QAbstractSlider::lineStep() const

    Use singleStep() instead.
*/

/*! \fn void QAbstractSlider::setMinValue(int v)

    Use setMinimum() instead.
*/

/*! \fn void QAbstractSlider::setMaxValue(int v)

    Use setMaximum() instead.
*/

/*! \fn void QAbstractSlider::setLineStep(int v)

    Use setSingleStep() instead.
*/

/*! \fn void QAbstractSlider::addPage()

    Use triggerAction(QAbstractSlider::SliderPageStepAdd) instead.
*/

/*! \fn void QAbstractSlider::subtractPage()

    Use triggerAction(QAbstractSlider::SliderPageStepSub) instead.
*/

/*! \fn void QAbstractSlider::addLine()

    Use triggerAction(QAbstractSlider::SliderSingleStepAdd) instead.
*/

/*! \fn void QAbstractSlider::subtractLine()

    Use triggerAction(QAbstractSlider::SliderSingleStepSub) instead.
*/

/*! \fn void QAbstractSlider::setSteps(int single, int page)

    Use setSingleStep(\a single) followed by setPageStep(\a page)
    instead.
*/
