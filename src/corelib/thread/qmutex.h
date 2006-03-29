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

#include <QtCore/qglobal.h>
#include <new>

QT_BEGIN_HEADER

QT_MODULE(Core)

#ifndef QT_NO_THREAD

class QMutexPrivate;

class Q_CORE_EXPORT QMutex
{
    friend class QWaitCondition;
    friend class QWaitConditionPrivate;

public:
    enum RecursionMode { NonRecursive, Recursive };

    explicit QMutex(RecursionMode mode = NonRecursive);
    ~QMutex();

    void lock();
    bool tryLock();
    void unlock();

#if defined(QT3_SUPPORT)
    inline QT3_SUPPORT bool locked()
    {
        if (!tryLock())
            return true;
        unlock();
        return false;
    }
    inline QT3_SUPPORT_CONSTRUCTOR QMutex(bool recursive)
    {
        new (this) QMutex(recursive ? Recursive : NonRecursive);
    }
#endif

private:
    Q_DISABLE_COPY(QMutex)

    QMutexPrivate *d;
};

class Q_CORE_EXPORT QMutexLocker
{
public:
    inline explicit QMutexLocker(QMutex *m) { data.mtx = m; relock(); }
    inline ~QMutexLocker() { unlock(); }

    inline void unlock()
    {
        if (data.mtx) {
            static quintptr unsigned_1 = 1u;
            if ((data.val & unsigned_1) == unsigned_1) {
                data.val &= ~unsigned_1;
                data.mtx->unlock();
            }
        }
    }

    inline void relock()
    {
        if (data.mtx) {
            static quintptr unsigned_1 = 1u;
            static quintptr unsigned_0 = 0u;
            if ((data.val & unsigned_1) == unsigned_0) {
                data.mtx->lock();
                data.val |= unsigned_1;
            }
        }
    }

    inline QMutex *mutex() const
    {
        static quintptr unsigned_1 = 1u;
        return reinterpret_cast<QMutex *>(data.val & ~unsigned_1);
    }

private:
    Q_DISABLE_COPY(QMutexLocker)

    union {
        QMutex *mtx;
        quintptr val;
    } data;
};

#else // QT_NO_THREAD


class Q_CORE_EXPORT QMutex
{
public:
    enum RecursionMode { NonRecursive, Recursive };

    inline explicit QMutex(RecursionMode mode = NonRecursive) { Q_UNUSED(mode); }
    inline ~QMutex() {}

    static inline void lock() {}
    static inline bool tryLock() { return true; }
    static void unlock() {}

#if defined(QT3_SUPPORT)
    static inline QT3_SUPPORT bool locked() { return false; }
#endif

private:
    Q_DISABLE_COPY(QMutex)
};

class Q_CORE_EXPORT QMutexLocker
{
public:
    inline explicit QMutexLocker(QMutex *) {}
    inline ~QMutexLocker() {}

    static inline void unlock() {}
    static void relock() {}
    static inline QMutex *mutex() { return 0; }

private:
    Q_DISABLE_COPY(QMutexLocker)
};

#endif // QT_NO_THREAD

QT_END_HEADER

#endif // QMUTEX_H
