/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qprocess.h>

#include <stdio.h>
#include <stdlib.h>

#include <qtimer.h>
#include <qapplication.h>

extern Qt::WindowsVersion qt_winver;

QProcessPrivate::QProcessPrivate( QProcess *proc )
{
    stdinBufRead = 0;
    pipeStdin[0] = 0;
    pipeStdin[1] = 0;
    pipeStdout[0] = 0;
    pipeStdout[1] = 0;
    pipeStderr[0] = 0;
    pipeStderr[1] = 0;

    lookup = new QTimer( proc );
    qApp->connect( lookup, SIGNAL(timeout()),
	    proc, SLOT(timeout()) );

    exitValuesCalculated = FALSE;
    exitStat = 0;
    exitNormal = FALSE;
}

QProcessPrivate::~QProcessPrivate()
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


bool QProcess::start()
{
    // open the pipes
    // make non-inheritable copies of input write and output read handles
    // to avoid non-closable handles
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
    args = d->command.latin1();
    for ( QStringList::Iterator it = d->arguments.begin(); it != d->arguments.end(); ++it ) {
	args += QString( " \'" ) + (*it).latin1() + QString( "\'" );
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
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1] };
	TCHAR *commandLine = (TCHAR*)qt_winTchar_new( args );
	success = CreateProcess( 0, commandLine,
		0, 0, TRUE, 0, 0,
		(TCHAR*)qt_winTchar(d->workingDir.absPath(),TRUE),
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
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1] };
	success = CreateProcessA( 0, args.local8Bit().data(),
		0, 0, TRUE, 0, 0,
		(const char*)d->workingDir.absPath().local8Bit(),
		&startupInfo, &d->pid );
    }
    if  ( !success ) {
	return FALSE;
    }

    CloseHandle( d->pipeStdin[0] );
    CloseHandle( d->pipeStdout[1] );
    CloseHandle( d->pipeStderr[1] );

    // start the timer
    d->lookup->start( 100 );

    // cleanup and return
    return TRUE;
}

bool QProcess::hangUp()
{
    // ### how to do it?
    return FALSE;
}

bool QProcess::kill()
{
    return ( TerminateProcess( d->pid.hProcess, 0 ) != 0 );
}

bool QProcess::isRunning()
{
    if ( WaitForSingleObject( d->pid.hProcess, 0) == WAIT_OBJECT_0 ) {
	// compute the exit values
	if ( !d->exitValuesCalculated ) {
	    DWORD exitCode;
	    if ( GetExitCodeProcess( d->pid.hProcess, &exitCode ) ) {
		if ( exitCode != STILL_ACTIVE ) { // this should ever be true?
		    d->exitNormal = TRUE;
		    d->exitStat = exitCode;
		}
	    }
	    d->exitValuesCalculated = TRUE;
	}
	return FALSE;
    } else {
        return TRUE;
    }
}

void QProcess::dataStdin( const QByteArray& buf )
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
    PeekNamedPipe( dev, &dummy, 1, &r, &i, 0 );
    if ( i > 0 ) {
	QByteArray buffer=readStddev( dev, i );
	int sz = buffer.size();
	if ( sz == 0 )
	    return;
	if ( fd == 1 ) {
	    emit dataStdout( buffer );
	} else {
	    emit dataStderr( buffer );
	}
	buffer.resize( sz+1 );
	buffer[ sz ] = 0;
	QString str( buffer );
	if ( fd == 1 ) {
	    emit dataStdout( str );
	} else {
	    emit dataStderr( str );
	}
    }
}

void QProcess::socketWrite( int fd )
{
    DWORD written;
    if ( d->stdinBuf.isEmpty() ) {
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
	socketWrite( fd );
    }
}

void QProcess::timeout()
{
//    socketWrite( 0 );
    socketRead( 1 ); // try stdout
    socketRead( 2 ); // try stderr

    // is process running?
    if ( !isRunning() ) {
	// isRunning() gets the exit values
	d->lookup->stop();
	emit processExited();
    }
}

// non-blocking read on the pipe
QByteArray QProcess::readStddev( HANDLE dev, ulong bytes )
{
    unsigned long i, r;
    if ( bytes == 0 ) {
	// get the number of bytes that are waiting to be read
	char dummy;
	if ( !PeekNamedPipe( dev, &dummy, 1, &r, &i, 0 ) ) {
	    i = 0;
	}
    } else {
	i = bytes;
    }
    // and read it!
    QByteArray readBuffer( i );
    if ( i > 0 ) {
	ReadFile( dev, readBuffer.data(), i, &r, 0 );
	if ( r != i ) {
	    readBuffer.resize( r );
	}
    }
    return readBuffer;
}
