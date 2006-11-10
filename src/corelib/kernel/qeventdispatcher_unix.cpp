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

#include "qplatformdefs.h"

#include "qcoreapplication.h"
#include "qpair.h"
#include "qsocketnotifier.h"
#include "qthread.h"

#include "qeventdispatcher_unix_p.h"
#include <private/qthread_p.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#if (_POSIX_MONOTONIC_CLOCK-0 <= 0)
#  include <sys/times.h>
#endif

Q_CORE_EXPORT bool qt_disable_lowpriority_timers=false;

/*****************************************************************************
 UNIX signal handling
 *****************************************************************************/

static sig_atomic_t signal_received;
static sig_atomic_t signals_fired[NSIG];

static void signalHandler(int sig)
{
    signals_fired[sig] = 1;
    signal_received = 1;
}




QEventDispatcherUNIXPrivate::QEventDispatcherUNIXPrivate()
{
    extern Qt::HANDLE qt_application_thread_id;
    mainThread = (QThread::currentThreadId() == qt_application_thread_id);

    // initialize the common parts of the event loop
    pipe(thread_pipe);
    fcntl(thread_pipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(thread_pipe[1], F_SETFD, FD_CLOEXEC);
    fcntl(thread_pipe[0], F_SETFL, fcntl(thread_pipe[0], F_GETFL) | O_NONBLOCK);
    fcntl(thread_pipe[1], F_SETFL, fcntl(thread_pipe[1], F_GETFL) | O_NONBLOCK);

    sn_highest = -1;

    interrupt = false;
}

QEventDispatcherUNIXPrivate::~QEventDispatcherUNIXPrivate()
{
    // cleanup the common parts of the event loop
    close(thread_pipe[0]);
    close(thread_pipe[1]);

    // cleanup timers
    qDeleteAll(timerList);
}

int QEventDispatcherUNIXPrivate::doSelect(QEventLoop::ProcessEventsFlags flags, timeval *timeout)
{
    Q_Q(QEventDispatcherUNIX);

    int nsel;
    do {
        if (mainThread) {
            while (signal_received) {
                signal_received = 0;
                for (int i = 0; i < NSIG; ++i) {
                    if (signals_fired[i]) {
                        signals_fired[i] = 0;
                        emit QCoreApplication::instance()->unixSignal(i);
                    }
                }
            }
        }

        // Process timers and socket notifiers - the common UNIX stuff
        int highest = 0;
        FD_ZERO(&sn_vec[0].select_fds);
        FD_ZERO(&sn_vec[1].select_fds);
        FD_ZERO(&sn_vec[2].select_fds);
        if (! (flags & QEventLoop::ExcludeSocketNotifiers) && (sn_highest >= 0)) {
            // return the highest fd we can wait for input on
            if (!sn_vec[0].list.isEmpty())
                sn_vec[0].select_fds = sn_vec[0].enabled_fds;
            if (!sn_vec[1].list.isEmpty())
                sn_vec[1].select_fds = sn_vec[1].enabled_fds;
            if (!sn_vec[2].list.isEmpty())
                sn_vec[2].select_fds = sn_vec[2].enabled_fds;
            highest = sn_highest;
        }

        FD_SET(thread_pipe[0], &sn_vec[0].select_fds);
        highest = qMax(highest, thread_pipe[0]);

        nsel = q->select(highest + 1,
                         &sn_vec[0].select_fds,
                         &sn_vec[1].select_fds,
                         &sn_vec[2].select_fds,
                         timeout);
    } while (nsel == -1 && (errno == EINTR || errno == EAGAIN));

    if (nsel == -1) {
        if (errno == EBADF) {
            // it seems a socket notifier has a bad fd... find out
            // which one it is and disable it
            fd_set fdset;
            timeval tm;
            tm.tv_sec = tm.tv_usec = 0l;

            for (int type = 0; type < 3; ++type) {
                QList<QSockNot *> &list = sn_vec[type].list;
                if (list.size() == 0)
                    continue;

                for (int i = 0; i < list.size(); ++i) {
                    QSockNot *sn = list.at(i);

                    FD_ZERO(&fdset);
                    FD_SET(sn->fd, &fdset);

                    int ret = -1;
                    do {
                        switch (type) {
                        case 0: // read
                            ret = select(sn->fd + 1, &fdset, 0, 0, &tm);
                            break;
                        case 1: // write
                            ret = select(sn->fd + 1, 0, &fdset, 0, &tm);
                            break;
                        case 2: // except
                            ret = select(sn->fd + 1, 0, 0, &fdset, &tm);
                            break;
                        }
                    } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

                    if (ret == -1 && errno == EBADF) {
                        // disable the invalid socket notifier
                        static const char *t[] = { "Read", "Write", "Exception" };
                        qWarning("QSocketNotifier: Invalid socket %d and type '%s', disabling...",
                                 sn->fd, t[type]);
                        sn->obj->setEnabled(false);
                    }
                }
            }
        } else {
            // EINVAL... shouldn't happen, so let's complain to stderr
            // and hope someone sends us a bug report
            perror("select");
        }
    }

    // some other thread woke us up... consume the data on the thread pipe so that
    // select doesn't immediately return next time
    int nevents = 0;
    if (nsel > 0 && FD_ISSET(thread_pipe[0], &sn_vec[0].select_fds)) {
        char c[16];
        while (::read(thread_pipe[0], c, sizeof(c)) > 0)
            ;
        if (!wakeUps.testAndSetRelease(1, 0)) {
            // hopefully, this is dead code
            qWarning("QEventDispatcherUNIX: internal error, wakeUps.testAndSetRelease(1, 0) failed!");
        }
        ++nevents;
    }

    // activate socket notifiers
    if (! (flags & QEventLoop::ExcludeSocketNotifiers) && nsel > 0 && sn_highest >= 0) {
        // if select says data is ready on any socket, then set the socket notifier
        // to pending
        for (int i=0; i<3; i++) {
            QList<QSockNot *> &list = sn_vec[i].list;
            for (int j = 0; j < list.size(); ++j) {
                QSockNot *sn = list.at(j);
                if (FD_ISSET(sn->fd, &sn_vec[i].select_fds))
                    q->setSocketNotifierPending(sn->obj);
            }
        }
    }
    return (nevents + q->activateSocketNotifiers());
}

/*
 * Internal functions for manipulating timer data structures.  The
 * timerBitVec array is used for keeping track of timer identifiers.
 */

QTimerInfoList::QTimerInfoList()
{
#if (_POSIX_MONOTONIC_CLOCK-0 <= 0)
    useMonotonicTimers = false;

#  if (_POSIX_MONOTONIC_CLOCK == 0)
    // detect if the system support monotonic timers
    long x = sysconf(_SC_MONOTONIC_CLOCK);
    useMonotonicTimers = x >= 200112L;
#  endif

    getTime(currentTime);

    if (!useMonotonicTimers) {
        // not using monotonic timers, initialize the timeChanged() machinery
        previousTime = currentTime;

        tms unused;
        previousTicks = times(&unused);

        ticksPerSecond = sysconf(_SC_CLK_TCK);
        msPerTick = 1000/ticksPerSecond;
    } else {
        // detected monotonic timers
        previousTime.tv_sec = previousTime.tv_usec = 0;
        previousTicks = 0;
        ticksPerSecond = 0;
        msPerTick = 0;
    }
#else
    // using monotonic timers unconditionally
    getTime(currentTime);
#endif
}

timeval QTimerInfoList::updateCurrentTime()
{
    getTime(currentTime);
    return currentTime;
}

#if (_POSIX_MONOTONIC_CLOCK-0 > 0)

void QTimerInfoList::getTime(timeval &t)
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t.tv_sec = ts.tv_sec;
    t.tv_usec = ts.tv_nsec / 1000;
}

