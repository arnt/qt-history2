#ifndef QEVENTDISPATCHER_MAC_P_H
#define QEVENTDISPATCHER_MAC_P_H

#include <private/qabstracteventdispatcher_p.h>

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

class QEventDispatcherMacPrivate : public QAbstractEventDispatcherPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherMac)

public:
    QEventDispatcherMacPrivate();

    int zero_timer_count;
    MacTimerList *macTimerList;
    void activateTimers();

    QHash<QSocketNotifier *, MacSocketInfo *> *macSockets;
};

#endif // QEVENTDISPATCHER_MAC_P_H
