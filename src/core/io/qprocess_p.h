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

#ifndef QPROCESS_P_H
#define QPROCESS_P_H

#include "qprocess.h"

#include <private/qinternal_p.h>
#include <private/qiodevice_p.h>
#include <qstringlist.h>

class QSocketNotifier;

class QProcessPrivate : public QIODevicePrivate
{
public:
    Q_DECLARE_PUBLIC(QProcess)

    QProcessPrivate();
    virtual ~QProcessPrivate();

    // private slots
    void readyReadStandardOutput(int);
    void readyReadStandardError(int);
    void readyWrite(int);
    void startupNotification(int);
    void processDied();

    QProcess::ProcessChannel processChannel;
    QProcess::ProcessError processError;
    QProcess::ProcessState processState;
    QString workingDirectory;
    Q_PID pid;

    QByteArray program;
    QStringList arguments;
    QStringList environment;

    QRingBuffer outputReadBuffer;
    QRingBuffer errorReadBuffer;
    QRingBuffer writeBuffer;

    int standardReadPipe[2];
    int errorReadPipe[2];
    int writePipe[2];
    int childStartedPipe[2];
    void createPipe(int *pipe);
    void destroyPipe(int *pipe);

    QSocketNotifier *standardReadSocketNotifier;
    QSocketNotifier *errorReadSocketNotifier;
    QSocketNotifier *writeSocketNotifier;
    QSocketNotifier *startupSocketNotifier;

    void startProcess();
    void execChild();
    bool processStarted();
    void killProcess();

    int exitCode;
    bool crashed;

    bool waitForStarted(int msecs = 30000);
    bool waitForReadyRead(int msecs = 30000);
    bool waitForFinished(int msecs = 30000);

    Q_LONGLONG bytesAvailableFromStdout() const;
    Q_LONGLONG bytesAvailableFromStderr() const;
    Q_LONGLONG readFromStdout(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG readFromStderr(char *data, Q_LONGLONG maxlen);

    void cleanup();
};

#endif
