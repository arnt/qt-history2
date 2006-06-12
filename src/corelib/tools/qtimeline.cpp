/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtimeline.h"

#include <private/qobject_p.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qcoreevent.h>
#include <math.h>

static const qreal pi = 3.14159265359;
static const qreal halfPi = pi / 2.0;

static inline qreal qt_sinProgress(qreal value)
{
    return ::sin((value * pi) - halfPi) / 2.0 + 0.5;
}

static inline qreal qt_smoothBeginEndMixFactor(qreal value)
{
    return qMin(qMax((1.0 - value * 2.0 + 0.3), 0.0), 1.0);
}

class QTimeLinePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QTimeLine)
public:
    inline QTimeLinePrivate()
        : startTime(0), duration(1000), startFrame(0), endFrame(0),
          updateInterval(1000 / 25),
          totalLoopCount(1), currentLoopCount(0), currentTime(0), timerId(0),
          direction(QTimeLine::Forward), curveShape(QTimeLine::EaseInOutCurve),
          state(QTimeLine::NotRunning)
    { }

    int startTime;
    int duration;
    int startFrame;
    int endFrame;
    int updateInterval;
    int totalLoopCount;
    int currentLoopCount;

    int currentTime;
    int timerId;
    QTime timer;
    
    QTimeLine::Direction direction;
    QTimeLine::CurveShape curveShape;
    QTimeLine::State state;
    inline void setState(QTimeLine::State newState)
    {
        Q_Q(QTimeLine);
        if (newState != state)
            emit q->stateChanged(state = newState);
    }

    void setCurrentTime(int msecs);
};

/*!
    \internal
*/
void QTimeLinePrivate::setCurrentTime(int msecs)
{
    Q_Q(QTimeLine);

    if (msecs == currentTime)
        return;

    qreal lastValue = q->currentValue();
    int lastFrame = q->currentFrame();

    currentTime = msecs;
    while (currentTime < 0)
        currentTime += duration;
    bool looped = (msecs < 0 || msecs > duration);
    currentTime %= (duration + 1);
    if (looped)
        ++currentLoopCount;

    bool finished = false;
    if (totalLoopCount && looped && currentLoopCount >= totalLoopCount) {
        finished = true;
        currentTime = (direction == QTimeLine::Backward) ? 0 : duration;
    }

    if (lastValue != q->currentValue())
        emit q->valueChanged(q->currentValue());
    if (lastFrame != q->currentFrame())
        emit q->frameChanged(q->currentFrame());
    if (finished) {
        emit q->finished();
        q->stop();
    }
}

/*!
    \class QTimeLine
    \brief The QTimeLine class provides a timeline for controlling animations.
    \since 4.2
    \ingroup multimedia

    It's most commonly used to animate a GUI control by calling a slot
    periodically. You can construct a timeline by passing its duration in
    milliseconds to QTimeLine's constructor. The timeline's duration describes
    for how long the animation will run. Then you set a suitable frame range
    by calling setFrameRange(). Finally connect the frameChanged() signal to a
    suitable slot in the widget you wish to animate (e.g., setValue() in
    QProgressBar). When you proceed to calling start(), QTimeLine will enter
    Running state, and start emitting frameChanged() at regular intervals,
    causing your widget's connected property's value to grow from the lower
    end to the upper and of your frame range, at a steady rate. You can
    specify the update interval by calling setUpdateInterval(). When done,
    QTimeLine enters NotRunning state, and emits finished().

    Example:

    \code
        ...
        progressBar = new QProgressBar(this);
        progressBar->setRange(0, 100);

        // Construct a 1-second timeline with a frame range of 0 - 100
        QTimeLine *timeLine = new QTimeLine(1000, this);
        timeLine->setFrameRange(0, 100);
        connect(timeLine, SIGNAL(frameChanged(int)), progressBar, SLOT(setValue(int)));

        // Clicking the pushbutton will start the progress bar animation
        pushButton = new QPushButton(tr("Start animation"), this);
        connect(pushButton, SIGNAL(clicked()), timeLine, SLOT(start()));
        ...
    \endcode

    By default the timeline runs once, from the beginning and towards the end,
    upon which you must call start() again to restart from the beginning. To
    make the timeline loop, you can call setLoopCount(), passing the number of
    times the timeline should run before finishing. The direction can also be
    changed, causing the timeline to run backward, by calling
    setDirection(). You can also pause and unpause the timeline while it's
    running by calling setPaused(). For interactive control, the
    setCurrentTime() function is provided, which sets the time position of the
    time line directly. Although most useful in NotRunning state, (e.g.,
    connected to a valueChanged() signal in a QSlider,) this function can be
    called at any time.

    The frame interface is useful for standard widgets, but QTimeLine can be
    used to control any type of animation. The heart of QTimeLine lies in the
    valueForTime() function, which generates a \e value between 0 and 1 for a
    given time. This value is typically used to describe the steps of an
    animation, where 0 is the first step of an animation, and 1 is the last
    step. When running, QTimeLine generates values between 0 and 1 by calling
    valueForTime() and emitting valueChanged(). By default, valueForTime()
    applies an interpolation algorithm to generate these value. You can choose
    from a set of predefined timeline algorithms by calling
    setCurveShape(). By default, QTimeLine uses the EaseInOut curve shape,
    which provides a value that grows slowly, then grows steadily, and
    finally grows slowly. For a custom timeline, you can reimplement
    valueForTime(), in which case QTimeLine's curveShape property is ignored.
*/

