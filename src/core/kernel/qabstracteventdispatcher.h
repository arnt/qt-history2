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

#ifndef QABSTRACTEVENTDISPATCHER_H
#define QABSTRACTEVENTDISPATCHER_H

#include "qobject.h"
#include "qeventloop.h"

class QAbstractEventDispatcherPrivate;
class QSocketNotifier;

class Q_CORE_EXPORT QAbstractEventDispatcher : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractEventDispatcher)

public:
    QAbstractEventDispatcher(QObject *parent = 0);
    ~QAbstractEventDispatcher();

    static QAbstractEventDispatcher *instance(QThread *thread = 0);

    virtual bool processEvents(QEventLoop::ProcessEventsFlags flags) = 0;
    virtual bool hasPendingEvents() = 0;

    virtual void registerSocketNotifier(QSocketNotifier *notifier) = 0;
    virtual void unregisterSocketNotifier(QSocketNotifier *notifier) = 0;

    virtual int registerTimer(int interval, QObject *object) = 0;
    virtual bool unregisterTimer(int timerId) = 0;
    virtual bool unregisterTimers(QObject *object) = 0;

    virtual void wakeUp() = 0;
    virtual void interrupt() = 0;
    virtual void flush() = 0;

    virtual void startingUp();
    virtual void closingDown();

    typedef bool(*EventFilter)(void *message);
    EventFilter setEventFilter(EventFilter filter);
    bool filterEvent(void *message);

signals:
    void aboutToBlock();
    void awake();

protected:
    QAbstractEventDispatcher(QAbstractEventDispatcherPrivate &, QObject *parent);
};

#endif // QABSTRACTEVENTDISPATCHER_H
