#include <qstring.h>
#include <qsql.h>
#include <qsqldatabase.h>
#include <qapplication.h>
#include <stdio.h>

#include <time.h>

#define TEST_RECS 10

QSqlDatabase* database;

void TestInsert()
{
    qDebug("Inserting records...");
    if ( database->hasTransactionSupport() )
	database->transaction();
    for ( int i=0; i<TEST_RECS; ++i ) {
	int r = database->exec( "insert into qsql_test values (" + QString::number(i) + ",'foo:" + QString::number(i) + "','blarg " + QString::number(i*10) + " blarg'," + QString::number(i*12.345) + ", '18-APR-1972');");
	ASSERT( r == 1 );
    }
    if ( database->hasTransactionSupport() )
	database->commit();
}

void TestView()
{
    clock_t start = clock();
    QSqlTable t = database->table("qsql_test");
    clock_t finish = clock();
    qDebug("Time to create table view:" + QString::number(((double)(finish - start) / CLOCKS_PER_SEC )));
    QSqlFieldInfoList l = t.fields();
    for (uint i=0; i<l.count(); ++i)
	qDebug("Field:" + l[i].name + " type:" + QString::number(l[i].type) + " length:" + QString::number(l[i].length) + " prec:" + QString::number(l[i].precision) );
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

    QSql selRecs = database->query( "select id, field1, field2, field3, field4 from qsql_test;");
    if ( !selRecs.isActive() ) {
	qDebug(selRecs.lastError().databaseText());
    }
    int i = 0;
    while( selRecs.next() ) {
	i++;
	qDebug("data:" + selRecs[0].toString() + " " + selRecs[1].toString() + " " + selRecs[2].toString() + " " + selRecs[3].toString() + " " + selRecs[4].toString());
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
    if ( database->hasTransactionSupport() )
	database->transaction();
    for ( int i=0; i<TEST_RECS; ++i ) {
	database->exec( "delete from qsql_test where id=" + QString::number(i) + ";" );
    }
    if ( database->hasTransactionSupport() )
	database->commit();
}

void TestTrans()
{
    if ( database->hasTransactionSupport() ) {
	qDebug("Testing transaction...");
	database->transaction();
	for ( int i=0; i<TEST_RECS; ++i ) {
	    database->exec( "insert into qsql_test values (" + QString::number(i) + ",'" + QString::number(i) + "');");
	}
	if ( database->rollback() )
	    qDebug("Transaction successful.");
	else {
	    QSql recsRemaining = database->query("select count(1) from qsql_test;");
	    recsRemaining.next();
    	    qDebug("Transaction FAILURE.  Records remaining in table:" + recsRemaining[0].toString() );
    	}
    }
}

int main()
{
    printf("Qt Oracle Test\n\n");

    printf("Opening database...\n");
    database = new QSqlDatabase( "QOCI" );
    database->reset("test", "iceblink\\db", "db", "iceblink");
    database->open();
    if ( database->isOpen() )
	qDebug("...success.");
    else
	qDebug("...FAILED.");

    qDebug("Creating sample table...");
    int count = database->exec( "create table qsql_test"
		    "(id number(10),"
		    "field1 char(10),"
		    "field2 varchar(200),"
		    "field3 decimal(15,2),"
		    "field4 date);");
    qDebug("create table result:" + QString::number(count));

    TestView();
    TestInsert();
    TestSelect();
    TestDelete();
//    TestTrans();
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