void QTimerInfoList::repairTimersIfNeeded()
{
}

#else

/*
  Returns true if the real time clock has changed by more than 10%
  relative to the processor time since the last time this function was
  called. This presumably means that the system time has been changed.

  If /a delta is nonzero, delta is set to our best guess at how much the system clock was changed.
*/
bool QTimerInfoList::timeChanged(timeval *delta)
{
    tms unused;
    clock_t currentTicks = times(&unused);

    int elapsedTicks = currentTicks - previousTicks;
    timeval elapsedTime = currentTime - previousTime;
    int elapsedMsecTicks = (elapsedTicks * 1000) / ticksPerSecond;
    int deltaMsecs = (elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000)
                     - elapsedMsecTicks;

    if (delta) {
	delta->tv_sec = deltaMsecs / 1000;
	delta->tv_usec = (deltaMsecs % 1000) * 1000;
    }
    previousTicks = currentTicks;
    previousTime = currentTime;

    // If tick drift is more than 10% off compared to realtime, we assume that the clock has
    // been set. Of course, we have to allow for the tick granularity as well.

     return (qAbs(deltaMsecs) - msPerTick) * 10 > elapsedMsecTicks;
}

void QTimerInfoList::getTime(timeval &t)
{
#if !defined(QT_NO_CLOCK_MONOTONIC)
    if (useMonotonicTimers) {
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        t.tv_sec = ts.tv_sec;
        t.tv_usec = ts.tv_nsec / 1000;
        return;
    }
#endif

    gettimeofday(&t, 0);
    // NTP-related fix
    while (t.tv_usec >= 1000000l) {
        t.tv_usec -= 1000000l;
        ++t.tv_sec;
    }
    while (t.tv_usec < 0l) {
        if (t.tv_sec > 0l) {
            t.tv_usec += 1000000l;
            --t.tv_sec;
        } else {
            t.tv_usec = 0l;
            break;
        }
    }
}

