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

#ifndef QEVENTDISPATCHER_UNIX_H
#define QEVENTDISPATCHER_UNIX_H

#include "QtCore/qabstracteventdispatcher.h"
#include <sys/types.h>
#include <sys/time.h>

class QEventDispatcherUNIXPrivate;

class Q_CORE_EXPORT QEventDispatcherUNIX : public QAbstractEventDispatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherUNIX)

public:
    explicit QEventDispatcherUNIX(QObject *parent = 0);
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

protected:
    QEventDispatcherUNIX(QEventDispatcherUNIXPrivate &, QObject *parent);

    void setSocketNotifierPending(QSocketNotifier *notifier);

    int activateTimers();
    int activateSocketNotifiers();

    virtual int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                       timeval *timeout);
};

#endif // QEVENTDISPATCHER_UNIX_H
