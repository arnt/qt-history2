#include "licprocapp.h"
#include <qdir.h>
#include <qfile.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qregexp.h>
#include <qsocketdevice.h>

#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream.h>

LicProcApp::LicProcApp( int argc, char** argv ) : QApplication( argc, argv )
{
    company = "nor";
    interval = 300;
    port = 801;

    cout << "LicProc initializing" << endl; cout.flush();
    cout << "Timer interval is: " << interval << " seconds." << endl; cout.flush();
    syncTimer.start( interval * 1000, false );
    connect( &syncTimer, SIGNAL( timeout() ), this, SLOT( syncLicenses() ) );
    connect( &sock, SIGNAL( connected() ), this, SLOT( sockConnected() ) );
    connect( &sock, SIGNAL( error( int ) ), this, SLOT( sockError( int ) ) );
    connect( &sock, SIGNAL( connectionClosed() ), this, SLOT( sockClosed() ) );
    connect( &sock, SIGNAL( readyRead() ), this, SLOT( readyRead() ) );
    distDB = QSqlDatabase::addDatabase( "QMYSQL3" );
    distDB->setUserName( "distributor" );
    distDB->setDatabaseName( "dist" );
    distDB->setPassword( "sendit" );
    distDB->setHostName( "dist.troll.no" );

    syncLicenses();
}

void LicProcApp::syncLicenses()
{
    QDir dir( "\\\\lupinella\\axapta\\licensefiles" );
    const QFileInfoList* list = dir.entryInfoList( company + "_*.txt", QDir::Files, QDir::Name );
    QFileInfoListIterator it( *list );
    QFileInfo* fi;

    cout << "Syncing licenses for \"" << company.latin1() << "\"" << endl; cout.flush();
    licenseList.clear();
    tag = "";

    while( ( fi = it.current() ) ) {
	ProcessFile( fi->fileName() );
	licenseList += fi->fileName().mid( 4, 7 );
	++it;
    }
    if( !licenseList.isEmpty() )
	updateDist( tag );

    cout << "Waiting" << endl; cout.flush();
}

void LicProcApp::sockConnected()
{
    cout << "connected" << endl; cout.flush();
    stream.setDevice( &sock );
}

void LicProcApp::readyRead()
{
    if( sock.canReadLine() ) {
	if( !sawGreeting )
	    stream << tag << endl; sock.flush();

	sawGreeting = true;
        QString buffer = sock.readLine();
        cout << ">> " << buffer; cout.flush();

        licenseList.clear();
    }
}

void LicProcApp::sockError( int i)
{
    cout << "error, code " << i << endl; cout.flush();
}

void LicProcApp::sockClosed()
{
    cout << "Connection closed." << endl; cout.flush();
}

void LicProcApp::updateDist( QString tag )
{

    QHostAddress addr;

    cout << "Triggering update by connecting to dist.troll.no:801" << endl; cout.flush();
    sawGreeting = false;
    sock.connectToHost( "dist.troll.no", port );
    cout << "connecting..." ; cout.flush();
}

void LicProcApp::ProcessFile( QString fileName )
{
    QString licenseShare( "\\\\lupinella\\axapta\\licensefiles" );

    cout << "Processing " << fileName.latin1() << endl; cout.flush();
    cout << "Opening database connection to \"dist.troll.no\""; cout.flush();
    if( distDB->open() ) {
	cout << " ... done" << endl; cout.flush();
	QDir licDir( licenseShare );
	if( licDir.exists( fileName ) ) {
	    QString tmpName( fileName + ".tmp" );
	    if( licDir.rename( fileName, tmpName ) ) {
		cout << "Renamed " << fileName.latin1() << " to " << tmpName.latin1() << endl; cout.flush();
		QFile inFile( licenseShare + "\\" + tmpName );
		if( inFile.open( IO_ReadOnly ) ) {
		    cout << "Opened " << tmpName.latin1() << endl; cout.flush();
		    QString currentLine;
		    if( inFile.readLine( currentLine, 1024 ) != -1 ) {
			if( ( currentLine != tag ) && !licenseList.isEmpty() )
			    updateDist( tag );
			tag = currentLine;
		    }
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
			QString usLicense = (*it++);
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
			    if( usLicense == "Y" )
				itemID += "-us";

			    QSqlCursor licenseCursor( "licenses" );
			    licenseCursor.select( "ID = " + licenseID );
			    bool licenseExists( false );
			    while( !licenseExists && licenseCursor.next() ) {
				// If there are records that pass the filter above, the license already exists
				licenseExists = true;
			    }
			    QSqlRecord* buffer;
			    if( !licenseExists )
				// If no license exists, create one
				buffer = licenseCursor.primeInsert();
			    else
				buffer = licenseCursor.primeUpdate();

			    buffer->setValue( "ID", licenseID );
			    buffer->setValue( "CustomerID", custID );
			    buffer->setValue( "Login", login );
			    buffer->setValue( "Password", password );
			    buffer->setValue( "Licensee", licensee );
			    buffer->setValue( "Email", licenseEmail );
			    buffer->setValue( "ExpiryDate", expiryDate );
			    if( !licenseExists )
				licenseCursor.insert();
			    else
				licenseCursor.update();

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
				if( usLicense == "Y" )
				    itemID += "-us";
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
		    cout << "Marked " << tmpName << " for delete." << endl; cout.flush();
		}
		else {
		    int err = errno;

		    cout << "Could not rename the file, error code " << err << strerror( err ) << endl; cout.flush();
		}
	    }
	}
    }
    else
	cout << " ... failed" << endl; cout.flush();
}
