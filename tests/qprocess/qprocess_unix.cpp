#include <stdio.h>
#include <stdlib.h>

#include "qapplication.h"
#include "qprocess.h"


QProcessPrivate::QProcessPrivate()
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

    exitStat = 0;
    exitNormal = FALSE;
}

QProcessPrivate::~QProcessPrivate()
{
    while ( !stdinBuf.isEmpty() ) {
	delete stdinBuf.dequeue();
    }
    if ( notifierStdin ) {
	notifierStdin->setEnabled( FALSE );
	delete notifierStdin;
    }
    if ( notifierStdout ) {
	notifierStdout->setEnabled( FALSE );
	delete notifierStdout;
    }
    if ( notifierStderr ) {
	notifierStderr->setEnabled( FALSE );
	delete notifierStderr;
    }
    if( socketStdin[1] != 0 )
	close( socketStdin[1] );
    if( socketStdout[0] != 0 )
	close( socketStdout[0] );
    if( socketStderr[0] != 0 )
	close( socketStderr[0] );
}


bool QProcess::start()
{
    // cleanup the notifiers
    if ( d->notifierStdin ) {
	d->notifierStdin->setEnabled( FALSE );
	delete d->notifierStdin;
	d->notifierStdin = 0;
    }
    if ( d->notifierStdout ) {
	d->notifierStdout->setEnabled( FALSE );
	delete d->notifierStdout;
	d->notifierStdout = 0;
    }
    if ( d->notifierStderr ) {
	d->notifierStderr->setEnabled( FALSE );
	delete d->notifierStderr;
	d->notifierStderr = 0;
    }

    // open sockets for piping
    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, d->socketStdin ) ) {
	return FALSE;
    }
    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, d->socketStdout ) ) {
	return FALSE;
    }
    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, d->socketStderr ) ) {
	return FALSE;
    }

    // construct the arguments for exec
    const char** arglist = new const char*[ d->arguments.count() + 2 ];
    arglist[0] = d->command.latin1();
    int i = 1;
    for ( QStringList::Iterator it = d->arguments.begin(); it != d->arguments.end(); ++it ) {
	arglist[ i++ ] = (*it).latin1();
    }
    arglist[i] = 0;

    // fork and exec
    QApplication::flushX();
    d->pid = fork();
    if ( d->pid == 0 ) {
	// child
	close( d->socketStdin[1] );
	close( d->socketStdout[0] );
	close( d->socketStderr[0] );
	dup2( d->socketStdin[0], STDIN_FILENO );
	dup2( d->socketStdout[1], STDOUT_FILENO );
	dup2( d->socketStderr[1], STDERR_FILENO );
	chdir( d->workingDir.absPath().latin1() );
	execvp( d->command.latin1(), (char*const*)arglist ); // ### a hack
	exit( -1 );
    } else if ( d->pid == -1 ) {
	// error forking
	close( d->socketStdin[1] );
	close( d->socketStdout[0] );
	close( d->socketStderr[0] );
	close( d->socketStdin[0] );
	close( d->socketStdout[1] );
	close( d->socketStderr[1] );
	delete[] arglist;
	return FALSE;
    }
    close( d->socketStdin[0] );
    close( d->socketStdout[1] );
    close( d->socketStderr[1] );
    // TODO? test if exec was successful

    // setup notifiers for the sockets
    d->notifierStdin = new QSocketNotifier( d->socketStdin[1],
	    QSocketNotifier::Write, this );
    d->notifierStdout = new QSocketNotifier( d->socketStdout[0],
	    QSocketNotifier::Read, this );
    d->notifierStderr = new QSocketNotifier( d->socketStderr[0],
	    QSocketNotifier::Read, this );
    connect( d->notifierStdin, SIGNAL(activated(int)),
	    this, SLOT(socketWrite(int)) );
    connect( d->notifierStdout, SIGNAL(activated(int)),
	    this, SLOT(socketRead(int)) );
    connect( d->notifierStderr, SIGNAL(activated(int)),
	    this, SLOT(socketRead(int)) );
    d->notifierStdin->setEnabled( TRUE );
    d->notifierStdout->setEnabled( TRUE );
    d->notifierStderr->setEnabled( TRUE );

    // cleanup and return
    delete[] arglist;
    return TRUE;
}


bool QProcess::hangUp()
{
    return TRUE;
}

bool QProcess::kill()
{
    return TRUE;
}

bool QProcess::isRunning()
{
    int status;
    if ( waitpid( d->pid, &status, WNOHANG ) == -1 )
	return FALSE;
    if ( WIFEXITED(status) == -1 )
	return FALSE;

    if ( d->socketStderr[0] == 0 && d->socketStdout[0] == 0 ) {
	return FALSE;
    } else {
	return TRUE;
    }
}

void QProcess::dataStdin( const QByteArray& buf )
{
    d->stdinBuf.enqueue( new QByteArray(buf) );
    if ( d->notifierStdin != 0 )
        d->notifierStdin->setEnabled( TRUE );
    socketWrite( d->socketStdin[1] );
}


void QProcess::closeStdin( )
{
    if ( d->socketStdin[1] !=0 ) {
	close( d->socketStdin[1] );
	d->socketStdin[1] =0 ;
    }
}


void QProcess::socketRead( int fd )
{
    const size_t bufsize = 4096;
    char *buffer = new char[bufsize];
    int n = read( fd, buffer, bufsize-1 );

    if ( n == 0 ) {
	if ( fd == d->socketStdout[0] ) {
	    d->notifierStdout->setEnabled( FALSE );
	    delete d->notifierStdout;
	    d->notifierStdout = 0;
	    close( d->socketStdout[0] );
	    d->socketStdout[0] = 0;
//	    return;
	} else {
	    d->notifierStderr->setEnabled( FALSE );
	    delete d->notifierStderr;
	    d->notifierStderr = 0;
	    close( d->socketStderr[0] );
	    d->socketStderr[0] = 0;
//	    return;
	}

	if ( d->socketStderr[0] == 0 && d->socketStdout[0] == 0 ) {
	    emit processExited();
	    return;
	}

	return;
	/* ### just have to think what to do best
	if ( err[ 0 ] == 0 && out[ 0 ] == 0 ) {
	    int s;
	    waitpid( d->pid, &s, WNOHANG );
	    if ( WIFEXITED( s ) ) {
		d->pid = 0;
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

    if ( fd == d->socketStdout[0] ) {
	emit dataStdout( buf );
    } else {
	emit dataStderr( buf );
    }
    if ( fd == d->socketStdout[0] ) {
	emit dataStdout( str );
    } else {
	emit dataStderr( str );
    }
}


void QProcess::socketWrite( int fd )
{
    if ( fd != d->socketStdin[1] || d->socketStdin[1] == 0 )
	return;
    if ( d->stdinBuf.isEmpty() ) {
	d->notifierStdin->setEnabled( FALSE );
	return;
    }
    d->stdinBufRead += write( fd,
	    d->stdinBuf.head()->data() + d->stdinBufRead,
	    d->stdinBuf.head()->size() - d->stdinBufRead );
    if ( d->stdinBufRead == (ssize_t)d->stdinBuf.head()->size() ) {
	d->stdinBufRead = 0;
	delete d->stdinBuf.dequeue();
	socketWrite( fd );
    }
}

// only used under Windows
void QProcess::timeout()
{
}
