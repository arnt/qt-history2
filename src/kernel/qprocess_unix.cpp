/****************************************************************************
** $Id:$
**
** Implementation of QProcess class for Unix
**
** Created : 20000905
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qprocess.h"

#ifndef QT_NO_PROCESS

#include "qapplication.h"
#include "qqueue.h"
#include "qlist.h"
#include "qsocketnotifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>

//#define QPROCESS_DEBUG

#ifdef __MIPSEL__
#ifndef SOCK_DGRAM
#define SOCK_DGRAM 1
#endif
#ifndef SOCK_STREAM
#define SOCK_STREAM 2
#endif
#endif

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

#if defined(_OS_OSF_) || ( defined(_OS_IRIX_) && defined(_CC_GNU_) ) || defined(_OS_MACX_)
static void qt_C_sigchldHnd();
#else
static void qt_C_sigchldHnd( int );
#endif

#if defined(Q_C_CALLBACKS)
}
#endif


/***********************************************************************
 *
 * QProcessPrivate
 *
 **********************************************************************/
class QProcessPrivate
{
public:
    QProcessPrivate( QProcess *proc ) : d( proc )
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

	// install a SIGCHLD handler
	if ( proclist == 0 ) {
	    proclist = new QList<QProcess>;

	    struct sigaction act;
	    act.sa_handler = qt_C_sigchldHnd;
	    sigemptyset( &(act.sa_mask) );
	    sigaddset( &(act.sa_mask), SIGCHLD );
	    act.sa_flags = SA_NOCLDSTOP;
#if defined(SA_RESTART)
	    act.sa_flags |= SA_RESTART;
#endif
#if defined(QPROCESS_DEBUG)
	    qDebug( "QProcessPrivate: install a sigchild handler" );
#endif
	    if ( sigaction( SIGCHLD, &act, oldact ) != 0 )
		qWarning( "Error installing SIGCHLD handler" );
	}
	proclist->append( d );
    }

    ~QProcessPrivate()
    {
#if defined(QPROCESS_DEBUG)
	qDebug( "QProcessPrivate: Destructor" );
#endif
	// restore SIGCHLD handler
	if ( proclist != 0 ) {
	    proclist->remove( d );
	    if ( proclist->count() == 0 ) {
		delete proclist;
		proclist = 0;
#if defined(QPROCESS_DEBUG)
		qDebug( "QProcessPrivate: restore old sigchild handler" );
#endif
		if ( sigaction( SIGCHLD, oldact, 0 ) != 0 )
		    qWarning( "Error restoring SIGCHLD handler" );
	    }
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

    static void sigchldHnd()
    {
#if defined(QPROCESS_DEBUG)
		qDebug( "QProcessPrivate::sigchldHnd()" );
#endif
	if ( !proclist )
	    return;
	QProcess *proc;
	for ( proc=proclist->first(); proc!=0; proc=proclist->next() ) {
	    if ( !proc->d->exitValuesCalculated && !proc->isRunning() ) {
#if defined(QPROCESS_DEBUG)
		qDebug( "QProcessPrivate::sigchldHnd(): process exited" );
#endif
		// read pending data
		proc->socketRead( proc->d->socketStdout[0] );
		proc->socketRead( proc->d->socketStderr[0] );

		if ( proc->notifyOnExit )
		    emit proc->processExited();
		// the slot might have deleted the last process...
		if ( !proclist )
		    return;
	    }
	}
    }

    QQueue<QByteArray> stdinBuf;

    QSocketNotifier *notifierStdin;
    QSocketNotifier *notifierStdout;
    QSocketNotifier *notifierStderr;
    int socketStdin[2];
    int socketStdout[2];
    int socketStderr[2];

    pid_t pid;
    ssize_t stdinBufRead;
    QProcess *d;
    static struct sigaction *oldact;
    static QList<QProcess> *proclist;

    bool exitValuesCalculated;
};

struct sigaction *QProcessPrivate::oldact = 0;
QList<QProcess> *QProcessPrivate::proclist = 0;


/***********************************************************************
 *
 * sigchld handler callback
 *
 **********************************************************************/
#if defined(_OS_OSF_) || ( defined(_OS_IRIX_) && defined(_CC_GNU_) ) || defined(_OS_MACX_)
void qt_C_sigchldHnd()
#else
void qt_C_sigchldHnd( int )
#endif
{
    QProcessPrivate::sigchldHnd();
}


/***********************************************************************
 *
 * QProcess
 *
 **********************************************************************/
/*!
  Basic initialization
*/
void QProcess::init()
{
    d = new QProcessPrivate( this );
    exitStat = 0;
    exitNormal = FALSE;
}

/*!
  Destructor; if the process is running, it is NOT terminated! Stdin, stdout
  and stderr of the process are closed.
*/
QProcess::~QProcess()
{
    delete d;
}

/*!
  Runs the process. You can write data to the stdin of the process with
  dataStdin(), you can close stdin with closeStdin() and you can terminate the
  process hangUp() resp. kill().

  Returns TRUE if the process could be started, otherwise FALSE.

  \sa launch()
*/
bool QProcess::start()
{
#if defined(QPROCESS_DEBUG)
    qDebug( "QProcess::start()" );
#endif
    d->exitValuesCalculated = FALSE;
    exitStat = 0;
    exitNormal = FALSE;

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
    QCString *arglistQ = new QCString[ arguments.count() + 1 ];
    const char** arglist = new const char*[ arguments.count() + 1 ];
    int i = 0;
    for ( QStringList::Iterator it = arguments.begin(); it != arguments.end(); ++it ) {
	arglistQ[i] = (*it).local8Bit();
	arglist[i] = arglistQ[i];
#if defined(QPROCESS_DEBUG)
	qDebug( "QProcess::start(): arg %d = %s", i, arglist[i] );
#endif
	i++;
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
	::close( STDIN_FILENO );
	::close( STDOUT_FILENO );
	::close( STDERR_FILENO );
	::dup2( d->socketStdin[0], STDIN_FILENO );
	::dup2( d->socketStdout[1], STDOUT_FILENO );
	::dup2( d->socketStderr[1], STDERR_FILENO );
	::chdir( workingDir.absPath().latin1() );
	::execvp( arglist[0], (char*const*)arglist ); // ### cast not nice
	::exit( -1 );
    } else if ( d->pid == -1 ) {
	// error forking
	::close( d->socketStdin[1] );
	::close( d->socketStdout[0] );
	::close( d->socketStderr[0] );
	::close( d->socketStdin[0] );
	::close( d->socketStdout[1] );
	::close( d->socketStderr[1] );
	delete[] arglistQ;
	delete[] arglist;
	return FALSE;
    }
    ::close( d->socketStdin[0] );
    ::close( d->socketStdout[1] );
    ::close( d->socketStderr[1] );
    // ### test if exec was successful? How?

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
    if ( !d->stdinBuf.isEmpty() ) {
	d->notifierStdin->setEnabled( TRUE );
    }
    if ( ioRedirection ) {
	d->notifierStdout->setEnabled( TRUE );
	d->notifierStderr->setEnabled( TRUE );
    }

    // cleanup and return
    delete[] arglistQ;
    delete[] arglist;
    return TRUE;
}


/*!
  Asks the process to terminate. If this does not work you can try kill()
  instead.

  Returns TRUE on success, otherwise FALSE.
*/
bool QProcess::hangUp()
{
    return ::kill( d->pid, SIGHUP ) == 0;
}

/*!
  Terminates the process. This is not a safe way to end a process; you should
  try hangUp() first and use this function only if it failed.

  Returns TRUE on success, otherwise FALSE.
*/
bool QProcess::kill()
{
    return ::kill( d->pid, SIGKILL ) == 0;
}

/*!
  Returns TRUE if the process is running, otherwise FALSE.
*/
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
	    exitNormal = WIFEXITED( status ) != 0;
	    if ( exitNormal ) {
		exitStat = WEXITSTATUS( status );
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

/*!
  Writes data to the stdin of the process. The process may or may not read this
  data. If the data was read, the signal wroteStdin() is emitted.
*/
void QProcess::dataStdin( const QByteArray& buf )
{
#if defined(QPROCESS_DEBUG)
//    qDebug( "QProcess::dataStdin(): write to stdin (%d)", d->socketStdin[1] );
#endif
    d->stdinBuf.enqueue( new QByteArray(buf) );
    if ( d->notifierStdin != 0 )
        d->notifierStdin->setEnabled( TRUE );
}


/*!
  Closes stdin.

  If there is pending data, ### what happens with it?
*/
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
	d->socketStdin[1] = 0;
    }
}


