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

#include "qsharedmemory.h"
#include "qsharedmemory_p.h"
#include "qsystemsemaphore.h"
#include <qdebug.h>

QSharedMemoryPrivate::QSharedMemoryPrivate() : QObjectPrivate(),
        memory(0), size(0), error(QSharedMemory::NoError),
           systemSemaphore(QString()), lockedByMe(false), hand(0)
{
}

void QSharedMemoryPrivate::setErrorString(const QString &function)
{
    Q_Q(QSharedMemory);
    BOOL windowsError = GetLastError();
    if (windowsError == 0)
        return;
    switch (windowsError) {
    case ERROR_ALREADY_EXISTS:
        error = QSharedMemory::AlreadyExists;
        errorString = function + QLatin1String(": ") + q->tr("already exists");
    break;
    case ERROR_FILE_NOT_FOUND:
        error = QSharedMemory::NotFound;
        errorString = function + QLatin1String(": ") + q->tr("doesn't exists");
        break;
    case ERROR_COMMITMENT_LIMIT:
        error = QSharedMemory::InvalidSize;
        errorString = function + QLatin1String(": ") + q->tr("invalid size");
        break;
    case ERROR_NOT_ENOUGH_MEMORY:
        errorString = function + QLatin1String(": ") + q->tr("out of resources");
        error = QSharedMemory::OutOfResources;
        break;
    default:
        errorString = function + QLatin1String(": ") + q->tr("unknown error")
            + QLatin1Char(' ') + windowsError;
        error = QSharedMemory::UnknownError;
#if defined QSHAREDMEMORY_DEBUG
        qDebug() << errorString << "key" << key;
#endif
    }
}

/*!
    \internal

    Return the handle used for accessing the shared memory.
*/
HANDLE QSharedMemoryPrivate::handle()
{
    Q_Q(QSharedMemory);
    if (!hand) {
        QString safeKey = makePlatformSafeKey(key);
        if (safeKey.isEmpty()) {
            error = QSharedMemory::KeyError;
            errorString = QLatin1String("QSharedMemory::handle: ") + q->tr("unable to make key");
            return false;
        }
    QT_WA({
            hand = OpenFileMappingW(FILE_MAP_ALL_ACCESS, false, (TCHAR*)safeKey.utf16());
        }, {
            hand = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, safeKey.toLocal8Bit().constData());
        });
        if (!hand) {
            setErrorString(QLatin1String("QSharedMemory::handle"));
            return false;
        }
    }
    return hand;
}

/*!
    \internal

    Reset the handle to the initial invalid state.
*/
bool QSharedMemoryPrivate::cleanHandle()
{
    if (hand != 0 && !CloseHandle(hand)) {
        hand = 0;
        return false;
        setErrorString(QLatin1String("QSharedMemory::cleanHandle"));
    }
    hand = 0;
    return true;
}

bool QSharedMemoryPrivate::create(int size)
{
    Q_Q(QSharedMemory);
    // Get a windows acceptable key
    QString safeKey = makePlatformSafeKey(key);
    if (safeKey.isEmpty()) {
        error = QSharedMemory::KeyError;
        errorString = QLatin1String("QSharedMemory::create: ") + q->tr("key error");
        return false;
    }

    // Create the file mapping.
    QT_WA( {
        hand = CreateFileMappingW(INVALID_HANDLE_VALUE,
               0, PAGE_READWRITE, 0, size, (TCHAR*)safeKey.utf16());
    }, {
        hand = CreateFileMappingA(INVALID_HANDLE_VALUE,
               0, PAGE_READWRITE, 0, size, safeKey.toLocal8Bit().constData());
    } );
    setErrorString(QLatin1String("QSharedMemory::create"));

    // hand is valid when it already exists unlike unix so explicitly check
    if (error == QSharedMemory::AlreadyExists || !hand)
        return false;

    return true;
}

bool QSharedMemoryPrivate::attach(QSharedMemory::OpenMode mode)
{
    Q_Q(QSharedMemory);
    // Grab a pointer to the memory block
    int permisions = (mode == QSharedMemory::ReadOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS);
    memory = (void *)MapViewOfFile(handle(), permisions, 0, 0, 0);
    if (0 == memory) {
        setErrorString(QLatin1String("QSharedMemory::attach"));
        cleanHandle();
        return false;
    }

    // Grab the size of the memory we have been given (a multiple of 4K on windows)
    MEMORY_BASIC_INFORMATION info;
    if (!VirtualQuery(memory, &info, sizeof(info))) {
        // Windows doesn't set an error code on this one,
        // it should only be a kernel memory error.
        error = QSharedMemory::UnknownError;
        errorString = QLatin1String("QSharedMemory::attach: ") + q->tr("size query failed");
        return false;
    }
    size = info.RegionSize;

    return true;
}

bool QSharedMemoryPrivate::detach()
{
    // umap memory
    if (!UnmapViewOfFile(memory)) {
        setErrorString(QLatin1String("QSharedMemory::detach"));
        return false;
    }
    memory = 0;

    // close handle
    return cleanHandle();
}

