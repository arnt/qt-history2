/****************************************************************************
**
** Implementation of QProcess class for Win32.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#include "qprocess.h"

#ifndef QT_NO_PROCESS

#include "qlist.h"
#include "qtimer.h"
#include "qregexp.h"
#include "private/qinternal_p.h"
#include "qt_windows.h"

//#define QT_QPROCESS_DEBUG

/***********************************************************************
 *
 * QProcessPrivate
 *
 **********************************************************************/
class QProcessPrivate
{
public:
    QProcessPrivate( QProcess *proc )
    {
	stdinBufRead = 0;
	pipeStdin[0] = 0;
	pipeStdin[1] = 0;
	pipeStdout[0] = 0;
	pipeStdout[1] = 0;
	pipeStderr[0] = 0;
	pipeStderr[1] = 0;
	exitValuesCalculated = FALSE;

	lookup = new QTimer( proc );
	QObject::connect(lookup, SIGNAL(timeout()),
			 proc, SLOT(timeout()) );

	pid = 0;
    }

    ~QProcessPrivate()
    {
	reset();
    }

    void reset()
    {
	while ( !stdinBuf.isEmpty() ) {
	    delete (stdinBuf.isEmpty() ? 0 : stdinBuf.takeAt(0));
	}
	closeHandles();
	stdinBufRead = 0;
	pipeStdin[0] = 0;
	pipeStdin[1] = 0;
	pipeStdout[0] = 0;
	pipeStdout[1] = 0;
	pipeStderr[0] = 0;
	pipeStderr[1] = 0;
	exitValuesCalculated = FALSE;

	deletePid();
    }

    void closeHandles()
    {
	if( pipeStdin[1] != 0 ) {
	    CloseHandle( pipeStdin[1] );
	    pipeStdin[1] = 0;
	}
	if( pipeStdout[0] != 0 ) {
	    CloseHandle( pipeStdout[0] );
	    pipeStdout[0] = 0;
	}
	if( pipeStderr[0] != 0 ) {
	    CloseHandle( pipeStderr[0] );
	    pipeStderr[0] = 0;
	}
    }

    void deletePid()
    {
	if ( pid ) {
	    CloseHandle( pid->hProcess );
	    CloseHandle( pid->hThread );
	    delete pid;
	    pid = 0;
	}
    }

    void newPid()
    {
	deletePid();
	pid = new PROCESS_INFORMATION;
	memset( pid, 0, sizeof(PROCESS_INFORMATION) );
    }

    QMembuf bufStdout;
    QMembuf bufStderr;

    QList<QByteArray*> stdinBuf;

    HANDLE pipeStdin[2];
    HANDLE pipeStdout[2];
    HANDLE pipeStderr[2];
    QTimer *lookup;

    PROCESS_INFORMATION *pid;
    uint stdinBufRead;

    bool exitValuesCalculated;
};


/***********************************************************************
 *
 * QProcess
 *
 **********************************************************************/
void QProcess::init()
{
    d = new QProcessPrivate( this );
    exitStat = 0;
    exitNormal = FALSE;
}

void QProcess::reset()
{
    d->reset();
    exitStat = 0;
    exitNormal = FALSE;
    d->bufStdout.clear();
    d->bufStderr.clear();
}

QMembuf* QProcess::membufStdout()
{
    if( d->pipeStdout[0] != 0 )
	socketRead( 1 );
    return &d->bufStdout;
}

QMembuf* QProcess::membufStderr()
{
    if( d->pipeStderr[0] != 0 )
	socketRead( 2 );
    return &d->bufStderr;
}

QProcess::~QProcess()
{
    delete d;
}

