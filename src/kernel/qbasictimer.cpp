/****************************************************************************
**
** Implementation of QBasicTimer class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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

    \brief The QBasicTimer class facilitates timer events for
    QObjects.

    \ingroup time
    \ingroup events

    This class exists for the sake of low level size and performance
    optimizations. In user code, we suggest using the higher level
    abstraction provided by QTimer instead.
*/


/*!
  \fn QBasicTimer::QBasicTimer()

  Contructs a basic timer.
*/
/*!
  \fn QBasicTimer::~QBasicTimer()

  Destroys the basic timer.
*/

/*!
  \fn bool QBasicTimer::isActive() const

    Returns true if the timer is running (pending); otherwise returns
    false.
*/

/*!
  \fn int QBasicTimer::timerId() const

  Returns the timer id.

  \sa QTimerEvent::timerId().
*/

/*!
    Starts or restarts the timer with a \a msec milliseconds timeout
    on object \a obj.

    The object will receive timer events.

    \sa QObject::timerEvent()
 */
void QBasicTimer::start(int msec, QObject *obj)
{
   stop();
   if (obj)
       id = obj->startTimer(msec);
}

/*!
  Stops the timer.
 */
void QBasicTimer::stop()
{
    if (id)
	QCoreApplication::eventLoop()->unregisterTimer(id);
    id = 0;
}

