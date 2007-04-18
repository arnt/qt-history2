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

#include "qwssignalhandler_p.h"

#ifndef QT_NO_QWS_SIGNALHANDLER

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#ifndef Q_OS_BSD4
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo  *__buf;
};
#endif


class QWSSignalHandlerPrivate : public QWSSignalHandler
{
public:
    QWSSignalHandlerPrivate() : QWSSignalHandler() {}
};


Q_GLOBAL_STATIC(QWSSignalHandlerPrivate, signalHandlerInstance);


QWSSignalHandler* QWSSignalHandler::instance()
{
    return signalHandlerInstance();
}

QWSSignalHandler::QWSSignalHandler()
{
    const int signums[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE,
                            SIGSEGV, SIGTERM, SIGBUS };
    const int n = sizeof(signums)/sizeof(int);

    for (int i = 0; i < n; ++i) {
        const int signum = signums[i];
        sighandler_t old = signal(signum, handleSignal);
        oldHandlers[signum] = (old == SIG_ERR ? SIG_DFL : old);
    }
}

QWSSignalHandler::~QWSSignalHandler()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    while (!semaphores.isEmpty())
        removeSemaphore(semaphores.last());
#endif
}

#ifndef QT_NO_QWS_MULTIPROCESS
void QWSSignalHandler::removeSemaphore(int semno)
{
    const int index = semaphores.lastIndexOf(semno);
    if (index != -1) {
        semun semval;
        semval.val = 0;
        semctl(semaphores.at(index), 0, IPC_RMID, semval);
        semaphores.remove(index);
    }
}
#endif // QT_NO_QWS_MULTIPROCESS

void QWSSignalHandler::handleSignal(int signum)
{
    QWSSignalHandler *h = instance();

    signal(signum, h->oldHandlers[signum]);

#ifndef QT_NO_QWS_MULTIPROCESS
    semun semval;
    semval.val = 0;
    for (int i = 0; i < h->semaphores.size(); ++i)
        semctl(h->semaphores.at(i), 0, IPC_RMID, semval);
#endif

    h->objects.clear();
    raise(signum);
}

#endif // QT_QWS_NO_SIGNALHANDLER
