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


void QProcess::init()
{
    notifierStdin = 0;
    notifierStdout = 0;
    notifierStderr = 0;

#if defined(UNIX)
    socketStdin[0] = 0;
    socketStdin[1] = 0;
    socketStdout[0] = 0;
    socketStdout[1] = 0;
    socketStderr[0] = 0;
    socketStderr[1] = 0;

    stdinBufRead = 0;
#endif
}

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
#if defined(UNIX)
    while ( !stdinBuf.isEmpty() ) {
	delete stdinBuf.dequeue();
    }
#endif
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
  \fn void QProcess::setPath( const QDir& dir )
  Set the path where the command is located.
*/
/*!
  \fn void QProcess::setWorkingDirectory( const QDir& dir )
  Set a working directory in which the command is executed.
*/


/*!
  Start the program.

  Return TRUE on success, otherwise FALSE.
*/
bool QProcess::start()
{
#if defined(UNIX)
    // cleanup the notifiers
    if ( notifierStdin ) {
	notifierStdin->setEnabled( FALSE );
	delete notifierStdin;
	notifierStdin = 0;
    }
    if ( notifierStdout ) {
	notifierStdout->setEnabled( FALSE );
	delete notifierStdout;
	notifierStdout = 0;
    }
    if ( notifierStderr ) {
	notifierStderr->setEnabled( FALSE );
	delete notifierStderr;
	notifierStderr = 0;
    }
    
    // open sockets for piping
    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, socketStdin ) ) {
	return FALSE;
    }
    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, socketStdout ) ) {
	return FALSE;
    }
    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, socketStderr ) ) {
	return FALSE;
    }

    // construct the arguments for exec
    const char** arglist = new const char*[ arguments.count() + 2 ];
    arglist[0] = command.latin1();
    int i = 1;
    for ( QStringList::Iterator it = arguments.begin(); it != arguments.end(); ++it ) {
	arglist[ i++ ] = (*it).latin1();
    }
    arglist[i] = 0;

    // fork and exec
    QApplication::flushX();
    pid = fork();
    if ( pid == 0 ) {
	// child
	close( socketStdin[1] );
	close( socketStdout[0] );
	close( socketStderr[0] );
	dup2( socketStdin[0], STDIN_FILENO );
	dup2( socketStdout[1], STDOUT_FILENO );
	dup2( socketStderr[1], STDERR_FILENO );
	chdir( workingDir.absPath().latin1() );
	execv( path.absFilePath(command).latin1(), (char*const*)arglist ); // ### a hack
	exit( -1 );
    } else if ( pid == -1 ) {
	// error forking
	close( socketStdin[1] );
	close( socketStdout[0] );
	close( socketStderr[0] );
	close( socketStdin[0] );
	close( socketStdout[1] );
	close( socketStderr[1] );
	delete[] arglist;
	return FALSE;
    }
    close( socketStdin[0] );
    close( socketStdout[1] );
    close( socketStderr[1] );

    // setup notifiers for the sockets
    notifierStdin = new QSocketNotifier( socketStdin[1],
	    QSocketNotifier::Write, this );
    notifierStdout = new QSocketNotifier( socketStdout[0],
	    QSocketNotifier::Read, this );
    notifierStderr = new QSocketNotifier( socketStderr[0],
	    QSocketNotifier::Read, this );
    connect( notifierStdin, SIGNAL(activated(int)),
	    this, SLOT(socketWrite(int)) );
    connect( notifierStdout, SIGNAL(activated(int)),
	    this, SLOT(socketRead(int)) );
    connect( notifierStderr, SIGNAL(activated(int)),
	    this, SLOT(socketRead(int)) );
    notifierStdin->setEnabled( TRUE );
    notifierStdout->setEnabled( TRUE );
    notifierStderr->setEnabled( TRUE );

    // cleanup and return
    delete[] arglist;
    return TRUE;
