#include <qstring.h>
#include <qsql.h>
#include <qsqldatabase.h>
#include <qsqlindex.h>
#include <qsqlfield.h>
#include <qsqlrowset.h>
#include <qsqlview.h>
#include <qapplication.h>

QSqlDatabase* database;

int main( int argc, char** argv )
{
    qDebug("Qt SQL Catalog Test");
    QApplication app( argc, argv );

    database = new QSqlDatabase( qApp->argv()[1] );
    database->reset( qApp->argv()[2], qApp->argv()[3], qApp->argv()[4], qApp->argv()[5]);

    qDebug("Opening database...");
    database->open();
    if ( database->isOpen() )
	qDebug("...success.");
    else
	qDebug("...FAILED.");

    uint i;
    qDebug("Getting list of tables and fields...");
    QStringList tables = database->tables();
    qDebug("Table count:" + QString::number( tables.count()) );
    for ( i = 0; i < tables.count(); ++i ) {
	qDebug( "..." + tables[i] );
	QSqlFieldList fil = database->fields( tables[i] );
	for ( uint j = 0; j < fil.count(); ++j )
	    qDebug("......" + fil.field(j).name() );
    }

    qDebug("Getting list of table primary index...");
    for ( i = 0; i < tables.count(); ++i ) {
	QSqlIndex pk = database->primaryIndex( tables[i] );
	qDebug( "..." + tables[i] );
	QSqlFieldList fil = pk.fields();
	for ( uint j = 0; j < fil.count(); ++j )
	    qDebug("......" + fil.field(j).name() );
    }

    qDebug("Creating rowset...");
    QSqlView v(database, "key_test");
    v["id"] = 999;
    v["name"] = "Dave";
    v.insert();
    v.select( v.primaryIndex() );
    ASSERT( v.next() );
    ASSERT( v["id"] == 999 );
    v.del();
    v.select( v.primaryIndex() );
    ASSERT( !v.next() );
    
    qDebug("Closing database...");
    database->close();
    if ( !database->isOpen() )
	qDebug("...success.");
    delete database;
    qDebug("Done.");
    return 0;
};
