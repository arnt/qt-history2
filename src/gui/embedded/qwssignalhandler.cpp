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

#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <qdebug.h>

QWSSignalHandler::QWSSignalHandler()
{
    QList<int> signums;

    signums << SIGTERM << SIGINT << SIGQUIT << SIGHUP << SIGILL << SIGSEGV
            << SIGABRT;

    for (int i = 0; i < signums.size(); ++i)
        signal(signums.at(i), handleSignal);
}

void QWSSignalHandler::handleSignal(int signum)
{
    Q_UNUSED(signum);

    QWSSignalHandler *h = instance();
    for (int i = 0; i < h->semaphores.size(); ++i)
        semctl(h->semaphores.at(i), 0, IPC_RMID, 0);

    exit(-1);
}
