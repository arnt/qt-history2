#include "qplatformdefs.h"
#include "qeventdispatcher_unix.h"

#include "qbitarray.h"
#include "qcoreapplication.h"
#include "qsocketnotifier.h"
#include "qthread.h"

#include "qeventdispatcher_unix_p.h"
#include <private/qthread_p.h>

#include <errno.h>
#include <stdio.h>

bool qt_disable_lowpriority_timers=false;

// Internal operator functions for timevals
static inline bool operator<(const timeval &t1, const timeval &t2)
{ return t1.tv_sec < t2.tv_sec || (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec); }
static inline bool operator==(const timeval &t1, const timeval &t2)
{ return t1.tv_sec == t2.tv_sec && t1.tv_usec == t2.tv_usec; }
static inline timeval &operator+=(timeval &t1, const timeval &t2)
{
    t1.tv_sec += t2.tv_sec;
    if ((t1.tv_usec += t2.tv_usec) >= 1000000l) {
        ++t1.tv_sec;
        t1.tv_usec -= 1000000l;
    }
    return t1;
}
static inline timeval operator+(const timeval &t1, const timeval &t2)
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec + t2.tv_sec;
    if ((tmp.tv_usec = t1.tv_usec + t2.tv_usec) >= 1000000l) {
        ++tmp.tv_sec;
        tmp.tv_usec -= 1000000l;
    }
    return tmp;
}
static inline timeval operator-(const timeval &t1, const timeval &t2)
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec - t2.tv_sec;
    if ((tmp.tv_usec = t1.tv_usec - t2.tv_usec) < 0l) {
        --tmp.tv_sec;
        tmp.tv_usec += 1000000l;
    }
    return tmp;
}

