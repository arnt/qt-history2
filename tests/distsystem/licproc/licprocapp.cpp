#include "licprocapp.h"
#include <qdir.h>
#include <qfile.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qregexp.h>
#include <qsocketdevice.h>

#include <time.h>
#include <stdlib.h>

LicProcApp::LicProcApp( int argc, char** argv ) : QApplication( argc, argv )
{
    company = "ts3";
    interval = 120;
    port = 801;

    syncTimer.start( interval * 1000, false );
    connect( &syncTimer, SIGNAL( timeout() ), this, SLOT( syncLicenses() ) );

    distDB = QSqlDatabase::addDatabase( "QMYSQL3" );
    distDB->setUserName( "distributor" );
    distDB->setDatabaseName( "dist" );
    distDB->setPassword( "sendit" );
    distDB->setHostName( "marijuana.troll.no" );

    connect( &dns, SIGNAL( resultsReady() ), this, SLOT( dnsReady() ) );

    syncLicenses();
}

void LicProcApp::syncLicenses()
{
    QDir dir( "\\\\soma\\licensefiles" );
    const QFileInfoList* list = dir.entryInfoList( company + "_*.txt", QDir::Files, QDir::Name );
    QFileInfoListIterator it( *list );
    QFileInfo* fi;

    licenseList.clear();
    while( ( fi = it.current() ) ) {
	ProcessFile( fi->fileName() );
	licenseList += fi->fileName().mid( 4, 7 );
	++it;
    }
    if( !licenseList.isEmpty() )
	dnsReady();
}

void LicProcApp::dnsReady()
{
    QSocketDevice sock;
    QHostAddress addr;

    bool b = addr.setAddress( "213.203.59.121" );

    if( sock.connect( addr, port ) ) {
	QTextStream sockStream( &sock );
	sockStream << licenseList.join( "," ) << endl;
    }
}

