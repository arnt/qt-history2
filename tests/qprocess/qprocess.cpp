#include <stdlib.h>

#include "qprocess.h"

/*!
  \class QProcess qprocess.h

  \brief The QProcess class provides means to start external programs and
  control their behaviour.

  \ingroup ?

  A QProcess allows you to start a external program, control its input and
  output, etc.
*/

/*!
  Constructs a QProcess.
*/
QProcess::QProcess()
{
}

/*!
  Constructs a QProcess with the given command (but does not start it).
*/
QProcess::QProcess( const QString& com )
{
}

/*!
  Constructs a QProcess with the given command and arguments (but does not
  start it).
*/
QProcess::QProcess( const QString& com, const QStringList& args )
{
}

/*!
*/
QProcess::~QProcess()
{
}

/*!
  Set the command that should be executed.
*/
void QProcess::setCommand( const QString& com )
{
}

/*!
  Set the arguments for the command. Previous set arguments will get deleted
  first.
*/
void QProcess::setArguments( const QStringList& args )
{
}

/*!
  Add a argument to the end of the existing list of arguments.
*/
void QProcess::addArgument( const QString& args )
{
}

/*!
  Set the path where the command is located.
*/
void QProcess::setPath( const QDir& dir )
{
}

/*!
  Set a working directory in which the command is executed.
*/
void QProcess::setWorkingDirectory( const QDir& dir )
{
}

/*!
  Start the program.

  Return TRUE on success, otherwise FALSE.
*/
bool QProcess::start()
{
    return TRUE;
}

/*!
  Ask the process to terminate.

  Return TRUE on success, otherwise FALSE.
*/
bool QProcess::hangUp()
{
    return TRUE;
}

/*!
  Terminate the process. This is not a safe way to end a process; you should
  try hangUp() first and use this function only if it failed.

  Return TRUE on success, otherwise FALSE.
*/
bool QProcess::kill()
{
    return TRUE;
}

/*!
  Return TRUE if the process is running, otherwise FALSE.
*/
bool QProcess::isRunning()
{
    return TRUE;
}

/*!
  Return TRUE if the process has exited normally, otherwise FALSE.
*/
bool QProcess::normalExit()
{
    return TRUE;
}

/*!
  Return the exit status of the process. This value is only valid if
  normalExit() is TRUE.
*/
int QProcess::exitStatus()
{
    return 0;
}

/*!
  \fn void QProcess::dataStdout( const QByteArray& buf )
  This signal is emitted if the process wrote data to stdout.
*/

/*!
  \fn void QProcess::dataStderr( const QByteArray& buf )
  This signal is emitted if the process wrote data to stderr.
*/

/*!
  \fn void QProcess::processExited()
  This signal is emitted if the process has exited.
*/

/*!
  \fn void QProcess::wroteStdin()
  This signal is emitted if the data send to stdin (via dataStdin()) was
  actually read by the process.
*/

/*!
  Write data to the stdin of the process. The process may or may not read this
  data. If the data gets read, the signal wroteStdin() is emitted.
*/
void QProcess::dataStdin( const QByteArray& buf )
{
}
