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

#ifndef QMUTEX_H
#define QMUTEX_H

#include "QtCore/qglobal.h"

#ifndef QT_NO_THREAD

class QMutexPrivate;

class Q_CORE_EXPORT QMutex
{
    friend class QWaitCondition;
    friend class QWaitConditionPrivate;

public:
    QMutex(bool recursive = false);
    ~QMutex();

    void lock();
    bool tryLock();
    void unlock();

#if defined(QT_COMPAT)
    inline QT_COMPAT bool locked()
    {
        if (!tryLock())
            return true;
        unlock();
        return false;
    }
#endif

private:
    Q_DISABLE_COPY(QMutex)

    QMutexPrivate * d;
};

class Q_CORE_EXPORT QMutexLocker
{
public:
    inline QMutexLocker(QMutex *m)
        : mtx(m)
    { relock(); }
    inline ~QMutexLocker()
    { unlock(); }

    inline void unlock()
    { if (mtx) mtx->unlock(); }

    inline void relock()
    { if (mtx) mtx->lock(); }

    inline QMutex *mutex() const
    { return mtx; }

private:
    Q_DISABLE_COPY(QMutexLocker)

    QMutex *mtx;
};

#else // QT_NO_THREAD


class Q_CORE_EXPORT QMutex
{
public:
    inline QMutex(bool = false) {}
    inline ~QMutex() {}

    static inline void lock() {}
    static inline bool tryLock() { return true; }
    static void unlock() {}

#if defined(QT_COMPAT)
    static inline QT_COMPAT bool locked()
    { return false; }
#endif

private:
    Q_DISABLE_COPY(QMutex)
};

class Q_CORE_EXPORT QMutexLocker
{
public:
    inline QMutexLocker(QMutex *) {}
    inline ~QMutexLocker() {}

    static inline void unlock() {}
    static void relock() {}
    static inline QMutex *mutex() { return 0; }

private:
    Q_DISABLE_COPY(QMutexLocker)
};

#endif // QT_NO_THREAD

#endif // QMUTEX_H
