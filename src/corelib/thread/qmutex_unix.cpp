/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#include "qmutex.h"
#include "qstring.h"

#ifndef QT_NO_THREAD
#include "qatomic.h"
#include "qmutex_p.h"

#include <errno.h>

static void report_error(int code, const char *where, const char *what)
{
    if (code != 0)
        qWarning("%s: %s failure: %s", where, what, qPrintable(qt_error_string(code)));
}


QMutexPrivate::QMutexPrivate(QMutex::RecursionMode mode)
    : recursive(mode == QMutex::Recursive), contenders(0), owner(0), count(0), wakeup(false)
{
    report_error(pthread_mutex_init(&mutex, NULL), "QMutex", "mutex init");
    report_error(pthread_cond_init(&cond, NULL), "QMutex", "cv init");
}

QMutexPrivate::~QMutexPrivate()
{
    report_error(pthread_cond_destroy(&cond), "QMutex", "cv destroy");
    report_error(pthread_mutex_destroy(&mutex), "QMutex", "mutex destroy");
}

ulong QMutexPrivate::self()
{ return (ulong) pthread_self(); }

bool QMutexPrivate::wait(int timeout)
{
    report_error(pthread_mutex_lock(&mutex), "QMutex::lock", "mutex lock");
    int errorCode = 0;
    while (!wakeup) {
        if (timeout < 0) {
            errorCode = pthread_cond_wait(&cond, &mutex);
        } else {
            struct timeval tv;
            gettimeofday(&tv, 0);

            timespec ti;
            ti.tv_nsec = (tv.tv_usec + (timeout % 1000) * 1000) * 1000;
            ti.tv_sec = tv.tv_sec + (timeout / 1000) + (ti.tv_nsec / 1000000000);
            ti.tv_nsec %= 1000000000;

            errorCode = pthread_cond_timedwait(&cond, &mutex, &ti);
        }
        if (errorCode) {
            if (errorCode == ETIMEDOUT)
                break;
            report_error(errorCode, "QMutex::lock()", "cv wait");
        }
    }
    wakeup = false;
    report_error(pthread_mutex_unlock(&mutex), "QMutex::lock", "mutex unlock");
    return errorCode == 0;
}

void QMutexPrivate::wakeUp()
{
    report_error(pthread_mutex_lock(&mutex), "QMutex::unlock", "mutex lock");
    wakeup = true;
    report_error(pthread_cond_signal(&cond), "QMutex::unlock", "cv signal");
    report_error(pthread_mutex_unlock(&mutex), "QMutex::unlock", "mutex unlock");
}

#endif // QT_NO_THREAD
