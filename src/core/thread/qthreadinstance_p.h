/****************************************************************************
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

#ifndef QTHREAD_P_H
#define QTHREAD_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QThread and QThreadStorage. This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qmutex.h"
#endif // QT_H

#ifdef Q_OS_UNIX
#include <pthread.h>
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#endif

struct QThreadInstance {
public:
    static QThreadInstance *current();

    void init(unsigned int stackSize);
    void deinit();

    QMutex *mutex() const;
    void terminate();

    unsigned int stacksize;
    void *args[2];
    void **thread_storage;
    bool finished : 1;
    bool running  : 1;
    bool orphan   : 1;

#ifdef Q_OS_UNIX
    pthread_cond_t thread_done;
    pthread_t thread_id;

    static void *start( void * );
    static void finish( void * );
#endif // Q_OS_UNIX

#ifdef Q_OS_WIN32
    HANDLE handle;
    unsigned int id;
    int waiters;

    static unsigned int __stdcall start( void * );
    static void finish( QThreadInstance * );
#endif // Q_OS_WIN32
};

#endif // QTHREAD_P_H
