#ifndef QEVENTDISPATCHER_UNIX_P_H
#define QEVENTDISPATCHER_UNIX_P_H

#include "qabstracteventdispatcher_p.h"

class QBitArray;

// internal timer info
struct QTimerInfo;

// list of TimerInfo structs
typedef QList<QTimerInfo*> QTimerList;

struct Q_CORE_EXPORT QSockNot
{
    QSocketNotifier *obj;
    int fd;
    fd_set *queue;
};

class Q_CORE_EXPORT QSockNotType
{
public:
    QSockNotType();
    ~QSockNotType();

    QList<QSockNot*> list;
    fd_set select_fds;
    fd_set enabled_fds;
    fd_set pending_fds;

};

class Q_CORE_EXPORT QEventDispatcherUNIXPrivate : public QAbstractEventDispatcherPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherUNIX)

public:
    QEventDispatcherUNIXPrivate();
    ~QEventDispatcherUNIXPrivate();

#if 0
    void handleSignals();
#endif

    int eventloopSelect(uint, timeval *);
    int thread_pipe[2];

    // watch if time is turned back
    timeval watchtime;

    // highest fd for all socket notifiers
    int sn_highest;
    // 3 socket notifier types - read, write and exception
    QSockNotType sn_vec[3];

    QBitArray *timerBitVec;
    QTimerList *timerList;
    bool timerWait(timeval &);
    void timerInsert(QTimerInfo *);
    void timerRepair(const timeval &);

    // pending socket notifiers list
    QList<QSockNot*> sn_pending_list;

    bool interrupt;
};

#endif // QEVENTDISPATCHER_UNIX_P_H
