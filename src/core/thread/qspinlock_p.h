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

#ifndef QSPINLOCK_P_H
#define QSPINLOCK_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of internal files.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_NO_THREAD

#include <qglobal.h>
#include <qatomic.h>
#include <qnamespace.h>

#ifdef Q_OS_UNIX
#  include <pthread.h>

class QSpinLockPrivate
{
public:
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    void initialize();
    void cleanup();
    void wait();
    void wake();
};

#endif // Q_OS_UNIX

#ifdef Q_OS_WIN

class Q_CORE_EXPORT QSpinLockPrivate
{
public:
    Qt::HANDLE event;

    void initialize();
    void cleanup();
    void wait();
    void wake();
};

#endif // Q_OS_WIN

/*
  QSpinLock is similar to QMutex, except that it has *VERY* strict
  rules that must be followed.  If you do not know these rules, do not
  use QSpinLock.
*/
class Q_CORE_EXPORT QSpinLock
{
public:
    inline QSpinLock()
    { lock = waiters = 0; d.initialize(); }
    inline ~QSpinLock()
    { d.cleanup(); }

    inline void acquire()
    {
        q_atomic_increment(&waiters);
        while (!q_atomic_test_and_set_int(&lock, 0, ~0))
            d.wait();
        q_atomic_decrement(&waiters);
    }
    inline void release()
    {
        (void) q_atomic_set_int(&lock, 0);
        if (waiters != 0) d.wake();
    }

private:
    volatile int lock;
    volatile int waiters;
    QSpinLockPrivate d;
};

// QStaticSpinLock allows you to have static-global spinlocks
typedef void *QStaticSpinLock;

// similar to QMutexLocker, but for spinlocks
class Q_CORE_EXPORT QSpinLockLocker
{
public:
    inline QSpinLockLocker(QStaticSpinLock &s)
    {
        if (!s) { // spinlock not yet initialized... do it now
            QSpinLock *x = new QSpinLock;
            if (!q_atomic_test_and_set_ptr(&s, 0, x))
                delete x; // someone beat us to it
        }
        sx = reinterpret_cast<QSpinLock *>(s);
        acquire();
    }
    inline QSpinLockLocker(QSpinLock *s)
        : sx(s)
    { acquire(); }
    inline ~QSpinLockLocker()
    { release(); }

    inline void acquire()
    { if (sx) sx->acquire(); }
    inline void release()
    { if (sx) sx->release(); }

    inline QSpinLock *spinLock() const
    { return sx; }

private:
    QSpinLock *sx;
};

#else // QT_NO_THREAD

class Q_CORE_EXPORT QSpinLock
{
public:
    inline QSpinLock() { }
    inline ~QSpinLock() { }
    static inline void acquire() { }
    static inline void release() { }
};

typedef void *QStaticSpinLock;

class Q_CORE_EXPORT QSpinLockLocker
{
public:
    inline QSpinLockLocker(QStaticSpinLock &) { }
    inline QSpinLockLocker(QSpinLock *) { }
    inline ~QSpinLockLocker() { }

    static inline void release() { }
    static inline void acquire() { }
    static inline QSpinLock *spinLock()
    { return 0; }
};

#endif //QT_NO_THREAD

#endif // QSPINLOCK_P_H
