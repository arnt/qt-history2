/****************************************************************************
**
** Implementation of QAbstractSlider class.
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

#include "qabstractslider.h"
#include "qevent.h"
#include "qabstractslider_p.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif
#include <limits.h>
#define d d_func()
#define q q_func()

/*!
    \class QAbstractSlider qabstractslider.h
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

    \i \l sliderPosition: The current position of the slider. If \l
    tracking is enabled (the default), this is identical to \l value.

    \endlist

    Unity (1) may be viewed as a third step size. setValue() lets you
    set the current value to any integer in the allowed range, not
    just mininum() + \e n * singleStep() for integer values of \e n.
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
    \endtable

    QAbstractSlider provides a virtual sliderChange() function that is
    well suited for updating the on-screen representation of
    sliders. By calling triggerAction(), subclasses trigger slider
    actions. Two helper functions QStyle::positionFromValue() and
    QStyle::valueFromPosition() help subclasses and styles to map
    screen coordinates to logical range values.

*/

/*!
    \fn void QAbstractSlider::valueChanged( int value )

    This signal is emitted when the slider value has changed, with the
    new slider \a value as argument.
*/

/*!
    \fn void QAbstractSlider::sliderPressed()

    This signal is emitted when the user presses the slider with the
    mouse.
*/

/*!
    \fn void QAbstractSlider::sliderMoved( int value )

    This signal is emitted when the slider is dragged by the user, with
    the new slider \a value as an argument.

    This signal is emitted even when tracking is turned off.

    \sa tracking() valueChanged()
*/

/*!
    \fn void QAbstractSlider::sliderReleased()

    This signal is emitted when the user releases the slider with the
    mouse.
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


QAbstractSliderPrivate::QAbstractSliderPrivate()
    :minimum(0), maximum(99), singleStep(1), pageStep(10),
     value(0), position(0), tracking(true), blocktracking(false),pressed(false),
     orientation(Qt::Horizontal), repeatAction(QAbstractSlider::SliderNoAction)
{

}

QAbstractSliderPrivate::~QAbstractSliderPrivate()
{
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

/*\internal */
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

void QAbstractSlider::setOrientation(Orientation orientation)
{
    d->orientation = orientation;
    if ( !testWState( WState_OwnSizePolicy ) ) {
	QSizePolicy sp = sizePolicy();
	sp.transpose();
	setSizePolicy( sp );
	clearWState( WState_OwnSizePolicy );
    }
    update();
    updateGeometry();
}

Qt::Orientation QAbstractSlider::orientation() const
{
    return d->orientation;
}


/*!
    \property QAbstractSlider::mininum
    \brief the sliders's minimum value

    When setting this property, the \l maximum is adjusted if
    necessary to ensure that the range remains valid.

*/

void QAbstractSlider::setMinimum(int min)
{
    d->minimum = min;
    d->maximum = qMax(d->maximum, d->minimum);
    sliderChange(SliderRangeChange);
    setValue(d->value); // re-bound
}

int QAbstractSlider::minimum() const
{
    return d->minimum;
}


/*!
    \property QAbstractSlider::maximum
    \brief the slider's maximum value

    When setting this property, the \l mininum is adjusted if
    necessary to ensure that the range remains valid.

*/

void QAbstractSlider::setMaximum(int max)
{
    d->maximum = max;
    d->minimum = qMin(d->minimum, d->maximum);
    sliderChange(SliderRangeChange);
    setValue(d->value); // re-bound
}

int QAbstractSlider::maximum() const
{
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
    d->singleStep = step;
    sliderChange(SliderStepsChange);
}

int QAbstractSlider::singleStep() const
{
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
    d->pageStep = step;
    sliderChange(SliderStepsChange);
}

int QAbstractSlider::pageStep() const
{
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
    d->tracking = enable;
}

bool QAbstractSlider::hasTracking() const
{
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
    d->pressed = down;
    if (!down && d->position != d->value)
	triggerAction(SliderMove);
}

bool QAbstractSlider::isSliderDown()
{
    return d->pressed;
}


/*!
    \property QAbstractSlider::sliderPosition
    \brief the current position of the slider.

    If \l tracking is enabled (the default), this is identical to \l
    value.
*/
void QAbstractSlider::setSliderPosition(int position)
{
    position = d->bound(position);
    if (position == d->position)
	return;
    d->position = position;
    if (!d->blocktracking)
	update();
    emit sliderMoved(position);
    if (d->tracking && !d->blocktracking)
	triggerAction(SliderMove);
}

int QAbstractSlider::sliderPosition() const
{
    return d->position;
}


/*!
    \property QAbstractSlider::value
    \brief the slider's current value

   The slider forces the value to be within the legal range (\lminimum
   <= \l value <= \l maximum).

   Changing the value also changes the \l sliderPosition.
*/


int QAbstractSlider::value() const
{
    return d->value;
}

void QAbstractSlider::setValue(int value)
{
    value = d->bound(value);
    if (d->value == value)
	return;
    d->value = value;
    if (d->position != value)
	emit sliderMoved((d->position = value));
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
    sliderChange(SliderValueChange);
    emit valueChanged(value);
}



/*!  Triggers a slider \a action.  Possible actions are \c
  SliderSingleStepAdd, \c SliderSingleStepSub, \c SliderPageStepAdd,
  \c SliderPageStepSub, \c SliderToMinimum, \c SliderToMaximum, and \c
  SliderMove.

  \sa actionTriggered()
 */
void QAbstractSlider::triggerAction(SliderAction action)
{
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
    d->repeatAction = action;
    d->repeatActionTime = repeatTime;
    d->repeatActionTimer.start(thresholdTime, this);
}

/*!
  Returns the current repeat action.
  \sa setRepeatAction()
 */
QAbstractSlider::SliderAction QAbstractSlider::repeatAction() const
{
    return d->repeatAction;
}

/*!\reimp
 */
void QAbstractSlider::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->repeatActionTimer.timerId()) {
	if (d->repeatActionTime) { // was threshold time, use repeat time next time
	    d->repeatActionTimer.start(d->repeatActionTime, this);
	    d->repeatActionTime = 0;
	}
	triggerAction(d->repeatAction);
    }
}

/*!  Reimplement this virtual function to track slider changes such as
  \c SliderRangeChange, \c SliderOrientationChange,
  \cSliderStepsChange, or \c SliderValueChange. The default
  implementation only updates the display.
 */
void QAbstractSlider::sliderChange(SliderChange)
{
    update();
}


/*!
    \reimp
*/
void QAbstractSlider::wheelEvent( QWheelEvent * e )
{
    if ( e->orientation() != d->orientation && !rect().contains(e->pos()) )
	return;

    static float offset = 0;
    static QAbstractSlider* offset_owner = 0;
    if (offset_owner != this){
	offset_owner = this;
	offset = 0;
    }
    offset += -e->delta()*qMax(d->pageStep,d->singleStep)/120;
    if (QABS(offset)<1)
	return;
    setValue(d->value + int(offset));
    offset -= int(offset);
    e->accept();
}
