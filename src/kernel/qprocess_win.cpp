/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprocess_win.cpp#26 $
**
** Implementation of QProcess class for Win32
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
#include "qtimer.h"

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>


extern Qt::WindowsVersion qt_winver;


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
	qApp->connect( lookup, SIGNAL(timeout()),
		proc, SLOT(timeout()) );
    }

    ~QProcessPrivate()
    {
	cleanup();
    }

    void cleanup()
    {
	while ( !stdinBuf.isEmpty() ) {
	    delete stdinBuf.dequeue();
	}
	if( pipeStdin[1] != 0 )
	    CloseHandle( pipeStdin[1] );
	if( pipeStdout[0] != 0 )
	    CloseHandle( pipeStdout[0] );
	if( pipeStderr[0] != 0 )
	    CloseHandle( pipeStderr[0] );
    }

    void reset()
    {
	cleanup();
	stdinBufRead = 0;
	pipeStdin[0] = 0;
	pipeStdin[1] = 0;
	pipeStdout[0] = 0;
	pipeStdout[1] = 0;
	pipeStderr[0] = 0;
	pipeStderr[1] = 0;
	exitValuesCalculated = FALSE;
    }

    QQueue<QByteArray> stdinBuf;

    HANDLE pipeStdin[2];
    HANDLE pipeStdout[2];
    HANDLE pipeStderr[2];
    QTimer *lookup;

    PROCESS_INFORMATION pid;
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
    bufStdout.resize( 0 );
    bufStderr.resize( 0 );
}


QProcess::~QProcess()
{
    delete d;
}

bool QProcess::start()
{
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcess::start()" );
#endif
    reset();

    // Open the pipes.  Make non-inheritable copies of input write and output
    // read handles to avoid non-closable handles.
    SECURITY_ATTRIBUTES secAtt = { sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE };
    HANDLE tmpStdin, tmpStdout, tmpStderr;
    if ( !CreatePipe( &d->pipeStdin[0], &tmpStdin, &secAtt, 0 ) ) {
	return FALSE;
    }
    if ( !CreatePipe( &tmpStdout, &d->pipeStdout[1], &secAtt, 0 ) ) {
	return FALSE;
    }
    if ( !CreatePipe( &tmpStderr, &d->pipeStderr[1], &secAtt, 0 ) ) {
	return FALSE;
    }
    if ( !DuplicateHandle( GetCurrentProcess(), tmpStdin,
		GetCurrentProcess(), &d->pipeStdin[1],
		0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	return FALSE;
    }
    if ( !DuplicateHandle( GetCurrentProcess(), tmpStdout,
		GetCurrentProcess(), &d->pipeStdout[0],
		0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	return FALSE;
    }
    if ( !DuplicateHandle( GetCurrentProcess(), tmpStderr,
		GetCurrentProcess(), &d->pipeStderr[0],
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
    QStringList::Iterator it = arguments.begin();
    args = *it;
    ++it;
    for ( ; it != arguments.end(); ++it ) {
	args += QString( " \"" ) + (*it) + QString( "\"" );
    }

    // CreateProcess()
    bool success;
#if defined(UNICODE)
    if ( qt_winver & WV_NT_based ) {
	STARTUPINFO startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
	    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1]
	};
	TCHAR *commandLine = (TCHAR*)qt_winTchar_new( args );
	success = CreateProcess( 0, commandLine,
		0, 0, TRUE, DETACHED_PROCESS, 0,
		(TCHAR*)qt_winTchar(workingDir.absPath(),TRUE),
		&startupInfo, &d->pid );
	delete[] commandLine;
    } else
#endif
    {
	STARTUPINFOA startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
	    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1]
	};
	success = CreateProcessA( 0, args.local8Bit().data(),
		0, 0, TRUE, DETACHED_PROCESS, 0,
		(const char*)workingDir.absPath().local8Bit(),
		&startupInfo, &d->pid );
    }
    if  ( !success ) {
	return FALSE;
    }

    CloseHandle( d->pipeStdin[0] );
    CloseHandle( d->pipeStdout[1] );
    CloseHandle( d->pipeStderr[1] );

    if ( ioRedirection || notifyOnExit ) {
	d->lookup->start( 100 );
    }

    // cleanup and return
    return TRUE;
}

