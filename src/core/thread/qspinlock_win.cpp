#include "qspinlock_p.h"

#ifndef QT_NO_THREAD
QSpinLock *QStaticSpinLock::static_lock = 0;

void QSpinLockPrivate::initialize()
{
    event = CreateEvent(0, FALSE, FALSE, 0);
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