bool QProcess::start( QStringList *env )
{
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcess::start()" );
#endif
    reset();

    if ( _arguments.isEmpty() )
	return FALSE;

    // Open the pipes.  Make non-inheritable copies of input write and output
    // read handles to avoid non-closable handles (this is done by the
    // DuplicateHandle() call).
    SECURITY_ATTRIBUTES secAtt = { sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE };
#ifndef Q_OS_TEMP
    // I guess there is no stdin stdout and stderr on Q_OS_TEMP to dup
    // CreatePipe and DupilcateHandle aren't avaliable for Q_OS_TEMP
    HANDLE tmpStdin, tmpStdout, tmpStderr;
    if ( comms & Stdin ) {
	if ( !CreatePipe( &d->pipeStdin[0], &tmpStdin, &secAtt, 0 ) ) {
	    d->closeHandles();
	    return FALSE;
	}
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStdin, GetCurrentProcess(), &d->pipeStdin[1], 0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	    d->closeHandles();
	    return FALSE;
	}
	if ( !CloseHandle( tmpStdin ) ) {
	    d->closeHandles();
	    return FALSE;
	}
    }
    if ( comms & Stdout ) {
	if ( !CreatePipe( &tmpStdout, &d->pipeStdout[1], &secAtt, 0 ) ) {
	    d->closeHandles();
	    return FALSE;
	}
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStdout, GetCurrentProcess(), &d->pipeStdout[0], 0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	    d->closeHandles();
	    return FALSE;
	}
	if ( !CloseHandle( tmpStdout ) ) {
	    d->closeHandles();
	    return FALSE;
	}
    }
    if ( comms & Stderr ) {
	if ( !CreatePipe( &tmpStderr, &d->pipeStderr[1], &secAtt, 0 ) ) {
	    d->closeHandles();
	    return FALSE;
	}
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStderr, GetCurrentProcess(), &d->pipeStderr[0], 0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	    d->closeHandles();
	    return FALSE;
	}
	if ( !CloseHandle( tmpStderr ) ) {
	    d->closeHandles();
	    return FALSE;
	}
    }
    if ( comms & DupStderr ) {
	CloseHandle( d->pipeStderr[1] );
	d->pipeStderr[1] = d->pipeStdout[1];
    }
#endif

    // construct the arguments for CreateProcess()
    QString args;
    QString appName = QString::null;
    QStringList::Iterator it = _arguments.begin();
    args = *it;
    ++it;
    if ( args.endsWith( ".bat" ) && args.contains( ' ' ) ) {
	// CreateProcess() seems to have a strange semantics (see also
	// http://www.experts-exchange.com/Programming/Programming_Platforms/Win_Prog/Q_11138647.html):
	// If you start a batch file with spaces in the filename, the first
	// argument to CreateProcess() must be the name of the batchfile
	// without quotes, but the second argument must start with the same
	// argument with quotes included. But if the same approach is used for
	// .exe files, it doesn't work.
	appName = args;
	args = '"' + args + '"';
    }
    for ( ; it != _arguments.end(); ++it ) {
	QString tmp = *it;
	// escape a single " because the arguments will be parsed
	tmp.replace( "\"", "\\\"" );
	if ( tmp.isEmpty() || tmp.contains( ' ' ) || tmp.contains( '\t' ) ) {
	    // The argument must not end with a \ since this would be interpreted
	    // as escaping the quote -- rather put the \ behind the quote: e.g.
	    // rather use "foo"\ than "foo\"
	    QString endQuote( "\"" );
	    uint i = tmp.length();
	    while ( i>=0 && tmp.at( i-1 ) == '\\' ) {
		--i;
		endQuote += "\\";
	    }
	    args += QString( " \"" ) + tmp.left( i ) + endQuote;
	} else {
	    args += ' ' + tmp;
	}
    }
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcess::start(): args [%s]", args.latin1() );
#endif

    // CreateProcess()
    bool success;
    d->newPid();
#ifdef UNICODE
    if( qt_winUnicode() ) {
	STARTUPINFOW startupInfo = {
	    sizeof( STARTUPINFO ), 0, 0, 0,
	    (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1]
	};
	TCHAR *applicationName;
	if ( appName.isNull() )
	    applicationName = 0;
	else
	    applicationName = _wcsdup( (TCHAR*)appName.ucs2() );
	TCHAR *commandLine = _wcsdup( (TCHAR*)args.ucs2() );
	QByteArray envlist;
	if ( env != 0 ) {
	    int pos = 0;
	    // add PATH if necessary (for DLL loading)
	    char *path = getenv( "PATH" );
	    if ( env->find(QRegExp("^PATH=",QString::CaseInsensitive)).isEmpty() && path ) {
		QString tmp = QString( "PATH=%1" ).arg( getenv( "PATH" ) );
		uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, tmp.ucs2(), tmpSize );
		pos += tmpSize;
	    }
	    // add the user environment
	    for ( QStringList::Iterator it = env->begin(); it != env->end(); it++ ) {
		QString tmp = *it;
		uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, tmp.ucs2(), tmpSize );
		pos += tmpSize;
	    }
	    // add the 2 terminating 0 (actually 4, just to be on the safe side)
	    envlist.resize( envlist.size()+4 );
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	}
	success = CreateProcessW( applicationName, commandLine,
		0, 0, TRUE, ( comms==0 ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW )
#ifndef Q_OS_TEMP
		| CREATE_UNICODE_ENVIRONMENT
#endif
		, env==0 ? 0 : envlist.data(),
		(TCHAR*)workingDir.absPath().ucs2(),
		&startupInfo, d->pid );
	free( applicationName );
	free( commandLine );
    } else
