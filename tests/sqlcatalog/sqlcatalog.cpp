#include <qstring.h>
#include <qsql.h>
#include <qsqldatabase.h>
#include <qsqlindex.h>
#include <qsqlfield.h>
#include <qsqlrowset.h>
#include <qsqlview.h>
#include <qvariant.h>
#include <qapplication.h>



int main( int argc, char** argv )
{
    qDebug("Qt SQL Catalog Test");
    QApplication app( argc, argv, FALSE );

    QSqlConnection::addDatabase( qApp->argv()[1],
				 qApp->argv()[2],
				 qApp->argv()[3],
				 qApp->argv()[4],
				 qApp->argv()[5]);
    QSqlDatabase* database = QSqlConnection::database();
    
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
	for ( uint j = 0; j < pk.count(); ++j )
	    qDebug("......" + pk.field(j).name() );
    }

    qDebug("Creating rowset...");
    QSqlView v( "key_test" );

    // insert a value
    v["id"] = 999;
    v["name"] = "xxxx";
    ASSERT( v.insert() == 1 );
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
    v["name"] = "Kaja";
    ASSERT( v.update( v.primaryIndex() ) == 1 );
    v.select( v.primaryIndex(), v.primaryIndex() );
    ASSERT( v.next() );
    qDebug("after next, ID:" + v["id"].toString() );
    qDebug("after next, name:" + v["name"].toString() );
    ASSERT( v["id"].toInt() == 999 );
    ASSERT( v["name"].toString() == "Kaja" );

    // delete the record
    v["id"] = 999;
    ASSERT( v.del( v.primaryIndex() ) == 1 );
    v.select( v.primaryIndex(), v.primaryIndex() );
    ASSERT( !v.next() );

    qDebug("Done.");
    return 0;
};

