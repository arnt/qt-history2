#ifndef QEVENTLOOP_H
#define QEVENTLOOP_H

#include <qobject.h>
#include <qsocketnotifier.h>

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
    QEventLoop( QObject *parent, const char *name = 0 );
    virtual ~QEventLoop();

    enum ProcessEvents {
	AllEvents              = 0x00,
	ExcludeUserInput       = 0x01,
	ExcludeSocketNotifiers = 0x02,
	ExcludePOSIXSignals    = 0x04
    };
    typedef uint ProcessEventsFlags;

    virtual void processOneEvent( ProcessEventsFlags flags );
    virtual void processEvents( ProcessEventsFlags flags, int maxtime = 3000 );
    virtual bool hasPendingEvents() const;

    virtual void registerSocketNotifier( QSocketNotifier * );
    virtual void unregisterSocketNotifier( QSocketNotifier * );
    void setSocketNotifierPending( QSocketNotifier * );
    int activateSocketNotifiers();

    int activateTimers();
    int timeToWait() const;

    virtual int exec();
    virtual void exit( int retcode = 0 );

    int enterLoop();
    void exitLoop();
    int loopLevel() const;

    virtual void wakeUp();

#if defined(QT_THREAD_SUPPORT)
    QMutex *mutex() const;
#endif // QT_THREAD_SUPPORT

signals:
    void awake();

protected:
    virtual bool processNextEvent( ProcessEventsFlags flags, bool canWait );

#if defined(Q_WS_MAC)
    int macHandleSelect(timeval *);
    void macHandleTimer(TimerInfo *);
#endif // Q_WS_MAC

private:
    // internal initialization/cleanup - implemented in various platform specific files
    void init();
    void cleanup();

    // data for the default implementation - other implementations should not
    // use/need this data
    QEventLoopPrivate *d;

    friend class QApplication;
};

#endif // QEVENTLOOP_H
