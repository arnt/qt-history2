#include "ftp.h"
#include "qurlinfo.h"
#include <stdlib.h>

#include <qstringlist.h>
#include <qregexp.h>

void FTP::hostFound()
{
}

void FTP::connected()
{
}

void FTP::closed()
{
}

void FTP::dataHostFound()
{
}

void FTP::dataConnected()
{
    QString cmd = "CWD " + path + "\r\n";
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
}

void FTP::dataClosed()
{
}

void FTP::dataReadyRead()
{
    QCString s;
    s.resize( dataSocket->bytesAvailable() );
    dataSocket->readBlock( s.data(), dataSocket->bytesAvailable() );
    QString ss = s.copy();
    QStringList lst = QStringList::split( '\n', ss );
    QStringList::Iterator it = lst.begin();
    for ( ; it != lst.end(); ++it ) {
	QUrlInfo inf;
	parseDir( *it, inf );
	if ( !inf.name().isEmpty() )
	    emit newEntry( inf );
    }
}

void FTP::parseDir( const QString &buffer, QUrlInfo &info )
{
    QStringList lst = QStringList::split( " ", buffer );
    QString tmp;

    // permissions
    tmp = lst[ 0 ];

    if ( tmp[ 0 ] == QChar( 'd' ) ) {
	info.setDir( TRUE );
	info.setFile( FALSE );
    } else if ( tmp[ 0 ] == QChar( '-' ) ) {
	info.setDir( FALSE );
	info.setFile( TRUE );
    } else
	return; // ### todo links

    // owner
    tmp = lst[ 2 ];
    info.setOwner( tmp );

    // group
    tmp = lst[ 3 ];
    info.setGroup( tmp );

    // date, time #### todo

    // name
    info.setName( lst[ 8 ].stripWhiteSpace() );

}

void FTP::readyRead()
{
    QCString s;
    s.resize( commandSocket->bytesAvailable() );
    commandSocket->readBlock( s.data(), commandSocket->bytesAvailable() );
	
    if ( s.contains( "220" ) ) {
	commandSocket->writeBlock( "USER anonymous\r\n", strlen( "USER anonymous\r\n" ) );
    } else if ( s.contains( "331" ) ) {
	commandSocket->writeBlock( "PASS reggie@troll.no\r\n", strlen( "PASS reggie@troll.no\r\n" ) );
    } else if ( s.contains( "230" ) ) {
	commandSocket->writeBlock( "PASV\r\n", strlen( "PASV\r\n") );
    } else if ( s.contains( "227" ) ) {
	int i = s.find( "(" );
	int i2 = s.find( ")" );
	s = s.mid( i + 1, i2 - i - 1 );
	if ( !dataSocket->host().isEmpty() )
	    dataSocket->close();
	QStringList lst = QStringList::split( ',', s );
	int port = ( lst[ 4 ].toInt() << 8 ) + lst[ 5 ].toInt();
	dataSocket->connectToHost( lst[ 0 ] + "." + lst[ 1 ] + "." + lst[ 2 ] + "." + lst[ 3 ], port );
    } else if ( s.contains( "250" ) ) {
	commandSocket->writeBlock( "LIST\r\n", strlen( "LIST\r\n" ) );
    }
}

FTP::FTP()
{
    commandSocket = new QSocket( this );
    dataSocket = new QSocket( this );
    connect( commandSocket, SIGNAL( hostFound() ),
	     this, SLOT( hostFound() ) );
    connect( commandSocket, SIGNAL( connected() ),
	     this, SLOT( connected() ) );
    connect( commandSocket, SIGNAL( closed() ),
	     this, SLOT( closed() ) );
    connect( commandSocket, SIGNAL( readyRead() ),
	     this, SLOT( readyRead() ) );

    connect( dataSocket, SIGNAL( hostFound() ),
	     this, SLOT( dataHostFound() ) );
    connect( dataSocket, SIGNAL( connected() ),
	     this, SLOT( dataConnected() ) );
    connect( dataSocket, SIGNAL( closed() ),
	     this, SLOT( dataClosed() ) );
    connect( dataSocket, SIGNAL( readyRead() ),
	     this, SLOT( dataReadyRead() ) );

}

void FTP::open( const QString &host_, int port, const QString &path_ )
{
    commandSocket->connectToHost( host_, port );
    host = host_;
    path = path_;
}

void FTP::close()
{	
    if ( !commandSocket->host().isEmpty() ) {
	commandSocket->writeBlock( "quit\r\n", strlen( "quit\r\n" ) );
	commandSocket->close();
    }
}

FTP::~FTP()
{
    if ( !commandSocket->host().isEmpty() ) {
	commandSocket->writeBlock( "quit\r\n", strlen( "quit\r\n" ) );
	commandSocket->close();
    }
    delete commandSocket;
    commandSocket = 0;
}

FTP &FTP::operator=( const FTP &ftp )
{
    disconnect( commandSocket, SIGNAL( hostFound() ),
		this, SLOT( hostFound() ) );
    disconnect( commandSocket, SIGNAL( connected() ),
		this, SLOT( connected() ) );
    disconnect( commandSocket, SIGNAL( closed() ),
		this, SLOT( closed() ) );
    disconnect( commandSocket, SIGNAL( readyRead() ),
	     this, SLOT( readyRead() ) );
    commandSocket = new QSocket( this );
    connect( commandSocket, SIGNAL( hostFound() ),
	     this, SLOT( hostFound() ) );
    connect( commandSocket, SIGNAL( connected() ),
	     this, SLOT( connected() ) );
    connect( commandSocket, SIGNAL( closed() ),
	     this, SLOT( closed() ) );
    connect( commandSocket, SIGNAL( readyRead() ),
	     this, SLOT( readyRead() ) );
    if ( !ftp.commandSocket->host().isEmpty() )
	commandSocket->connectToHost( ftp.commandSocket->host(),
				      ftp.commandSocket->port() );
    host = ftp.host;
    path = ftp.path;
    buffer = ftp.buffer;

    return *this;
}
