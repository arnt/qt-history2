/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprocess_unix.cpp#42 $
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
#include "qtimer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>

//#define QT_QPROCESS_DEBUG

#ifdef __MIPSEL__
#ifndef SOCK_DGRAM
#define SOCK_DGRAM 1
#endif
#ifndef SOCK_STREAM
#define SOCK_STREAM 2
#endif
#endif

#if defined(SIGNAL_HACK)
#undef SIGNAL_HACK
#endif

#if defined(Q_OS_OSF) || defined(Q_OS_MACX)
// these platforms work one way
#define SIGNAL_HACK
#elif defined(Q_CC_GNU) && ( defined(Q_OS_IRIX) || defined(Q_OS_SUN) )
// gcc on these platforms works above, but native compiler doesn't.
#define SIGNAL_HACK
#endif


#if defined(Q_C_CALLBACKS)
extern "C" {
#endif
    // this looks like two unconnected platform differences that just
    // happen to match up... perhaps we should use two defines, one
    // for each yuckyness.
#if defined(SIGNAL_HACK)
static void qt_C_sigign() { (*SIG_IGN)(SIGPIPE); }
static void qt_C_sigchldHnd();
#else
#define qt_C_sigign SIG_IGN
static void qt_C_sigchldHnd( int );
#endif
#if defined(Q_C_CALLBACKS)
}
#endif

class QProc;
class QProcessManager;
class QProcessPrivate
{
public:
    QProcessPrivate();
    ~QProcessPrivate();

    void closeOpenSocketsForChild();
    void newProc( pid_t pid, QProcess *process );

    QQueue<QByteArray> stdinBuf;

    QSocketNotifier *notifierStdin;
    QSocketNotifier *notifierStdout;
    QSocketNotifier *notifierStderr;
    int socketStdin[2];
    int socketStdout[2];
    int socketStderr[2];

    ssize_t stdinBufRead;
    QProc *proc;

    bool exitValuesCalculated;

    static QProcessManager *procManager;
};


/***********************************************************************
 *
 * QProc
 *
 **********************************************************************/
/*
  The class QProcess does not necessarily map exactly to the running
  child processes: if the process is finished, the QProcess class may still be
  there; furthermore a user can use QProcess to start more than one process.

  The helper-class QProc has the semantics that one instance of this class maps
  directly to a running child process.
*/
class QProc
{
public:
    QProc( pid_t p, QProcess *proc=0 ) : pid(p), process(proc)
    {
#if defined(QT_QPROCESS_DEBUG)
	qDebug( "QProc: Constructor for pid %d and QProcess %p", pid, process );
#endif
    }
    ~QProc()
    {
#if defined(QT_QPROCESS_DEBUG)
	qDebug( "QProc: Destructor for pid %d and QProcess %p", pid, process );
#endif
	if ( process != 0 )
	    process->d->proc = 0;
    }

    pid_t pid;
    QProcess *process;
};

/***********************************************************************
 *
 * QProcessManager
 *
 **********************************************************************/
class QProcessManager : public QObject
{
    Q_OBJECT

public:
    QProcessManager();
    ~QProcessManager();

    void append( QProc *p );
    void remove( QProc *p );

public slots:
    void sigchldHnd( int );

public:
    struct sigaction oldactChld;
    struct sigaction oldactPipe;
    QList<QProc> *procList;
    int sigchldFd[2];
};

QProcessManager::QProcessManager()
{
    procList = new QList<QProc>;
    procList->setAutoDelete( TRUE );

    // The SIGCHLD handler writes to a socket to tell the manager that
    // something happened. This is done to get the processing in sync with the
    // event reporting.
    if ( ::socketpair( AF_UNIX, SOCK_STREAM, 0, sigchldFd ) ) {
	sigchldFd[0] = 0;
	sigchldFd[1] = 0;
    } else {
#if defined(QT_QPROCESS_DEBUG)
	qDebug( "QProcessManager: install socket notifier (%d)", sigchldFd[1] );
#endif
	QSocketNotifier *sn = new QSocketNotifier( sigchldFd[1],
		QSocketNotifier::Read, this );
	connect( sn, SIGNAL(activated(int)),
		this, SLOT(sigchldHnd(int)) );
	sn->setEnabled( TRUE );
    }

    // install a SIGCHLD handler and ignore SIGPIPE
    struct sigaction act;

#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcessManager: install a sigchild handler" );
#endif
    act.sa_handler = qt_C_sigchldHnd;
    sigemptyset( &(act.sa_mask) );
    sigaddset( &(act.sa_mask), SIGCHLD );
    act.sa_flags = SA_NOCLDSTOP;
#if defined(SA_RESTART)
    act.sa_flags |= SA_RESTART;
#endif
    if ( sigaction( SIGCHLD, &act, &oldactChld ) != 0 )
	qWarning( "Error installing SIGCHLD handler" );

#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcessManager: install a sigpipe handler (SIG_IGN)" );
#endif
    act.sa_handler = qt_C_sigign;
    sigemptyset( &(act.sa_mask) );
    sigaddset( &(act.sa_mask), SIGPIPE );
    act.sa_flags = 0;
    if ( sigaction( SIGPIPE, &act, &oldactPipe ) != 0 )
	qWarning( "Error installing SIGPIPE handler" );
}

