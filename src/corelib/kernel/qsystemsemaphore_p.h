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

#ifndef QSYSTEMSEMAPHORE_P_H
#define QSYSTEMSEMAPHORE_P_H

#ifndef QT_NO_SYSTEMSEMAPHORE

#include "qsystemsemaphore.h"
#include "qsharedmemory_p.h"

class QSystemSemaphorePrivate
{

public:
    QSystemSemaphorePrivate();

    QString makeKeyFileName()
    {
        return QSharedMemoryPrivate::makePlatformSafeKey(key, QLatin1String("qipc_systemsem_"));
    }

#ifdef Q_OS_WIN
    HANDLE handle(QSystemSemaphore::AccessMode mode = QSystemSemaphore::Open);
#else
    key_t handle(QSystemSemaphore::AccessMode mode = QSystemSemaphore::Open);
#endif
    void cleanHandle();
    bool modifySemaphore(int count);

    QString key;
    QString fileName;
    int initialValue;
#ifdef Q_OS_WIN
    HANDLE semaphore;
    HANDLE semaphoreLock;
#else
    int semaphore;
    bool createdFile;
    bool createdSemaphore;
    key_t unix_key;
#endif

};

#endif // QT_NO_SYSTEMSEMAPHORE

#endif // QSYSTEMSEMAPHORE_P_H

