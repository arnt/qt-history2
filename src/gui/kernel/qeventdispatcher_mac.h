#ifndef QEVENTDISPATCHER_MAC_H
#define QEVENTDISPATCHER_MAC_H

#include "qabstracteventdispatcher.h"
#include "qwindowdefs.h"

class QEventDispatcherMacPrivate;

class QEventDispatcherMac : public QAbstractEventDispatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherMac)

public:
    QEventDispatcherMac(QObject *parent = 0);
    ~QEventDispatcherMac();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void registerSocketNotifier(QSocketNotifier *notifier);
    void unregisterSocketNotifier(QSocketNotifier *notifier);

    int registerTimer(int interval, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);

    void wakeUp();
    void flush();

private:
    friend void qt_mac_select_timer_callbk(__EventLoopTimer*, void*);

};

#endif // QEVENTDISPATCHER_MAC_H
