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

#include "qtimer.h"
#include "qabstracteventdispatcher.h"
#include "qcoreapplication.h"

/*!
    \class QTimer
    \brief The QTimer class provides repetitive and single-shot timers.

    \ingroup time
    \ingroup events
    \mainclass

    The QTimer class provides a high-level programming interface for
    timers. To use it, create a QTimer, connect its timeout() signal
    to the appropriate slots, and call start(). From then on it will
    emit the timeout() signal at constant intervals.

    Example for a one second (1000 millisecond) timer (from the
    \l{widgets/analogclock}{Analog Clock} example):

    \quotefromfile widgets/analogclock/analogclock.cpp
    \skipto = new QTimer
    \printline = new
    \printline connect
    \printline start(1000)

    From then on, the \c update() slot is called every second.

    You can set a timer to time out only once by calling
    setSingleShot(true). You can also use the static
    QTimer::singleShot() function to call a slot after a specified
    interval:

    \quotefromfile snippets/timers/timers.cpp
    \skipto singleShot
    \printline singleShot

    In multithreaded applications, you can use QTimer in any thread
    that has an event loop. To start an event loop from a non-GUI
    thread, use QThread::exec(). Qt uses the the timer's
    \l{QObject::thread()}{thread affinity} to determine which thread
    will emit the \l{QTimer::}{timeout()} signal. Because of this, you
    must start and stop the timer in its thread; it is not possible to
    start a timer from another thread.

    As a special case, a QTimer with a timeout of 0 will time out as
    soon as all the events in the window system's event queue have
    been processed. This can be used to do heavy work while providing
    a snappy user interface:

    \skipto ZERO-CASE
    \skipline ZERO
    \printline = new QTimer
    \printline connect
    \printline start

    \c processOneThing() will from then on be called repeatedly. It
    should be written in such a way that it always returns quickly
    (typically after processing one data item) so that Qt can deliver
    events to widgets and stop the timer as soon as it has done all
    its work. This is the traditional way of implementing heavy work
    in GUI applications; multithreading is now becoming available on
    more and more platforms, and we expect that zero-millisecond
    QTimers will gradually be replaced by \l{QThread}s.

    Note that QTimer's accuracy depends on the underlying operating
    system and hardware. Most platforms support an accuracy of
    1 millisecond, but Windows 98 supports only 55. If Qt is
    unable to deliver the requested number of timer clicks, it will
    silently discard some.

    An alternative to using QTimer is to call QObject::startTimer()
    for your object and reimplement the QObject::timerEvent() event
    handler in your class (which must inherit QObject). The
    disadvantage is that timerEvent() does not support such
    high-level features as single-shot timers or signals.

    Another alternative to using QTimer is to use QBasicTimer. It is
    typically less cumbersome than using QObject::startTimer()
    directly. See \l{Timers} for an overview of all three approaches.

    Some operating systems limit the number of timers that may be
    used; Qt tries to work around these limitations.

    \sa QBasicTimer, QTimerEvent, QObject::timerEvent(), Timers,
        {Analog Clock Example}, {Wiggly Example}
*/


static const int INV_TIMER = -1;                // invalid timer id

/*!
    Constructs a timer with the given \a parent.
*/

QTimer::QTimer(QObject *parent)
    : QObject(parent), id(INV_TIMER), inter(0), del(0), single(0), nulltimer(0)
{
}


#ifdef QT3_SUPPORT
/*!
    Constructs a timer called \a name, with a \a parent.
*/

QTimer::QTimer(QObject *parent, const char *name)
    : QObject(parent), id(INV_TIMER), single(0), nulltimer(0)
{
    setObjectName(QString::fromAscii(name));
}
#endif

/*!
    Destroys the timer.
*/

QTimer::~QTimer()
{
    if (id != INV_TIMER)                        // stop running timer
        stop();
}


/*!
    \fn void QTimer::timeout()

    This signal is emitted when the timer times out.

    \sa interval, start(), stop()
*/

/*!
    \fn bool QTimer::isActive() const

    Returns true if the timer is running (pending); otherwise returns
    false.
*/

