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

#include "blockingprocess.h"

#include <qeventloop.h>

BlockingProcess::BlockingProcess()
{
    connect(this, SIGNAL(readyReadStdout()), this, SLOT(readOut()));
    connect(this, SIGNAL(readyReadStderr()), this, SLOT(readErr()));
    connect(this, SIGNAL(processExited()), this, SLOT(exited()));
    outUsed = errUsed = 0;
    eventLoop = new QEventLoop(this);
}

void BlockingProcess::readOut()
{
    QByteArray rout = readStdout();
    if (outUsed + rout.size() > out.size())
        out.resize(outUsed + rout.size());
    memcpy(out.data() + outUsed, rout, rout.size());
    outUsed += rout.size();
}

void BlockingProcess::readErr()
{
    QByteArray rerr = readStderr();
    if (errUsed + rerr.size() > err.size())
        err.resize(errUsed + rerr.size());
    memcpy(err.data() + errUsed, rerr, rerr.size());
    errUsed += rerr.size();
}

void BlockingProcess::exited()
{
    eventLoop->exit();
}

bool BlockingProcess::start(QStringList *env)
{
    bool returnValue = QProcess::start(env);
    if (returnValue)
        eventLoop->exec();
    return returnValue;
}
