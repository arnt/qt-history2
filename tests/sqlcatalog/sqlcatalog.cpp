#include <qstring.h>
#include <qsqlquery.h>
#include <qsqldatabase.h>
#include <qsqlindex.h>
#include <qsqlfield.h>
#include <qsqlcursor.h>
#include <qvariant.h>
#include <qapplication.h>

int main( int argc, char** argv )
{
    qDebug("Qt SQL Catalog Test");
    QApplication app( argc, argv, FALSE );
    
    QSqlDatabase* database = QSqlDatabase::addDatabase( qApp->argv()[1] );
    database->setDatabaseName( qApp->argv()[2] );
    database->setUserName( qApp->argv()[3] );
    database->setPassword( qApp->argv()[4] );
    database->setHostName( qApp->argv()[5] );
    database->open();
    
    uint i;
    qDebug("Getting list of tables and fields...");
    QStringList tables = database->tables();
    qDebug("Table count:" + QString::number( tables.count()) );
    for ( i = 0; i < tables.count(); ++i ) {
	qDebug( "..." + tables[i] );
	QSqlRecord fil = database->record( tables[i] );
	for ( uint j = 0; j < fil.count(); ++j ) {
	    qDebug("......" + fil.field(j)->name() );
	    if ( fil.field(j)->isPrimaryIndex() )
		qDebug(".........PRIMARY INDEX" );
	}
    }

    qDebug("Creating rowset...");
    QSqlCursor v( "key_test" );

    // insert a value
    v.clearValues( TRUE );
    v["id"] = 999;
    v["name"] = "xxxx";
    if ( v.insert() != 1 )
	qDebug("ERROR:" + v.lastError().databaseText());
    qDebug("after insert, ID:" + v["id"].toString() );
    qDebug("after insert, name:" + v["name"].toString() );

    v.select( v.primaryIndex(), v.primaryIndex() );
    ASSERT( v.next() );
    qDebug("after next, ID:" + v["id"].toString() );
    qDebug("after next, name:" + v["name"].toString() );

    ASSERT( v["id"].toInt() == 999 );
    ASSERT( QString(v["name"].toString()) == QString("xxxx") );
    qDebug("name:" + v["name"].toString());

    // put back a new value
    v["name"] = "barf";
    ASSERT( v.update( v.primaryIndex() ) == 1 );
    v.select( v.primaryIndex(), v.primaryIndex() );
    ASSERT( v.next() );
    qDebug("after next, ID:" + v["id"].toString() );
    qDebug("after next, name:" + v["name"].toString() );
    ASSERT( v["id"].toInt() == 999 );
    ASSERT( v["name"].toString() == "barf" );

    // delete the record
    v["id"] = 999;
    ASSERT( v.del( v.primaryIndex() ) == 1 );
    v.select( v.primaryIndex(), v.primaryIndex() );
    ASSERT( !v.next() );

    qDebug("Done.");
    return 0;
};