void QTimerInfoList::repairTimersIfNeeded()
{
    if (useMonotonicTimers)
        return;
    timeval delta;
    if (timeChanged(&delta))
        timerRepair(delta);
}

#endif

/*
  insert timer info into list
*/
void QTimerInfoList::timerInsert(QTimerInfo *ti)
{
    int index = size();
    while (index--) {
        register const QTimerInfo * const t = at(index);
        if (!(ti->timeout < t->timeout))
            break;
    }
    insert(index+1, ti);
}

/*
  repair broken timer
*/
void QTimerInfoList::timerRepair(const timeval &diff)
{
    // repair all timers
    for (int i = 0; i < size(); ++i) {
        register QTimerInfo *t = at(i);
        t->timeout = t->timeout - diff;
    }
}

/*
  Returns the time to wait for the next timer, or null if no timers
  are waiting.
*/
bool QTimerInfoList::timerWait(timeval &tm)
{
    timeval currentTime = updateCurrentTime();
    repairTimersIfNeeded();

    if (isEmpty())
        return false;

    QTimerInfo *t = first();        // first waiting timer
    if (currentTime < t->timeout) {
        // time to wait
        tm = t->timeout - currentTime;
    } else {
        // no time to wait
        tm.tv_sec  = 0;
        tm.tv_usec = 0;
    }

    return true;
}

QEventDispatcherUNIX::QEventDispatcherUNIX(QObject *parent)
    : QAbstractEventDispatcher(*new QEventDispatcherUNIXPrivate, parent)
{ }

QEventDispatcherUNIX::QEventDispatcherUNIX(QEventDispatcherUNIXPrivate &dd, QObject *parent)
    : QAbstractEventDispatcher(dd, parent)
{ }

QEventDispatcherUNIX::~QEventDispatcherUNIX()
{ }

int QEventDispatcherUNIX::select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                                 timeval *timeout)
{
    Q_D(QEventDispatcherUNIX);
    if (timeout) {
        // handle the case where select returns with a timeout, too
        // soon.
        timeval tvStart = d->timerList.currentTime;
        timeval tvCurrent = tvStart;
        timeval originalTimeout = *timeout;

        int nsel;
        do {
            timeval tvRest = originalTimeout + tvStart - tvCurrent;
            nsel = ::select(nfds, readfds, writefds, exceptfds, &tvRest);
            d->timerList.getTime(tvCurrent);
        } while (nsel == 0 && (tvCurrent - tvStart) < originalTimeout);

        return nsel;
    }

    return ::select(nfds, readfds, writefds, exceptfds, timeout);
}

