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
