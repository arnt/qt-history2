// LicProc.cpp : Implementation of LicProc
#include "stdafx.h"
#include "Licproc_com.h"
#include "LicProc.h"

#include "qstring.h"
#include "qsqldatabase.h"
#include "qsqlcursor.h"
#include "qregexp.h"

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

STDMETHODIMP LicProc::dummy(DWORD bar)
{
    DWORD foo = bar;
    return S_OK;
}

STDMETHODIMP LicProc::publishLicense(DWORD licenseId, BSTR custId, BSTR licensee, BSTR licenseeEmail, BSTR login, BSTR password, BSTR expiryDate, BSTR companyId)
{
    if( !distDb->open() )
	return E_FAIL;
    if( ( BSTR2QString( companyId ) != 'ts3' ) )
	return S_OK;

    QSqlRecord* buffer;

    QSqlCursor licenseCursor( "licenses" );
    licenseCursor.select( QString( "ID = %1" ).arg( licenseId ) );
    licenseCursor.first();
    if( licenseCursor.isValid() ) {
	// If there are records that pass the filter above, the license already exists
	// We will delete it, in that case.
	QSqlQuery q;
	q.exec( QString( "DELETE from licenses where LicenseId = %1" ).arg( licenseId ) );
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
    if( !distDb->open() )
	return E_FAIL;
    if( ( BSTR2QString( companyId ) != 'ts3' ) )
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

	QSqlCursor itemsCursor( "items" );
	buffer = itemsCursor.primeInsert();
	QString itemString = _itemId;
	itemString.replace( QRegExp( "\\d" ), QString::null );
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
    Beep( 440, 200 );
    distDb = QSqlDatabase::addDatabase( "QMYSQL3" );
    distDb->setUserName( BSTR2QString( user ) );
    distDb->setDatabaseName( BSTR2QString( db ) );
    distDb->setPassword( BSTR2QString( password ) );
    distDb->setHostName( BSTR2QString( srv ) );

    return S_OK;
}

STDMETHODIMP LicProc::updateDb(DWORD licenseId, BSTR versionTag, BSTR companyId)
{
    if( ( BSTR2QString( companyId ) != 'ts3' ) )
	return S_OK;

	return S_OK;
}
