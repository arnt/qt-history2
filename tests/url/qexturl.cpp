#include "qexturl.h"

QExtUrl::QExtUrl()
    : QUrl()
{
    connect( &ftp, SIGNAL( newEntry( const QUrlInfo & ) ),
	     this, SLOT( sendNewEntry( const QUrlInfo & ) ) );
    connect( &ftp, SIGNAL( listFinished() ),
	     this, SLOT( listFinished() ) );
}

QExtUrl::QExtUrl( const QString& url )
    : QUrl( url )
{
    connect( &ftp, SIGNAL( newEntry( const QUrlInfo & ) ),
	     this, SLOT( sendNewEntry( const QUrlInfo & ) ) );
    connect( &ftp, SIGNAL( listFinished() ),
	     this, SLOT( listFinished() ) );
}

QExtUrl::QExtUrl( const QUrl& url )
    : QUrl( url )
{
    connect( &ftp, SIGNAL( newEntry( const QUrlInfo & ) ),
	     this, SLOT( sendNewEntry( const QUrlInfo & ) ) );
    connect( &ftp, SIGNAL( listFinished() ),
	     this, SLOT( listFinished() ) );
}

QExtUrl::QExtUrl( const QUrl& url, const QString& relUrl_ )
    : QUrl( url, relUrl_ )
{
    connect( &ftp, SIGNAL( newEntry( const QUrlInfo & ) ),
	     this, SLOT( sendNewEntry( const QUrlInfo & ) ) );
    connect( &ftp, SIGNAL( listFinished() ),
	     this, SLOT( listFinished() ) );
}

void QExtUrl::listEntries( const QString &nameFilter, int filterSpec = QDir::DefaultFilter,
			   int sortSpec = QDir::DefaultSort )
{
    if ( isLocalFile() )
	QUrl::listEntries( nameFilter, filterSpec, sortSpec );
    else if ( protocol() == "ftp" ) {
	ftp.close();
	if ( user().isEmpty() )
	    ftp.open( host(), 21, path().isEmpty() ? QString( "/" ) : path(),
		      "anonymous", "Qt@cool", QFtp::List );
	else
	    ftp.open( host(), 21, path().isEmpty() ? QString( "/" ) : path(),
		      user(), pass(), QFtp::List );
	
	emit start();
    }
}

void QExtUrl::mkdir( const QString &dirname )
{
    if ( isLocalFile() )
	QUrl::mkdir( dirname );
    else if ( protocol() == "ftp" ) {
	ftp.close();
	if ( user().isEmpty() )
	    ftp.open( host(), 21, path().isEmpty() ? QString( "/" ) : path(),
		      "anonymous", "Qt@cool", QFtp::Mkdir, dirname );
	else
	    ftp.open( host(), 21, path().isEmpty() ? QString( "/" ) : path(),
		      user(), pass(), QFtp::Mkdir, dirname );
    }
}

QString QExtUrl::toString() const
{
    if ( isLocalFile() )
	return QUrl::toString();
    else if ( protocol() == "ftp" ) {
	if ( !user().isEmpty() )
	    return protocol() + "://" + user() + ":" + pass() + "@" + host() + QDir::cleanDirPath( path() ).stripWhiteSpace(); // #### todo
	else
	    return protocol() + "://" + host() + QDir::cleanDirPath( path() ).stripWhiteSpace(); // #### todo
    }
    
    return QString::null;
}

