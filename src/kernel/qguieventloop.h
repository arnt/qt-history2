#ifndef QGUIEVENTLOOP_H
#define QGUIEVENTLOOP_H

#include <qeventloop.h>
#include <qwindowdefs.h>

class QGuiEventLoopPrivate;

class QGuiEventLoop : public QEventLoop
{
    Q_OBJECT
public:
    QGuiEventLoop(QObject *parent = 0);
    ~QGuiEventLoop();

    virtual bool processEvents( ProcessEventsFlags flags );

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
    virtual bool hasPendingEvents() const;
#elif defined(Q_WS_MAC)
    int registerTimer(int interval, QObject *obj);
    bool unregisterTimer(int id);
    bool unregisterTimers(QObject *obj);
    void registerSocketNotifier(QSocketNotifier *notifier);
    void unregisterSocketNotifier(QSocketNotifier *notifier);
    bool hasPendingEvents() const;
    int  activateTimers();
    void wakeUp();
#endif

    void flush();

protected:
#if defined(Q_WS_X11)
    virtual void appStartingUp();
    virtual void appClosingDown();
#endif
private:
    // internal initialization/cleanup - implemented in various platform specific files
    void init();
    void cleanup();

#if defined(Q_WS_MAC)
    friend class QApplication;
    friend QMAC_PASCAL void qt_mac_select_timer_callbk(EventLoopTimerRef, void *);
#endif
    Q_DECL_PRIVATE(QGuiEventLoop);
};


#endif
