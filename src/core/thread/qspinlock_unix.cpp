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

#ifndef QT_NO_THREAD
void QSpinLock::initialize()
{
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&cond, 0);
}

void QSpinLock::cleanup()
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void QSpinLock::wait()
{
    pthread_mutex_lock(&mutex);
    while (!q_atomic_test_and_set_int(&lock, 0, ~0))
        pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
}

void QSpinLock::wake()
{
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}
#endif // QT_NO_THREAD
