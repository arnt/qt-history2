#include "packageserver.h"
#include <qfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <qdir.h>
#include <qstringlist.h>
#if defined(Q_OS_WIN32)
#include <qt_windows.h>
#endif

PackageSocket::PackageSocket( int socket, QObject *parent )
    : QSocket( parent )
{
    connect( this, SIGNAL( readyRead() ),
	     SLOT( readClient() ) );
    connect( this, SIGNAL( connectionClosed() ),
	     SLOT( connectionClosed() ) );
    setSocket( socket );
    size = -1;
}

void PackageSocket::readClient()
{
    if ( size == -1 ) {
	readBlock( (char*)&size, 4 );
	pos = 0;
    }
    buffer.resize( size );
    int s = bytesAvailable();
    readBlock( buffer.data() + pos, bytesAvailable() );
    pos += s;
    if ( pos == size ) {
	QFile f( "a.zip" );
	if ( !f.open( IO_WriteOnly ) )
	    qFatal( "couldn't open" );
	f.writeBlock( buffer );

	f.flush();

#if defined(Q_OS_WIN32)
	SleepEx( 1000, FALSE );
#endif	
	system( "unzip -o a.zip" );
	system( "build-selfextractor.bat" );
	
	QDir d;
	QStringList l = d.entryList( "q*.exe" );

	f.close();
	f.setName( l[0] );
	qDebug( l[0] );
	if ( !f.open( IO_ReadOnly ) )
	    qFatal( "could not open" );

	QByteArray ba( f.size() );
	f.readBlock( ba.data(), f.size() );

	int s = ba.size();
	writeBlock( (char*)&s, 4 );
	writeBlock( ba.data(), s );
	pos = -1;
    }
}

void PackageSocket::connectionClosed()
{
    //delete this;
}

PackageServer::PackageServer( QObject *parent )
    : QServerSocket( 7878, 1, parent )
{

}

void PackageServer::newConnection( int socket )
{
    new PackageSocket( socket, this );
}