QProcessManager::~QProcessManager()
{
    // ### delete the elements
    delete procList;

    if ( sigchldFd[0] != 0 )
	::close( sigchldFd[0] );
    if ( sigchldFd[1] != 0 )
	::close( sigchldFd[1] );

    // restore SIGCHLD handler
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcessManager: restore old sigchild handler" );
#endif
    if ( sigaction( SIGCHLD, &oldactChld, 0 ) != 0 )
	qWarning( "Error restoring SIGCHLD handler" );

#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcessManager: restore old sigpipe handler" );
#endif
    if ( sigaction( SIGPIPE, &oldactPipe, 0 ) != 0 )
	qWarning( "Error restoring SIGPIPE handler" );
}

void QProcessManager::append( QProc *p )
{
    procList->append( p );
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcessManager: append process (procList.count(): %d)", procList->count() );
#endif
}

void QProcessManager::remove( QProc *p )
{
    procList->remove( p );
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcessManager: remove process (procList.count(): %d)", procList->count() );
#endif
    if ( procList->count() == 0 ) {
	// ### this looks dangerous: do that better with a single shot timer
	QProcessPrivate::procManager = 0;
	delete this;
    }
}

void QProcessManager::sigchldHnd( int fd )
{
    char tmp;
    ::read( fd, &tmp, sizeof(tmp) );
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcessManager::sigchldHnd()" );
#endif
    QProc *proc;
    QProcess *process;
    bool removeProc;
    proc = procList->first();
    while ( proc != 0 ) {
	removeProc = FALSE;
	process = proc->process;
	if ( process != 0 ) {
	    if ( !process->isRunning() ) {
#if defined(QT_QPROCESS_DEBUG)
		qDebug( "QProcessManager::sigchldHnd(): process exited (QProcess available)" );
#endif
		// read pending data
		process->socketRead( process->d->socketStdout[0] );
		process->socketRead( process->d->socketStderr[0] );

		if ( process->notifyOnExit )
		    emit process->processExited();

		removeProc = TRUE;
	    }
	} else {
	    int status;
	    if ( ::waitpid( proc->pid, &status, WNOHANG ) == proc->pid ) {
#if defined(QT_QPROCESS_DEBUG)
		qDebug( "QProcessManager::sigchldHnd(): process exited (QProcess not available)" );
#endif
		removeProc = TRUE;
	    }
	}
	if ( removeProc ) {
	    QProc *oldproc = proc;
	    proc = procList->next();
	    remove( oldproc );
	} else {
	    proc = procList->next();
	}
    }
}

#include "qprocess_unix.moc"


/***********************************************************************
 *
 * QProcessPrivate
 *
 **********************************************************************/
QProcessManager *QProcessPrivate::procManager = 0;

QProcessPrivate::QProcessPrivate()
{
#if defined(QT_QPROCESS_DEBUG)
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

    proc = 0;
}

QProcessPrivate::~QProcessPrivate()
{
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcessPrivate: Destructor" );
#endif

    if ( proc != 0 )
	proc->process = 0;
#if 0
    // ### where should I destroy the procManager?
    if ( procManager != 0 ) {
	if ( procManager->remove( d ) ) {
	    delete procManager;
	    procManager = 0;
	}
    }
#endif

    while ( !stdinBuf.isEmpty() ) {
	delete stdinBuf.dequeue();
    }
    if ( notifierStdin ) {
	delete notifierStdin;
    }
    if ( notifierStdout ) {
	delete notifierStdout;
    }
    if ( notifierStderr ) {
	delete notifierStderr;
    }
    if( socketStdin[1] != 0 )
	::close( socketStdin[1] );
    if( socketStdout[0] != 0 )
	::close( socketStdout[0] );
    if( socketStderr[0] != 0 )
	::close( socketStderr[0] );
}

