/****************************************************************************
** $Id$
**
** Implementation of QMutex class for Win32
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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
    const unsigned long self = GetCurrentThreadId();
    const unsigned long none = 0;

    ++d->waiters;
    while (! qAtomicCompareAndSetPtr(&d->owner, none, self)) {
	if (d->recursive && d->owner == self) {
	    break;
	}
#ifdef QT_CHECK_STATE
	else if (d->owner == self) {
	    qWarning("QMutex::lock(): Deadlock detected in Thread %d", d->owner);
	}
#endif

	WaitForSingleObject(d->event, INFINITE);
    }
    --d->waiters;
    ++d->count;
}

bool QMutex::tryLock()
{
    const unsigned long self = GetCurrentThreadId();
    const unsigned long none = 0;

    if (! qAtomicCompareAndSetPtr(&d->owner, none, self)) {
	if (! d->recursive || d->owner != self)
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

    if (! --d->count) {
	qAtomicCompareAndSetPtr(&d->owner, self, none);
	if (d->waiters != 0)
	    SetEvent(d->event);
    }
}

bool QMutex::isLocked()
{
    if (! tryLock())
	return false;
    unlock();
    return true;
}
