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

#include <QtCore/qiodevice.h>
#include <QtCore/qstringlist.h>

#if defined(Q_OS_WIN32)
#include <QtCore/qt_windows.h>
typedef PROCESS_INFORMATION* Q_PID;
#else
typedef Q_LONG Q_PID;
#endif

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
        Running
    };
    enum ProcessChannel {
        StandardOutput,
        StandardError
    };
    enum ProcessChannelMode {
        SeparateChannels,
        MergedChannels,
        ForwardedChannels
    };

    explicit QProcess(QObject *parent = 0);
    virtual ~QProcess();

    void start(const QString &program, const QStringList &arguments = QStringList(), OpenMode mode = ReadWrite);

    ProcessChannelMode inputChannelMode() const;
    void setInputChannelMode(ProcessChannelMode mode);

    ProcessChannel inputChannel() const;
    void setInputChannel(ProcessChannel channel);

    void closeInputChannel(ProcessChannel channel);
    void closeOutputChannel();

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &dir);

    void setEnvironment(const QStringList &environment);
    QStringList environment() const;

    QProcess::ProcessError error() const;
    QProcess::ProcessState state() const;

    Q_PID pid() const;

    bool waitForStarted(int msecs = 30000);
    bool waitForReadyRead(int msecs = 30000);
    bool waitForBytesWritten(int msecs = 30000);
    bool waitForFinished(int msecs = 30000);

    void terminate();

    QByteArray readAllStandardOutput();
    QByteArray readAllStandardError();

    int exitCode() const;

    // QIODevice
    Q_LONGLONG bytesAvailable() const;
    Q_LONGLONG bytesToWrite() const;
    bool isSequential() const;
    bool canReadLine() const;
    void close();

signals:
    void started();
    void finished(int exitCode);
    void error(ProcessError error);
    void stateChanged(ProcessState state);

    void readyReadStandardOutput();
    void readyReadStandardError();

protected:
    void setProcessState(ProcessState state);

    // QIODevice
    Q_LONGLONG readData(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG writeData(const char *data, Q_LONGLONG len);

private:
    Q_DECLARE_PRIVATE(QProcess)
    Q_DISABLE_COPY(QProcess)

    Q_PRIVATE_SLOT(d, bool canReadStandardOutput())
    Q_PRIVATE_SLOT(d, bool canReadStandardError())
    Q_PRIVATE_SLOT(d, bool canWrite())
    Q_PRIVATE_SLOT(d, bool startupNotification())
    Q_PRIVATE_SLOT(d, void processDied())
    Q_PRIVATE_SLOT(d, void notified())
    friend class QProcessManager;
};

#endif // QPROCESS_H
