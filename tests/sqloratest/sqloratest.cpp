#include <qstring.h>
#include <qsqlquery.h>
#include <qsqldatabase.h>
#include <qsqldriver.h>
#include <qapplication.h>
#include <stdio.h>

#include <time.h>

#define TEST_RECS 10

QSqlDatabase* database;

void TestInsert()
{
    if ( database->driver()->hasTransactionSupport() ) {
	database->transaction();
    }
    for ( int i=0; i<TEST_RECS; ++i ) {
	QSqlQuery r = database->exec( "insert into qsql_test values (" + QString::number(i) + ",'foo:" + QString::number(i) + "','blarg " + QString::number(i*10) + " blarg'," + QString::number(i*12.345) + ", '18-APR-1972');");
	Q_ASSERT( r.numRowsAffected() == 1 );
    }
    QSqlQuery r = database->exec( "insert into qsql_test values (NULL,NULL,'should have nulls',NULL,NULL);");
    Q_ASSERT( r.numRowsAffected() == 1);
    if ( database->driver()->hasTransactionSupport() )
	database->commit();
}

void TestSelect()
{
    printf("Selecting records...\n");
//    QSql recCount = database->query("select count(1) from qsql_test;");
//    if ( !recCount.isActive() ) {
//	qDebug("Query failed:" + recCount.lastError().databaseText());
//    }
//    if ( recCount.next() )
//	qDebug("Number of records:" + recCount[0] );
//    else
//	qDebug("FAILED counting records: " + recCount.lastError().databaseText());

    QSqlQuery selRecs = database->exec( "select id, field1, field2, field3, field4 from qsql_test;");
    if ( !selRecs.isActive() ) {
	qDebug(selRecs.lastError().databaseText());
    }
    int i = 0;
    while( selRecs.next() ) {
	i++;
	qDebug("data:" + selRecs.value(0).toString() + " " + selRecs.value(1).toString() +
	       " " + selRecs.value(2).toString() + " " + selRecs.value(3).toString() +
	       " " + selRecs.value(4).toString());
	if ( selRecs.isNull(0) )
	    qDebug("field1 IS NULL");
    }
    qDebug("records selected:" + QString::number(i));
    qDebug("done with select");
//    while( selRecs->previous() ) {
////	qDebug("prev:" + QString::number(selRecs->at()));
//	QString dummy = selRecs[0];
//    }
//    if ( selRecs->at() != QSqlResult::BeforeFirst )
//	qDebug("selection FAILURE");
//    const QSqlResultInfo* info = selRecs->info();
//    if ( info ) {
//	qDebug("Random selections...");
//	selRecs->last();
//	if ( selRecs->at() != info->size()-1 )
//	    qDebug("last() FAILURE");
//	selRecs->first();
//	if ( selRecs->at() != 0 )
//	    qDebug("first() FAILURE, at:" + QString::number(selRecs->at()));
//	selRecs->seek( TEST_RECS/2 );
//	if ( selRecs->at() != TEST_RECS/2 )
//	    qDebug("seek() FAILURE, at:" + QString::number(selRecs->at()));
//	selRecs->seek(9000);
//	qDebug("After making bad seek, at:" + QString::number(selRecs->at()));
//    }
}

void TestDelete()
{
    qDebug("Deleting records...");
    if ( database->driver()->hasTransactionSupport() )
	database->transaction();
    for ( int i=0; i<TEST_RECS; ++i ) {
	database->exec( "delete from qsql_test where id=" + QString::number(i) + ";" );
    }
    if ( database->driver()->hasTransactionSupport() )
	database->commit();
}

void TestTrans()
{
    if ( database->driver()->hasTransactionSupport() ) {
	qDebug("Testing transaction...");
	database->transaction();
	for ( int i=0; i<TEST_RECS; ++i ) {
	    database->exec( "insert into qsql_test values (" + QString::number(i) + ",'foo:" + QString::number(i) + "','blarg " + QString::number(i*10) + " blarg'," + QString::number(i*12.345) + ", '18-APR-1972');");
	}
	if ( database->rollback() )
	    qDebug("Transaction successful.");
	else {
	    QSqlQuery recsRemaining = database->exec("select count(1) from qsql_test;");
	    recsRemaining.next();
	    qDebug("Transaction FAILURE.  Records remaining in table:" + recsRemaining.value(0).toString() );
	}
    }
}

int main()
{
    printf("Qt Oracle Test\n\n");

    printf("Opening database...\n");
    database = QSqlDatabase::addDatabase( "QOCI8" );
    database->setDatabaseName( "trol" );
    database->setUserName( "scott" );
    database->setPassword( "tiger" );
    database->setHostName( "anarki" );
    database->open();

    if ( database->isOpen() )
	qDebug("...success.");
    else
	qDebug("...FAILED.");

    qDebug("Creating sample table...");
    database->exec( "create table qsql_test"
		    "(id number(10),"
		    "field1 char(10),"
		    "field2 varchar(200),"
		    "field3 decimal(15,2),"
		    "field4 date);");
    qDebug("creating list of database tables...");
    QStringList tables = database->tables();
    for ( uint i = 0; i < tables.count(); ++i )
	qDebug( tables[i] );

    TestInsert();
    TestSelect();
    TestDelete();
    TestTrans();

    qDebug("Dropping sample table...");
    database->exec( "drop table qsql_test;" );

    qDebug("Closing database...");
    database->close();
    if ( !database->isOpen() )
	qDebug("...success.");
    delete database;
    qDebug("Done.");
    return 0;
};
