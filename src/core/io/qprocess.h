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

#ifndef QPROCESS_H
#define QPROCESS_H

#include <qiodevice.h>
#include <qstringlist.h>

class QProcessPrivate;

class Q_CORE_EXPORT QProcess : public QIODevice
{
    Q_OBJECT
public:
    enum ProcessError {
        FailedToStart, //### file not found
        Crashed,
        Timedout,
        ReadError,
        WriteError,
        UnknownError
    };
    enum ProcessState {
        NotRunning,
        Starting,
        Running,
        Finishing
    };
    enum ProcessChannel {
        StandardOutput,
        StandardError
    };

    QProcess(QObject *parent = 0);
    virtual ~QProcess();

    void start(const QString &program, const QStringList &arguments = QStringList());

    ProcessChannel inputChannel() const;
    void setInputChannel(ProcessChannel channel);

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &path);

    void setEnvironment(const QStringList &environment);
    QStringList environment() const;

    QProcess::ProcessError processError() const;
    QProcess::ProcessState processState() const;

    Q_PID pid() const;

    bool waitForStarted(int msecs = 30000);
    bool waitForReadyRead(int msecs = 30000);
    bool waitForBytesWritten(int msecs = 30000);
    bool waitForFinished(int msecs = 30000);

    void kill();

    QByteArray readAllStandardOutput();
    QByteArray readAllStandardError();

    // QIODevice
    Q_LONGLONG bytesAvailable() const;
    bool isSequential() const;
    bool canReadLine() const;
    void close();
    bool flush();

signals:
    void started();
    void finishing();
    void finished(int exitStatus);
    void error(QProcess::ProcessError error);
    void stateChanged(QProcess::ProcessState state);

    void readyReadStandardOutput();
    void readyReadStandardError();

protected:
    void setProcessState(QProcess::ProcessState state);

    // QIODevice
    Q_LONGLONG readData(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG writeData(const char *data, Q_LONGLONG len);

private:
    Q_DECLARE_PRIVATE(QProcess)
    Q_DISABLE_COPY(QProcess)

    Q_PRIVATE_SLOT(d, void canReadStandardOutput());
    Q_PRIVATE_SLOT(d, void canReadStandardError());
    Q_PRIVATE_SLOT(d, void canWrite());
    Q_PRIVATE_SLOT(d, void startupNotification());
    Q_PRIVATE_SLOT(d, void processDied());
    Q_PRIVATE_SLOT(d, void notified());
    friend class QProcessManager;
};

#endif
