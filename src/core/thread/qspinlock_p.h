/****************************************************************************
**
** Declaration/Implementation of QSpinLock class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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
#include <arch/qatomic.h>

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

class QSpinLockPrivate
{
public:
    HANDLE event;

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
class QSpinLock
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

#else // QT_NO_THREAD

class QSpinLock
{
public:
    inline QSpinLock() {}
    inline ~QSpinLock() {}
    static inline void acquire() {}
    static inline void release() {}
};

#endif //QT_NO_THREAD

#endif // QSPINLOCK_P_H
