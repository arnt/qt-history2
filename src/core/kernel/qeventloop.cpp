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
#include "qdatetime.h"

#include "qobject_p.h"
#include <private/qthread_p.h>

class QEventLoopPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QEventLoop)
public:
    inline QEventLoopPrivate()
        : exit(false), returnCode(0)
    { }
    bool exit;
    int returnCode;
};

QEventLoop::QEventLoop(QObject *parent)
    : QObject(*new QEventLoopPrivate, parent)
{
    QThreadData *data = QThreadData::current();
    if (!data)
        qWarning("QEventLoop can only be used with threads started with QThread");
}

QEventLoop::~QEventLoop()
{ }

bool QEventLoop::processEvents(ProcessEventsFlags flags)
{
    return QAbstractEventDispatcher::instance(thread())->processEvents(flags);
}

/*!
    Enters the main event loop and waits until exit() is called.
    Returns the value that was passed to exit().

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
    Q_D(QEventLoop);
    d->exit = false;

    QThreadData *data = QThreadData::current();
    Q_ASSERT_X(data != 0, "QEventLoop::exec()", "internal error");
    data->eventLoops.push(this);

    while (!d->exit)
        processEvents(flags | WaitForMoreEvents);

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
    input, i.e. by using the \c ExcludeUserInput flag.

    Note: This function does not process events continuously; it
    returns after all available events are processed.

    Note: Specifying the \c WaitForMoreEvents flag makes no sense
    and will be ignored.
*/
void QEventLoop::processEvents(ProcessEventsFlags flags, int maxTime)
{
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

    \sa QApplication::quit(), exec()
*/
void QEventLoop::exit(int returnCode)
{
    Q_D(QEventLoop);
    d->returnCode = returnCode;
    d->exit = true;
    QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance(thread());
    if (eventDispatcher)
        eventDispatcher->interrupt();
}

void QEventLoop::wakeUp()
{
    QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance(thread());
    if (eventDispatcher)
        eventDispatcher->wakeUp();
}

void QEventLoop::quit()
{ exit(0); }
