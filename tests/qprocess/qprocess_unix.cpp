#include <stdio.h>
#include <stdlib.h>

#include "qapplication.h"
#include "qprocess.h"


void QProcess::init()
{
    stdinBufRead = 0;

    notifierStdin = 0;
    notifierStdout = 0;
    notifierStderr = 0;

    socketStdin[0] = 0;
    socketStdin[1] = 0;
    socketStdout[0] = 0;
    socketStdout[1] = 0;
    socketStderr[0] = 0;
    socketStderr[1] = 0;
}


bool QProcess::start()
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
}


bool QProcess::kill()
{
    return TRUE;
}

bool QProcess::isRunning()
{
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
}

void QProcess::dataStdin( const QByteArray& buf )
{
    stdinBuf.enqueue( new QByteArray(buf) );
    if ( notifierStdin != 0 )
        notifierStdin->setEnabled( TRUE );
    socketWrite( socketStdin[1] );
}


void QProcess::closeStdin( )
{
    if ( socketStdin[1] !=0 ) {
	close( socketStdin[1] );
	socketStdin[1] =0 ;
    }
}


void QProcess::socketRead( int fd )
{
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
}


void QProcess::socketWrite( int fd )
{
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
}

void QProcess::timeout()
{
}
