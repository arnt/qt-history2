#include <stdio.h>
#include <stdlib.h>

#include <winsock2.h>

#include "qapplication.h"
#include "qprocess.h"


void QProcess::init()
{
    stdinBufRead = 0;

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

#if 0
	overlapStdin.Internal = 0;
	overlapStdin.InternalHigh = 0;
	overlapStdin.Offset = 0;
	overlapStdin.OffsetHigh = 0;
	overlapStdin.hEvent = 0;
	overlapStdout.Internal = 0;
	overlapStdout.InternalHigh = 0;
	overlapStdout.Offset = 0;
	overlapStdout.OffsetHigh = 0;
	overlapStdout.hEvent = 0;
	overlapStderr.Internal = 0;
	overlapStderr.InternalHigh = 0;
	overlapStderr.Offset = 0;
	overlapStderr.OffsetHigh = 0;
	overlapStderr.hEvent = 0;
#endif

	WSADATA wsaData;
	WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    } else {
	pipeStdin[0] = 0;
	pipeStdin[1] = 0;
	pipeStdout[0] = 0;
	pipeStdout[1] = 0;
	pipeStderr[0] = 0;
	pipeStderr[1] = 0;
    }
}


static bool socketpair( int type, int s[2] )
{
#if 1
    SOCKET so;
    struct sockaddr_in sock_in;
    int len = sizeof( sock_in );

    // create socket and bind it to unused port
//    so = socket( AF_INET, type, 0 );
    so = WSASocket( AF_INET, type, 0, 0, 0, 0 );
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
//    s[1] = socket (AF_INET, type, 0);
    s[1] = WSASocket( AF_INET, type, 0, 0, 0, 0 );
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
    struct sockaddr_in sock_in;

    s[0] = socket( AF_INET, type, 0 );
    if ( s[0] == INVALID_SOCKET ) {
	s[0] = 0;
	return FALSE;
    }
    s[1] = s[0];

    sock_in.sin_family = AF_INET;
    sock_in.sin_port = 0;
    sock_in.sin_addr.s_addr = INADDR_ANY;
    if ( bind( s[0], (struct sockaddr *) &sock_in, sizeof( sock_in ) ) < 0 ) {
	closesocket( s[0] );
	return FALSE;
    }
#endif
}


bool QProcess::start()
{
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
}


bool QProcess::isRunning()
{
    return TRUE;
}


void QProcess::dataStdin( const QByteArray& buf )
{
    stdinBuf.enqueue( new QByteArray(buf) );
    if ( notifierStdin != 0 )
        notifierStdin->setEnabled( TRUE );
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	socketWrite( socketStdin[1] );
    } else {
	socketWrite( 0 );
    }
}


void QProcess::closeStdin( )
{
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
}


void QProcess::socketRead( int fd )
{
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	qDebug( "Hot Wild Sex!" );
    } else {
    }
}


void QProcess::socketWrite( int fd )
{
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
	    &written, 0 ) ) {//&overlapStdin ) ) {
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

#if 1
    // ### try to read (just for test purposes)
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	char hmpfl[10];
	DWORD read;
	ReadFile( (HANDLE)(socketStdout[0]), hmpfl, 10, &read, 0 );//&overlapStdout );
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
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	unsigned long r, i = 2;
	QByteArray readBuffer( i );
	if ( i > 0 ) {
	    ReadFile( pipeStdout[0], readBuffer.data(), i, &r, 0 );
	}
        return readBuffer;
    } else {
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
    }
#endif
}
