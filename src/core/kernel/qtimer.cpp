/****************************************************************************
**
** Implementation of QTimer class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcoreapplication.h"
#include "qeventloop.h"
#include "qtimer.h"
#include "qsignal.h"

/*!
    \class QTimer qtimer.h
    \brief The QTimer class provides timer signals and single-shot timers.

    \ingroup time
    \ingroup events
    \mainclass

    QTimer uses \link QTimerEvent timer events\endlink internally to
    provide a versatile timer. QTimer is very easy to use:
    create a QTimer, connect its timeout() signal to the appropriate
    slots, and call start() to start it. When the time is up it will
    emit the timeout() signal.

    Note that a QTimer object is destroyed automatically when its
    parent object is destroyed.

    Example:
    \code
        QTimer *timer = new QTimer(myObject);
        connect(timer, SIGNAL(timeout()), myObject, SLOT(timerDone()));
        timer->start(2000, true); // 2 seconds single-shot timer
    \endcode

    You can also use the static singleShot() function to create a
    single shot timer.

    As a special case, a QTimer with a timeout of 0 times out as soon
    as all the events in the window system's event queue have been
    processed.

    This can be used to do heavy work while providing a snappy
    user interface:
    \code
        QTimer *t = new QTimer(myObject);
        connect(t, SIGNAL(timeout()), SLOT(processOneThing()));
        t->start(0, false);
    \endcode

    myObject->processOneThing() will be called repeatedly and should
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
    handler in your class (which must, of course, inherit QObject).
    The disadvantage is that timerEvent() does not support such
    high-level features as single-shot timers or signals.

    Some operating systems limit the number of timers that may be
    used; Qt tries to work around these limitations.
*/


static const int INV_TIMER = -1;                // invalid timer id

/*!
    Constructs a timer with a \a parent.

    Note that the parent object's destructor will destroy this timer
    object.
*/

QTimer::QTimer(QObject *parent)
    : QObject(parent), id(INV_TIMER), single(0), nulltimer(0)
{
}


#ifdef QT_COMPAT
/*!
    Constructs a timer called \a name, with a \a parent.

    Note that the parent object's destructor will destroy this timer
    object.
*/

QTimer::QTimer(QObject *parent, const char *name)
    : QObject(parent), id(INV_TIMER), single(0), nulltimer(0)
{
    setObjectName(name);
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

    This signal is emitted when the timer is activated.
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


/*!
    Starts the timer with a \a msec milliseconds timeout. Returns
    the ID of the timer, or zero if the timer failed to start.

    If \a sshot is true, the timer will be activated only once;
    otherwise it will continue until it is stopped.

    Any pending timer will be stopped.

    \sa singleShot() stop(), changeInterval(), isActive()
*/

int QTimer::start(int msec, bool sshot)
{
    if (id >=0 && nulltimer && !msec && sshot)
        return id;
    if (id != INV_TIMER)                        // stop running timer
        stop();
    single = sshot;
    nulltimer = (!msec && sshot);
    return id = startTimer(msec);
}


/*!
    Changes the timeout interval to \a msec milliseconds.

    If the timer signal is pending, it will be stopped and restarted;
    otherwise it will be started.

    \sa start(), isActive()
*/

void QTimer::changeInterval(int msec)
{
    if (id == INV_TIMER) {                        // create new timer
        start(msec);
    } else {
        killTimer(id);                        // restart timer
        id = startTimer(msec);
    }
}

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
    : QObject(QEventLoop::instance())
{
    connect(this, SIGNAL(timeout()), receiver, member);
    startTimer(msec);
}

void QSingleShotTimer::timerEvent(QTimerEvent *)
{
    // need to unregister the timer _before_ we emit timeout() in case the slot connected
    // to timeout calls processEvents()
    QEventLoop *eventloop = QEventLoop::instance(thread());
    if (eventloop)
        eventloop->unregisterTimers(this);

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
