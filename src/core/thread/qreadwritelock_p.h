#ifndef QREADWRITELOCK_P_H
#define QREADWRITELOCK_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qreadwritelock_unix.cpp and qreadwritelock_win.cpp.  This header file may change
// from version to version without notice, or even be removed.
//
// We mean it.
//

#ifdef Q_OS_UNIX
struct QReadWriteLockPrivate
{
    volatile int accessCount;
    int maxReaders;
    QAtomic waitingWriters;
    QAtomic waitingReaders;
    pthread_mutex_t mutex;
    pthread_cond_t readerWait;
    pthread_cond_t writerWait;

};
#endif

#ifdef Q_OS_WIN32
struct QReadWriteLockPrivate
{
    volatile int accessCount;
    int maxReaders;
    QAtomic waitingWriters;
    HANDLE readerWait;
    HANDLE writerWait;
};
#endif

#endif