// get time of day
static inline void getTime(timeval &t)
{
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

struct QTimerInfo {
    int id;           // - timer identifier
    timeval interval; // - timer interval
    timeval timeout;  // - when to sent event
    QObject *obj;     // - object to receive event
};


QEventDispatcherUNIXPrivate::QEventDispatcherUNIXPrivate()
{
    // initialize the common parts of the event loop
    pipe(thread_pipe);
    fcntl(thread_pipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(thread_pipe[1], F_SETFD, FD_CLOEXEC);

    sn_highest = -1;
    timerList = 0;
    timerBitVec = 0;
}

QEventDispatcherUNIXPrivate::~QEventDispatcherUNIXPrivate()
{
    // cleanup the common parts of the event loop
    close(thread_pipe[0]);
    close(thread_pipe[1]);

    //cleanup timer
    if (timerList) {
        while (!timerList->isEmpty())
            delete timerList->takeFirst();
        delete timerList;
        timerList = 0;
    }
    delete timerBitVec;
    timerBitVec = 0;
}

/*
 * Internal functions for manipulating timer data structures.  The
 * timerBitVec array is used for keeping track of timer identifiers.
 */

/*
  insert timer info into list
*/ void QEventDispatcherUNIXPrivate::timerInsert(QTimerInfo *ti)
{
    int index = 0;
#if defined(QT_DEBUG)
    int dangerCount = 0;
#endif
    for (; index < timerList->size(); ++index) {
        register const QTimerInfo * const t = timerList->at(index);
#if defined(QT_DEBUG)
        if (t->obj == ti->obj) ++dangerCount;
#endif
        if (ti->timeout < t->timeout) break;
    }
    timerList->insert(index, ti);

#if defined(QT_DEBUG)
    if (dangerCount > 16)
        qDebug("QObject: %d timers now exist for object %s::%s",
                dangerCount, ti->obj->metaObject()->className(),
                ti->obj->objectName().local8Bit());
#endif
}

/*
  repair broken timer
*/
void QEventDispatcherUNIXPrivate::timerRepair(const timeval &time)
{
    timeval diff = watchtime - time;
    // repair all timers
    for (int i = 0; i < timerList->size(); ++i) {
        register QTimerInfo *t = timerList->at(i);
        t->timeout = t->timeout - diff;
    }
}

/*
  Returns the time to wait for the next timer, or null if no timers
  are waiting.
*/
bool QEventDispatcherUNIXPrivate::timerWait(timeval &tm)
{
    if (!timerList || timerList->isEmpty())
        return false;

    timeval currentTime;
    getTime(currentTime);
    if (currentTime < watchtime)        // clock was turned back
        timerRepair(currentTime);
    watchtime = currentTime;

    QTimerInfo *t = timerList->first();        // first waiting timer
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
{ return ::select(nfds, readfds, writefds, exceptfds, timeout); }

/*!
    \internal
*/
int QEventDispatcherUNIX::registerTimer(int interval, QObject *obj)
{
    Q_ASSERT_X(obj->thread() == thread() && thread() == QThread::currentQThread(),
               "QEventDispatcherUNIX::registerTimer",
               "timers cannot be started from another thread");

    Q_D(QEventDispatcherUNIX);
    if (!d->timerList) {                                // initialize timer data
        d->timerBitVec = new QBitArray(128);
        int i = d->timerBitVec->size();
        while(i-- > 0)
            d->timerBitVec->clearBit(i);
        d->timerList = new QTimerList;
        gettimeofday(&d->watchtime, 0);
    }

    int id = 0;
    {
        int i = d->timerBitVec->size()-1;
        while (i >= 0 && (*d->timerBitVec)[i])
            i--;
        if (i < 0) {
            i = d->timerBitVec->size();
            d->timerBitVec->resize(4 * i);
            for(int j=d->timerBitVec->size()-1; j > i; j--)
                d->timerBitVec->clearBit(j);
        }
        d->timerBitVec->setBit(i);
        id = i+1;
    }
    if (id <= 0 ||
         id > (int)d->timerBitVec->size() || !obj)// cannot create timer
        return 0;
    d->timerBitVec->setBit(id-1);                // set timer active
    QTimerInfo *t = new QTimerInfo;                // create timer
    t->id = id;
    t->interval.tv_sec  = interval / 1000;
    t->interval.tv_usec = (interval % 1000) * 1000;
    timeval currentTime;
    getTime(currentTime);
    t->timeout = currentTime + t->interval;
    t->obj = obj;
    d->timerInsert(t);                                // put timer in list
    return id;
}

/*!
    \internal
*/
bool QEventDispatcherUNIX::unregisterTimer(int id)
{
    Q_D(QEventDispatcherUNIX);
    if (!d->timerList || id <= 0 ||
        id > (int)d->timerBitVec->size() || !d->timerBitVec->testBit(id-1)) {
        qDebug("QEventDispatcherUNIX::unregisterTimer(): %d is not not a valid timer", id);
        return false;                                // not init'd or invalid timer
    }

    for (int i = 0; i < d->timerList->size(); ++i) {
        register QTimerInfo *t = d->timerList->at(i);
        if (t->id == id) {
            d->timerBitVec->clearBit(id - 1);        // set timer inactive
            d->timerList->removeAt(i);
            delete t;
            return true;
        }
    }

    qDebug("QEventDispatcherUNIX::unregisterTimer(): %d not found", id);
    return false; // id not found
}

/*!
    \internal
*/
bool QEventDispatcherUNIX::unregisterTimers(QObject *obj)
{
    Q_ASSERT_X(obj->thread() == thread() && thread() == QThread::currentQThread(),
               "QEventDispatcherUNIX::unregisterTimers",
               "timers cannot be stopped from another thread");

    Q_D(QEventDispatcherUNIX);
    if (!d->timerList)                                // not initialized
        return false;

    for (int i = 0; i < d->timerList->size(); ++i) {
        register QTimerInfo *t = d->timerList->at(i);
        if (t->obj == obj) {
            // object found
            d->timerBitVec->clearBit(t->id - 1);
            d->timerList->removeAt(i);
            delete t;

            // move back one so that we don't skip the new current item
            --i;
        }
    }

    return true;
}

#if 0
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

void QEventDispatcherUNIX::watchUnixSignal(int sig, bool watch)
{
    if (sig < NSIG) {
        struct sigaction sa;
        sigemptyset(&(sa.sa_mask));
        sa.sa_flags = 0;
        sa.sa_handler = watch ? signalHandler : SIG_DFL;
        sigaction(sig, &sa, 0);
    }
}


void QEventDispatcherUNIXPrivate::handleSignals()
{
    for (int i=0; i < NSIG; ++i) {
        if (signals_fired[i]) {
            signals_fired[i]=0;
            emit q->unixSignal(i);
        }
    }
}
#endif

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
    Q_ASSERT(notifier->thread() == thread() && thread() == QThread::currentQThread());

    int sockfd = notifier->socket();
    int type = notifier->type();
    if (sockfd < 0 || sockfd >= FD_SETSIZE || type < 0 || type > 2 || notifier == 0) {
        qWarning("QSocketNotifier: Internal error");
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
            static const char *t[] = { "read", "write", "exception" };
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
    Q_ASSERT(notifier->thread() == thread() && thread() == QThread::currentQThread());

    int sockfd = notifier->socket();
    int type = notifier->type();
    if (sockfd < 0 || type < 0 || type > 2 || notifier == 0) {
        qWarning("QSocketNotifier: Internal error");
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
    Q_ASSERT(notifier->thread() == thread() && thread() == QThread::currentQThread());

    int sockfd = notifier->socket();
    int type = notifier->type();
    if (sockfd < 0 || type < 0 || type > 2 || notifier == 0) {
        qWarning("QSocketNotifier: Internal error");
        return;
    }

    Q_D(QEventDispatcherUNIX);
    QList<QSockNot *> &list = d->sn_vec[type].list;
    QSockNot *sn;
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
        d->sn_pending_list.insert((rand() & 0xff) %
                                   (d->sn_pending_list.size()+1), sn);
        FD_SET(sn->fd, sn->queue);
    }
}

void QEventDispatcherUNIX::wakeUp()
{
    /*
      Apparently, there is not consistency among different operating
      systems on how to use FIONREAD.

      FreeBSD, Linux and Solaris all expect the 3rd argument to
      ioctl() to be an int, which is normally 32-bit even on 64-bit
      machines.

      IRIX, on the other hand, expects a size_t, which is 64-bit on
      64-bit machines.

      So, the solution is to use size_t initialized to zero to make
      sure all bits are set to zero, preventing underflow with the
      FreeBSD/Linux/Solaris ioctls.
    */
    size_t nbytes = 0;
    char c = 0;
    Q_D(QEventDispatcherUNIX);
    if (::ioctl(d->thread_pipe[0], FIONREAD, (char*)&nbytes) >= 0 && nbytes == 0) {
        ::write( d->thread_pipe[1], &c, 1 );
    }
}

int QEventDispatcherUNIX::timeToWait()
{
    timeval tm;
    Q_D(QEventDispatcherUNIX);
    if (!d->timerWait(tm))
        return -1; // no active timers
    return (tm.tv_sec * 1000l) + (tm.tv_usec / 1000l);
}

int QEventDispatcherUNIX::activateTimers()
{
    Q_ASSERT(thread() == QThread::currentQThread());

    Q_D(QEventDispatcherUNIX);
    if (qt_disable_lowpriority_timers || !d->timerList || d->timerList->isEmpty())
        return 0; // nothing to do

    bool first = true;
    timeval currentTime;
    int n_act = 0, maxCount = d->timerList->size();
    QTimerInfo *begin = 0;

    while (maxCount--) {
        getTime(currentTime);
        if (first) {
            if (currentTime < d->watchtime)
                d->timerRepair(currentTime); // clock was turned back

            first = false;
            d->watchtime = currentTime;
        }

        if (d->timerList->isEmpty()) break;
        register QTimerInfo *t = d->timerList->first();
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
        d->timerList->removeFirst();
        t->timeout += t->interval;
        if (t->timeout < currentTime)
            t->timeout = currentTime + t->interval;

        // reinsert timer
        d->timerInsert(t);
        if (t->interval.tv_usec > 0 || t->interval.tv_sec > 0)
            n_act++;

        // send event
        QTimerEvent e(t->id);
        QCoreApplication::sendEvent(t->obj, &e);

        if (!d->timerList->contains(begin)) begin = 0;
    }
    return n_act;
}

int QEventDispatcherUNIX::activateSocketNotifiers()
{
    Q_D(QEventDispatcherUNIX);
    if (d->sn_pending_list.isEmpty())
        return 0;

    // activate entries
    int n_act = 0;
    QEvent event(QEvent::SockAct);
    while (!d->sn_pending_list.isEmpty()) {
        QSockNot *sn = d->sn_pending_list.takeFirst();
        if (FD_ISSET(sn->fd, sn->queue)) {
            FD_CLR(sn->fd, sn->queue);
            QCoreApplication::sendEvent(sn->obj, &event);
            n_act++;
        }
    }

    return n_act;
}

int QEventDispatcherUNIXPrivate::eventloopSelect(uint flags, timeval *t)
{
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

#if 0
#ifdef Q_WS_X11
    if (xfd != -1) {
        // select for events on the event socket - only on X11
        FD_SET(xfd, &sn_vec[0].select_fds);
        highest = qMax(highest, xfd);
    }
#endif
#endif

    FD_SET(thread_pipe[0], &sn_vec[0].select_fds);
    highest = qMax(highest, thread_pipe[0]);

    int nsel;
    do {
#if 0
        while (signal_received) {
            signal_received=0;
            d->handleSignals();
        }
#endif
        nsel = q_func()->select(highest + 1,
                                &sn_vec[0].select_fds,
                                &sn_vec[1].select_fds,
                                &sn_vec[2].select_fds,
                                t);
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
                        qWarning("QSocketNotifier: invalid socket %d and type '%s', disabling...",
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
    if (nsel > 0 && FD_ISSET(thread_pipe[0], &sn_vec[0].select_fds)) {
        char c;
        ::read(thread_pipe[0], &c, 1);
    }

    // activate socket notifiers
    Q_Q(QEventDispatcherUNIX);
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
    return q->activateSocketNotifiers();
}

bool QEventDispatcherUNIX::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QEventDispatcherUNIX);

    int nevents = 0;

#if 0
    if (d->shortcut)
        return false;
#endif

    QCoreApplication::sendPostedEvents();

    QThreadData *data = QThreadData::current();
    const bool canWait = (data->postEventList.size() == 0
#if 0
                          && !d->shortcut
                          && !d->exitloop
                          && !d->quitnow
#endif
                          && (flags & QEventLoop::WaitForMoreEvents));

    if (canWait)
        emit aboutToBlock();

#if 0
    if (d->shortcut)
        return false;
#endif

    // return the maximum time we can wait for an event.
    timeval *tm = 0;
    timeval wait_tm = { 0l, 0l };
    if (!(flags & 0x08)) {                        // 0x08 == ExcludeTimers for X11 only
        if (d->timerWait(wait_tm))
            tm = &wait_tm;

        if (!canWait) {
            if (!tm) tm = &wait_tm;

            // no time to wait
            tm->tv_sec  = 0l;
            tm->tv_usec = 0l;
        }
    }

    nevents += d->eventloopSelect(flags, tm);

#if 0
    if (d->shortcut)
        return false;
#endif

    // we are awake, broadcast it
    emit awake();

    // activate timers
    if (! (flags & 0x08)) {
        // 0x08 == ExcludeTimers for X11 only
        nevents += activateTimers();
    }

    // return true if we handled events, false otherwise
    return (nevents > 0);
}

bool QEventDispatcherUNIX::hasPendingEvents()
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return qGlobalPostedEventsCount();
}

void QEventDispatcherUNIX::flush()
{ }
