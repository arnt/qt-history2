#include <qstring.h>
#include <qsql.h>
#include <qsqldatabase.h>
#include <qsqlindex.h>
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
	QSqlFieldInfoList fil = database->fields( tables[i] );
	for ( uint j = 0; j < fil.count(); ++j )
	    qDebug("......" + fil[j].name );
    }

    qDebug("Getting list of table primary index...");
    for ( i = 0; i < tables.count(); ++i ) {
	QSqlIndex pk = database->primaryIndex( tables[i] );
	qDebug( "..." + tables[i] );
	QSqlFieldInfoList fil = pk.fields();
	for ( uint j = 0; j < fil.count(); ++j )
	    qDebug("......" + fil[j].name );
    }

    qDebug("Closing database...");
    database->close();
    if ( !database->isOpen() )
	qDebug("...success.");
    delete database;
    qDebug("Done.");
    return 0;
};
