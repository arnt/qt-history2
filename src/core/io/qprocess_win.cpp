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

#include "qprocess.h"
#include "qprocess_p.h"

void QProcessPrivate::createPipe(int *pipe)
{
    Q_UNUSED(pipe);
    qWarning("QProcessPrivate::createPipe() unimplemented for win32 (use Q3Process instead)");
}

void QProcessPrivate::destroyPipe(int *pipe)
{
    Q_UNUSED(pipe);
    qWarning("QProcessPrivate::destroyPipe() unimplemented for win32 (use Q3Process instead)");
}

void QProcessPrivate::startProcess()
{
    qWarning("QProcessPrivate::startProcess() unimplemented for win32 (use Q3Process instead)");
}

void QProcessPrivate::execChild()
{
    qWarning("QProcessPrivate::execChild() unimplemented for win32 (use Q3Process instead)");
}

bool QProcessPrivate::processStarted()
{
    qWarning("QProcessPrivate::processStarted() unimplemented for win32 (use Q3Process instead)");
    return false;
}

Q_LONGLONG QProcessPrivate::bytesAvailableFromStdout() const
{
    qWarning("QProcessPrivate::bytesAvailableFromStdout() unimplemented for win32 (use Q3Process instead)");
    return -1;
}

Q_LONGLONG QProcessPrivate::bytesAvailableFromStderr() const
{
    qWarning("QProcessPrivate::bytesAvailableFromStderr() unimplemented for win32 (use Q3Process instead)");
    return -1;
}

Q_LONGLONG QProcessPrivate::readFromStdout(char *data, Q_LONGLONG maxlen)
{
    qWarning("QProcessPrivate::readFromStdout() unimplemented for win32 (use Q3Process instead)");
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return -1;
}

Q_LONGLONG QProcessPrivate::readFromStderr(char *data, Q_LONGLONG maxlen)
{
    qWarning("QProcessPrivate::readFromStderr() unimplemented for win32 (use Q3Process instead)");
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return -1;
}

int QProcessPrivate::waitForChild()
{
    qWarning("QProcessPrivate::waitForChild() unimplemented for win32 (use Q3Process instead)");
    return -1;
}

void QProcessPrivate::killProcess()
{
    qWarning("QProcessPrivate::killProcess() unimplemented for win32 (use Q3Process instead)");
}

bool QProcessPrivate::waitForStarted(int msecs)
{
    qWarning("QProcessPrivate::waitForStarted() unimplemented for win32 (use Q3Process instead)");
    Q_UNUSED(msecs);
    return false;
}

bool QProcessPrivate::waitForReadyRead(int msecs)
{
    qWarning("QProcessPrivate::waitForReadyRead() unimplemented for win32 (use Q3Process instead)");
    Q_UNUSED(msecs);
    return false;
}

bool QProcessPrivate::waitForFinished(int msecs)
{
    qWarning("QProcessPrivate::waitForFinished() unimplemented for win32 (use Q3Process instead)");
    Q_UNUSED(msecs);
    return false;
}
