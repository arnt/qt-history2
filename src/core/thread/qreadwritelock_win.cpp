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


QReadWriteLock::QReadWriteLock(const int maxReaders)
:d(new QReadWriteLockPrivate())
{
    d->maxReaders=maxReaders;
    d->accessCount=0;
    d->waitingWriters=0;
    d->readerWait = CreateEvent(0, false, false, 0);
    d->writerWait = CreateEvent(0, false, false, 0);
}

QReadWriteLock::~QReadWriteLock()
{
    CloseHandle(d->readerWait);
    CloseHandle(d->writerWait);
    delete d;
}

void QReadWriteLock::lock(AccessMode mode)
{
    if(mode==ReadAccess) {
        for(;;){
            int localAccessCount(d->accessCount);
            if(d->waitingWriters == 0 && localAccessCount != -1 && localAccessCount <= d->maxReaders) {
                if (q_atomic_test_and_set_int(&d->accessCount, localAccessCount, localAccessCount + 1))
                     break;
            }else {
                WaitForSingleObject(d->readerWait, INFINITE);
                continue;
            }
        }
    } else { //mode==WriteAccess
        ++d->waitingWriters;
        for(;;) {
            int localAccessCount(d->accessCount);
            if(localAccessCount == 0){
                if (q_atomic_test_and_set_int(&d->accessCount, 0, -1))
                    break;
            } else {
                WaitForSingleObject(d->writerWait, INFINITE);
            }
        }
        --d->waitingWriters;
    }
}

bool QReadWriteLock::tryLock(AccessMode mode)
{
    bool result;
    if(mode==ReadAccess) {
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
    } else { //mode==WriteAccess
        ++d->waitingWriters;
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
        --d->waitingWriters;
    }
    return result;
}

void QReadWriteLock::unlock()
{
    for (;;) {
        int localAccessCount=d->accessCount;
        if (localAccessCount==0) {
            qFatal("QReadWriteLock::unlock(): Trying to unlock a unlocked lock");
            break;
        }
        if (localAccessCount==-1) {
            if(q_atomic_test_and_set_int(&d->accessCount, -1, 0))
                break;
        } else {
            if(q_atomic_test_and_set_int(&d->accessCount, localAccessCount, localAccessCount - 1 ))
                break;
        }
    }
    if(d->waitingWriters != 0)
        SetEvent(d->writerWait);
    else
        SetEvent(d->readerWait);
}
