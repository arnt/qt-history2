#include <stdio.h>
#include <stdlib.h>

#include "qapplication.h"
#include "qprocess.h"


/*!
  \class QProcess qprocess.h

  \brief The QProcess class provides means to start external programs and
  control their behaviour.

  \ingroup ?

  A QProcess allows you to start a external program, control its input and
  output, etc.
*/

#if 0
void QProcess::init()
{
    stdinBufRead = 0;

#if defined( _WS_WIN_ )
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
#endif
	notifierStdin = 0;
	notifierStdout = 0;
	notifierStderr = 0;

	socketStdin[0] = 0;
	socketStdin[1] = 0;
	socketStdout[0] = 0;
	socketStdout[1] = 0;
	socketStderr[0] = 0;
	socketStderr[1] = 0;
#if defined( _WS_WIN_ )
	WORD wVersionRequested;WSADATA wsaData;
	wVersionRequested = MAKEWORD( 2, 2 ); 
	WSAStartup( wVersionRequested, &wsaData );
    } else {
	pipeStdin[0] = 0;
	pipeStdin[1] = 0;
	pipeStdout[0] = 0;
	pipeStdout[1] = 0;
	pipeStderr[0] = 0;
	pipeStderr[1] = 0;
    }
#endif
}
#endif

/*!
  Constructs a QProcess.
*/
QProcess::QProcess()
{
    init();
}

/*!
  Constructs a QProcess with the given command (but does not start it).
*/
QProcess::QProcess( const QString& com )
{
    init();
    setCommand( com );
}

/*!
  Constructs a QProcess with the given command and arguments (but does not
  start it).
*/
QProcess::QProcess( const QString& com, const QStringList& args )
{
    init();
    setCommand( com );
    setArguments( args );
}

/*!
  Destructor; if the process is running it is NOT terminated!
*/
QProcess::~QProcess()
{
    while ( !stdinBuf.isEmpty() ) {
	delete stdinBuf.dequeue();
    }
}


/*!
  \fn void QProcess::setCommand( const QString& com )
  Set the command that should be executed.
*/
/*!
  \fn void QProcess::setArguments( const QStringList& args )
  Set the arguments for the command. Previous set arguments will get deleted
  first.
*/
/*!
  \fn void QProcess::addArgument( const QString& args )
  Add a argument to the end of the existing list of arguments.
*/
/*!
  \fn void QProcess::setWorkingDirectory( const QDir& dir )
  Set a working directory in which the command is executed.
*/


/*!
  \fn bool QProcess::start()
  Start the program.

  Return TRUE on success, otherwise FALSE.
*/


/*!
  Ask the process to terminate. If this does not work you can try kill()
  instead.

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
  \fn bool QProcess::isRunning()
  Return TRUE if the process is running, otherwise FALSE.
*/


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
  \fn void QProcess::dataStdin( const QByteArray& buf )
  Write data to the stdin of the process. The process may or may not read this
  data. If the data gets read, the signal wroteStdin() is emitted.
*/
/*!
  Write data to the stdin of the process. The string is handled as a text. So
  what is written to the stdin is the QString::latin1(). The process may or may
  not read this data. If the data gets read, the signal wroteStdin() is
  emitted.
*/
void QProcess::dataStdin( const QString& buf )
{
    QByteArray bbuf;
    bbuf.duplicate( buf.latin1(), buf.length() );
    dataStdin( bbuf );
}

/*!
  \fn void QProcess::closeStdin( )
  Close stdin.
*/


/*!
  \fn void QProcess::socketRead( int fd )
  The process has output data to either stdout or sderr.
*/

/*!
  \fn void QProcess::socketWrite( int fd )
  The process tries to read data from stdin.
*/
