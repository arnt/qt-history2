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

//class Q_EXPORT QProcess : public QObject
class QProcess : public QObject
{
    Q_OBJECT
public:
    QProcess();
    QProcess( const QString& com );
    QProcess( const QString& com, const QStringList& args );
    ~QProcess();

    // set the command, arguments, etc.
    void setCommand( const QString& com );
    void setArguments( const QStringList& args );
    void addArgument( const QString& args );
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

private:
    void init();
#if defined( _WS_WIN_ )
    QByteArray readStdout( ulong bytes = 0 );
#endif

private slots:
    void socketRead( int fd );
    void socketWrite( int fd );
#if defined( _WS_WIN_ )
    void timeout();
#endif
};


/*****************************************************************************
  QProcess inline functions
 *****************************************************************************/

inline void QProcess::setCommand( const QString& com )
{ command = com; }

inline void QProcess::setArguments( const QStringList& args )
{ arguments = args; }

inline void QProcess::addArgument( const QString& args )
{ arguments.append( args ); }

inline void QProcess::setWorkingDirectory( const QDir& dir )
{ workingDir = dir; }

#endif // QPROCESS_H
