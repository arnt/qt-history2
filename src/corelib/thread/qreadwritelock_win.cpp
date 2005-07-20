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

#include "qreadwritelock.h"
#include <qatomic.h>
#include <windows.h>
#include "qreadwritelock_p.h"


QReadWriteLock::QReadWriteLock()
:d(new QReadWriteLockPrivate())
{
    d->accessCount = 0;
    d->waitingWriters = 0;
    d->waitingReaders = 0;
    d->readerWait = QT_WA_INLINE(CreateEventW(0, false, false, 0),
                                 CreateEventA(0, false, false, 0));
    if (!d->readerWait)
        qWarning("QReadWriteLock::QReadWriteLock(): Creating reader event failed");
    d->writerWait = QT_WA_INLINE(CreateEventW(0, false, false, 0),
                                 CreateEventA(0, false, false, 0));
    if (!d->writerWait)
        qWarning("QReadWriteLock::QReadWriteLock(): Creating writer event failed");
}

QReadWriteLock::~QReadWriteLock()
{
    CloseHandle(d->readerWait);
    CloseHandle(d->writerWait);
    delete d;
}

void QReadWriteLock::lockForRead()
{
    d->waitingReaders.ref();
    for(;;){
        int localAccessCount(d->accessCount);
        if(d->waitingWriters == 0 && localAccessCount != -1 && localAccessCount < INT_MAX) {
            if (q_atomic_test_and_set_int(&d->accessCount, localAccessCount, localAccessCount + 1))
                break;
        } else {
            if (WaitForSingleObject(d->readerWait, INFINITE) != WAIT_OBJECT_0)
                qWarning("QReadWriteLock::lockForRead(): Waiting on event failed");
            continue;
        }
    }
    d->waitingReaders.deref();
}

bool QReadWriteLock::tryLockForRead()
{
    bool result;
    for(;;){
        int localAccessCount(d->accessCount);
        if(d->waitingWriters == 0 && localAccessCount != -1) {
            if (q_atomic_test_and_set_int(&d->accessCount, localAccessCount, localAccessCount + 1)) {
                result=true;
                break;
            }
        } else {
            result=false;
            break;
        }
    }
    return result;
}

void QReadWriteLock::lockForWrite()
{
    d->waitingWriters.ref();
    for(;;) {
        int localAccessCount(d->accessCount);
        if(localAccessCount == 0){
            if (q_atomic_test_and_set_int(&d->accessCount, 0, -1))
                break;
        } else {
            if (WaitForSingleObject(d->writerWait, INFINITE) != WAIT_OBJECT_0)
                qWarning("QReadWriteLock::lockForWrite(): Waiting on event failed");
        }
    }
    d->waitingWriters.deref();
}

bool QReadWriteLock::tryLockForWrite()
{
    bool result;
    d->waitingWriters.ref();
    for(;;){
        int localAccessCount(d->accessCount);
        if(localAccessCount == 0){
            if (q_atomic_test_and_set_int(&d->accessCount, 0, -1)) {
                result=true;
                break;
            }
        } else {
            result=false;
            break;
        }
    }
    d->waitingWriters.deref();
    return result;
}

void QReadWriteLock::unlock()
{
    Q_ASSERT_X(d->accessCount != 0, "QReadWriteLock::unlock()", "Cannot unlock an unlocked lock");

    bool unlocked = q_atomic_test_and_set_int(&d->accessCount, -1, 0) != 0;
    if (!unlocked) {
        unlocked = q_atomic_decrement(&d->accessCount) == 0;
        if (!unlocked)
            return; // still locked, can't wake anyone up
    }

    if (d->waitingWriters != 0)
        SetEvent(d->writerWait);
    else if (d->waitingReaders != 0)
        SetEvent(d->readerWait);
}
