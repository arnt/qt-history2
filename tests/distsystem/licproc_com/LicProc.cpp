// LicProc.cpp : Implementation of LicProc
#include "stdafx.h"
#include "Licproc_com.h"
#include "LicProc.h"

#include "qstring.h"
#include "qsqldatabase.h"
#include "qsqlcursor.h"
#include "qregexp.h"

#include <io.h>

/////////////////////////////////////////////////////////////////////////////
// LicProc


static QString BSTR2QString( BSTR src )
{
    QString tmp;

    for( unsigned int i = 0; i < SysStringLen( src ); i++ ) {
	QChar c( src[ i ] );
	tmp += c;
    }
    return tmp;
}

static bool checkCompanyId( QString companyId )
{
    if( ( companyId == "nor" ) || ( companyId == "usa" ) )
	return true;

    return false;
}

STDMETHODIMP LicProc::dummy(DWORD bar)
{
    DWORD foo = bar;
    return S_OK;
}

STDMETHODIMP LicProc::publishLicense(DWORD licenseId, BSTR custId, BSTR licensee, BSTR licenseeEmail, BSTR login, BSTR password, BSTR expiryDate, BSTR companyId)
{
    if( !distDb->isOpen() )
	return E_FAIL;
    if( !checkCompanyId( BSTR2QString( companyId ) ) )
	return S_OK;

    QSqlRecord* buffer;

    QSqlCursor licenseCursor( "licenses" );
    licenseCursor.select( QString( "ID = %1" ).arg( licenseId ) );
    licenseCursor.first();
    if( licenseCursor.isValid() ) {
	// If there are records that pass the filter above, the license already exists
	// We will delete it, in that case.
	QSqlQuery q;
	q.exec( QString( "DELETE from licenses where Id = %1" ).arg( licenseId ) );
	q.exec( QString( "DELETE from items where LicenseId = %1" ).arg( licenseId ) );
    }
    buffer = licenseCursor.primeInsert();

    buffer->setValue( "ID", QString( "%1" ).arg( licenseId ) );
    buffer->setValue( "CustomerID", BSTR2QString( custId ) );
    buffer->setValue( "Login", BSTR2QString( login ) );
    buffer->setValue( "Password", BSTR2QString( password ) );
    buffer->setValue( "Licensee", BSTR2QString( licensee ) );
    buffer->setValue( "Email", BSTR2QString( licenseeEmail ) );
    buffer->setValue( "ExpiryDate", BSTR2QString( expiryDate ) );
    licenseCursor.insert();

    return S_OK;
}