/*!
    \internal
*/
void QEventDispatcherUNIX::registerTimer(int timerId, int interval, QObject *obj)
{
    if (timerId < 1 || interval < 0 || !obj) {
        qWarning("QEventDispatcherUNIX::registerTimer: invalid arguments");
        return;
    } else if (obj->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QObject::startTimer: timers cannot be started from another thread");
        return;
    }

    Q_D(QEventDispatcherUNIX);

    QTimerInfo *t = new QTimerInfo;                // create timer
    t->id = timerId;
    t->interval.tv_sec  = interval / 1000;
    t->interval.tv_usec = (interval % 1000) * 1000;
    t->timeout = d->timerList.updateCurrentTime() + t->interval;
    t->obj = obj;
    t->inTimerEvent = false;

    d->timerList.timerInsert(t);                                // put timer in list
}

/*!
    \internal
*/
bool QEventDispatcherUNIX::unregisterTimer(int timerId)
{
    if (timerId < 1) {
        qWarning("QEventDispatcherUNIX::unregisterTimer: invalid argument");
        return false;
    } else if (thread() != QThread::currentThread()) {
        qWarning("QObject::killTimer: timers cannot be stopped from another thread");
        return false;
    }

    Q_D(QEventDispatcherUNIX);
    // set timer inactive
    for (int i = 0; i < d->timerList.size(); ++i) {
        register QTimerInfo *t = d->timerList.at(i);
        if (t->id == timerId) {
            d->timerList.removeAt(i);
            delete t;
            return true;
        }
    }
    // id not found
    return false;
}

/*!
    \internal
*/
bool QEventDispatcherUNIX::unregisterTimers(QObject *object)
{
    if (!object) {
        qWarning("QEventDispatcherUNIX::unregisterTimers: invalid argument");
        return false;
    } else if (object->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QObject::killTimers: timers cannot be stopped from another thread");
        return false;
    }

    Q_D(QEventDispatcherUNIX);
    if (d->timerList.isEmpty())
        return false;
    for (int i = 0; i < d->timerList.size(); ++i) {
        register QTimerInfo *t = d->timerList.at(i);
        if (t->obj == object) {
            // object found
            d->timerList.removeAt(i);
            delete t;
            // move back one so that we don't skip the new current item
            --i;
        }
    }
    return true;
}

QList<QEventDispatcherUNIX::TimerInfo>
QEventDispatcherUNIX::registeredTimers(QObject *object) const
{
    if (!object) {
        qWarning("QEventDispatcherUNIX:registeredTimers: invalid argument");
        return QList<TimerInfo>();
    }

    Q_D(const QEventDispatcherUNIX);
    QList<TimerInfo> list;
    for (int i = 0; i < d->timerList.size(); ++i) {
        register const QTimerInfo * const t = d->timerList.at(i);
        if (t->obj == object)
            list << TimerInfo(t->id, t->interval.tv_sec * 1000 + t->interval.tv_usec / 1000);
    }
    return list;
}

/*****************************************************************************
 Socket notifier type
 *****************************************************************************/
QSockNotType::QSockNotType()
{
    FD_ZERO(&select_fds);
    FD_ZERO(&enabled_fds);
    FD_ZERO(&pending_fds);
}

QSockNotType::~QSockNotType()
{
    while (!list.isEmpty())
        delete list.takeFirst();
}

/*****************************************************************************
 QEventDispatcher implementations for UNIX
 *****************************************************************************/

void QEventDispatcherUNIX::registerSocketNotifier(QSocketNotifier *notifier)
{
    int sockfd;
    int type;
    if (!notifier
        || (sockfd = notifier->socket()) < 0
        || unsigned(sockfd) >= FD_SETSIZE
        || (type = notifier->type()) < 0
        || type > 2) {
        qWarning("QSocketNotifier: Internal error");
        return;
    } else if (notifier->thread() != thread()
               || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifiers cannot be enabled from another thread");
        return;
    }

    Q_D(QEventDispatcherUNIX);
    QList<QSockNot *> &list = d->sn_vec[type].list;
    fd_set *fds  = &d->sn_vec[type].enabled_fds;
    QSockNot *sn;

    sn = new QSockNot;
    sn->obj = notifier;
    sn->fd = sockfd;
    sn->queue = &d->sn_vec[type].pending_fds;

    int i;
    for (i = 0; i < list.size(); ++i) {
        QSockNot *p = list.at(i);
        if (p->fd < sockfd)
            break;
        if (p->fd == sockfd) {
            static const char *t[] = { "Read", "Write", "Exception" };
            qWarning("QSocketNotifier: Multiple socket notifiers for "
                      "same socket %d and type %s", sockfd, t[type]);
        }
    }
    list.insert(i, sn);

    FD_SET(sockfd, fds);
    d->sn_highest = qMax(d->sn_highest, sockfd);
}

