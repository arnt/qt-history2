#include "app.h"
#include <qsqlcursor.h>
#include <qregexp.h>
#include <qfile.h>

#define DATAAREA "nor"

ImportApp::ImportApp( int argc, char** argv ) : QApplication( argc, argv )
{
    internDB = QSqlDatabase::addDatabase( "QMYSQL3", "intern" );
    internDB->setUserName( QString::null );
    internDB->setDatabaseName( "troll" );
    internDB->setPassword( QString::null );
    internDB->setHostName( "lupinella.troll.no" );
    axaptaDB = QSqlDatabase::addDatabase( "QOCI8", "axapta" );
    axaptaDB->setUserName( "bmssa" );
    axaptaDB->setDatabaseName( "axdb.troll.no" );
    axaptaDB->setPassword( "bmssa_pwd" );
    axaptaDB->setHostName( "minitrue.troll.no" );
}

void ImportApp::doImport()
{
    QStringList skippedIds;

    if( internDB->open() ) {
	if( axaptaDB->open() ) {
	    QFile logFile( "skipped.log" );
	    if( logFile.open( IO_WriteOnly | IO_Translate ) ) {
		QTextStream log( &logFile );

		QSqlCursor internCursor( "tempCustomer", true, internDB );

		internCursor.select();
		internCursor.first();
		while( internCursor.isValid() ) {
		    QString customerID = internCursor.value( "customerid" ).toString();
		    QString customerName = internCursor.value( "customername" ).toString();
		    QString customerAddress = internCursor.value( "address" ).toString();

		    QSqlCursor axaptaCursor( "CustTable", true, axaptaDB );

		    axaptaCursor.select( "Name = '" + customerName.left( 30 ) + "' and Address = '" + customerAddress + "' and dataareaid = '" + DATAAREA + "'" );
		    axaptaCursor.first();
		    if( axaptaCursor.isValid() ) {
			    // We found a record, hooray :)
			    QSqlQuery q( QString::null, axaptaDB );
			    QString tmp = "UPDATE CUSTTABLE SET INTERNID = '" + customerID + "' WHERE DATAAREAID = '" + DATAAREA + "' AND ACCOUNTNUM = '" + axaptaCursor.value( "ACCOUNTNUM" ).toString() + "'";
			    bool b = q.exec( tmp );
			    if( b )
				log << customerID << " ... OK" << endl;
			    else
				log << customerID << " ... FAILED (SQL error)" << endl;
		    }
		    else {
			axaptaCursor.select( "Name = '" + customerName + "' and dataareaid = 'ts3'" );
			axaptaCursor.first();
			if( axaptaCursor.isValid() ) {
			    // We found a record, hooray :)
			    QSqlQuery q( QString::null, axaptaDB );
			    QString tmp = "UPDATE CUSTTABLE SET INTERNID = '" + customerID + "' WHERE DATAAREAID = '" + DATAAREA + "' AND ACCOUNTNUM = '" + axaptaCursor.value( "ACCOUNTNUM" ).toString() + "'";
			    bool b = q.exec( tmp );
			    if( b ) {
				QString simpleAxaptaAddress = axaptaCursor.value( "ADDRESS" ).toString();
				QString simpleInternAddress = customerAddress.left( 30 );

				simpleAxaptaAddress.replace( QRegExp( "[\r\n ]" ), " " );
				simpleInternAddress.replace( QRegExp( "[\r\n ]" ), " " );
				if( simpleAxaptaAddress == simpleInternAddress ) {
				    // The addresses resolve to the same when the whitespace is processed
				    log << customerID << "... OK" << endl;
				}
				else
				    log << customerID << " ... MAYBE (Name OK, but address mismatch) A: \"" << simpleAxaptaAddress << "\" I: \"" << simpleInternAddress << "\"" << endl;
			    }
			    else
				log << customerID << " ... FAILED (SQL error)" << endl;
			}
			else
			    log << customerID << " ... FAILED (\"" << customerName << "\" not found)" << endl;
		    }
		    logFile.flush();
		    internCursor.next();
		}
		logFile.close();
	    }
	}
    }
}