/*!
    \enum QTimeLine::State

    This enum describes the state of the timeline.
    
    \value NotRunning The timeline is not running. This is the initial state
    of QTimeLine, and the state QTimeLine reenters when finished. The current
    time, frame and value remain unchanged until either setCurrentTime() is
    called, or the timeline is started by calling start().

    \value Paused The timeline is paused (i.e., temporarily
    suspended). Calling setPaused(false) will resume timeline activity.

    \value Running The timeline is running. While control is in the event
    loop, QTimeLine will update its current time at regular intervals,
    emitting valueChanged() and frameChanged() when appropriate.
*/

/*!
    \enum QTimeLine::Direction

    This enum describes the direction of the timeline when in \l Running state.
    
    \value Forward The current time of the timeline increases with time (i.e.,
    moves from 0 and towards the end / duration).

    \value Backward The current time of the timeline decreases with time (i.e.,
    moves from the end / duration and towards 0).
*/

/*!
    \enum QTimeLine::CurveShape

    This enum describes the default shape of QTimeLine's value curve. The
    default, shape is EaseInOutCurve. The curve defines the relation
    between the value and the timeline.
    
    \value EaseInCurve The value starts growing slowly, then increases in speed.
    \value EaseOutCurve The value starts growing steadily, then ends slowly.
    \value EaseInOutCurve The value starts growing slowly, the runs steadily, then grows slowly again.
    \value LinearCurve The value grows linearily (e.g., if the duration is 1000 ms,
           the value at time 500ms is 0.5).
*/

/*!
    \fn QTimeLine::valueChanged(qreal value)

    QTimeLine emits this signal at regular intervals when in \l Running state,
    but only if the current value changes. \a value is the current value.

    \sa QTimeLine::setDuration(), QTimeLine::valueForTime(), QTimeLine::updateInterval
*/

/*!
    \fn QTimeLine::frameChanged(int frame)

    QTimeLine emits this signal at regular intervals when in \l Running state,
    but only if the current frame changes. \a frame is the current frame number.

    \sa QTimeLine::setFrameRange(), QTimeLine::updateInterval
*/

/*!
    \fn QTimeLine::stateChanged(QTimeLine::State newState)

    This signal is emitted whenever QTimeLine's state changes. The new state
    is \a newState.
*/

/*!
    \fn QTimeLine::finished()

    This signal is emitted when QTimeLine finishes (i.e., reaches the end of
    its time line), and does not loop.
*/
    
/*!
    Constructs a timeline with a duration of \a duration milliseconds. \a
    parent is passed to QObject's constructor. The default duration is 1000
    milliseconds.
 */
QTimeLine::QTimeLine(int duration, QObject *parent)
    : QObject(*new QTimeLinePrivate, parent)
{
    setDuration(duration);
}

/*!
    Destroys the timeline.
 */
QTimeLine::~QTimeLine()
{
}

/*!
    Returns the state of the timeline.

    \sa start(), setPaused(), stop()
*/
QTimeLine::State QTimeLine::state() const
{
    Q_D(const QTimeLine);
    return d->state;
}

/*!
    \property QTimeLine::loopCount
    \brief the number of times the timeline should loop before it's finished.

    A loop count of of 0 means that the timeline will loop forever.
*/
int QTimeLine::loopCount() const
{
    Q_D(const QTimeLine);
    return d->totalLoopCount;
}
void QTimeLine::setLoopCount(int count)
{
    Q_D(QTimeLine);
    d->totalLoopCount = count;
}

/*!
    \property QTimeLine::direction
    \brief the direction of the timeline when QTimeLine is in \l Running
    state.

    This direction indicates whether the time moves from 0 towards the
    timeline duration, or from the value of the duration and towards 0 after
    start() has been called.
*/
QTimeLine::Direction QTimeLine::direction() const
{
    Q_D(const QTimeLine);
    return d->direction;
}
void QTimeLine::setDirection(Direction direction)
{
    Q_D(QTimeLine);
    d->direction = direction;
    d->startTime = d->currentTime;
    d->timer.start();
}

