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
    if ( expectList ) {
	QCString s;
	s.resize( commandSocket->bytesAvailable() );
	commandSocket->readBlock( s.data(), commandSocket->bytesAvailable() );
	QString ss = s.copy();
	QStringList lst = QStringList::split( '\n', ss );
	QStringList::Iterator it = lst.begin();
	for ( ; it != lst.end(); ++it ) {
	    QUrlInfo inf;
	    parseDir( *it, inf );
	    if ( !inf.name().isEmpty() )
		emit newEntry( inf );
	}
	
	if ( s.contains( "213 End" ) )
	    expectList = FALSE;
    } else {

	QCString s;
	s.resize( commandSocket->bytesAvailable() );
	commandSocket->readBlock( s.data(), commandSocket->bytesAvailable() );

	if ( s.contains( "220" ) ) {
	    int i = commandSocket->writeBlock( "USER anonymous\r\n", strlen( "USER anonymous\r\n" ) );
	} else if ( s.contains( "331" ) ) {
	    int i = commandSocket->writeBlock( "PASS reggie@troll.no\r\n", strlen( "PASS reggie@troll.no\r\n" ) );
	} else if ( s.contains( "230" ) ) {
	    QString cmd = "STAT " + path + "\r\n";
	    int i = commandSocket->writeBlock( cmd.latin1(), cmd.length() );
	    expectList = TRUE;
	    buffer = "";
	}
    }	
}

FTP::FTP()
    : expectList( FALSE )
{
    commandSocket = new QSocket( this );
    connect( commandSocket, SIGNAL( hostFound() ),
	     this, SLOT( hostFound() ) );
    connect( commandSocket, SIGNAL( connected() ),
	     this, SLOT( connected() ) );
    connect( commandSocket, SIGNAL( closed() ),
	     this, SLOT( closed() ) );
    connect( commandSocket, SIGNAL( readyRead() ),
	     this, SLOT( readyRead() ) );

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
    expectList = FALSE;
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