void LicProcApp::ProcessFile( QString fileName )
{
    QString licenseShare( "\\\\soma\\licensefiles" );

    if( distDB->open() ) {
	QDir licDir( licenseShare );
	if( licDir.exists( fileName ) ) {
	    QString tmpName( fileName + ".tmp" );
	    if( licDir.rename( fileName, tmpName ) ) {
		QFile inFile( licenseShare + "\\" + tmpName );
		if( inFile.open( IO_ReadOnly ) ) {
		    QString currentLine;
		    while( inFile.readLine( currentLine, 1024 ) != -1 ) {
			QStringList itemComponents = QStringList::split( ";", currentLine, true );
			QStringList::Iterator it = itemComponents.begin();

			QString orderDate = (*it++);
			QString custID = (*it++);
			QString salesID = (*it++);
			QString itemID = (*it++);
			QString licenseID = (*it++);
			QString licensee = (*it++);
			QString licenseEmail = (*it++);
			QString login = (*it++);
			QString password = (*it++);
			QDate expiryDate = QDate::fromString( *it++, Qt::ISODate );

			/*
			** First choice of handling.
			** Items ending with an 'm' can be processed directly.
			** Items like 'z*u*' are upgrade products.
			** All other items should be skipped.
			*/

			if( ( itemID == "zqum" ) )
			    continue;
			else if( itemID.right( 1 ) == "m" ) {
			    /*
			    ** We will also get items of 'zqum' in upgrades, but as these are
			    ** not connected to any files, they will not matter.
			    */
			    if( fileName.mid( 8, 3 ) == "USA" )
				itemID += "-us";

			    QSqlCursor licenseCursor( "licenses" );
			    licenseCursor.select( "ID = " + licenseID );
			    bool licenseExists( false );
			    while( !licenseExists && licenseCursor.next() ) {
				// If there are records that pass the filter above, the license already exists
				licenseExists = true;
			    }
			    if( !licenseExists ) {
				// If no license exists, create one
				QSqlRecord* buffer = licenseCursor.primeInsert();
				buffer->setValue( "ID", licenseID );
				buffer->setValue( "CustomerID", custID );
				buffer->setValue( "Login", login );
				buffer->setValue( "Password", password );
				buffer->setValue( "Licensee", licensee );
				buffer->setValue( "Email", licenseEmail );
				buffer->setValue( "ExpiryDate", expiryDate );
				licenseCursor.insert();
			    }
			    else {
				QSqlRecord* buffer = licenseCursor.primeUpdate();
				buffer->setValue( "ExpiryDate", expiryDate );
				licenseCursor.update();
			    }
			    // The license should have been created now.
			    // TODO: Add a second test to verify that the license is present in the database
			    QSqlCursor itemsCursor( "items" );
			    itemsCursor.select( "LicenseID = " + licenseID + " and ItemID = '" + itemID + "'" );
			    bool itemExists( false );
			    while( !itemExists && itemsCursor.next() )
				itemExists = true;
			    if( !itemExists ) {
				QSqlRecord* buffer = itemsCursor.primeInsert();
				QString itemString = itemID;
				itemString.replace( QRegExp( "\\d" ), QString::null );
				buffer->setValue( "LicenseID", licenseID );
				buffer->setValue( "ItemID", itemString );
				itemsCursor.insert();
			    }
			}
			else if( ( itemID[ 0 ] == 'z' ) && ( itemID.find( 'u' ) != -1 ) ) {
			    // This is an upgrade product, try to decode the item ID to identify the upgrade.
			    bool singlePack = ( itemID[ 2 ] == '1' );
			    bool duoPack = ( itemID[ 2 ] == '2' );
			    bool pro2Enterprise = ( itemID.right( 3 ) == "upe" );
			    bool single2Duo = ( itemID.right( 1 ) == "u" );

			    if( singlePack && pro2Enterprise ) {
				QString item2Upgrade = itemID;
				item2Upgrade = item2Upgrade.replace( QRegExp( "[\\due]" ), QString::null ) + "m";
				QString upgradedItem = item2Upgrade;
				upgradedItem = upgradedItem.replace( QRegExp( "p" ), "e" );

				QSqlCursor itemsCursor( "items" );
				itemsCursor.select( "LicenseID = " + licenseID + " and ItemID = '" + item2Upgrade + "'" );
				bool itemExists( false );
				while( !itemExists && itemsCursor.next() )
				    itemExists = true;
				if( itemExists ) {
				    // We are on the correct record.
				    QSqlRecord* buffer = itemsCursor.primeUpdate();
				    buffer->setValue( "ItemID", upgradedItem );
				    itemsCursor.update();
				}
			    }
			    else if( duoPack && pro2Enterprise ) {
				QSqlCursor itemsCursor( "items" );

				itemsCursor.select( "LicenseID = " + licenseID );
				while( itemsCursor.next() ) {
				    QSqlRecord* buffer = itemsCursor.primeUpdate();
				    QString itemString = buffer->value( "ItemID" ).asString();
				    itemString.replace( QRegExp( "fpm" ), "fem" );
				    buffer->setValue( "ItemID", itemString );
				    itemsCursor.update( false );
				}
			    }
			    else if( single2Duo ) {
				QSqlCursor itemsCursor( "items" );

				QSqlRecord* buffer = itemsCursor.primeInsert();
				QString itemString = itemID;
				itemString = itemString.replace( QRegExp( "\\d" ), QString::null ).left( 5 ) + "m";
				buffer->setValue( "LicenseID", licenseID );
				buffer->setValue( "ItemID", itemString );
				itemsCursor.insert();
			    }
			}
		    }
		    inFile.close();
		    QFile::remove( licenseShare + "/" + tmpName );
		}
	    }
	}
    }
}

QString LicProcApp::CreatePassword()
{
    QString passwd;
    QString pwChars( "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_+()=*!#%&/" );

    srand( time( NULL ) );

    for( int i = 0; i < 8; i++ ) {
	int tmp = rand();
	passwd += pwChars[ tmp % 73 ];
    }
    return passwd;
}
