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
#include "qatomic.h"
#include <windows.h>
#include "qreadwritelock_p.h"

QReadWriteLockPrivate::QReadWriteLockPrivate()
    : lock(0), accessCount(0), waitingReaders(0), waitingWriters(0)
{
    InitializeCriticalSection(&cs);
    readerWait = QT_WA_INLINE(CreateEventW(0, true, false, 0),
                              CreateEventA(0, true, false, 0));
    if (!readerWait)
        qWarning("QReadWriteLock::QReadWriteLock(): Creating reader event failed");
    writerWait = QT_WA_INLINE(CreateEventW(0, false, false, 0),
                              CreateEventA(0, false, false, 0));
    if (!writerWait)
        qWarning("QReadWriteLock::QReadWriteLock(): Creating writer event failed");
}

QReadWriteLockPrivate::~QReadWriteLockPrivate()
{
    DeleteCriticalSection(&cs);
    CloseHandle(readerWait);
    CloseHandle(writerWait);
}

void QReadWriteLockPrivate::wait(bool reader)
{
    EnterCriticalSection(&cs);
    if (reader)
        ++waitingReaders;
    else
        ++waitingWriters;
    LeaveCriticalSection(&cs);
    if (WaitForSingleObject(reader ? readerWait : writerWait, INFINITE) != WAIT_OBJECT_0)
        qWarning("QReadWriteLock::lockForRead(): Waiting on event failed");
    EnterCriticalSection(&cs);
    if (reader) {
        if (!--waitingReaders)
            ResetEvent(readerWait);
    } else {
        --waitingWriters;
    }
    LeaveCriticalSection(&cs);
}

void QReadWriteLockPrivate::wakeUp()
{
    EnterCriticalSection(&cs);
    if (waitingWriters > 0)
        SetEvent(writerWait);
    else
        SetEvent(readerWait);
    LeaveCriticalSection(&cs);
}