#else
    // construct the arguments for spawn
    const char** arglist = new const char*[ arguments.count() + 2 ];
    arglist[0] = command.latin1();
    int i = 1;
    for ( QStringList::Iterator it = arguments.begin(); it != arguments.end(); ++it ) {
	arglist[ i++ ] = (*it).latin1();
    }
    arglist[i] = 0;

    // spawn
    QApplication::flushX();
    char buffer[_MAX_PATH];
    if ( _getcwd( buffer, _MAX_PATH ) == NULL )
	return FALSE;
    _chdir( workingDir.absPath().latin1() );
    pid = _spawnvp( P_NOWAIT, path.absFilePath(command).latin1(), arglist );
    _chdir( buffer );

    return TRUE;
#endif
}

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
#if defined(UNIX)
    stdinBuf.enqueue( new QByteArray(buf) );
    notifierStdin->setEnabled( TRUE );
    socketWrite( socketStdin[1] );
#endif
}

/*!
  Write data to the stdin of the process. The string is handled as a text. So
  what is written to the stdin is the QString::latin1(). The process may or may
  not read this data. If the data gets read, the signal wroteStdin() is
  emitted.
*/
void QProcess::dataStdin( const QString& buf )
{
#if defined(UNIX)
    QByteArray bbuf;
    bbuf.duplicate( buf.latin1(), buf.length() );
    dataStdin( bbuf );
#endif
}

/*!
  Close stdin.
*/
void QProcess::closeStdin( )
{
#if defined(UNIX)
    if ( socketStdin[1] !=0 ) {
	close( socketStdin[1] );
	socketStdin[1] =0 ;
    }
#endif
}

/*!
  The process output data to either stdout or sderr.
*/
void QProcess::socketRead( int fd )
{
#if defined(UNIX)
    const size_t bufsize = 4096;
    char *buffer = new char[bufsize];
    int n = read( fd, buffer, bufsize-1 );

    if ( n == 0 ) {
	if ( fd == socketStdout[0] ) {
	    notifierStdout->setEnabled( FALSE );
	    delete notifierStdout;
	    notifierStdout = 0;
	    close( socketStdout[0] );
	    socketStdout[0] = 0;
//	    return;
	} else {
	    notifierStderr->setEnabled( FALSE );
	    delete notifierStderr;
	    notifierStderr = 0;
	    close( socketStderr[0] );
	    socketStderr[0] = 0;
//	    return;
	}

	if ( socketStderr[0] == 0 && socketStdout[0] == 0 ) {
	    emit processExited();
	    return;
	}
	/* ### just have to think what to do best
	if ( err[ 0 ] == 0 && out[ 0 ] == 0 ) {
	    int s;
	    waitpid( pid, &s, WNOHANG );
	    if ( WIFEXITED( s ) ) {
		pid = 0;
		int ret = WEXITSTATUS( s );

		if ( ret == 0 )
		    emit finished();
		else
		    emit failed();
	    }
	    return;
	}
	*/
    }

    QByteArray buf;
    buf.assign( buffer, n );
    if ( fd == socketStdout[0] ) {
	emit dataStdout( buf );
    } else {
	emit dataStderr( buf );
    }

    buffer[n] = 0;
    QString str( buffer );
    if ( fd == socketStdout[0] ) {
	emit dataStdout( str );
    } else {
	emit dataStderr( str );
    }
#endif
}

/*!
  The process tries to read data from stdin.
*/
void QProcess::socketWrite( int fd )
{
#if defined(UNIX)
    if ( fd != socketStdin[1] || socketStdin[1] == 0 )
	return;
    if ( stdinBuf.isEmpty() ) {
	notifierStdin->setEnabled( FALSE );
	return;
    }
    stdinBufRead += write( fd,
	    stdinBuf.head()->data() + stdinBufRead,
	    stdinBuf.head()->size() - stdinBufRead );
    if ( stdinBufRead == (ssize_t)stdinBuf.head()->size() ) {
	stdinBufRead = 0;
	delete stdinBuf.dequeue();
	socketWrite( fd );
    }
#endif
}
