/****************************************************************************
**
** Implementation of QBasicTimer class.
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

#include "qbasictimer.h"
#include "qcoreapplication.h"
#include "qeventloop.h"

/*!
    \class QBasicTimer qbasictimer.h
    \brief The QBasicTimer class provides timer events for QObjects.

    \ingroup time
    \ingroup events

    This is a fast, lightweight, and low-level class used by Qt
    internally. We recommend using the higher-level QTimer class
    rather than this class if you want to use timers in your
    applications.

    To use this class, create a QBasicTimer, and call its start()
    function with a single-shot timeout interval and with a pointer to
    a QObject subclass. When the timer times out it will send a timer
    event to the QObject subclass. The timer can be stopped at any
    time using stop(). isActive() returns true for a timer that is
    running; i.e. it has been started, has not reached the timeout time,
    and has not been stopped. The timer's ID can be retrieved using
    timerId().
*/


/*!
    \fn QBasicTimer::QBasicTimer()

    Contructs a basic timer.

    \sa start()
*/
/*!
    \fn QBasicTimer::~QBasicTimer()

    Destroys the basic timer.
*/

/*!
    \fn bool QBasicTimer::isActive() const

    Returns true if the timer is running, has not yet timed
    out, and has not been stopped; otherwise returns false.

    \sa start() stop()
*/

/*!
    \fn int QBasicTimer::timerId() const

    Returns the timer's id.

    \sa QTimerEvent::timerId().
*/

/*!
    \fn void QBasicTimer::start(int msec, QObject *object)

    Starts (or restarts) the timer with a \a msec milliseconds
    timeout.

    The given \a object will receive timer events.

    \sa stop() isActive() QObject::timerEvent()
 */
void QBasicTimer::start(int msec, QObject *obj)
{
   stop();
   if (obj)
       id = obj->startTimer(msec);
}

/*!
    Stops the timer.

    \sa start() isActive()
*/
void QBasicTimer::stop()
{
    if (id && QCoreApplication::eventLoop())
        QCoreApplication::eventLoop()->unregisterTimer(id);
    id = 0;
}

