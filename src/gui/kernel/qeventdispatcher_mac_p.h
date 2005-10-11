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

#include "QtGui/qwindowdefs.h"
#include "qhash.h"
#include "private/qeventdispatcher_unix_p.h"
#include "private/qt_mac_p.h"

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

    void registerTimer(int timerId, int interval, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);
    QList<TimerInfo> registeredTimers(QObject *object) const;

    void wakeUp();
    void flush();

private:
    friend void qt_mac_select_timer_callbk(__EventLoopTimer*, void*);
    friend class QApplicationPrivate;
};

struct MacTimerInfo {
    int id;
    int interval;
    QObject *obj;
    bool pending;
    EventLoopTimerRef mac_timer;
};
typedef QList<MacTimerInfo> MacTimerList;

struct MacSocketInfo {
    MacSocketInfo()
    :socket(0), refcount(0) {}
    CFSocketRef socket;
    int refcount;
};
typedef QHash<int, MacSocketInfo *> MacSocketHash; 

class QEventDispatcherMacPrivate : public QEventDispatcherUNIXPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherMac)

public:
    QEventDispatcherMacPrivate();

    int zero_timer_count;
    MacTimerList *macTimerList;
    int activateTimers();

    MacSocketHash macSockets;
    QList<EventRef> queuedUserInputEvents;
};

#endif // QEVENTDISPATCHER_MAC_P_H
