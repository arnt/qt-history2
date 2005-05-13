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

#include <windows.h>

#include "qmutex.h"
#include <qatomic.h>
#include "qmutex_p.h"

QMutexPrivate::QMutexPrivate(QMutex::RecursionMode mode)
    : lock(0), owner(0), count(0), recursive(mode == QMutex::Recursive),
      event(QT_WA_INLINE(CreateEventW(0, false, false, 0),
                         CreateEventA(0, false, false, 0)))
{
    if (!event)
        qWarning("QMutexPrivate::QMutexPrivate(): Creating event failed");
}

QMutexPrivate::~QMutexPrivate()
{ CloseHandle(event); }

ulong QMutexPrivate::self()
{ return GetCurrentThreadId(); }

void QMutexPrivate::wait()
{ 
    if (WaitForSingleObject(event, INFINITE) != WAIT_OBJECT_0)
        qWarning("QMutexPrivate::wait(): Waiting on event failed");
}

void QMutexPrivate::wakeUp()
{ SetEvent(event); }
