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
	// delete any old qt-*.exe files
	QDir d;
	QStringList l = d.entryList( "qt-*.exe" );
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
	    if ( d.remove(*it) )
		qDebug( "Deleted: %s", (*it).latin1() );
	}
	l.clear();

	// write incoming buffer from client to a.zip
	QFile f( "a.zip" );
	if ( !f.open( IO_WriteOnly | IO_Truncate ) )
	    qFatal( "Couldn't open a.zip" );
	f.writeBlock( buffer );

	f.flush();

#if defined(Q_OS_WIN32)
	SleepEx( 1000, FALSE );
#endif
	// unzip and run bat script
	system( "unzip -o a.zip" );
	system( "build-selfextractor.bat" );

	// send first qt-*.exe back to the client (should be only one exe)
	l = d.entryList( "qt-*.exe" );
	if ( l.count() != 1 )
	    qWarning( "Multiple qt-*.exe files found!!" );

	f.close();
	f.setName( l[0] );
	qDebug( l[0] );
	if ( !f.open( IO_ReadOnly ) )
	    qFatal( "Could not open: %s", l[0].latin1() );

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