STDMETHODIMP LicProc::publishLine(DWORD licenseId, BSTR itemId, DWORD usLicense, BSTR companyId)
{
    if( !distDb->isOpen() )
	return E_FAIL;
    if( !checkCompanyId( BSTR2QString( companyId ) ) )
	return S_OK;

    QSqlRecord* buffer;
    QString _itemId = BSTR2QString( itemId );
    /*
    ** First choice of handling.
    ** Items ending with an 'm' can be processed directly.
    ** Items like 'z*u*' are upgrade products.
    ** All other items should be skipped.
    */

    if( ( _itemId != "zqum" ) && ( _itemId.right( 1 ) == "m" ) ) {
	/*
	** We will also get items of 'zqum' in upgrades, but as these are
	** not connected to any files, they will not matter.
	*/
	if( usLicense )
	    _itemId += "-us";

	QString itemString = _itemId;
	itemString.replace( QRegExp( "\\d" ), QString::null );

	QSqlCursor itemsCursor( "items" );
	buffer = itemsCursor.primeInsert();
	buffer->setValue( "LicenseID", QString( "%1" ).arg( licenseId ) );
	buffer->setValue( "ItemID", itemString );
	itemsCursor.insert();
    }
    else if( ( _itemId[ 0 ] == 'z' ) && ( _itemId.find( 'u' ) != -1 ) ) {
	// This is an upgrade product, try to decode the item ID to identify the upgrade.
	bool pro2Enterprise = ( _itemId.right( 3 ) == "upe" );
	bool addPlatform = ( _itemId.right( 1 ) == "u" );

	if( pro2Enterprise ) {
	    // Pro to Enterprise upgrades are handled similarly, independant
	    // of the package size.  Alter all '*p*' items to '*e*'
	    QSqlCursor itemsCursor( "items" );
	    itemsCursor.select( QString( "LicenseID = %1 and ItemID like '%%fpm%%'" ).arg( licenseId ) );
	    while( itemsCursor.next() ) {
		buffer = itemsCursor.primeUpdate();
		QString itemString = buffer->value( "ItemID" ).asString();
		itemString.replace( QRegExp( "fpm" ), "fem" );
		buffer->setValue( "ItemID", itemString );
		itemsCursor.update( false );
	    }
	}
	else if( addPlatform ) {
	    if( usLicense )
		_itemId += "-us";
	    QString itemString = _itemId;
	    itemString = itemString.replace( QRegExp( "\\d" ), QString::null ).left( 5 ) + "m";

	    QSqlCursor itemsCursor( "items" );
	    buffer = itemsCursor.primeInsert();
	    buffer->setValue( "LicenseID", QString( "%1" ).arg( licenseId ) );
	    buffer->setValue( "ItemID", itemString );
	    itemsCursor.insert();
	}
    }

    return S_OK;
}