/*
  Closes all open sockets in the child process that are not needed by the child
  process. Otherwise one child may have an open socket on standard input, etc.
  of another child.
*/
void QProcessPrivate::closeOpenSocketsForChild()
{
    ::close( socketStdin[1] );
    ::close( socketStdout[0] );
    ::close( socketStderr[0] );

    if ( procManager != 0 ) {
	if ( procManager->sigchldFd[0] != 0 )
	    ::close( procManager->sigchldFd[0] );
	if ( procManager->sigchldFd[1] != 0 )
	    ::close( procManager->sigchldFd[1] );

	// delete also the sockets from other QProcess instances
	QProc *proc;
	QProcess *process;
	for ( proc=procManager->procList->first(); proc!=0; proc=procManager->procList->next() ) {
	    process = proc->process;
	    if ( process != 0 ) {
		::close( process->d->socketStdin[1] );
		::close( process->d->socketStdout[0] );
		::close( process->d->socketStderr[0] );
	    }
	}
    }
}

void QProcessPrivate::newProc( pid_t pid, QProcess *process )
{
    proc = new QProc( pid, process );
    if ( procManager == 0 ) {
	procManager = new QProcessManager;
    }
    // the QProcessManager takes care of deleting the QProc instances
    procManager->append( proc );
}

/***********************************************************************
 *
 * sigchld handler callback
 *
 **********************************************************************/
#if defined(SIGNAL_HACK)
void qt_C_sigchldHnd()
#else
void qt_C_sigchldHnd( int )
#endif
{
    if ( QProcessPrivate::procManager == 0 )
	return;
    if ( QProcessPrivate::procManager->sigchldFd[0] == 0 )
	return;

    char a = 1;
    ::write( QProcessPrivate::procManager->sigchldFd[0], &a, sizeof(a) );
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
    d = new QProcessPrivate();
    exitStat = 0;
    exitNormal = FALSE;
}

/*!
  Reset the process variables, etc. so that it can be used for another process
  to start.
*/
void QProcess::reset()
{
    delete d;
    d = new QProcessPrivate();
    exitStat = 0;
    exitNormal = FALSE;
    bufStdout.resize( 0 );
    bufStderr.resize( 0 );
}


/*!
  Destructs the class.

  If the process is running, it is NOT terminated! Standard input, standard
  output and standard error of the process are closed.

  \sa hangUp() kill()
*/
QProcess::~QProcess()
{
    delete d;
}

