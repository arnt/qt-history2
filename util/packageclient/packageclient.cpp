#include "packageclient.h"
#include <qcstring.h>
#include <stdlib.h>
#include <qfile.h>
#include <qtimer.h>

PackageClient::PackageClient( const QString &source, const QString &dest, const QString &host )
    : QSocket( 0 )
{
    connect( this, SIGNAL( connected() ), this, SLOT( startCopy() ) );
    connect( this, SIGNAL( readyRead() ), this, SLOT( readResult() ) );
    connect( this, SIGNAL( error( int ) ), this, SLOT( arrrrrrg() ) );
    filename = source;
    destination = dest;
    hostname = host;
    QTimer::singleShot( 0, this, SLOT( doConnect() ) );
    size = -1;
}

void PackageClient::doConnect()
{
    connectToHost( hostname, 7878 );
}

void PackageClient::startCopy()
{
    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
	qFatal( "couldn't open %s", filename.latin1() );
    QByteArray ba( f.size() );
    f.readBlock( ba.data(), f.size() );
    qDebug( "write %d", f.size() );
    int s = f.size();
    writeBlock( (char*)&s, 4 );
    writeBlock( ba.data(), ba.size() );
}

void PackageClient::readResult()
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
	QFile f( destination );
	if ( !f.open( IO_WriteOnly ) )
	    qFatal( "couldn't open" );
	f.writeBlock( buffer );

	// #### run the stuff
	int s = buffer.size();
	writeBlock( (char*)&s, 4 );
	writeBlock( buffer.data(), buffer.size() );
	::exit( 0 );
    }
}

void PackageClient::arrrrrrg()
{
    qDebug( "error!!" );
}