void QEventDispatcherUNIX::unregisterSocketNotifier(QSocketNotifier *notifier)
{
    int sockfd;
    int type;
    if (!notifier
        || (sockfd = notifier->socket()) < 0
        || unsigned(sockfd) >= FD_SETSIZE
        || (type = notifier->type()) < 0
        || type > 2) {
        qWarning("QSocketNotifier: Internal error");
        return;
    } else if (notifier->thread() != thread()
               || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifiers cannot be disabled from another thread");
        return;
    }

    Q_D(QEventDispatcherUNIX);
    QList<QSockNot *> &list = d->sn_vec[type].list;
    fd_set *fds  =  &d->sn_vec[type].enabled_fds;
    QSockNot *sn;
    int i;
    for (i = 0; i < list.size(); ++i) {
        sn = list.at(i);
        if(sn->obj == notifier && sn->fd == sockfd)
            break;
    }
    if (i == list.size()) // not found
        return;

    FD_CLR(sockfd, fds);                        // clear fd bit
    FD_CLR(sockfd, sn->queue);
    d->sn_pending_list.removeAll(sn);                // remove from activation list
    list.removeAt(i);                                // remove notifier found above
    delete sn;

    if (d->sn_highest == sockfd) {                // find highest fd
        d->sn_highest = -1;
        for (int i=0; i<3; i++) {
            if (!d->sn_vec[i].list.isEmpty())
                d->sn_highest = qMax(d->sn_highest,  // list is fd-sorted
                                     d->sn_vec[i].list.first()->fd);
        }
    }
}

void QEventDispatcherUNIX::setSocketNotifierPending(QSocketNotifier *notifier)
{
    int sockfd;
    int type;
    if (!notifier
        || (sockfd = notifier->socket()) < 0
        || sockfd > FD_SETSIZE
        || (type = notifier->type()) < 0
        || type > 2) {
        qWarning("QSocketNotifier: Internal error");
        return;
    }

    Q_ASSERT(notifier->thread() == thread() && thread() == QThread::currentThread());

    Q_D(QEventDispatcherUNIX);
    QList<QSockNot *> &list = d->sn_vec[type].list;
    QSockNot *sn = 0;
    int i;
    for (i = 0; i < list.size(); ++i) {
        sn = list.at(i);
        if(sn->obj == notifier && sn->fd == sockfd)
            break;
    }
    if (i == list.size()) // not found
        return;

    // We choose a random activation order to be more fair under high load.
    // If a constant order is used and a peer early in the list can
    // saturate the IO, it might grab our attention completely.
    // Also, if we're using a straight list, the callback routines may
    // delete other entries from the list before those other entries are
    // processed.
    if (! FD_ISSET(sn->fd, sn->queue)) {
        d->sn_pending_list.insert((qrand() & 0xff) %
                                  (d->sn_pending_list.size()+1), sn);
        FD_SET(sn->fd, sn->queue);
    }
}

int QEventDispatcherUNIX::activateTimers()
{
    Q_ASSERT(thread() == QThread::currentThread());

    Q_D(QEventDispatcherUNIX);
    if (qt_disable_lowpriority_timers || d->timerList.isEmpty())
        return 0; // nothing to do

    bool first = true;
    timeval currentTime;
    int n_act = 0, maxCount = d->timerList.size();
    QTimerInfo *begin = 0;

    while (maxCount--) {
        currentTime = d->timerList.updateCurrentTime();
        if (first) {
            d->timerList.repairTimersIfNeeded();
            first = false;
        }

        if (d->timerList.isEmpty()) break;
        QTimerInfo *t = d->timerList.first();
        if (currentTime < t->timeout)
            break; // no timer has expired

        if (!begin) {
            begin = t;
        } else if (begin == t) {
            // avoid sending the same timer multiple times
            break;
        } else if (t->interval <  begin->interval || t->interval == begin->interval) {
            begin = t;
        }

        // remove from list
        d->timerList.removeFirst();
        t->timeout += t->interval;
        if (t->timeout < currentTime)
            t->timeout = currentTime + t->interval;

        // reinsert timer
        d->timerList.timerInsert(t);
        if (t->interval.tv_usec > 0 || t->interval.tv_sec > 0)
            n_act++;

        if (!t->inTimerEvent) {
            // send event, but don't allow it to recurse
            t->inTimerEvent = true;

            QTimerEvent e(t->id);
            QCoreApplication::sendEvent(t->obj, &e);

            if (d->timerList.contains(t))
                t->inTimerEvent = false;
        }

        if (!d->timerList.contains(begin)) begin = 0;
    }
    return n_act;
}