#endif // UNICODE
    {
#ifndef Q_OS_TEMP
	STARTUPINFOA startupInfo = { sizeof( STARTUPINFOA ), 0, 0, 0,
	    (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1]
	};
	QByteArray envlist;
	if ( env != 0 ) {
	    int pos = 0;
	    // add PATH if necessary (for DLL loading)
	    char *path = getenv( "PATH" );
	    if ( env->find(QRegExp("^PATH=",QString::CaseInsensitive)).isEmpty() && path ) {
		QByteArray tmp = QString( "PATH=%1" ).arg( getenv( "PATH" ) ).toLocal8Bit();
		uint tmpSize = tmp.length() + 1;
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, tmp.constData(), tmpSize );
		pos += tmpSize;
	    }
	    // add the user environment
	    for ( QStringList::Iterator it = env->begin(); it != env->end(); it++ ) {
		QByteArray tmp = (*it).toLocal8Bit();
		uint tmpSize = tmp.length() + 1;
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, tmp.constData(), tmpSize );
		pos += tmpSize;
	    }
	    // add the terminating 0 (actually 2, just to be on the safe side)
	    envlist.resize( envlist.size()+2 );
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	}
	char *applicationName;
	if ( appName.isNull() )
	    applicationName = 0;
	else
	    applicationName = appName.toLocal8Bit().data();
	success = CreateProcessA( applicationName, args.toLocal8Bit().data(),
		0, 0, TRUE, comms==0 ? CREATE_NEW_CONSOLE : DETACHED_PROCESS,
		env==0 ? 0 : envlist.data(),
		workingDir.absPath().local8Bit(),
		&startupInfo, d->pid );
#endif // Q_OS_TEMP
    }
    if  ( !success ) {
	d->deletePid();
	return FALSE;
    }

#ifndef Q_OS_TEMP
    if ( comms & Stdin )
	CloseHandle( d->pipeStdin[0] );
    if ( comms & Stdout )
        CloseHandle( d->pipeStdout[1] );
    if ( (comms & Stderr) && !(comms & DupStderr) )
	CloseHandle( d->pipeStderr[1] );
#endif

    if ( ioRedirection || notifyOnExit ) {
	d->lookup->start( 100 );
    }

    // cleanup and return
    return TRUE;
}

static BOOL CALLBACK qt_terminateApp( HWND hwnd, LPARAM procId )
{
    DWORD procId_win;
    GetWindowThreadProcessId( hwnd, &procId_win );
    if( procId_win == (DWORD)procId )
	PostMessage( hwnd, WM_CLOSE, 0, 0 );

    return TRUE;
}

void QProcess::tryTerminate() const
{
    if ( d->pid )
	EnumWindows( qt_terminateApp, (LPARAM)d->pid->dwProcessId );
}

void QProcess::kill() const
{
    if ( d->pid )
	TerminateProcess( d->pid->hProcess, 0xf291 );
}

bool QProcess::isRunning() const
{
    if ( !d->pid )
	return FALSE;

    if ( WaitForSingleObject( d->pid->hProcess, 0) == WAIT_OBJECT_0 ) {
	// there might be data to read
	QProcess *that = (QProcess*)this;
	that->socketRead( 1 ); // try stdout
	that->socketRead( 2 ); // try stderr
	// compute the exit values
	if ( !d->exitValuesCalculated ) {
	    DWORD exitCode;
	    if ( GetExitCodeProcess( d->pid->hProcess, &exitCode ) ) {
		if ( exitCode != STILL_ACTIVE ) { // this should ever be true?
		    that->exitNormal = exitCode != 0xf291;
		    that->exitStat = exitCode;
		}
	    }
	    d->exitValuesCalculated = TRUE;
	}
	d->deletePid();
	d->closeHandles();
	return FALSE;
    } else {
        return TRUE;
    }
}

bool QProcess::canReadLineStdout() const
{
    if( !d->pipeStdout[0] )
	return d->bufStdout.size() != 0;

    QProcess *that = (QProcess*)this;
    return that->membufStdout()->scanNewline( 0 );
}

bool QProcess::canReadLineStderr() const
{
    if( !d->pipeStderr[0] )
	return d->bufStderr.size() != 0;

    QProcess *that = (QProcess*)this;
    return that->membufStderr()->scanNewline( 0 );
}

void QProcess::writeToStdin( const QByteArray& buf )
{
    d->stdinBuf.append( new QByteArray(buf) );
    socketWrite( 0 );
}

