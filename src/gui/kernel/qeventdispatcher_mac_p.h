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

#ifndef QEVENTDISPATCHER_MAC_P_H
#define QEVENTDISPATCHER_MAC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qeventdispatcher_unix_p.h>
#include "qwindowdefs.h"
#include <private/qt_mac_p.h>

class QEventDispatcherMacPrivate;

class QEventDispatcherMac : public QEventDispatcherUNIX
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherMac)

public:
    explicit QEventDispatcherMac(QObject *parent = 0);
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
    int activateTimers();

    QHash<QSocketNotifier *, MacSocketInfo *> *macSockets;
    QList<EventRef> queuedUserInputEvents;
};

#endif // QEVENTDISPATCHER_MAC_P_H
