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

#ifndef QSYSTEMLOCK_H
#define QSYSTEMLOCK_H

#include <QtCore/qstring.h>

QT_BEGIN_HEADER

#ifndef QT_NO_SYSTEMLOCK

class QSystemLockPrivate;

class QSystemLock
{

public:
    enum SystemLockError
    {
        NoError,
        UnknownError
    };

    QSystemLock(const QString &key);
    ~QSystemLock();

    void setKey(const QString &key);
    QString key() const;

    enum LockMode
    {
        ReadOnly,
        ReadWrite
    };

    bool lock(LockMode mode = ReadWrite);
    bool unlock();

    SystemLockError error() const;
    QString errorString() const;

private:
    Q_DISABLE_COPY(QSystemLock)

    QSystemLockPrivate *d;
};

class QSystemLocker
{

public:
    inline QSystemLocker(QSystemLock *systemLock,
                         QSystemLock::LockMode mode = QSystemLock::ReadWrite) : q_lock(systemLock)
    {
        autoUnLocked = relock(mode);
    }

    inline ~QSystemLocker()
    {
        if (autoUnLocked)
            unlock();
    }

    inline QSystemLock *systemLock() const
    {
        return q_lock;
    }

    inline bool relock(QSystemLock::LockMode mode = QSystemLock::ReadWrite)
    {
        return (q_lock && q_lock->lock(mode));
    }

    inline bool unlock()
    {
        if (q_lock && q_lock->unlock()) {
            autoUnLocked = false;
            return true;
        }
        return false;
    }

private:
    Q_DISABLE_COPY(QSystemLocker)

    bool autoUnLocked;
    QSystemLock *q_lock;
};

#endif // QT_NO_SYSTEMLOCK

QT_END_HEADER

#endif // QSYSTEMLOCK_H

