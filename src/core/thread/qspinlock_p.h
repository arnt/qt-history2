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

#if defined(QT_THREAD_SUPPORT)

#include <arch/qatomic.h>

/*
  QSpinLock is similar to QMutex, except that it has *VERY* strict
  rules that must be followed.  If you do not know these rults, do not
  use QSpinLock.
*/
struct QSpinLock
{
    volatile int lock;

    inline void initialize()
    { lock = 0; }

    inline void acquire()
    { while (!q_atomic_test_and_set_int(&lock, 0, ~0)); }
    inline void release()
    { (void) q_atomic_test_and_set_int(&lock, ~0, 0); }
};

#define Q_SPINLOCK_INITIALIZER {0}

/*
  QSpinLockLocker is similar to QMutexLocker.
*/
class QSpinLockLocker
{
    QSpinLock *sx;

public:
    inline QSpinLockLocker(QSpinLock *s)
	: sx(s)
    { acquire(); }
    inline ~QSpinLockLocker()
    { release(); }

    inline void release()
    { if (sx) sx->release(); }
    inline void acquire()
    { if (sx) sx->acquire(); }

    inline QSpinLock *spinlock() const
    { return sx; }
};

#endif // QT_THREAD_SUPPORT

#endif // QSPINLOCK_P_H
