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

#include <QtCore/qmutex.h>
#include <QtCore/qwaitcondition.h>

#include <stdlib.h>
#include <dbus/dbus.h>

struct DBusMutex: public QMutex
{
    inline DBusMutex(QMutex::RecursionMode mode = QMutex::NonRecursive)
        : QMutex(mode)
    { }

    static DBusMutex* mutex_new()
    {
        return new DBusMutex;
    }

    static DBusMutex* recursive_mutex_new()
    {
        return new DBusMutex(QMutex::Recursive);
    }
     
    static void mutex_free(DBusMutex *mutex)
    {
        delete mutex;
    }

    static dbus_bool_t mutex_lock(DBusMutex *mutex)
    {
        mutex->lock();
        return true;
    }

    static dbus_bool_t mutex_unlock(DBusMutex *mutex)
    {
        mutex->unlock();
        return true;
    }

    static void recursive_mutex_lock(DBusMutex *mutex)
    {
        mutex_lock(mutex);
    }

    static void recursive_mutex_unlock(DBusMutex *mutex)
    {
        mutex_unlock(mutex);
    }

};

struct DBusCondVar: public QWaitCondition
{
    inline DBusCondVar()
    { }

    static DBusCondVar* condvar_new()
    {
        return new DBusCondVar;
    }

    static void condvar_free(DBusCondVar *cond)
    {
        delete cond;
    }

    static void condvar_wait(DBusCondVar *cond, DBusMutex *mutex)
    {
        cond->wait(mutex);
    }

    static dbus_bool_t condvar_wait_timeout(DBusCondVar *cond, DBusMutex *mutex, int msec)
    {
        return cond->wait(mutex, msec);
    }

    static void condvar_wake_one(DBusCondVar *cond)
    {
        cond->wakeOne();
    }

    static void condvar_wake_all(DBusCondVar *cond)
    {
        cond->wakeAll();
    }
};

bool qDBusInitThreads()
{

#ifdef dbus_threads_init_default

    return dbus_threads_init_default();

#else
    // ### Disable the recursive mutex functions.
    static DBusThreadFunctions fcn = {
        DBUS_THREAD_FUNCTIONS_MUTEX_NEW_MASK |
        DBUS_THREAD_FUNCTIONS_MUTEX_FREE_MASK |
        DBUS_THREAD_FUNCTIONS_MUTEX_LOCK_MASK |
        DBUS_THREAD_FUNCTIONS_MUTEX_UNLOCK_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_NEW_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_FREE_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAIT_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAIT_TIMEOUT_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAKE_ONE_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAKE_ALL_MASK,
#if 0
        DBUS_THREAD_FUNCTIONS_RECURSIVE_MUTEX_NEW_MASK |
        DBUS_THREAD_FUNCTIONS_RECURSIVE_MUTEX_FREE_MASK |
        DBUS_THREAD_FUNCTIONS_RECURSIVE_MUTEX_LOCK_MASK |
        DBUS_THREAD_FUNCTIONS_RECURSIVE_MUTEX_UNLOCK_MASK,
#endif
        DBusMutex::mutex_new,
        DBusMutex::mutex_free,
        DBusMutex::mutex_lock,
        DBusMutex::mutex_unlock,
        DBusCondVar::condvar_new,
        DBusCondVar::condvar_free,
        DBusCondVar::condvar_wait,
        DBusCondVar::condvar_wait_timeout,
        DBusCondVar::condvar_wake_one,
        DBusCondVar::condvar_wake_all,
#if 0
        DBusMutex::recursive_mutex_new,
        DBusMutex::mutex_free,
        DBusMutex::recursive_mutex_lock,
        DBusMutex::recursive_mutex_unlock,
#else
        0, 0, 0, 0,
#endif
        0, 0, 0, 0
    };

    dbus_threads_init(&fcn);

    return true;
#endif
}
