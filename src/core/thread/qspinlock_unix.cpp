#include "qspinlock_p.h"

#ifndef QT_NO_THREAD
QSpinLock *QStaticSpinLock::static_lock = 0;

void QSpinLockPrivate::initialize()
{
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&cond, 0);
}

void QSpinLockPrivate::cleanup()
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void QSpinLockPrivate::wait()
{
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
}

void QSpinLockPrivate::wake()
{
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}
#endif // QT_NO_THREAD
