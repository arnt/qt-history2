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
#include <sys/socket.h>
#else
#include <process.h>
#include <direct.h>
#endif

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
    void setPath( const QDir& dir );
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
    QDir        path;
    QDir        workingDir;
    QStringList arguments;
    QSocketNotifier *notifierStdin;
    QSocketNotifier *notifierStdout;
    QSocketNotifier *notifierStderr;
#if defined(UNIX)
    int socketStdin[2];
    int socketStdout[2];
    int socketStderr[2];
    pid_t pid;
    QQueue<QByteArray> stdinBuf;
    ssize_t stdinBufRead;
#else
    int pid;
#endif

    void init();

private slots:
    void socketRead( int fd );
    void socketWrite( int fd );
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

inline void QProcess::setPath( const QDir& dir )
{ path = dir; }

inline void QProcess::setWorkingDirectory( const QDir& dir )
{ workingDir = dir; }

#endif // QPROCESS_H
