#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

#include "qapplication.h"
#include "qlist.h"
#include "qprocess.h"

#define QPROCESS_DEBUG

struct sigaction *QProcessPrivate::oldact;
QList<QProcess> *QProcessPrivate::proclist = 0;

QProcessPrivate::QProcessPrivate( QProcess *proc ) : d( proc )
{
#if defined(QPROCESS_DEBUG)
    qDebug( "QProcessPrivate: Constructor" );
#endif
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

    exitValuesCalculated = FALSE;
    exitStat = 0;
    exitNormal = FALSE;

    // install a SIGCHLD handler
    if ( proclist == 0 ) {
	proclist = new QList<QProcess>;

	struct sigaction act;
	act.sa_handler = sigchldHnd;
	sigemptyset( &(act.sa_mask) );
	sigaddset( &(act.sa_mask), SIGCHLD );
	act.sa_flags = SA_NOCLDSTOP;
#ifdef SA_RESTART
	act.sa_flags |= SA_RESTART;
#endif
	if ( sigaction( SIGCHLD, &act, oldact ) != 0 )
	    qWarning( "Error installing SIGCHLD handler" );
    }
    proclist->append( d );
}

QProcessPrivate::~QProcessPrivate()
{
#if defined(QPROCESS_DEBUG)
    qDebug( "QProcessPrivate: Destructor" );
#endif
    // restore SIGCHLD handler
    proclist->remove( d );
    if ( proclist->count() == 0 ) {
	delete proclist;
	if ( sigaction( SIGCHLD, oldact, 0 ) != 0 )
	    qWarning( "Error restoring SIGCHLD handler" );
    }

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
	::close( socketStdin[1] );
    if( socketStdout[0] != 0 )
	::close( socketStdout[0] );
    if( socketStderr[0] != 0 )
	::close( socketStderr[0] );
}

void QProcessPrivate::sigchldHnd( int )
{
    QProcess *proc;
    for ( proc=proclist->first(); proc!=0; proc=proclist->next() ) {
	if ( !proc->d->exitValuesCalculated && !proc->isRunning() ) {
#if defined(QPROCESS_DEBUG)
	    qDebug( "QProcessPrivate::sigchldHnd(): process exited" );
#endif
	    emit proc->processExited();
	}
    }
}


bool QProcess::start()
{
#if defined(QPROCESS_DEBUG)
    qDebug( "QProcess::start()" );
#endif
    d->exitValuesCalculated = FALSE;
    d->exitStat = 0;
    d->exitNormal = FALSE;

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
    if ( ::socketpair( AF_UNIX, SOCK_STREAM, 0, d->socketStdin ) ) {
	return FALSE;
    }
    if ( ::socketpair( AF_UNIX, SOCK_STREAM, 0, d->socketStdout ) ) {
	return FALSE;
    }
    if ( ::socketpair( AF_UNIX, SOCK_STREAM, 0, d->socketStderr ) ) {
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
	::close( d->socketStdin[1] );
	::close( d->socketStdout[0] );
	::close( d->socketStderr[0] );
	::dup2( d->socketStdin[0], STDIN_FILENO );
	::dup2( d->socketStdout[1], STDOUT_FILENO );
	::dup2( d->socketStderr[1], STDERR_FILENO );
	::chdir( d->workingDir.absPath().latin1() );
	::execvp( d->command.latin1(), (char*const*)arglist ); // ### a hack
	::exit( -1 );
    } else if ( d->pid == -1 ) {
	// error forking
	::close( d->socketStdin[1] );
	::close( d->socketStdout[0] );
	::close( d->socketStderr[0] );
	::close( d->socketStdin[0] );
	::close( d->socketStdout[1] );
	::close( d->socketStderr[1] );
	delete[] arglist;
	return FALSE;
    }
    ::close( d->socketStdin[0] );
    ::close( d->socketStdout[1] );
    ::close( d->socketStderr[1] );
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
    return ::kill( d->pid, SIGHUP ) == 0;
}

bool QProcess::kill()
{
    return ::kill( d->pid, SIGKILL ) == 0;
}

bool QProcess::isRunning()
{
    if ( d->exitValuesCalculated ) {
#if defined(QPROCESS_DEBUG)
	qDebug( "QProcess::isRunning(): FALSE (already computed)" );
#endif
	return FALSE;
    } else {
	int status;
	if ( ::waitpid( d->pid, &status, WNOHANG ) == d->pid )
	{
	    // compute the exit values
	    d->exitNormal = WIFEXITED( status ) != 0;
	    if ( d->exitNormal ) {
		d->exitStat = WEXITSTATUS( status );
	    }
	    d->exitValuesCalculated = TRUE;
#if defined(QPROCESS_DEBUG)
	    qDebug( "QProcess::isRunning(): FALSE" );
#endif
	    return FALSE;
	} else {
#if defined(QPROCESS_DEBUG)
	    qDebug( "QProcess::isRunning(): TRUE" );
#endif
	    return TRUE;
	}
    }
}

void QProcess::dataStdin( const QByteArray& buf )
{
#if defined(QPROCESS_DEBUG)
//    qDebug( "QProcess::dataStdin(): write to stdin (%d)", d->socketStdin[1] );
#endif
    d->stdinBuf.enqueue( new QByteArray(buf) );
    if ( d->notifierStdin != 0 )
        d->notifierStdin->setEnabled( TRUE );
    socketWrite( d->socketStdin[1] );
}


void QProcess::closeStdin()
{
    if ( d->socketStdin[1] !=0 ) {
	// ### what is with pending data?
	if ( d->notifierStdin ) {
	    d->notifierStdin->setEnabled( FALSE );
	    delete d->notifierStdin;
	    d->notifierStdin = 0;
	}
	if ( ::close( d->socketStdin[1] ) != 0 ) {
	    qWarning( "Could not close stdin of child process" );
	}
#if defined(QPROCESS_DEBUG)
	qDebug( "QProcess::closeStdin(): stdin (%d) closed", d->socketStdin[1] );
#endif
	d->socketStdin[1] =0 ;
    }
}


void QProcess::socketRead( int fd )
{
#if defined(QPROCESS_DEBUG)
    qDebug( "QProcess::socketRead(): %d", fd );
#endif
    const size_t bufsize = 4096;
    char *buffer = new char[bufsize];
    int n = ::read( fd, buffer, bufsize-1 );

    if ( n == 0 ) {
	if ( fd == d->socketStdout[0] ) {
#if defined(QPROCESS_DEBUG)
	    qDebug( "QProcess::socketRead(): stdout (%d) closed", fd );
#endif
	    d->notifierStdout->setEnabled( FALSE );
	    delete d->notifierStdout;
	    d->notifierStdout = 0;
	    ::close( d->socketStdout[0] );
	    d->socketStdout[0] = 0;
	    return;
	} else {
#if defined(QPROCESS_DEBUG)
	    qDebug( "QProcess::socketRead(): stderr (%d) closed", fd );
#endif
	    d->notifierStderr->setEnabled( FALSE );
	    delete d->notifierStderr;
	    d->notifierStderr = 0;
	    ::close( d->socketStderr[0] );
	    d->socketStderr[0] = 0;
	    return;
	}

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
#if defined(QPROCESS_DEBUG)
	qDebug( "QProcess::socketRead(): read from stdout (%d)", fd );
#endif
	emit dataStdout( buf );
    } else {
#if defined(QPROCESS_DEBUG)
	qDebug( "QProcess::socketRead(): read from stderr (%d)", fd );
#endif
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
#if defined(QPROCESS_DEBUG)
    qDebug( "QProcess::socketWrite(): write to stdin (%d)", fd );
#endif
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