void QProcess::hangUp() const
{
    // ### how to do it?
}

void QProcess::kill() const
{
    TerminateProcess( d->pid.hProcess, 0 );
}

bool QProcess::isRunning() const
{
    if ( WaitForSingleObject( d->pid.hProcess, 0) == WAIT_OBJECT_0 ) {
	// compute the exit values
	if ( !d->exitValuesCalculated ) {
	    DWORD exitCode;
	    if ( GetExitCodeProcess( d->pid.hProcess, &exitCode ) ) {
		if ( exitCode != STILL_ACTIVE ) { // this should ever be true?
		    QProcess *that = (QProcess*)this; // mutable 
		    that->exitNormal = TRUE;
		    that->exitStat = exitCode;
		}
	    }
	    d->exitValuesCalculated = TRUE;
	}
	return FALSE;
    } else {
        return TRUE;
    }
}

void QProcess::writeToStdin( const QByteArray& buf )
{
    d->stdinBuf.enqueue( new QByteArray(buf) );
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
    // get the number of bytes that are waiting to be read
    unsigned long i, r;
    char dummy;
    if ( !PeekNamedPipe( dev, &dummy, 1, &r, &i, 0 ) ) {
	return; // ### is it worth to dig for the reason of the error?
    }
    if ( i > 0 ) {
	QByteArray buffer;
	uint oldSize;
	if ( fd == 1 ) {
	    buffer = bufStdout;
	} else {
	    buffer = bufStderr;
	}

	oldSize = buffer.size();
	buffer.resize( oldSize + i );
	uint sz = readStddev( dev, buffer.data()+oldSize, i );
	if ( sz != i )
	    buffer.resize( oldSize + i );

	if ( sz == 0 )
	    return;
	if ( fd == 1 )
	    emit readyReadStdout();
	else
	    emit readyReadStderr();
    }
}

void QProcess::socketWrite( int fd )
{
    DWORD written;
    if ( d->stdinBuf.isEmpty() || !isRunning() ) {
	return;
    }
    if ( !WriteFile( d->pipeStdin[1],
		d->stdinBuf.head()->data() + d->stdinBufRead,
		d->stdinBuf.head()->size() - d->stdinBufRead,
		&written, 0 ) ) {
	return;
    }
    d->stdinBufRead += written;
    if ( d->stdinBufRead == d->stdinBuf.head()->size() ) {
	d->stdinBufRead = 0;
	delete d->stdinBuf.dequeue();
	if ( wroteToStdinConnected && d->stdinBuf.isEmpty() ) {
	    emit wroteToStdin();
	}
	socketWrite( fd );
	// start timer if there is still pending data
	if ( !d->stdinBuf.isEmpty() ) {
	    d->lookup->start( 100 );
	}
    }
}

/*
  Use a timer for polling misc. stuff.
*/
void QProcess::timeout()
{
    // try to write pending data to stdin
    if ( !d->stdinBuf.isEmpty() ) {
	socketWrite( 0 );
	// stop timer if it is not needed any longer
	if ( d->stdinBuf.isEmpty() && !ioRedirection && !notifyOnExit )
	    d->lookup->stop();
    }

    if ( ioRedirection ) {
	socketRead( 1 ); // try stdout
	socketRead( 2 ); // try stderr
    }

    // stop timer if process is not running also emit processExited() signal
    if ( !isRunning() ) {
	d->lookup->stop();
	if ( notifyOnExit ) {
	    emit processExited();
	}
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

#endif // QT_NO_PROCESS
