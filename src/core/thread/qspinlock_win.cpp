#include "qspinlock_p.h"

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
