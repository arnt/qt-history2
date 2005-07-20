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

#include "qplatformdefs.h"
#include "qreadwritelock.h"
#include "qatomic.h"
#include "qreadwritelock_p.h"

#include <errno.h>
#include <string.h>
#include <pthread.h>

/*
    Duplicated code from qmutex_unix.cpp
*/
static void report_error(int code, const char *where, const char *what)
{
    if (code != 0)
        qWarning("%s: %s failure: %s", where, what, strerror(code));
}


QReadWriteLockPrivate::QReadWriteLockPrivate()
    : lock(0), accessCount(0), waitingReaders(0), waitingWriters(0), wakeup(0)
{
    report_error(pthread_mutex_init(&mutex, NULL), "QReadWriteLock", "mutex init");
    report_error(pthread_cond_init(&readerWait, NULL), "QReadWriteLock", "reader cv init");
    report_error(pthread_cond_init(&writerWait, NULL), "QReadWriteLock", "writer cv init");
}

QReadWriteLockPrivate::~QReadWriteLockPrivate()
{
    report_error(pthread_cond_destroy(&writerWait), "QReadWriteLock", "writer cv destroy");
    report_error(pthread_cond_destroy(&readerWait), "QReadWriteLock", "reader cv destroy");
    report_error(pthread_mutex_destroy(&mutex), "QReadWriteLock", "mutex destroy");
}

void QReadWriteLockPrivate::wait(bool reader)
{
    report_error(pthread_mutex_lock(&mutex), "QReadWriteLock", "mutex lock");
    do {
        if (reader) {
            ++waitingReaders;
            report_error(pthread_cond_wait(&readerWait, &mutex),
                         "QReadWriteLock",
                         "reader cv wait");
            --waitingReaders;
        } else {
            ++waitingWriters;
            report_error(pthread_cond_wait(&writerWait, &mutex),
                         "QReadWriteLock",
                         "writer cv wait");
            --waitingWriters;
        }
    } while (!wakeup);
    --wakeup;
    report_error(pthread_mutex_unlock(&mutex), "QReadWriteLock", "mutex unlock");
}

void QReadWriteLockPrivate::wakeUp()
{
    report_error(pthread_mutex_lock(&mutex), "QReadWriteLock", "mutex lock");
    if (waitingWriters > 0) {
        // wake 1 writer
        wakeup = 1;
        report_error(pthread_cond_signal(&writerWait), "QReadWriteLock", "writer cv signal");
    } else {
        // wake all readers
        wakeup = waitingReaders;
        report_error(pthread_cond_broadcast(&readerWait), "QReadWriteLock", "reader cv signal");
    }
    report_error(pthread_mutex_unlock(&mutex), "QReadWriteLock", "mutex unlock");
}
