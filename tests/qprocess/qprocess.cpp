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

#if defined( _WS_WIN_ )
static bool socketpair( int type, int s[2] )
{
#if 1
    SOCKET so;
    struct sockaddr_in sock_in;
    int len = sizeof( sock_in );

    // create socket and bind it to unused port
    so = socket( AF_INET, type, 0 );
    if ( so == INVALID_SOCKET ) {
	return FALSE;
    }
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = 0;
    sock_in.sin_addr.s_addr = INADDR_ANY;
    if ( bind( so, (struct sockaddr *) &sock_in, sizeof( sock_in ) ) < 0 ) {
	closesocket( so );
	return FALSE;
    }
    if ( getsockname( so, (struct sockaddr *) &sock_in, &len ) < 0 ) {
	closesocket( so );
	return FALSE;
    }
    listen( so, 2 );

    // create the outsocket
    s[1] = socket (AF_INET, type, 0);
    if ( s[1] == INVALID_SOCKET ) {
	closesocket( so );
	s[1] = 0;
	return FALSE;
    }
    sock_in.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    if ( connect( s[1], (struct sockaddr *) &sock_in, sizeof( sock_in ) ) < 0 ) {
	closesocket( so );
	closesocket( s[1] );
	s[1] = 0;
	return FALSE;
    }

    // create the insocket
    s[0] = accept( so, (struct sockaddr *) &sock_in, &len );
    if ( s[0] == INVALID_SOCKET ) {
	closesocket( so );
	closesocket( s[1] );
	s[1] = 0;
	s[0] = 0;
	return FALSE;
    }
    closesocket (so);

    return TRUE;
#else
    s[0] = socket( AF_INET, type, 0 );
    if ( s[0] == INVALID_SOCKET ) {
	s[0] = 0;
	return FALSE;
    }
    s[1] = s[0];

    return TRUE;
#endif
}
#endif

/*!
  Start the program.

  Return TRUE on success, otherwise FALSE.
*/
bool QProcess::start()
{
#if defined( _WS_WIN_ )
    if ( QApplication::winVersion() & Qt::WV_NT_based )
#endif
    {
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
    }
    
#if defined( UNIX )
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
	execvp( command.latin1(), (char*const*)arglist ); // ### a hack
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
    // TODO? test if exec was successful

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
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	// open sockets for piping
	if ( !socketpair( SOCK_STREAM, socketStdin ) ) {
	    return FALSE;
	}
	if ( !socketpair( SOCK_STREAM, socketStdout ) ) {
	    return FALSE;
	}
	if ( !socketpair( SOCK_STREAM, socketStderr ) ) {
	    return FALSE;
	}

	// construct the arguments for CreateProcess()
	QString args;
	args = command.latin1();
	for ( QStringList::Iterator it = arguments.begin(); it != arguments.end(); ++it ) {
	    args += QString( " \'" ) + (*it).latin1() + QString( "\'" );
	}

	// CreateProcess()
	char *argsC = new char[args.length()+1];
	strcpy( argsC, args.latin1() );
	STARTUPINFO startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
	    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    (HANDLE)(socketStdin[0]), (HANDLE)(socketStdout[1]), (HANDLE)(socketStderr[1]) };

	if  ( !CreateProcess( 0, argsC, 0, 0, TRUE, 0, 0, workingDir.absPath().latin1(), &startupInfo, &pid ) ) {
	    delete[] argsC;
	    return FALSE;
	}
	delete[] argsC;
	closesocket( socketStdin[0] );
	closesocket( socketStdout[1] );
	closesocket( socketStderr[1] );

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
	return TRUE;
    } else {
	// open the pipes
	// make non-inheritable copies of input write and output read handles
	// to avoid non-closable handles
	SECURITY_ATTRIBUTES secAtt = { sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE };
	HANDLE tmpStdin, tmpStdout, tmpStderr;
	if ( !CreatePipe( &pipeStdin[0], &tmpStdin, &secAtt, 0 ) ) {
	    return FALSE;
	}
	if ( !CreatePipe( &tmpStdout, &pipeStdout[1], &secAtt, 0 ) ) {
	    return FALSE;
	}
	if ( !CreatePipe( &tmpStderr, &pipeStderr[1], &secAtt, 0 ) ) {
	    return FALSE;
	}
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStdin,
	    GetCurrentProcess(), &pipeStdin[1],
	    0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	    return FALSE;
	}
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStdout,
	    GetCurrentProcess(), &pipeStdout[0],
	    0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	    return FALSE;
	}
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStderr,
	    GetCurrentProcess(), &pipeStderr[0],
	    0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	    return FALSE;
	}
	if ( !CloseHandle( tmpStdin ) ) {
	    return FALSE;
	}
	if ( !CloseHandle( tmpStdout ) ) {
	    return FALSE;
	}
	if ( !CloseHandle( tmpStderr ) ) {
	    return FALSE;
	}

	// construct the arguments for CreateProcess()
	QString args;
	args = command.latin1();
	for ( QStringList::Iterator it = arguments.begin(); it != arguments.end(); ++it ) {
	    args += QString( " \'" ) + (*it).latin1() + QString( "\'" );
	}

	// CreateProcess()
	char *argsC = new char[args.length()+1];
	strcpy( argsC, args.latin1() );
	STARTUPINFO startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
	    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    pipeStdin[0], pipeStdout[1], pipeStderr[1] };

	if  ( !CreateProcess( 0, argsC, 0, 0, TRUE, 0, 0, workingDir.absPath().latin1(), &startupInfo, &pid ) ) {
	    delete[] argsC;
	    return FALSE;
	}
	delete[] argsC;
	CloseHandle( pipeStdin[0] );
	CloseHandle( pipeStdout[1] );
	CloseHandle( pipeStderr[1] );

	// cleanup and return
	return TRUE;
    }
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
#if defined(UNIX)
    int status;
    if ( waitpid( pid, &status, WNOHANG ) == -1 )
	return FALSE;
    if ( WIFEXITED(status) == -1 )
	return FALSE;

    if ( socketStderr[0] == 0 && socketStdout[0] == 0 ) {
	return FALSE;
    } else {
	return TRUE;
    }