/*!
    \property QTimeLine::duration
    \brief the total duration of the timeline in milliseconds.

    By default, this value is 1000 (i.e., 1 second), but you can change this
    by either passing a duration to QTimeLine's constructor, or by calling
    setDuration().
*/
int QTimeLine::duration() const
{
    Q_D(const QTimeLine);
    return d->duration;
}
void QTimeLine::setDuration(int duration)
{
    Q_D(QTimeLine);
    d->duration = duration;
}

/*!
    Returns the start frame, which is the frame corresponding to the start of
    the timeline (i.e., the frame for which the current value is 0).

    \sa setStartFrame(), setFrameRange()
*/
int QTimeLine::startFrame() const
{
    Q_D(const QTimeLine);
    return d->startFrame;
}

/*!
    Sets the start frame, which is the frame corresponding to the start of the
    timeline (i.e., the frame for which the current value is 0), to \a frame.

    \sa startFrame(), endFrame(), setFrameRange()
*/
void QTimeLine::setStartFrame(int frame)
{
    Q_D(QTimeLine);
    d->startFrame = frame;
}

/*!
    Returns the end frame, which is the frame corresponding to the end of the
    timeline (i.e., the frame for which the current value is 1).

    \sa setEndFrame(), setFrameRange()
*/
int QTimeLine::endFrame() const
{
    Q_D(const QTimeLine);
    return d->endFrame;
}

/*!
    Sets the end frame, which is the frame corresponding to the end of the
    timeline (i.e., the frame for which the current value is 1), to \a frame.

    \sa endFrame(), startFrame(), setFrameRange()
*/
void QTimeLine::setEndFrame(int frame)
{
    Q_D(QTimeLine);
    d->endFrame = frame;
}

/*!
    Sets the timeline's frame counter to start at \a startFrame, and end and
    \a endFrame. For each time value, QTimeLine will find the corresponding
    frame when you call currentFrame() or frameForTime() by interpolating,
    using the return value of valueForTime().

    When in Running state, QTimeLine also emits the frameChanged() signal when
    the frame changes.

    \sa startFrame(), endFrame(), start(), currentFrame()
*/
void QTimeLine::setFrameRange(int startFrame, int endFrame)
{
    Q_D(QTimeLine);
    d->startFrame = startFrame;
    d->endFrame = endFrame;
}

/*!
    \property QTimeLine::updateInterval
    \brief the time in milliseconds between each time QTimeLine updates its
    current time.

    When updating the current time, QTimeLine will emit valueChanged() if the
    current value changed, and frameChanged() if the frame changed.

    By default, the interval is 40ms, which corresponds to a rate of 25
    updates per second.
*/
int QTimeLine::updateInterval() const
{
    Q_D(const QTimeLine);
    return d->updateInterval;
}
void QTimeLine::setUpdateInterval(int interval)
{
    Q_D(QTimeLine);
    d->updateInterval = interval;
}

/*!
    \property QTimeLine::curveShape
    \brief the shape of the timeline curve.

    The curve shape describes the relation between the time and value for the
    base implementation of valueForTime().

    If you have reimplemented valueForTime(), this value is ignored.

    \sa valueForTime()
*/
QTimeLine::CurveShape QTimeLine::curveShape() const
{
    Q_D(const QTimeLine);
    return d->curveShape;
}
void QTimeLine::setCurveShape(CurveShape shape)
{
    Q_D(QTimeLine);
    d->curveShape = shape;
}

/*!
    \property QTimeLine::currentTime
    \brief the current time of the time line.

    When QTimeLine is in Running state, this value is updated continuously as
    a function of the duration and direction of the timeline. Otherwise, it is
    value that was current when stop() was called last, or the value set by
    setCurrentTime().
*/
int QTimeLine::currentTime() const
{
    Q_D(const QTimeLine);
    return d->currentTime;
}
void QTimeLine::setCurrentTime(int msec)
{
    Q_D(QTimeLine);
    d->startTime = 0;
    d->timer.restart();
    d->setCurrentTime(msec);
}

/*!
    Returns the frame corresponding to the current time.

    \sa currentTime(), frameForTime(), setFrameRange()
*/
int QTimeLine::currentFrame() const
{
    Q_D(const QTimeLine);
    return frameForTime(d->currentTime);
}

/*!
    Returns the value corresponding to the current time.

    \sa valueForTime(), currentFrame()
*/
qreal QTimeLine::currentValue() const
{
    Q_D(const QTimeLine);
    return valueForTime(d->currentTime);
}