/*!
    \fn int QTimer::timerId() const

    Returns the ID of the timer if the timer is running; otherwise returns
    -1.
*/


/*! \overload

    Starts or restarts the timer with the timeout specified in \l interval.

    If \l singleShot is true, the timer will be activated only once.
*/
void QTimer::start()
{
    if (id != INV_TIMER)                        // stop running timer
        stop();
    nulltimer = (!inter && single);
    id = QObject::startTimer(inter);
}

/*!
    Starts or restarts the timer with a timeout interval of \a msec
    milliseconds.
*/
void QTimer::start(int msec)
{
    setInterval(msec);
    start();
}


#ifdef QT3_SUPPORT
/*! \overload

Call setSingleShot(\a sshot) and start(\a msec) instead.
*/

int QTimer::start(int msec, bool sshot)
{
    if (id >=0 && nulltimer && !msec && sshot)
        return id;
    stop();
    setInterval(msec);
    setSingleShot(sshot);
    start();
    return timerId();
}
#endif


/*!
    Stops the timer.

    \sa start()
*/

void QTimer::stop()
{
    if (id != INV_TIMER) {
        QObject::killTimer(id);
        id = INV_TIMER;
    }
}


/*!
    \reimp
*/
void QTimer::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == id) {
        if (single)
            stop();
        emit timeout();
    }
}

class QSingleShotTimer : public QObject
{
    Q_OBJECT
    int timerId;
public:
    ~QSingleShotTimer();
    QSingleShotTimer(int msec, QObject *r, const char * m);
signals:
    void timeout();
protected:
    void timerEvent(QTimerEvent *);
};

QSingleShotTimer::QSingleShotTimer(int msec, QObject *receiver, const char *member)
    : QObject(QAbstractEventDispatcher::instance())
{
    connect(this, SIGNAL(timeout()), receiver, member);
    timerId = startTimer(msec);
}

QSingleShotTimer::~QSingleShotTimer()
{
    if (timerId > 0)
        killTimer(timerId);
}

void QSingleShotTimer::timerEvent(QTimerEvent *)
{
    // need to kill the timer _before_ we emit timeout() in case the
    // slot connected to timeout calls processEvents()
    if (timerId > 0)
        killTimer(timerId);
    timerId = -1;
    emit timeout();
    delete this;
}

#include "qtimer.moc"

/*!
    \reentrant
    This static function calls a slot after a given time interval.

    It is very convenient to use this function because you do not need
    to bother with a \link QObject::timerEvent() timerEvent\endlink or
    create a local QTimer object.

    Example:
    \code
        #include <QApplication>
        #include <QTimer>

        int main(int argc, char *argv[])
        {
            QApplication app(argc, argv);
            QTimer::singleShot(600000, &app, SLOT(quit()));
            ...
            return app.exec();
        }
    \endcode

    This sample program automatically terminates after 10 minutes
    (600,000 milliseconds).

    The \a receiver is the receiving object and the \a member is the
    slot. The time interval is \a msec milliseconds.

    \sa start()
*/

void QTimer::singleShot(int msec, QObject *receiver, const char *member)
{
    if (receiver && member)
        (void) new QSingleShotTimer(msec, receiver, member);
}

/*!
    \property QTimer::singleShot
    \brief whether the timer is a single-shot timer

    A single-shot timer fires only once, non-single-shot timers fire
    every \l interval milliseconds.

    \sa interval, singleShot()
*/

/*!
    \property QTimer::interval
    \brief the timeout interval in milliseconds

    The default value for this property is 0.  A QTimer with a timeout
    interval of 0 will time out as soon as all the events in the window
    system's event queue have been processed.

    Setting the interval of an active timer changes its timerId().

    \sa singleShot
*/
void QTimer::setInterval(int msec)
{
    inter = msec;
    if (id != INV_TIMER) {                        // create new timer
        QObject::killTimer(id);                        // restart timer
        id = QObject::startTimer(msec);
    }
}

/*! \fn void QTimer::changeInterval(int msec)

   Use setInterval(msec) or start(msec) instead.
*/