STDMETHODIMP LicProc::setServer(BSTR srv, BSTR user, BSTR password, BSTR db)
{
    srvName = BSTR2QString( srv );

    distDb = QSqlDatabase::addDatabase( "QMYSQL3" );
    distDb->setUserName( BSTR2QString( user ) );
    distDb->setDatabaseName( BSTR2QString( db ) );
    distDb->setPassword( BSTR2QString( password ) );
    distDb->setHostName( srvName );
    if( !distDb->open() )
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP LicProc::updateDb(DWORD licenseId, BSTR versionTag, BSTR companyId)
{
    QString tag = BSTR2QString( versionTag ) + "\n";

    if( !checkCompanyId( BSTR2QString( companyId ) ) )
	return S_OK;

    int sock = socket( AF_INET, SOCK_STREAM, 0 );
    if( !sock )
	return E_FAIL;

    struct sockaddr_in srvAddr;

    memset( &srvAddr, 0, sizeof( srvAddr ) );
    struct hostent* he = gethostbyname( srvName.latin1() );

    srvAddr.sin_family = AF_INET;
    srvAddr.sin_addr = *(in_addr*)( he->h_addr_list[ 0 ] );
    srvAddr.sin_port = htons( 801 );
    if( !connect( sock, (struct sockaddr*)&srvAddr, sizeof( srvAddr ) ) ) {
	char buffer( 0 );
	QString str;
	bool sawGreeting( false );

	while( 1 ) {
	    while( buffer != '\n' ) {
		if( recv( sock, &buffer, sizeof( buffer ), 0 ) == 1) {
		    str += buffer;
		}
		else {
		    close( sock );
		    return S_OK;
		}
	    }
	    if( !sawGreeting ) {
		sawGreeting = true;
		send( sock, tag.latin1(), tag.length(), 0 );
	    }
	    buffer = 0;
	    str = "";
	}
	close( sock );
    }
    
    return S_OK;
}

/*
** For some reason, this function is required for release builds,
** but not debug...
*/
int main(int argc, char** argv )
{
    return 0;
}

STDMETHODIMP LicProc::publishVersionTag(BSTR tag, BSTR versionString, BSTR subDir, BSTR companyId)
{
    if( !distDb->isOpen() )
	return E_FAIL;
    if( !checkCompanyId( BSTR2QString( companyId ) ) )
	return S_OK;

    QSqlRecord* buffer;

    QSqlCursor versionCursor( "versions" );
    versionCursor.select( QString( "tag = '%1'" ).arg( BSTR2QString( tag ) ) );
    versionCursor.first();
    if( versionCursor.isValid() ) {
	QSqlQuery q;
	q.exec( QString( "DELETE from versions where tag = '%1'" ).arg( BSTR2QString( tag ) ) );
    }
    buffer = versionCursor.primeInsert();

    buffer->setValue( "tag", BSTR2QString( tag ) );
    buffer->setValue( "subdir", BSTR2QString( subDir ) );
    buffer->setValue( "version", BSTR2QString( versionString ) );
    versionCursor.insert();

    return S_OK;
}

STDMETHODIMP LicProc::publishFilemap(BSTR tag, BSTR itemId, BSTR fileName, BSTR fileDesc, BSTR companyId)
{
    if( !distDb->isOpen() )
	return E_FAIL;
    if( !checkCompanyId( BSTR2QString( companyId ) ) )
	return S_OK;

    QSqlRecord* buffer;

    QSqlCursor mapCursor( "filesmap" );
    mapCursor.select( QString( "ItemID = '%1' and FileName = '%2' and VersionTag = '%3'" ).arg( BSTR2QString( itemId ) ).arg( BSTR2QString( fileName ) ).arg( BSTR2QString( tag ) ) );
    mapCursor.first();
    if( mapCursor.isValid() ) {
	QSqlQuery q;
	q.exec( QString( "DELETE from map where ItemID = '%1' and FileName = '%2' and VersionTag = '%3'" ).arg( BSTR2QString( itemId ) ).arg( BSTR2QString( fileName ) ).arg( BSTR2QString( tag ) ) );
    }
    buffer = mapCursor.primeInsert();

    buffer->setValue( "VersionTag", BSTR2QString( tag ) );
    buffer->setValue( "ItemID", BSTR2QString( itemId ) );
    buffer->setValue( "FileName", BSTR2QString( fileName ) );
    buffer->setValue( "FileDec", BSTR2QString( fileDesc ) );
    mapCursor.insert();

    return S_OK;
}

STDMETHODIMP LicProc::clearVersionTags(BSTR companyId)
{
    if( !distDb->isOpen() )
	return E_FAIL;
    if( !checkCompanyId( BSTR2QString( companyId ) ) )
	return S_OK;

    QSqlQuery q;
    q.exec( QString( "DELETE from versions" ) );

    return S_OK;
}

STDMETHODIMP LicProc::clearFilemap(BSTR companyId)
{
    if( !distDb->isOpen() )
	return E_FAIL;
    if( !checkCompanyId( BSTR2QString( companyId ) ) )
	return S_OK;

    QSqlQuery q;
    q.exec( QString( "DELETE from filesmap" ) );

    return S_OK;
}

STDMETHODIMP LicProc::deleteVersionTag(BSTR tag, BSTR companyId)
{
    if( !distDb->isOpen() )
	return E_FAIL;
    if( !checkCompanyId( BSTR2QString( companyId ) ) )
	return S_OK;

    QSqlQuery q;
    q.exec( QString( "DELETE from versions where tag = '%1'" ).arg( BSTR2QString( tag ) ) );

    return S_OK;
}

STDMETHODIMP LicProc::deleteFilemap(BSTR tag, BSTR itemId, BSTR fileName, BSTR companyId)
{
    if( !distDb->isOpen() )
	return E_FAIL;
    if( !checkCompanyId( BSTR2QString( companyId ) ) )
	return S_OK;

    QSqlQuery q;
    q.exec( QString( "DELETE from map where ItemID = '%1' and FileName = '%2' and VersionTag = '%3'" ).arg( BSTR2QString( itemId ) ).arg( BSTR2QString( fileName ) ).arg( BSTR2QString( tag ) ) );

    return S_OK;
}
