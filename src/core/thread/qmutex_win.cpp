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
    const int self = GetCurrentThreadId();
    const int none = 0;

    ++d->waiters;
    while (!q_atomic_test_and_set_int(&d->owner, none, self)) {
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
    const int self = GetCurrentThreadId();
    const int none = 0;

    if (!q_atomic_test_and_set_int(&d->owner, none, self)) {
	if (!d->recursive || d->owner != self)
	    return false;
    }
    ++d->count;
    return true;
}

void QMutex::unlock()
{
    const int none = 0;

    Q_ASSERT(d->owner == GetCurrentThreadId());

    if (!--d->count) {
	q_atomic_set_int(&d->owner, none);
	if (d->waiters != 0)
	    SetEvent(d->event);
    }
}
