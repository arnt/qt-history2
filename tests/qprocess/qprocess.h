#ifndef QPROCESS_H
#define QPROCESS_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qdir.h"
#endif // QT_H


class Q_EXPORT QProcess : public QObject
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
    void dataStdout( const QByteArray& buf );
    void dataStderr( const QByteArray& buf );

    // notification stuff
    void processExited();
    void wroteStdin();

public slots:
    // input
    void dataStdin( const QByteArray& buf );

private:
    QString     command;
    QDir        path;
    QDir        workingDir;
    QStringList arguments;
};


/*****************************************************************************
  QProcess inline functions
 *****************************************************************************/


#endif // QPROCESS_H
