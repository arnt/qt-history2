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

#include "qtimer.h"

#include "qabstracteventdispatcher.h"
#include "qcoreapplication.h"
#include "qsignal.h"

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

    Example for a five second (5000 millisecond) timer:
    \code
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(timerDone()));
        timer->start(5000);
    \endcode

   From then on the \c timerDone() slot is called every five seconds.

    You can set a timer to time out only once by calling
    setSingleShot(true). You can also use the static singleShot()
    function to call a slot after a specified interval:

    \code
        QTimer::singleShot(5000, this, SLOT(timerDone()));
    \endcode

    As a special case, a QTimer with a timeout of 0 will time out as
    soon as all the events in the window system's event queue have
    been processed. This can be used to do heavy work while providing
    a snappy user interface:
    \code
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(processOneThing()));
        timer->start();
    \endcode

    \c processOneThing() will be called repeatedly and should
    return quickly (typically after processing one data item) so that
    Qt can deliver events to widgets and stop the timer as soon as it
    has done all its work. This is the traditional way of
    implementing heavy work in GUI applications; multithreading is
    now becoming available on more and more platforms, and we expect
    that null events will eventually be replaced by threading.

    Note that QTimer's accuracy depends on the underlying operating
    system and hardware. Most platforms support an accuracy of 20ms;
    some provide more. If Qt is unable to deliver the requested
    number of timer clicks, it will silently discard some.

    An alternative to using QTimer is to call QObject::startTimer()
    for your object and reimplement the QObject::timerEvent() event
    handler in your class (which must inherit QObject).  The
    disadvantage is that timerEvent() does not support such high-level
    features as single-shot timers or signals.

    Some operating systems limit the number of timers that may be
    used; Qt tries to work around these limitations.

    \sa QTimerEvent
*/


static const int INV_TIMER = -1;                // invalid timer id

/*!
    Constructs a timer with a \a parent.
*/

QTimer::QTimer(QObject *parent)
    : QObject(parent), id(INV_TIMER), inter(0), del(0), single(0), nulltimer(0)
{
}


#ifdef QT_COMPAT
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
    id = startTimer(inter);
}

/*!
  Starts or restarts the timer with a timeout interval of \a milliseconds.

    If \l singleShot is true, the timer will be activated only once.
 */
void QTimer::start(int msec)
{
    setInterval(msec);
    start();
}


#ifdef QT_COMPAT
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
        killTimer(id);
        id = INV_TIMER;
    }
}


/*!
    \reimp
*/
bool QTimer::event(QEvent *e)
{
    if (e->type() != QEvent::Timer)                // ignore all other events
        return false;
    if (single)                                // stop single shot timer
        stop();
    emit timeout();                                // emit timeout signal
    return true;
}


class QSingleShotTimer : public QObject
{
    Q_OBJECT
public:
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
    startTimer(msec);
}

void QSingleShotTimer::timerEvent(QTimerEvent *)
{
    // need to unregister the timer _before_ we emit timeout() in case the slot connected
    // to timeout calls processEvents()
    QAbstractEventDispatcher *eventDispather = QAbstractEventDispatcher::instance(thread());
    if (eventDispather)
        eventDispather->unregisterTimers(this);
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
        #include <qapplication.h>
        #include <qtimer.h>

        int main(int argc, char **argv)
        {
            QApplication a(argc, argv);
            QTimer::singleShot(10*60*1000, &a, SLOT(quit()));
                ... // create and show your widgets
            return a.exec();
        }
    \endcode

    This sample program automatically terminates after 10 minutes
    (600000 milliseconds).

    The \a receiver is the receiving object and the \a member is the
    slot. The time interval is \a msec milliseconds.
*/

void QTimer::singleShot(int msec, QObject *receiver, const char *member)
{
    if (receiver && member)
        (void) new QSingleShotTimer(msec, receiver, member);
}

/*\property QTimer::singleShot
    \brief whether the timer is a single-shot timer

    A single-shot timer fires only once, non-single-shot timers fire
    every \l interval milliseconds.

 */

/*\property QTimer::interval
  \brief the timeout interval in milliseconds

  The default value for this property is 0.  A QTimer with a timeout
  interval of 0 will time out as soon as all the events in the window
  system's event queue have been processed.

  Setting the interval of an active timer changes its timerId().

  \sa  singleShot
*/
void QTimer::setInterval(int msec)
{
    inter = msec;
    if (id != INV_TIMER) {                        // create new timer
        killTimer(id);                        // restart timer
        id = startTimer(msec);
    }
}

/*! \fn void QTimer::changeInterval(int msec)

   Use setInterval(msec) or start(msec) instead.
 */
