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

#include "qspinlock_p.h"

#include "qt_windows.h"

#ifndef QT_NO_THREAD
void QSpinLock::initialize()
{
    event = CreateEvent(0, false, false, 0);
}

void QSpinLock::cleanup()
{
    CloseHandle(event);
}

void QSpinLock::wait()
{
    WaitForSingleObject(event, INFINITE);
}

void QSpinLock::wake()
{
    SetEvent(event);
}
#endif // QT_NO_THREAD
