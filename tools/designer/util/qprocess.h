/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

/*
 * ATTENTION: This class is not part of the Qt library yet. You should not use
 * it in your programs. This class will be included in a future version of Qt
 * with a changed API!
 */

#ifndef QPROCESS_H
#define QPROCESS_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qdir.h"
#include "qsocketnotifier.h"
#include "qqueue.h"
#if defined(_OS_UNIX_)
#include "qlist.h"
#endif
#endif // QT_H

#if defined(_OS_UNIX_)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#else
#include <Windows.h>
#endif

// this class is under development; so don't look at all the
// ugly defines (RMS_*); just for testing...
class QProcess;

class QProcessPrivate
{
private:
    QProcessPrivate( QProcess *proc );
    ~QProcessPrivate();

    QString     command;
    QDir        workingDir;
    QStringList arguments;
    QQueue<QByteArray> stdinBuf;

#if defined (_WS_WIN_)
    HANDLE pipeStdin[2];
    HANDLE pipeStdout[2];
    HANDLE pipeStderr[2];
    QTimer *lookup;
#if defined ( RMS_USE_SOCKETS )
    QSocketNotifier *notifierStdin;
    QSocketNotifier *notifierStdout;
    QSocketNotifier *notifierStderr;
    int socketStdin[2];
    int socketStdout[2];
    int socketStderr[2];
#endif
#else
    QSocketNotifier *notifierStdin;
    QSocketNotifier *notifierStdout;
    QSocketNotifier *notifierStderr;
    int socketStdin[2];
    int socketStdout[2];
    int socketStderr[2];
#endif

#if defined(_WS_WIN_)
    PROCESS_INFORMATION pid;
    uint stdinBufRead;
#else
    pid_t pid;
    ssize_t stdinBufRead;
#endif
#if defined(_OS_UNIX_)
    QProcess *d;
    static struct sigaction *oldact;
    static QList<QProcess> *proclist;
#if defined(_OS_OSF_)
    static void sigchldHnd();
#else
    static void sigchldHnd( int );
#endif
#endif
    bool exitValuesCalculated;
    int  exitStat;
    bool exitNormal;

    friend class QProcess;
};

//class Q_EXPORT QProcess : public QObject
class QProcess : public QObject
{
    Q_OBJECT
public:
    QProcess( QObject *parent=0, const char *name=0 );
    QProcess( const QString& com, QObject *parent=0, const char *name=0 );
    QProcess( const QString& com, const QStringList& args, QObject *parent=0, const char *name=0 );
    ~QProcess();

    // set the command, arguments, etc.
    void setCommand( const QString& com );
    void setArguments( const QStringList& args );
    void addArgument( const QString& arg );
    void setWorkingDirectory( const QDir& dir );

    // control the execution
    bool start();
    bool hangUp();
    bool kill();

    // inquire the status
    bool isRunning();
    bool normalExit();
    int exitStatus();

signals:
    // output
    void dataStdout( const QString& buf );
    void dataStdout( const QByteArray& buf );
    void dataStderr( const QString& buf );
    void dataStderr( const QByteArray& buf );

    // notification stuff
    void processExited();
    void wroteStdin();

public slots:
    // input
    void dataStdin( const QByteArray& buf );
    void dataStdin( const QString& buf );
    void closeStdin();

private:
    QProcessPrivate *d;

private:
#if defined( _WS_WIN_ )
    QByteArray readStddev( HANDLE dev, ulong bytes = 0 );
#endif

private slots:
    void socketRead( int fd );
    void socketWrite( int fd );
    void timeout();

private:
    friend class QProcessPrivate;
};

#endif // QPROCESS_H