int QEventDispatcherUNIX::activateSocketNotifiers()
{
    Q_D(QEventDispatcherUNIX);
    if (d->sn_pending_list.isEmpty())
        return 0;

    // create sets that match the current pending sets (note: we skip
    // the first one, because we activate first and check second)
    fd_set rset, wset, xset;
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_ZERO(&xset);
    for (int i = 1; i < d->sn_pending_list.size(); ++i) {
        const QSockNot * const sn = d->sn_pending_list.at(i);
        switch (sn->obj->type()) {
        case QSocketNotifier::Read:
            FD_SET(sn->fd, &rset);
            break;
        case QSocketNotifier::Write:
            FD_SET(sn->fd, &wset);
            break;
        case QSocketNotifier::Exception:
            FD_SET(sn->fd, &xset);
            break;
        }
    }

    // activate entries
    int n_act = 0;
    QEvent event(QEvent::SockAct);
    while (!d->sn_pending_list.isEmpty()) {
        QSockNot *sn = d->sn_pending_list.takeFirst();
        if (FD_ISSET(sn->fd, sn->queue)) {
            FD_CLR(sn->fd, sn->queue);
            QCoreApplication::sendEvent(sn->obj, &event);
            ++n_act;
        }
    }
    return n_act;
}

bool QEventDispatcherUNIX::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QEventDispatcherUNIX);
    d->interrupt = false;

    // we are awake, broadcast it
    emit awake();
    QCoreApplication::sendPostedEvents(0, (flags & QEventLoop::DeferredDeletion) ? -1 : 0);

    int nevents = 0;
    const bool canWait = (d->threadData->canWait
                          && !d->interrupt
                          && (flags & QEventLoop::WaitForMoreEvents));

    if (canWait)
        emit aboutToBlock();

    if (!d->interrupt) {
        // return the maximum time we can wait for an event.
        timeval *tm = 0;
        timeval wait_tm = { 0l, 0l };
        if (!(flags & QEventLoop::X11ExcludeTimers)) {
            if (d->timerList.timerWait(wait_tm))
                tm = &wait_tm;

            if (!canWait) {
                if (!tm)
                    tm = &wait_tm;

                // no time to wait
                tm->tv_sec  = 0l;
                tm->tv_usec = 0l;
            }
        }

        nevents = d->doSelect(flags, tm);

        // activate timers
        if (! (flags & QEventLoop::X11ExcludeTimers)) {
            nevents += activateTimers();
        }
    }
    // return true if we handled events, false otherwise
    return (nevents > 0);
}

bool QEventDispatcherUNIX::hasPendingEvents()
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return qGlobalPostedEventsCount();
}

void QEventDispatcherUNIX::wakeUp()
{
    Q_D(QEventDispatcherUNIX);
    if (d->wakeUps.testAndSetAcquire(0, 1)) {
        char c = 0;
        ::write( d->thread_pipe[1], &c, 1 );
    }
}

void QEventDispatcherUNIX::interrupt()
{
    Q_D(QEventDispatcherUNIX);
    d->interrupt = true;
    wakeUp();
}

void QEventDispatcherUNIX::flush()
{ }




void QCoreApplication::watchUnixSignal(int sig, bool watch)
{
    if (sig < NSIG) {
        struct sigaction sa;
        sigemptyset(&(sa.sa_mask));
        sa.sa_flags = 0;
        if (watch)
            sa.sa_handler = signalHandler;
        else
            sa.sa_handler = SIG_DFL;
        sigaction(sig, &sa, 0);
    }
}
