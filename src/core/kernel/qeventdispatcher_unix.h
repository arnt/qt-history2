#ifndef QEVENTDISPATCHER_UNIX_H
#define QEVENTDISPATCHER_UNIX_H

#include <sys/types.h>
#include <sys/time.h>
#include "qabstracteventdispatcher.h"

class QEventDispatcherUNIXPrivate;

class QEventDispatcherUNIX : public QAbstractEventDispatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherUNIX)

public:
    QEventDispatcherUNIX(QObject *parent = 0);
    ~QEventDispatcherUNIX();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void registerSocketNotifier(QSocketNotifier *notifier);
    void unregisterSocketNotifier(QSocketNotifier *notifier);

    int registerTimer(int timerInterval, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);

    void wakeUp();
    void interrupt();
    void flush();

#if 0
    void watchUnixSignal(int signal, bool);
signals:
    void unixSignal(int);
#endif

protected:
    QEventDispatcherUNIX(QEventDispatcherUNIXPrivate &, QObject *parent);

    int timeToWait();
    void setSocketNotifierPending(QSocketNotifier *notifier);

    int activateTimers();
    int activateSocketNotifiers();

    virtual int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                       timeval *timeout);
};

#endif // QEVENTDISPATCHER_UNIX_H
