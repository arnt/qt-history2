#include "qspinlock_p.h"

#include "qt_windows.h"

#ifndef QT_NO_THREAD
void QSpinLockPrivate::initialize()
{
    event = CreateEvent(0, false, false, 0);
}

void QSpinLockPrivate::cleanup()
{
    CloseHandle(event);
}

void QSpinLockPrivate::wait()
{
    WaitForSingleObject(event, INFINITE);
}

void QSpinLockPrivate::wake()
{
    SetEvent(event);
}
#endif // QT_NO_THREAD
