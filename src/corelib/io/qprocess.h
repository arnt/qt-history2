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
typedef qint64 Q_PID;
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

    void start(const QString &program, const QStringList &arguments, OpenMode mode = ReadWrite);
    void start(const QString &program, OpenMode mode = ReadWrite);

    ProcessChannelMode readChannelMode() const;
    void setReadChannelMode(ProcessChannelMode mode);

    ProcessChannel readChannel() const;
    void setReadChannel(ProcessChannel channel);

    void closeReadChannel(ProcessChannel channel);
    void closeWriteChannel();

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
    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;
    bool isSequential() const;
    bool canReadLine() const;
    void close();
    bool atEnd() const;

    static int execute(const QString &program, const QStringList &arguments);
    static int execute(const QString &program);

    static bool startDetached(const QString &program, const QStringList &arguments);
    static bool startDetached(const QString &program);

signals:
    void started();
    void finished(int exitCode);
    void error(ProcessError error);
    void stateChanged(ProcessState state);

    void readyReadStandardOutput();
    void readyReadStandardError();

protected:
    void setProcessState(ProcessState state);

    virtual void setupChildProcess();

    // QIODevice
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    Q_DECLARE_PRIVATE(QProcess)
    Q_DISABLE_COPY(QProcess)

    Q_PRIVATE_SLOT(d_func(), bool canReadStandardOutput())
    Q_PRIVATE_SLOT(d_func(), bool canReadStandardError())
    Q_PRIVATE_SLOT(d_func(), bool canWrite())
    Q_PRIVATE_SLOT(d_func(), bool startupNotification())
    Q_PRIVATE_SLOT(d_func(), void processDied())
    Q_PRIVATE_SLOT(d_func(), void notified())
    friend class QProcessManager;
};

#endif // QPROCESS_H