/*!
    Returns the frame corresponding to the time \a msec. This value is
    calculated using a linear interpolation of the start and end frame, based
    on the value returned by valueForTime().

    \sa valueForTime(), setFrameRange()
*/
int QTimeLine::frameForTime(int msec) const
{
    Q_D(const QTimeLine);
    return d->startFrame + int((d->endFrame - d->startFrame) * valueForTime(msec));
}

/*!
    Returns the timeline value for the time \a msec. The returned value, which
    varies depending on the curve shape, is always between 0 and 1. If \a msec
    is 0, the default implementation always returns 0.

    Reimplement this function to provide a custom curve shape for your
    timeline.

    \sa CurveShape, frameForTime()
*/
qreal QTimeLine::valueForTime(int msec) const
{
    Q_D(const QTimeLine);
    msec = qMin(qMax(msec, 0), d->duration);

    // Simple linear interpolation
    qreal value = msec / qreal(d->duration);
    
    switch (d->curveShape) {
    case EaseInOutCurve:
        value = qt_sinProgress(value);
        break;
        // SmoothBegin blends Smooth and Linear Interpolation.
        // Progress 0 - 0.3      : Smooth only
        // Progress 0.3 - ~ 0.5  : Mix of Smooth and Linear
        // Progress ~ 0.5  - 1   : Linear only
    case EaseInCurve: {
        const qreal sinProgress = qt_sinProgress(value);
        const qreal linearProgress = value;
        const qreal mix = qt_smoothBeginEndMixFactor(value);
        value = sinProgress * mix + linearProgress * (1.0 - mix);
        break;
    }
    case EaseOutCurve: {
        const qreal sinProgress = qt_sinProgress(value);
        const qreal linearProgress = value;
        const qreal mix = qt_smoothBeginEndMixFactor(1.0 - value);
        value = sinProgress * mix + linearProgress * (1.0 - mix);
        break;
    }
    case SineCurve:
        value = (::sin(((msec * pi * 2) / d->duration) - pi/2.0) + 1.0) / 2.0;
        break;
    default:
        break;
    }

    return value;
}

/*!
    Starts the timeline. QTimeLine will enter Running state, and once it
    enters the event loop, it will update its current time, frame and value at
    regular intervals. The default interval is 40ms (i.e., 25 times per
    second,). You can change the update interval by calling setUpdateInterval().

    \sa updateInterval(), frameChanged(), valueChanged()
*/
void QTimeLine::start()
{
    Q_D(QTimeLine);
    if (d->timerId) {
        qWarning("QTimeLine::start: already running");
        return;
    }
    if (d->currentTime == d->duration && d->direction == Forward)
        d->currentTime = 0;
    else if (d->currentTime == 0 && d->direction == Backward)
        d->currentTime = d->duration;
    d->timerId = startTimer(d->updateInterval);
    d->startTime = d->currentTime;
    d->timer.start();
    d->setState(Running);
}

/*!
    Stops the timeline, causing QTimeLine to enter NotRunning state.

    \sa start()
*/
void QTimeLine::stop()
{
    Q_D(QTimeLine);
    if (d->timerId)
        killTimer(d->timerId);
    d->setState(NotRunning);
    d->timerId = 0;
}

/*!
    If \a paused is true, the timeline is paused, causing QTimeLine to enter
    Paused state. No updates will be signaled until either start() or
    setPaused(false) is called. If \a paused is false, the timeline is resumed
    and continues where it left.

    \sa state(), start()
*/
void QTimeLine::setPaused(bool paused)
{
    Q_D(QTimeLine);
    if (d->state == NotRunning) {
        qWarning("QTimeLine::setPaused: Not running");
        return;
    }
    if (paused && d->state != Paused) {
        d->startTime = d->currentTime;
        killTimer(d->timerId);
        d->timerId = 0;
        d->setState(Paused);
    } else if (!paused && d->state == Paused) {
        d->timerId = startTimer(d->updateInterval);
        d->setState(Running);
    }
}

/*!
    Toggles the direction of the timeline. If the direction was Forward, it
    becomes Backward, and vice verca.

    \sa setDirection()
*/
void QTimeLine::toggleDirection()
{
    Q_D(QTimeLine);
    setDirection(d->direction == Forward ? Backward : Forward);
}

/*!
    \reimp
*/
void QTimeLine::timerEvent(QTimerEvent *event)
{
    Q_D(QTimeLine);
    if (event->timerId() != d->timerId) {
        event->ignore();
        return;
    }
    event->accept();

    if (d->direction == Forward) {
        d->setCurrentTime(d->startTime + d->timer.elapsed());
    } else {
        d->setCurrentTime(d->startTime - d->timer.elapsed());
    }
}
