#include <qstring.h>
#include <qsqlquery.h>
#include <qsqldatabase.h>
#include <qsqlindex.h>
#include <qsqlfield.h>
#include <qsqlcursor.h>
#include <qvariant.h>
#include <iostream.h>
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
	    qDebug("......" + fil.field(j)->name() + " " + QString( fil.field(j)->value().typeName() ) );
	    if ( database->primaryIndex( tables[i] ).field( fil.field(j)->name() ) )
		qDebug(".........PRIMARY INDEX" );
	}
    }

    qDebug("Done.");
    return 0;
       
    
    
    /* db test code    
       
        //    QSqlQuery q( "select * from cdef$;" ); // works
    
    //    QSqlQuery q( "select CON#, CONDITION, OBJ#, CONDITION from CDEF$;" ); works
    
    //    QSqlQuery q( "select COLUMNS, D_OBJ#, ORDER#, TYPES from ACCESS$;" ); dies
    
    QSqlQuery q( "select TOID  from ATTRIBUTE$;" );
    
    uint count = database->record( q ).count();
    while( q.next() ) {
	for ( uint i = 0; i < count; ++i )
	    std::cout << " " << q.value(i).toString();
	std::cout << std::endl;
    }
    
    return 0;
    
    QSqlCursor c ( "CDEF$" );
    c.select();
    while( c.next() ) {
	for ( uint i = 0; i < c.count(); ++i )
	    std::cout << c.value(i).toString();
	std::cout << std::endl;
    }
    return 0; 

    */
    
    
    
};

