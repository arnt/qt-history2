/****************************************************************************
**
** Definition of QMutex class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMUTEX_H
#define QMUTEX_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

typedef void *QStaticMutex;

#ifndef QT_NO_THREAD

class QMutexPrivate;

class Q_CORE_EXPORT QMutex
{
    friend class QWaitCondition;
    friend class QWaitConditionPrivate;

public:
    QMutex(bool recursive = true);
    ~QMutex();

    void lock();
    bool tryLock();
    void unlock();

    inline bool isLocked()
    {
        if (!tryLock()) return true;
        unlock();
        return false;
    }

#if defined(QT_COMPAT)
    inline QT_COMPAT bool locked()
    { return isLocked(); }
#endif

private:
    QMutexPrivate * d;

#if defined(Q_DISABLE_COPY)
    QMutex(const QMutex &);
    QMutex &operator=(const QMutex &);
#endif
};

class Q_CORE_EXPORT QMutexLocker
{
public:
    inline QMutexLocker(QMutex *m)
        : mtx(m)
    { relock(); }
    QMutexLocker(QStaticMutex &m);
    inline ~QMutexLocker()
    { unlock(); }

    inline void unlock()
    { if (mtx) mtx->unlock(); }

    inline void relock()
    { if (mtx) mtx->lock(); }

    inline QMutex *mutex() const
    { return mtx; }

private:
    QMutex *mtx;

#if defined(Q_DISABLE_COPY)
    QMutexLocker(const QMutexLocker &);
    QMutexLocker &operator=(const QMutexLocker &);
#endif
};

#else // QT_NO_THREAD


class Q_CORE_EXPORT QMutex
{
public:
    inline QMutex(bool) {}
    inline ~QMutex() {}

    static inline void lock() {}
    static inline bool tryLock() { return true; }
    static void unlock() {}

    static bool isLocked() { return false; }

#if defined(QT_COMPAT)
    static inline QT_COMPAT bool locked()
    { return false; }
#endif

private:
#if defined(Q_DISABLE_COPY)
    QMutex(const QMutex &);
    QMutex &operator=(const QMutex &);
#endif
};

class Q_CORE_EXPORT QMutexLocker
{
public:
    inline QMutexLocker(QMutex *) {}
    inline QMutexLocker(QStaticMutex &) {}
    inline ~QMutexLocker() {}

    static inline void unlock() {}
    static void relock() {}
    static inline QMutex *mutex() { return 0; }

private:
#if defined(Q_DISABLE_COPY)
    QMutexLocker(const QMutexLocker &);
    QMutexLocker &operator=(const QMutexLocker &);
#endif
};

#endif // QT_NO_THREAD

class Q_CORE_EXPORT QStaticLocker : public QMutexLocker
{
    static QStaticMutex staticLocker;
public:
    inline QStaticLocker()
        :QMutexLocker(staticLocker){}
};


#endif // QMUTEX_H