#else
    return TRUE;
#endif
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
    stdinBuf.enqueue( new QByteArray(buf) );
    if ( notifierStdin != 0 )
        notifierStdin->setEnabled( TRUE );
#if defined(UNIX)
    socketWrite( socketStdin[1] );
#else
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	socketWrite( socketStdin[1] );
    } else {
	socketWrite( 0 );
    }
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
    QByteArray bbuf;
    bbuf.duplicate( buf.latin1(), buf.length() );
    dataStdin( bbuf );
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
#else
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	if ( socketStdin[1] != 0 ) {
	    closesocket( socketStdin[1] );
	    socketStdin[1] = 0;
	}
    } else {
	if ( pipeStdin[1] != 0 ) {
            CloseHandle( pipeStdin[1] );
	    pipeStdin[1] = 0;
	}
    }
#endif
}

/*!
  The process has output data to either stdout or sderr.
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

	return;
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

    buffer[n] = 0;
    QString str( QString::fromLocal8Bit( buffer ) );
    QByteArray buf;
    buf.assign( buffer, n );

    if ( fd == socketStdout[0] ) {
	emit dataStdout( buf );
    } else {
	emit dataStderr( buf );
    }
    if ( fd == socketStdout[0] ) {
	emit dataStdout( str );
    } else {
	emit dataStderr( str );
    }
#else
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	qDebug( "Hot Wild Sex!" );
    } else {
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
#else
    DWORD written;
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	if ( fd != socketStdin[1] || socketStdin[1] == 0 )
	    return;
	if ( stdinBuf.isEmpty() ) {
	    notifierStdin->setEnabled( FALSE );
	    return;
	}
	if ( !WriteFile( (HANDLE)(socketStdin[1]),
	    stdinBuf.head()->data() + stdinBufRead,
	    stdinBuf.head()->size() - stdinBufRead,
	    &written, 0 ) ) {
	    return;
	}
    } else {
	if ( !WriteFile( pipeStdin[1],
	    stdinBuf.head()->data() + stdinBufRead,
	    stdinBuf.head()->size() - stdinBufRead,
	    &written, 0 ) ) {
	    return;
	}
    }
    stdinBufRead += written;
    if ( stdinBufRead == stdinBuf.head()->size() ) {
	stdinBufRead = 0;
	delete stdinBuf.dequeue();
	socketWrite( fd );
    }

    // ### try to read (just for test purposes)
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
    } else {
	QByteArray buffer=readStdout();
	int sz = buffer.size();
	if ( sz == 0 )
	    return;
	buffer.resize( sz+1 );
	buffer[ sz ] = 0;
	QString str( buffer );
	emit dataStdout( str );
    }
#endif
}

#if defined( _WS_WIN_ )
// testing if non blocking pipes are working
QByteArray QProcess::readStdout()
{
#if 0
    const int defsize = 1024;
    unsigned long r, i = 0;
    QByteArray readBuffer;
    do {
	readBuffer.resize( (i+1) * defsize );
	ReadFile( pipeStdout[0], readBuffer.data() + (i*defsize), defsize, &r, &overlapped );
//	PeekNamedPipe( pipeStdout[0], readBuffer.data() + (i*defsize), defsize, &r, 0, 0 );
	i++;
    } while ( r == defsize );
    readBuffer.resize( (i-1) * defsize + r );
    return readBuffer;
#else
    // get the number of bytes that are waiting to be read
    char dummy;
    unsigned long r, i;
    PeekNamedPipe( pipeStdout[0], &dummy, 1, &r, &i, 0 );
    // and read it!
    QByteArray readBuffer( i );
    if ( i > 0 ) {
	ReadFile( pipeStdout[0], readBuffer.data(), i, &r, 0 );
    }
    return readBuffer;
#endif
}
#endif
