#include "app.h"
#include <qsqlcursor.h>
#include <qregexp.h>
#include <qfile.h>
#include <qdatetime.h>

ImportApp::ImportApp( int argc, char** argv ) : QApplication( argc, argv )
{
    internDB = QSqlDatabase::addDatabase( "QMYSQL3", "intern" );
    internDB->setUserName( QString::null );
    internDB->setDatabaseName( "troll" );
    internDB->setPassword( QString::null );
    internDB->setHostName( "lupinella.troll.no" );
    axaptaDB = QSqlDatabase::addDatabase( "QOCI8", "axapta" );
    axaptaDB->setUserName( "erik" );
    axaptaDB->setDatabaseName( "axdb.troll.no" );
    axaptaDB->setPassword( "Alial77" );
    axaptaDB->setHostName( "minitrue.troll.no" );
}

void ImportApp::doImport()
{
    QStringList skippedIds;

    if( internDB->open() ) {
	if( axaptaDB->open() ) {
	    QSqlCursor internCursor( "License", true, internDB );
	    QSqlCursor axaptaCursor( "Licenses", true, axaptaDB );
	    QSqlCursor itemCursor( "InvoiceItem", true, internDB );

	    internCursor.select( "License.state != 'Inactive' and to_days( License.expiryDate ) >= to_days( now() )" );
	    internCursor.first();
	    while( internCursor.isValid() ) {
		QString tmp = "invoiceItemID = " + internCursor.value( "invoiceItemID" ).toString();
		itemCursor.select( tmp );
		itemCursor.first();
		if( itemCursor.isValid() ) {
		    QSqlRecord* buffer = axaptaCursor.primeInsert();
		    buffer->setValue( "LICENSEID", internCursor.value( "licenseID" ) );
		    buffer->setValue( "LICENSEE", internCursor.value( "Name" ) );
		    buffer->setValue( "LOGIN", internCursor.value( "login" ) );
		    buffer->setValue( "PASSWORD", internCursor.value( "password" ) );
		    buffer->setValue( "EXPIRYDATE", internCursor.value( "expiryDate" ) );
		    buffer->setValue( "EMAIL", internCursor.value( "email" ) );
		    buffer->setValue( "ITEMID", itemCursor.value( "productID" ) );
		    axaptaCursor.insert();
		}
		internCursor.next();
	    }
	}
    }
}