/*!
  Tries to run a process for the command and arguments that were specified with
  setArguments(), addArgument() or that were specified in the constructor. The
  command is searched in the path for executable programs; you can also use an
  absolute path to the command.

  Returns TRUE if the process could be started, otherwise FALSE.

  You can write data to standard input of the process with
  writeToStdin(), you can close standard input with closeStdin() and you can
  terminate the process hangUp() resp. kill().

  You can call this function when a process that was started with this instance
  still runs. In this case, it closes standard input of that process and it
  deletes pending data - you loose all control over that process, but the
  process is not terminated.

  \sa launch() closeStdin()
*/
bool QProcess::start()
{
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcess::start()" );
#endif
    reset();

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

    // the following pipe is only used to determine if the process could be
    // started
    int fd[2];
    if ( pipe( fd ) < 0 ) {
	// non critical error, go on
	fd[0] = 0;
	fd[1] = 0;
    }

    // construct the arguments for exec
    QCString *arglistQ = new QCString[ arguments.count() + 1 ];
    const char** arglist = new const char*[ arguments.count() + 1 ];
    int i = 0;
    for ( QStringList::Iterator it = arguments.begin(); it != arguments.end(); ++it ) {
	arglistQ[i] = (*it).local8Bit();
	arglist[i] = arglistQ[i];
#if defined(QT_QPROCESS_DEBUG)
	qDebug( "QProcess::start(): arg %d = %s", i, arglist[i] );
#endif
	i++;
    }
    arglist[i] = 0;

    // fork and exec
    QApplication::flushX();
    pid_t pid = fork();
    if ( pid == 0 ) {
	// child
	d->closeOpenSocketsForChild();
	::dup2( d->socketStdin[0], STDIN_FILENO );
	::dup2( d->socketStdout[1], STDOUT_FILENO );
	::dup2( d->socketStderr[1], STDERR_FILENO );
	::chdir( workingDir.absPath().latin1() );
	if ( fd[0] )
	    ::close( fd[0] );
	if ( fd[1] )
	    ::fcntl( fd[1], F_SETFD, FD_CLOEXEC ); // close on exec shows sucess
	::execvp( arglist[0], (char*const*)arglist ); // ### cast not nice
	if ( fd[1] ) {
	    char buf = 0;
	    ::write( fd[1], &buf, 1 );
	    ::close( fd[1] );
	}
	::exit( -1 );
    } else if ( pid == -1 ) {
	// error forking
	goto error;
    }
    // test if exec was successful
    if ( fd[1] )
	close( fd[1] );
    if ( fd[0] ) {
	char buf;
	while ( TRUE ) {
	    int n = ::read( fd[0], &buf, 1 );
	    if ( n==1 ) {
		// socket was not closed => error
		goto error;
	    } else if ( n==-1 ) {
		if ( errno==EAGAIN || errno==EINTR )
		    // try it again
		    continue;
	    }
	    break;
	}
    }

    d->newProc( pid, this );

    ::close( d->socketStdin[0] );
    ::close( d->socketStdout[1] );
    ::close( d->socketStderr[1] );

    // setup notifiers for the sockets
    d->notifierStdin = new QSocketNotifier( d->socketStdin[1],
	    QSocketNotifier::Write );
    d->notifierStdout = new QSocketNotifier( d->socketStdout[0],
	    QSocketNotifier::Read );
    d->notifierStderr = new QSocketNotifier( d->socketStderr[0],
	    QSocketNotifier::Read );
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

error:
    ::close( d->socketStdin[1] );
    ::close( d->socketStdout[0] );
    ::close( d->socketStderr[0] );
    ::close( d->socketStdin[0] );
    ::close( d->socketStdout[1] );
    ::close( d->socketStderr[1] );
    d->socketStdin[0] = 0;
    d->socketStdin[1] = 0;
    d->socketStdout[0] = 0;
    d->socketStdout[1] = 0;
    d->socketStderr[0] = 0;
    d->socketStderr[1] = 0;
    ::close( fd[0] );
    ::close( fd[1] );
    delete[] arglistQ;
    delete[] arglist;
    return FALSE;
}


/*!
  Asks the process to terminate. Processes can ignore this wish. If you want to
  be sure that the process really terminates, you must use kill()
  instead.

  When the process really exited, the signal processExited() is emitted.

  \sa kill() processExited()
*/
void QProcess::hangUp()
{
    if ( d->proc != 0 )
	::kill( d->proc->pid, SIGHUP );
}

/*!
  Terminates the process. This is not a safe way to end a process since the
  process will not be able to do cleanup. hangUp() is the saver way to do it,
  but processes might ignore a hangUp().

  When the process really exited, the signal processExited() is emitted.

  \sa hangUp() processExited()
*/
void QProcess::kill()
{
    if ( d->proc != 0 )
	::kill( d->proc->pid, SIGKILL );
}

/*!
  Returns TRUE if the process is running, otherwise FALSE.

  \sa normalExit() exitStatus() processExited()
*/
bool QProcess::isRunning()
{
    if ( d->exitValuesCalculated ) {
#if defined(QT_QPROCESS_DEBUG)
	qDebug( "QProcess::isRunning(): FALSE (already computed)" );
#endif
	return FALSE;
    }
    if ( d->proc == 0 )
	return FALSE;
    int status;
    if ( ::waitpid( d->proc->pid, &status, WNOHANG ) == d->proc->pid )
    {
	// compute the exit values
	exitNormal = WIFEXITED( status ) != 0;
	if ( exitNormal ) {
	    exitStat = WEXITSTATUS( status );
	}
	d->exitValuesCalculated = TRUE;
#if defined(QT_QPROCESS_DEBUG)
	qDebug( "QProcess::isRunning(): FALSE" );
#endif
	return FALSE;
    }
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcess::isRunning(): TRUE" );
#endif
    return TRUE;
}

