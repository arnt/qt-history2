#ifndef QTHREAD_P_H
#define QTHREAD_P_H

#include <private/qobject_p.h>
#include "qwaitcondition.h"

class QEventLoop;

class QPostEvent
{
public:
    QObject *receiver;
    QEvent *event;
    inline QPostEvent()
        : receiver(0), event(0)
    { }
    inline QPostEvent(QObject *r, QEvent *e)
        : receiver(r), event(e)
    { }
};

class QPostEventList : public QList<QPostEvent>
{
public:
    int offset;
    QSpinLock spinlock;

    inline QPostEventList()
        : QList<QPostEvent>(), offset(0)
    { }
};

class Q_CORE_EXPORT QThreadPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QThread)

public:
    QThreadPrivate();

    QMutex *mutex() const;

    bool running;
    bool finished;
    bool terminated;

    uint stackSize;

#ifdef Q_OS_UNIX
    pthread_t thread_id;
    QWaitCondition thread_done;

    static void *start(void *arg);
    static void finish(void *arg);
#endif

#ifdef Q_OS_WIN32
    HANDLE handle;
    unsigned int id;
    int waiters;

    static unsigned int __stdcall start(void *);
    static void finish(QThreadInstance *);
#endif // Q_OS_WIN32

    QEventLoop *eventloop;
    static void setEventLoop(QThread *thread, QEventLoop *eventLoop);
    static QEventLoop *eventLoop(QThread *thread);

    QPostEventList postedEvents;
    static QPostEventList *postEventList(QThread *thread);

    void **tls;
    static void **&threadLocalStorage(QThread *thread);
};

#endif // QTHREAD_P_H
