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

#ifndef QEVENTLOOP_H
#define QEVENTLOOP_H

#include "qobject.h"

class QApplication;
class QCoreApplication;
class QSocketNotifier;
class QTimer;

//#define Q_WIN_EVENT_NOTIFIER
#ifdef Q_WIN_EVENT_NOTIFIER
class QWinEventNotifier;
#endif 

class QEventLoopPrivate;

#ifdef Q_OS_WIN
# include <windows.h>
#endif
class Q_CORE_EXPORT QEventLoop : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventLoop)

public:
    QEventLoop(QObject *parent = 0);
    QEventLoop(QEventLoopPrivate &, QObject *parent);
    ~QEventLoop();

    static QEventLoop *instance(QThread *thread = 0);

    enum ProcessEvents {
        AllEvents = 0x00,
        ExcludeUserInput = 0x01,
        ExcludeSocketNotifiers = 0x02,
        WaitForMore = 0x04
    };
    typedef uint ProcessEventsFlags;

    void processEvents(ProcessEventsFlags flags, int maxtime);
    virtual bool processEvents(ProcessEventsFlags flags);
    virtual bool hasPendingEvents() const;

    virtual void registerSocketNotifier(QSocketNotifier *);
    virtual void unregisterSocketNotifier(QSocketNotifier *);
    void setSocketNotifierPending(QSocketNotifier *);
    int activateSocketNotifiers();

#ifdef Q_WIN_EVENT_NOTIFIER
    void registerWinEventNotifier(QWinEventNotifier *);
    void unregisterWinEventNotifier(QWinEventNotifier *);
    int activateWinEventNotifiers();
#endif

    int activateTimers();
    int timeToWait() const;

    virtual int exec();
    virtual void exit(int retcode = 0);

    virtual int registerTimer(int, QObject *);
    virtual bool unregisterTimer(int);
    virtual bool unregisterTimers(QObject *);

    virtual int enterLoop();
    virtual void exitLoop();
    virtual int loopLevel() const;

    virtual void wakeUp();
    virtual void flush();

    typedef bool(*ProcessEventHandler)(void *message);
    typedef bool(*EventFilter)(void *message, long *result);

#ifdef Q_OS_WIN
    virtual void winProcessEvent(void *message);
    virtual bool winEventFilter(void *message, long *result);
#endif

    ProcessEventHandler setProcessEventHandler(ProcessEventHandler handler);
    EventFilter setEventFilter(EventFilter filter);

signals:
    void awake();
    void aboutToBlock();

protected:
    virtual void appStartingUp();
    virtual void appClosingDown();

private:
    // internal initialization/cleanup - implemented in various platform specific files
    void init();
    void cleanup();

    friend class QApplication;
    friend class QCoreApplication;

#ifdef Q_OS_WIN
    friend Q_CORE_EXPORT bool qt_dispatch_timer(uint timerId, MSG *msg);
    friend bool qt_dispatch_socketnotifier(MSG *msg);
#endif
};

#endif // QEVENTLOOP_H