/*!
  Writes the data \a buf to the standard input of the process. The process may
  or may not read this data. If the data was written to the process, the signal
  wroteStdin() is emitted. This does not mean that the process really read the
  data.

  \sa wroteStdin() closeStdin() readStdout() readStderr()
*/
void QProcess::writeToStdin( const QByteArray& buf )
{
#if defined(QT_QPROCESS_DEBUG)
//    qDebug( "QProcess::writeToStdin(): write to stdin (%d)", d->socketStdin[1] );
#endif
    d->stdinBuf.enqueue( new QByteArray(buf) );
    if ( d->notifierStdin != 0 )
        d->notifierStdin->setEnabled( TRUE );
}


/*!
  Closes standard input of the process.

  This function also deletes pending data that is not written to standard input
  yet.
*/
void QProcess::closeStdin()
{
    if ( d->socketStdin[1] !=0 ) {
	while ( !d->stdinBuf.isEmpty() ) {
	    delete d->stdinBuf.dequeue();
	}
	if ( d->notifierStdin ) {
	    delete d->notifierStdin;
	    d->notifierStdin = 0;
	}
	if ( ::close( d->socketStdin[1] ) != 0 ) {
	    qWarning( "Could not close stdin of child process" );
	}
#if defined(QT_QPROCESS_DEBUG)
	qDebug( "QProcess::closeStdin(): stdin (%d) closed", d->socketStdin[1] );
#endif
	d->socketStdin[1] = 0;
    }
}


/*
  The process has outputted data to either standard output or standard error.
*/
void QProcess::socketRead( int fd )
{
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcess::socketRead(): %d", fd );
#endif
    if ( fd == 0 )
	return;
    const int bufsize = 4096;
    QByteArray buffer;
    uint oldSize;
    int n;
    if ( fd == d->socketStdout[0] ) {
	buffer = bufStdout;
    } else {
	buffer = bufStderr;
    }

    // read data
    oldSize = buffer.size();
    buffer.resize( oldSize + 4096 );
    n = ::read( fd, buffer.data()+oldSize, bufsize );
    if ( n > 0 )
	buffer.resize( oldSize + n );
    // eof or error?
    if ( n == 0 || n == -1 ) {
	if ( fd == d->socketStdout[0] ) {
#if defined(QT_QPROCESS_DEBUG)
	    qDebug( "QProcess::socketRead(): stdout (%d) closed", fd );
#endif
	    d->notifierStdout->setEnabled( FALSE );
	    delete d->notifierStdout;
	    d->notifierStdout = 0;
	    ::close( d->socketStdout[0] );
	    d->socketStdout[0] = 0;
	    return;
	} else {
#if defined(QT_QPROCESS_DEBUG)
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
    // read all data that is available
    while ( n == bufsize ) {
	oldSize = buffer.size();
	buffer.resize( oldSize + 4096 );
	n = ::read( fd, buffer.data()+oldSize, bufsize );
	if ( n > 0 )
	    buffer.resize( oldSize + n );
    }

    if ( fd == d->socketStdout[0] ) {
#if defined(QT_QPROCESS_DEBUG)
	qDebug( "QProcess::socketRead(): %d bytes read from stdout (%d)",
		buffer.size()-oldSize, fd );
#endif
	emit readyReadStdout();
    } else {
#if defined(QT_QPROCESS_DEBUG)
	qDebug( "QProcess::socketRead(): %d bytes read from stderr (%d)",
		buffer.size()-oldSize, fd );
#endif
	emit readyReadStderr();
    }
}


/*
  The process tries to read data from standard input.
*/
void QProcess::socketWrite( int fd )
{
    if ( fd != d->socketStdin[1] || d->socketStdin[1] == 0 )
	return;
    if ( d->stdinBuf.isEmpty() ) {
	d->notifierStdin->setEnabled( FALSE );
	return;
    }
#if defined(QT_QPROCESS_DEBUG)
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

/*
  Only used under Windows (but moc does not know about #if defined()).
*/
void QProcess::timeout()
{
}


/*
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

/*
  Used by connectNotify() and disconnectNotify() to change the value of
  notifyOnExit (and related behaviour)
*/
void QProcess::setNotifyOnExit( bool value )
{
    notifyOnExit = value;
}

/*
  Used by connectNotify() and disconnectNotify() to change the value of
  wroteStdinConnected (and related behaviour)
*/
void QProcess::setWroteStdinConnected( bool value )
{
    wroteStdinConnected = value;
}

#endif // QT_NO_PROCESS