/*!
  The process has outputted data to either stdout or stderr.
*/
void QProcess::socketRead( int fd )
{
#if defined(QPROCESS_DEBUG)
    qDebug( "QProcess::socketRead(): %d", fd );
#endif
    if ( fd == 0 )
	return;
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


/*!
  The process tries to read data from stdin.
*/
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
    ssize_t ret = ::write( fd,
	    d->stdinBuf.head()->data() + d->stdinBufRead,
	    d->stdinBuf.head()->size() - d->stdinBufRead );
    if ( ret > 0 )
	d->stdinBufRead += ret;
    if ( d->stdinBufRead == (ssize_t)d->stdinBuf.head()->size() ) {
	d->stdinBufRead = 0;
	delete d->stdinBuf.dequeue();
	if ( wroteStdinConnected && d->stdinBuf.isEmpty() )
	    emit wroteStdin();
	socketWrite( fd );
    }
}

/*!
  Only used under Windows (but moc does not know about #if defined()).
*/
void QProcess::timeout()
{
}


/*!
  Used by connectNotify() and disconnectNotify() to change the value of
  ioRedirection (and related behaviour)
*/
void QProcess::setIoRedirection( bool value )
{
    ioRedirection = value;
    if ( ioRedirection ) {
	if ( d->notifierStdout )
	    d->notifierStdout->setEnabled( TRUE );
	if ( d->notifierStderr )
	    d->notifierStderr->setEnabled( TRUE );
    } else {
	if ( d->notifierStdout )
	    d->notifierStdout->setEnabled( FALSE );
	if ( d->notifierStderr )
	    d->notifierStderr->setEnabled( FALSE );
    }
}

/*!
  Used by connectNotify() and disconnectNotify() to change the value of
  notifyOnExit (and related behaviour)
*/
void QProcess::setNotifyOnExit( bool value )
{
    notifyOnExit = value;
}

/*!
  Used by connectNotify() and disconnectNotify() to change the value of
  wroteStdinConnected (and related behaviour)
*/
void QProcess::setWroteStdinConnected( bool value )
{
    wroteStdinConnected = value;
}

#endif // QT_NO_PROCESS
