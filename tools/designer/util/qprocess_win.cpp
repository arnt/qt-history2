/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <qtimer.h>
#include <qapplication.h>

#include "qprocess.h"


QProcessPrivate::QProcessPrivate( QProcess *proc )
{
    stdinBufRead = 0;
#if defined ( RMS_USE_SOCKETS )
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	notifierStdin = 0;
	notifierStdout = 0;
	notifierStderr = 0;

	socketStdin[0] = 0;
	socketStdin[1] = 0;
	socketStdout[0] = 0;
	socketStdout[1] = 0;
	socketStderr[0] = 0;
	socketStderr[1] = 0;

	WSADATA wsaData;
	WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    } else
#endif
    {
	pipeStdin[0] = 0;
	pipeStdin[1] = 0;
	pipeStdout[0] = 0;
	pipeStdout[1] = 0;
	pipeStderr[0] = 0;
	pipeStderr[1] = 0;

	lookup = new QTimer( proc );
	qApp->connect( lookup, SIGNAL(timeout()),
		proc, SLOT(timeout()) );
    }

    exitValuesCalculated = FALSE;
    exitStat = 0;
    exitNormal = FALSE;
}

QProcessPrivate::~QProcessPrivate()
{
#if defined ( RMS_USE_SOCKETS )
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
    } else
#endif
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
}


#if defined ( RMS_USE_SOCKETS )
static bool socketpair( int type, int s[2] )
{
    // make non-overlapped sockets
    int optionValue = SO_SYNCHRONOUS_NONALERT; 
    setsockopt( INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE,
	(char*)&optionValue, sizeof(optionValue) ); 

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
}
#endif


bool QProcess::start()
{
#if defined ( RMS_USE_SOCKETS )
    if ( QApplication::winVersion() & Qt::WV_NT_based )
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
#if 0
	// ### test my socketpair function
	char grmpf[] = "Wet wet wet...";
	char hmpfl[10];
	DWORD written, read;
	//OVERLAPPED ov = { 0, 0, 0, 0, 0 };
	OVERLAPPED *ovp = 0;
	if ( !WriteFile( (HANDLE)(socketStdin[1]), grmpf, 5, &written, ovp ) ) {
	    LPVOID lpMsgBuf;
	    FormatMessage(     FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		    FORMAT_MESSAGE_FROM_SYSTEM |     FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
		    GetLastError(),
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    (LPTSTR) &lpMsgBuf,    0,    NULL );
	} else {
	    // try to read
	    ReadFile( (HANDLE)(socketStdin[0]), hmpfl, 10, &read, ovp );
	}
#endif

	// construct the arguments for CreateProcess()
	QString args;
	args = d->command.latin1();
	for ( QStringList::Iterator it = d->arguments.begin(); it != d->arguments.end(); ++it ) {
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

	if  ( !CreateProcess( 0, argsC, 0, 0, TRUE, 0, 0, d->workingDir.absPath().latin1(), &startupInfo, &d->pid ) ) {
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
    } else
#endif
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
	char *argsC = new char[args.length()+1];
	strcpy( argsC, args.latin1() );
	STARTUPINFO startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
	    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1] };

	if  ( !CreateProcess( 0, argsC, 0, 0, TRUE, 0, 0, d->workingDir.absPath().latin1(), &startupInfo, &d->pid ) ) {
	    delete[] argsC;
	    return FALSE;
	}
	delete[] argsC;
	CloseHandle( d->pipeStdin[0] );
	CloseHandle( d->pipeStdout[1] );
	CloseHandle( d->pipeStderr[1] );

	// start the timer
	d->lookup->start( 100 );

	// cleanup and return
	return TRUE;
    }
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
#if defined ( RMS_USE_SOCKETS )
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	if ( notifierStdin != 0 ) {
	    notifierStdin->setEnabled( TRUE );
	}
	socketWrite( socketStdin[1] );
    } else
#endif
    {
	socketWrite( 0 );
    }
}

void QProcess::closeStdin( )
{
#if defined ( RMS_USE_SOCKETS )
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	if ( socketStdin[1] != 0 ) {
	    closesocket( socketStdin[1] );
	    socketStdin[1] = 0;
	}
    } else
#endif
    {
	if ( d->pipeStdin[1] != 0 ) {
            CloseHandle( d->pipeStdin[1] );
	    d->pipeStdin[1] = 0;
	}
    }
}

void QProcess::socketRead( int fd )
{
#if defined ( RMS_USE_SOCKETS )
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	qDebug( "Hot Wild Sex!" );
    } else
#endif
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
}

void QProcess::socketWrite( int fd )
{
    DWORD written;
#if defined ( RMS_USE_SOCKETS )
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	if ( fd != socketStdin[1] || socketStdin[1] == 0 )
	    return;
	if ( d->stdinBuf.isEmpty() ) {
	    notifierStdin->setEnabled( FALSE );
	    return;
	}
	if ( !WriteFile( (HANDLE)(socketStdin[1]),
	    d->stdinBuf.head()->data() + d->stdinBufRead,
	    d->stdinBuf.head()->size() - d->stdinBufRead,
	    &written, 0 ) ) {//&overlapStdin ) ) {
	    return;
	}
    } else
#endif
    {
	if ( d->stdinBuf.isEmpty() ) {
	    return;
	}
	if ( !WriteFile( d->pipeStdin[1],
	    d->stdinBuf.head()->data() + d->stdinBufRead,
	    d->stdinBuf.head()->size() - d->stdinBufRead,
	    &written, 0 ) ) {
	    return;
	}
    }
    d->stdinBufRead += written;
    if ( d->stdinBufRead == d->stdinBuf.head()->size() ) {
	d->stdinBufRead = 0;
	delete d->stdinBuf.dequeue();
	socketWrite( fd );
    }

#if 0
    // ### try to read (just for test purposes)
#if defined ( RMS_USE_SOCKETS )
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	char hmpfl[10];
	DWORD read;
	ReadFile( (HANDLE)(socketStdout[0]), hmpfl, 10, &read, 0 );//&overlapStdout );
    } else
#endif
    {
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
#if defined ( RMS_USE_SOCKETS )
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	unsigned long r, i = 2;
	QByteArray readBuffer( i );
	if ( i > 0 ) {
	    ReadFile( d->pipeStdout[0], readBuffer.data(), i, &r, 0 );
	}
        return readBuffer;
    } else
#endif
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
}
