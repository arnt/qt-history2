#include "qplatformdefs.h"

#include <QtCore/qatomic.h>

static pthread_once_t genericWarning = PTHREAD_ONCE_INIT;
static void printGenericWarning()
{
    qWarning("Qt: WARNING! Using generic QAtomicInt and QAtomicPointer "
             "implementations, which use a single pthread_mutex_t to protect all "
             "atomic operations. This implementation is the slow (but safe) fallback "
             "implementation for architectures Qt does not yet support.");
}

static pthread_mutex_t qAtomicMutex = PTHREAD_MUTEX_INITIALIZER;

Q_CORE_EXPORT
bool QBasicAtomicInt_testAndSetOrdered(volatile int *_q_value, int expectedValue, int newValue)
{
    pthread_once(&genericWarning, printGenericWarning);

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
    pthread_once(&genericWarning, printGenericWarning);

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
    pthread_once(&genericWarning, printGenericWarning);

    int returnValue;
    pthread_mutex_lock(&qAtomicMutex);
    returnValue = *_q_value;
    *_q_value += valueToAdd;
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
bool QBasicAtomicPointer_testAndSetOrdered(volatile void **_q_value,
                                           void *expectedValue,
                                           void *newValue)
{
    pthread_once(&genericWarning, printGenericWarning);

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
void *QBasicAtomicPointer_fetchAndStoreOrdered(volatile void **_q_value, void *newValue)
{
    pthread_once(&genericWarning, printGenericWarning);

    void *returnValue;
    pthread_mutex_lock(&qAtomicMutex);
    returnValue = const_cast<void *>(*_q_value);
    *_q_value = newValue;
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
void *QBasicAtomicPointer_fetchAndAddOrdered(volatile void **_q_value, qptrdiff valueToAdd)
{
    pthread_once(&genericWarning, printGenericWarning);

    void *returnValue;
    pthread_mutex_lock(&qAtomicMutex);
    returnValue = const_cast<void *>(*_q_value);
    *_q_value = reinterpret_cast<char *>(returnValue) + valueToAdd;
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}
