/****************************************************************************
**
** Definition of QMutex class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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

#ifdef QT_THREAD_SUPPORT

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

class QMutexPrivate;

class Q_CORE_EXPORT QMutex
{
    friend class QThread;
    friend class QWaitCondition;
    friend class QWaitConditionPrivate;

public:
    QMutex(bool recursive = false);
    ~QMutex();

    void lock();
    bool tryLock();
    void unlock();

    bool isLocked();

private:
    QMutexPrivate * d;

#if defined(Q_DISABLE_COPY)
    QMutex( const QMutex & );
    QMutex &operator=( const QMutex & );
#endif
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
    QMutex *mtx;

#if defined(Q_DISABLE_COPY)
    QMutexLocker( const QMutexLocker & );
    QMutexLocker &operator=( const QMutexLocker & );
#endif
};

#endif // QT_THREAD_SUPPORT

#endif // QMUTEX_H
