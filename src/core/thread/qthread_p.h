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

#ifndef QTHREAD_P_H
#define QTHREAD_P_H

#include <private/qobject_p.h>
#include "qmutex.h"
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
    QMutex mutex;

    inline QPostEventList()
        : QList<QPostEvent>(), offset(0)
    { }
};

class Q_CORE_EXPORT QThreadData
{
public:
    QThreadData();

    static QThreadData *current();
    static void setCurrent(QThreadData *data);

    static QThreadData *get(QThread *thread);

    QEventLoop *eventLoop;
    QPostEventList postEventList;
    void **tls;
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
    static void finish(void *, bool lockAnyway=true);
#endif // Q_OS_WIN32

    QThreadData data;
};

#endif // QTHREAD_P_H
