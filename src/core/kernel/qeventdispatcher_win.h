#ifndef QEVENTDISPATCHERWIN32_H
#define QEVENTDISPATCHERWIN32_H

#include "qabstracteventdispatcher.h"
#include "qt_windows.h"

class QWinEventNotifier;
class QEventDispatcherWin32Private;

class Q_CORE_EXPORT QEventDispatcherWin32 : public QAbstractEventDispatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherWin32)

public:
    QEventDispatcherWin32(QObject *parent = 0);
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

    void wakeUp();
    void interrupt();
    void flush();

private:
    friend LRESULT CALLBACK qt_socketnotifier_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);
    friend void CALLBACK qt_timer_proc(HWND, UINT, UINT idEvent, DWORD);
};

#endif