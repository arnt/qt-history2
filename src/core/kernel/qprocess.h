//depot/qt/main/src/core/kernel/qprocess.h#9 - edit change 149107 (text)
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

#include "qobject.h"
#include "qstringlist.h"
#include "qdir.h"

#ifndef QT_NO_PROCESS

class QProcessPrivate;
class QMembuf;

#if defined (Q_OS_WIN32)
#include <qt_windows.h>
#endif // Q_OS_WIN32

class Q_CORE_EXPORT QProcess : public QObject
{
    Q_OBJECT
public:
    QProcess(QObject *parent = 0);
    QProcess(const QString& arg0, QObject *parent = 0);
    QProcess(const QStringList& args, QObject *parent = 0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QProcess(QObject *parent, const char *name);
    QT_COMPAT_CONSTRUCTOR QProcess(const QString& arg0, QObject *parent, const char *name);
    QT_COMPAT_CONSTRUCTOR QProcess(const QStringList& args, QObject *parent, const char *name);
#endif
    ~QProcess();

    // set and get the arguments and working directory
    QStringList arguments() const;
    void clearArguments();
    virtual void setArguments(const QStringList& args);
    virtual void addArgument(const QString& arg);
#ifndef QT_NO_DIR
    QDir workingDirectory() const;
    virtual void setWorkingDirectory(const QDir& dir);
#endif

    // set and get the comms wanted
    enum Communication { Stdin=0x01, Stdout=0x02, Stderr=0x04, DupStderr=0x08 };
    void setCommunication(int c);
    int communication() const;

    // start the execution
    // #### why are those virtual?
    virtual bool start(QStringList *env=0);
    virtual bool launch(const QByteArray& buf, QStringList *env=0 );
    virtual bool launch(const QString& buf, QStringList *env=0 );
    inline bool launch(const char * buf, QStringList *env=0 )
        { return launch(QByteArray(buf), env); }

    virtual void childStarting();

    // inquire the status
    bool isRunning() const;
    bool normalExit() const;
    int exitStatus() const;

    // reading
    virtual QByteArray readStdout();
    virtual QByteArray readStderr();
    bool canReadLineStdout() const;
    bool canReadLineStderr() const;
    virtual QString readLineStdout();
    virtual QString readLineStderr();

    // get platform dependent process information
#if defined(Q_OS_WIN32)
    typedef void* PID;
#else
    typedef Q_LONG PID;
#endif
    PID processIdentifier();

    void flushStdin();

signals:
    void readyReadStdout();
    void readyReadStderr();
    void processExited();
    void wroteToStdin();
    void launchFinished();

public slots:
    // end the execution
    void tryTerminate() const;
    void kill() const;

    // input
    virtual void writeToStdin(const QByteArray& buf);
    virtual void writeToStdin(const QString& buf);
    virtual void closeStdin();

protected: // ### or private?
    void connectNotify(const char * signal);
    void disconnectNotify(const char * signal);
private:
    void setIoRedirection(bool value);
    void setNotifyOnExit(bool value);
    void setWroteStdinConnected(bool value);

    void init();
    void reset();
#if defined(Q_OS_WIN32)
    uint readStddev(HANDLE dev, char *buf, uint bytes);
#endif
    QMembuf* membufStdout();
    QMembuf* membufStderr();

private slots:
    void socketRead(int fd);
    void socketWrite(int fd);
    void timeout();
    void closeStdinLaunch();

private:
    QProcessPrivate *d;
#ifndef QT_NO_DIR
    QDir        workingDir;
#endif
    QStringList _arguments;

    mutable int  exitStat; // exit status
    mutable bool exitNormal; // normal exit?
    bool ioRedirection; // automatically set be (dis)connectNotify
    bool notifyOnExit; // automatically set be (dis)connectNotify
    bool wroteToStdinConnected; // automatically set be (dis)connectNotify

    bool readStdoutCalled;
    bool readStderrCalled;
    int comms;

    friend class QProcessPrivate;
#if defined(Q_OS_UNIX)
    friend class QProcessManager;
    friend class QProc;
#endif

    Q_DISABLE_COPY(QProcess)
};

#endif // QT_NO_PROCESS

#endif // QPROCESS_H
