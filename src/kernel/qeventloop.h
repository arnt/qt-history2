/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QEVENTLOOP_H
#define QEVENTLOOP_H

#ifndef QT_H
#include "qobject.h"
#include "qsocketnotifier.h"
#endif // QT_H

class QEventLoopPrivate;
class QSocketNotifier;
class QTimer;
#ifdef Q_WS_MAC
struct timeval; //stdc struct
struct TimerInfo; //internal structure (qeventloop_mac.cpp)
#endif

#if defined(QT_THREAD_SUPPORT)
class QMutex;
#endif // QT_THREAD_SUPPORT


class Q_EXPORT QEventLoop : public QObject
{
    Q_OBJECT

public:
    QEventLoop( QObject *parent = 0, const char *name = 0 );
    QEventLoop(QEventLoopPrivate *, QObject *parent = 0, const char *name = 0);
    ~QEventLoop();

    enum ProcessEvents {
	AllEvents		= 0x00,
	ExcludeUserInput	= 0x01,
	ExcludeSocketNotifiers	= 0x02,
	WaitForMore		= 0x04
    };
    typedef uint ProcessEventsFlags;

    void processEvents( ProcessEventsFlags flags, int maxtime );
    virtual bool processEvents( ProcessEventsFlags flags );

    virtual bool hasPendingEvents() const;

    virtual void registerSocketNotifier( QSocketNotifier * );
    virtual void unregisterSocketNotifier( QSocketNotifier * );
    void setSocketNotifierPending( QSocketNotifier * );
    int activateSocketNotifiers();

    int activateTimers();
    int timeToWait() const;

    virtual int exec();
    virtual void exit( int retcode = 0 );

    virtual int enterLoop();
    virtual void exitLoop();
    virtual int loopLevel() const;

    virtual void wakeUp();

signals:
    void awake();
    void aboutToBlock();

protected:
    virtual void appStartingUp();
    virtual void appClosingDown();

private:
#if defined(Q_WS_MAC)
    friend void qt_mac_internal_select_callbk(int, int, QEventLoop *);
    friend QMAC_PASCAL void qt_mac_select_timer_callbk(EventLoopTimerRef, void *);
    int macHandleSelect(timeval *);
    void macHandleTimer(TimerInfo *);
#endif // Q_WS_MAC

    // internal initialization/cleanup - implemented in various platform specific files
    void init();
    void cleanup();

    Q_DECL_PRIVATE(QEventLoop);

    friend class QApplication;
};

#endif // QEVENTLOOP_H
