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

#include "qeventloop.h"

#include "qabstracteventdispatcher.h"
#include "qcoreapplication.h"
#include "qdatetime.h"

#include "qobject_p.h"
#include <private/qthread_p.h>

class QEventLoopPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QEventLoop)
public:
    inline QEventLoopPrivate()
        : exit(true), returnCode(-1)
    { }
    bool exit;
    int returnCode;
};

/*!
    \class QEventLoop
    \brief The QEventLoop class provides a means of entering and leaving an event loop.

    At any time, you can create a QEventLoop object and call exec()
    on it to start a local event loop. From within the event loop,
    calling exit() will force exec() to return.

    \sa QAbstractEventDispatcher
*/

/*! \enum QEventLoop::ProcessEventsFlag

    This enum controls the types of events processed by the
    processEvents() functions.

    \value AllEvents All events are processed

    \value ExcludeUserInputEvents Do not process user input events,
    such as ButtonPress and KeyPress. Note that the events are not
    discarded; they will be delivered the next time processEvents() is
    called without the ExcludeUserInputEvents flag.

    \value ExcludeSocketNotifiers Do not process socket notifier
    events. Note that the events are not discarded; they will be
    delivered the next time processEvents() is called without the
    ExcludeSocketNotifiers flag.

    \value WaitForMoreEvents Wait for events if no pending events are
    available.

    \omitvalue ExcludeUserInput
    \omitvalue WaitForMore

    \sa processEvents()
*/

/*!
    Constructs an event loop object with the given \a parent.
*/
QEventLoop::QEventLoop(QObject *parent)
    : QObject(*new QEventLoopPrivate, parent)
{
    if (!QCoreApplication::instance())
        qWarning("QEventLoop cannot be used without QApplication");
    else if (!thread())
        qWarning("QEventLoop can only be used with threads started with QThread");
}

/*!
    Destroys the event loop object.
*/
QEventLoop::~QEventLoop()
{ }


/*!
    Processes pending events that match \a flags until there are no
    more events to process.

    This function is especially useful if you have a long running
    operation and want to show its progress without allowing user
    input, i.e. by using the \l ExcludeUserInputEvents flag.

    This function is simply a wrapper for
    QAbstractEventDispatcher::processEvents(). See the documentation
    for that function for details.
*/
bool QEventLoop::processEvents(ProcessEventsFlags flags)
{
    QThread *thr = thread();
    if (!thr)
        return false;

    return QAbstractEventDispatcher::instance(thr)->processEvents(flags);
}

/*!
    Enters the main event loop and waits until exit() is called.
    Returns the value that was passed to exit().

    If \a flags are specified, only events of the types allowed by
    the \a flags will be processed.

    It is necessary to call this function to start event handling. The
    main event loop receives events from the window system and
    dispatches these to the application widgets.

    Generally speaking, no user interaction can take place before
    calling exec(). As a special case, modal widgets like QMessageBox
    can be used before calling exec(), because modal widgets call
    use their own local event loop.

    To make your application perform idle processing, i.e. executing a
    special function whenever there are no pending events, use a
    QTimer with 0 timeout. More sophisticated idle processing schemes
    can be achieved using processEvents().

    \sa QApplication::quit(), exit(), processEvents()
*/
int QEventLoop::exec(ProcessEventsFlags flags)
{
    QThread *thr = thread();
    if (!thr)
        return -1;
    QThreadData *data = QThreadData::get(thr);
    if (data->quitNow)
        return -1;

    Q_D(QEventLoop);
    d->exit = false;
    data->eventLoops.push(this);

    while (!d->exit)
        processEvents(flags | WaitForMoreEvents | ProcessEventsFlag(QEventLoop::DeferredDeletion));


    QEventLoop *eventLoop = data->eventLoops.pop();
    Q_ASSERT_X(eventLoop == this, "QEventLoop::exec()", "internal error");
    Q_UNUSED(eventLoop); // --release warning

    return d->returnCode;
}

/*!
    Process pending events that match \a flags for a maximum of \a
    maxTime milliseconds, or until there are no more events to
    process, whichever is shorter.

    This function is especially useful if you have a long running
    operation and want to show its progress without allowing user
    input, i.e. by using the \l ExcludeUserInputEvents flag.

    Note: This function does not process events continuously; it
    returns after all available events are processed.

    Note: Specifying the \l WaitForMoreEvents flag makes no sense
    and will be ignored.
*/
void QEventLoop::processEvents(ProcessEventsFlags flags, int maxTime)
{
    QThread *thr = thread();
    if (!thr)
        return;

    QTime start;
    start.start();
    while (processEvents(flags & ~WaitForMoreEvents)) {
        if (start.elapsed() > maxTime)
            break;
    }
}

/*!
    Tells the event loop to exit with a return code.

    After this function has been called, the event loop returns from
    the call to exec(). The exec() function returns \a returnCode.

    By convention, a \a returnCode of 0 means success, and any non-zero
    value indicates an error.

    Note that unlike the C library function of the same name, this
    function \e does return to the caller -- it is event processing that
    stops.

    \sa QCoreApplication::quit(), quit(), exec()
*/
void QEventLoop::exit(int returnCode)
{
    QThread *thr = thread();
    if (!thr)
        return;

    Q_D(QEventLoop);
    d->returnCode = returnCode;
    d->exit = true;
    QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance(thr);
    if (eventDispatcher)
        eventDispatcher->interrupt();
}

/*!
    Returns true if the event loop is running; otherwise returns
    false. The event loop is considered running from the time when
    exec() is called until exit() is called.

    \sa exec() exit()
 */
bool QEventLoop::isRunning() const
{
    Q_D(const QEventLoop);
    return !d->exit;
}

/*!
    Wakes up the event loop.

    \sa QAbstractEventDispatcher::wakeUp()
*/
void QEventLoop::wakeUp()
{
    QThread *thr = thread();
    if (!thr)
        return;

    QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance(thr);
    if (eventDispatcher)
        eventDispatcher->wakeUp();
}

/*!
    Tells the event loop to exit normally.

    Same as exit(0).

    \sa QCoreApplication::quit(), exit()
*/
void QEventLoop::quit()
{ exit(0); }
