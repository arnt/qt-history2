#ifndef QEVENTDISPATCHERWIN32_H
#define QEVENTDISPATCHERWIN32_H

#include "qabstracteventdispatcher.h"

class QEventDispatcherWin32Private;

class Q_CORE_EXPORT QEventDispatcherWin32 : public QAbstractEventDispatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherWin32)

public:
    QEventDispatcherWin32(QObject *parent = 0);
    ~QEventDispatcherWin32();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void registerSocketNotifier(QSocketNotifier *notifier);
    void unregisterSocketNotifier(QSocketNotifier *notifier);

    int registerTimer(int interval, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);

    void wakeUp();
    void interrupt();
    void flush();

    virtual void winProcessEvent(void *message);
    virtual bool winEventFilter(void *message, long *res);

private:
    friend bool qt_dispatch_socketnotifier(MSG *msg);
    friend Q_CORE_EXPORT bool qt_dispatch_timer(uint timerId, MSG *msg);
};

#endif