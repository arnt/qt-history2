/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"

#include <QtCore/qatomic.h>

static pthread_mutex_t qAtomicMutex = PTHREAD_MUTEX_INITIALIZER;

Q_CORE_EXPORT
bool QBasicAtomicInt_testAndSetOrdered(volatile int *_q_value, int expectedValue, int newValue)
{
    bool returnValue = false;
    pthread_mutex_lock(&qAtomicMutex);
    if (*_q_value == expectedValue) {
        *_q_value = newValue;
        returnValue = true;
    }
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
int QBasicAtomicInt_fetchAndStoreOrdered(volatile int *_q_value, int newValue)
{
    int returnValue;
    pthread_mutex_lock(&qAtomicMutex);
    returnValue = *_q_value;
    *_q_value = newValue;
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
int QBasicAtomicInt_fetchAndAddOrdered(volatile int *_q_value, int valueToAdd)
{
    int returnValue;
    pthread_mutex_lock(&qAtomicMutex);
    returnValue = *_q_value;
    *_q_value += valueToAdd;
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
bool QBasicAtomicPointer_testAndSetOrdered(void * volatile *_q_value,
                                           void *expectedValue,
                                           void *newValue)
{
    bool returnValue = false;
    pthread_mutex_lock(&qAtomicMutex);
    if (*_q_value == expectedValue) {
        *_q_value = newValue;
        returnValue = true;
    }
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
void *QBasicAtomicPointer_fetchAndStoreOrdered(void * volatile *_q_value, void *newValue)
{
    void *returnValue;
    pthread_mutex_lock(&qAtomicMutex);
    returnValue = *_q_value;
    *_q_value = newValue;
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
void *QBasicAtomicPointer_fetchAndAddOrdered(void * volatile *_q_value, qptrdiff valueToAdd)
{
    void *returnValue;
    pthread_mutex_lock(&qAtomicMutex);
    returnValue = *_q_value;
    *_q_value = reinterpret_cast<char *>(returnValue) + valueToAdd;
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}
