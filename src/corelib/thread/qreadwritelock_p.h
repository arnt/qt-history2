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

#ifndef QREADWRITELOCK_P_H
#define QREADWRITELOCK_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

struct QReadWriteLockPrivate
{
    QReadWriteLockPrivate();
    ~QReadWriteLockPrivate();

    void wait(bool reader);
    void wakeUp();

    QAtomic lock;
    int accessCount;
    int waitingReaders;
    int waitingWriters;

#ifdef Q_OS_UNIX
    int wakeup;
    pthread_mutex_t mutex;
    pthread_cond_t readerWait;
    pthread_cond_t writerWait;
#endif

#ifdef Q_OS_WIN32
    CRITICAL_SECTION cs;
    HANDLE readerWait;
    HANDLE writerWait;
#endif
};

#endif // QREADWRITELOCK_P_H