void QProcess::closeStdin( )
{
    if ( d->pipeStdin[1] != 0 ) {
	CloseHandle( d->pipeStdin[1] );
	d->pipeStdin[1] = 0;
    }
}

void QProcess::socketRead( int fd )
{
    // fd == 1: stdout, fd == 2: stderr
    HANDLE dev;
    if ( fd == 1 ) {
	dev = d->pipeStdout[0];
    } else if ( fd == 2 ) {
	dev = d->pipeStderr[0];
    } else {
	return;
    }
#ifndef Q_OS_TEMP
    // get the number of bytes that are waiting to be read
    unsigned long i, r;
    char dummy;
    if ( !PeekNamedPipe( dev, &dummy, 1, &r, &i, 0 ) ) {
	return; // ### is it worth to dig for the reason of the error?
    }
#else
    unsigned long i = 1000;
#endif
    if ( i > 0 ) {
	QMembuf *buffer;
	if ( fd == 1 )
	    buffer = &d->bufStdout;
	else
	    buffer = &d->bufStderr;

	QByteArray *ba = new QByteArray();
	ba->resize(i);
	uint sz = readStddev( dev, ba->data(), i );
	if ( sz != i )
	    ba->resize( i );

	if ( sz == 0 ) {
	    delete ba;
	    return;
	}
	buffer->append( ba );
	if ( fd == 1 )
	    emit readyReadStdout();
	else
	    emit readyReadStderr();
    }
}

void QProcess::socketWrite( int )
{
    DWORD written;
    while ( !d->stdinBuf.isEmpty() && isRunning() ) {
	if ( !WriteFile( d->pipeStdin[1],
		    d->stdinBuf.first()->data() + d->stdinBufRead,
		    qMin( (uint)8192, d->stdinBuf.first()->size() - d->stdinBufRead ),
		    &written, 0 ) ) {
	    d->lookup->start( 100 );
	    return;
	}
	d->stdinBufRead += written;
	if ( d->stdinBufRead == d->stdinBuf.first()->size() ) {
	    d->stdinBufRead = 0;
	    delete (d->stdinBuf.isEmpty() ? 0 : d->stdinBuf.takeAt(0));
	    if ( wroteToStdinConnected && d->stdinBuf.isEmpty() )
		emit wroteToStdin();
	}
    }
}

void QProcess::flushStdin()
{
    socketWrite( 0 );
}

/*
  Use a timer for polling misc. stuff.
*/
void QProcess::timeout()
{
    // Disable the timer temporary since one of the slots that are connected to
    // the readyRead...(), etc. signals might trigger recursion if
    // processEvents() is called.
    d->lookup->stop();

    // try to write pending data to stdin
    if ( !d->stdinBuf.isEmpty() )
	socketWrite( 0 );

    if ( ioRedirection ) {
	socketRead( 1 ); // try stdout
	socketRead( 2 ); // try stderr
    }

    if ( isRunning() ) {
	// enable timer again, if needed
	if ( !d->stdinBuf.isEmpty() || ioRedirection || notifyOnExit )
	    d->lookup->start( 100 );
    } else if ( notifyOnExit ) {
	emit processExited();
    }
}

/*
  read on the pipe
*/
uint QProcess::readStddev( HANDLE dev, char *buf, uint bytes )
{
    if ( bytes > 0 ) {
	ulong r;
	if ( ReadFile( dev, buf, bytes, &r, 0 ) )
	    return r;
    }
    return 0;
}

/*
  Used by connectNotify() and disconnectNotify() to change the value of
  ioRedirection (and related behaviour)
*/
void QProcess::setIoRedirection( bool value )
{
    ioRedirection = value;
    if ( !ioRedirection && !notifyOnExit )
	d->lookup->stop();
    if ( ioRedirection ) {
	if ( isRunning() )
	    d->lookup->start( 100 );
    }
}

/*
  Used by connectNotify() and disconnectNotify() to change the value of
  notifyOnExit (and related behaviour)
*/
void QProcess::setNotifyOnExit( bool value )
{
    notifyOnExit = value;
    if ( !ioRedirection && !notifyOnExit )
	d->lookup->stop();
    if ( notifyOnExit ) {
	if ( isRunning() )
	    d->lookup->start( 100 );
    }
}

/*
  Used by connectNotify() and disconnectNotify() to change the value of
  wroteToStdinConnected (and related behaviour)
*/
void QProcess::setWroteStdinConnected( bool value )
{
    wroteToStdinConnected = value;
}

QProcess::PID QProcess::processIdentifier()
{
    return d->pid;
}

#endif // QT_NO_PROCESS
