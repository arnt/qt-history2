/****************************************************************************
**
** Implementation of QMutex class for Win32.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <windows.h>

#include "qmutex.h"
#include <qatomic.h>
#include "qmutex_p.h"

#if defined(Q_CC_BOR)
static bool compare_and_set(unsigned long * volatile pointer, long expected, long newval)
#else
inline bool compare_and_set(unsigned long * volatile pointer, long expected, long newval)
#endif
{
    unsigned char result;
    __asm {
	mov EBX,pointer
	mov EAX,expected
	mov ECX,newval
	lock cmpxchg dword ptr[EBX],ECX
	sete result
    }
    return (result != 0);
}

QMutex::QMutex(bool recursive)
{
    d = new QMutexPrivate;
    d->recursive = recursive;
    d->owner = 0;
    d->count = 0;
    d->waiters = 0;
    d->event = CreateEvent(0, FALSE, FALSE, 0);
}

QMutex::~QMutex()
{
    CloseHandle(d->event);
    delete d;
}

void QMutex::lock()
{
    const unsigned long self = GetCurrentThreadId();
    const unsigned long none = 0;

    ++d->waiters;
    while (!compare_and_set(&d->owner, none, self)) {
	if (d->recursive && d->owner == self) {
	    break;
	} else if (d->owner == self) {
	    qWarning("QMutex::lock(): Deadlock detected in Thread %d", d->owner);
	}

	WaitForSingleObject(d->event, INFINITE);
    }
    --d->waiters;
    ++d->count;
}

bool QMutex::tryLock()
{
    const unsigned long self = GetCurrentThreadId();
    const unsigned long none = 0;

    if (!compare_and_set(&d->owner, none, self)) {
	if (!d->recursive || d->owner != self)
	    return false;
    }
    ++d->count;
    return true;
}

void QMutex::unlock()
{
    const unsigned long self = GetCurrentThreadId();
    const unsigned long none = 0;

    Q_ASSERT(d->owner == self);

    if (!--d->count) {
	Q_ASSERT_X(compare_and_set(&d->owner, self, none),
		   "QMutex::unlock", "failed to reset owner");
	if (d->waiters != 0)
	    SetEvent(d->event);
    }
}

bool QMutex::isLocked()
{
    if (!tryLock())
	return false;
    unlock();
    return true;
}
