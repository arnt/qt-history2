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

#ifndef QEVENTDISPATCHER_WIN_H
#define QEVENTDISPATCHER_WIN_H

#include "QtCore/qabstracteventdispatcher.h"
#include "QtCore/qt_windows.h"

class QWinEventNotifier;
class QEventDispatcherWin32Private;

class Q_CORE_EXPORT QEventDispatcherWin32 : public QAbstractEventDispatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherWin32)

public:
    explicit QEventDispatcherWin32(QObject *parent = 0);
    ~QEventDispatcherWin32();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void registerSocketNotifier(QSocketNotifier *notifier);
    void unregisterSocketNotifier(QSocketNotifier *notifier);

    int registerTimer(int interval, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);

    bool registerEventNotifier(QWinEventNotifier *notifier);
    void unregisterEventNotifier(QWinEventNotifier *notifier);
    void activateEventNotifiers();

    void wakeUp();
    void interrupt();
    void flush();

    void startingUp();

private:
    friend LRESULT CALLBACK qt_socketnotifier_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);
    friend void CALLBACK qt_timer_proc(HWND, UINT, UINT idEvent, DWORD);
};

#endif // QEVENTDISPATCHER_WIN_H
