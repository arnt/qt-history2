#ifndef QPROCESS_H
#define QPROCESS_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qdir.h"
#include "qsocketnotifier.h"
#include "qqueue.h"
#endif // QT_H

#if defined(UNIX)
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

#if defined ( _WS_WIN_ )
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

#if defined( _WS_WIN_ )
    PROCESS_INFORMATION pid;
    uint stdinBufRead;
#else
    pid_t pid;
    ssize_t stdinBufRead;
#endif
    int exitStat;
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
    QByteArray readStdout( ulong bytes = 0 );
#endif

private slots:
    void socketRead( int fd );
    void socketWrite( int fd );
    void timeout();
};

#endif // QPROCESS_H
