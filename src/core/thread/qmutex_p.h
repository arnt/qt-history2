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

#ifndef QMUTEX_P_H
#define QMUTEX_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qmutex_unix.cpp and qmutex_win.cpp.  This header file may change
// from version to version without notice, or even be removed.
//
// We mean it.
//

#ifdef Q_OS_UNIX
struct QMutexPrivate
{
    bool recursive;

    QAtomic waiters;
    QAtomicPointer<void> owner;
    unsigned int count;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
};
#endif

#ifdef Q_OS_WIN32
class QMutexPrivate {
public:
    bool recursive;

    int owner;
    unsigned int count;
    QAtomic waiters;

    HANDLE event;
};
#endif

#endif // QMUTEX_P_H
