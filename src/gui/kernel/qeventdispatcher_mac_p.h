#ifndef QEVENTDISPATCHER_MAC_P_H
#define QEVENTDISPATCHER_MAC_P_H

#include "qeventdispatcher_unix.h"
#include "qwindowdefs.h"
#include <private/qt_mac_p.h>

class QEventDispatcherMacPrivate;

class QEventDispatcherMac : public QEventDispatcherUNIX
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
    friend class QApplicationPrivate;
};

#include <private/qeventdispatcher_unix_p.h>

struct MacTimerInfo {
    int id;
    int interval;
    QObject *obj;
    bool pending;
    EventLoopTimerRef mac_timer;
};
typedef QList<MacTimerInfo> MacTimerList;

struct MacSocketInfo {
    union {
        CFReadStreamRef read_not;
        CFWriteStreamRef write_not;
    };
};

class QEventDispatcherMacPrivate : public QEventDispatcherUNIXPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherMac)

public:
    QEventDispatcherMacPrivate();

    int zero_timer_count;
    EventLoopTimerRef select_timer;
    MacTimerList *macTimerList;
    void activateTimers();

    QHash<QSocketNotifier *, MacSocketInfo *> *macSockets;
};

#endif // QEVENTDISPATCHER_MAC_P_H
