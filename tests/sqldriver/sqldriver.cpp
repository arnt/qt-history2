#include <qstring.h>
#include <qsql.h>
#include <qsqlfield.h>
#include <qsqldatabase.h>
#include <qapplication.h>
#include <stdio.h>

#include <time.h>

#define TEST_RECS 100
#define TABLE_NAME QString("qsql_test")

void TestCreateTable()
{
    QString stmt(  "create table " + TABLE_NAME + \
		    "(id numeric(10) primary key,"
		    "field1 char(10),"
		    "field2 varchar(200),"
		    "field3 decimal(15,2),"
		    "field4 date);");
    QSqlDatabase* db = QSqlConnection::database();
    qDebug("Creating sample table...");
    db->exec( stmt );
    if ( db->lastError().type() != QSqlError::None  )
	qDebug("ERROR:" + db->lastError().databaseText());
    qDebug("Table field list:" );
    QSqlFieldList fl = QSqlConnection::database()->fields( TABLE_NAME );
    for ( uint i = 0; i <fl.count(); ++i )
	qDebug( fl.field(i).name() + (fl.field(i).isPrimaryIndex() ? QString(" primary index") : QString::null ) );
    qDebug("Done.");
}

void TestInsert()
{
    QSqlDatabase* database = QSqlConnection::database();
    qDebug("Inserting records...");
    if ( database->hasTransactionSupport() )
	database->transaction();
    for ( int i=0; i<TEST_RECS; ++i ) {
	int r = database->exec( "insert into " + TABLE_NAME + " values (" + QString::number(i) + ",'foo:" + QString::number(i) + "','blarg " + QString::number(i*10) + " blarg'," + QString::number(i*12.345) + ", '18-APR-1972');");
	ASSERT( r == 1 );
    }
    int r = database->exec( "insert into " + TABLE_NAME + " values (9999,NULL,'should have nulls',NULL,NULL);");
    if ( r != 1)
	qDebug("ERROR:" + database->lastError().databaseText());
    ASSERT( r == 1);
    if ( database->hasTransactionSupport() )
	database->commit();
    qDebug("Done.");	
}

void TestSelect()
{
    qDebug("Selecting records...");
    QSql recCount( "select count(1) from " + TABLE_NAME + ";");
    if ( !recCount.isActive() ) 
	qDebug("Query (" + recCount.query() + ") failed:" + recCount.lastError().databaseText());
    if ( recCount.next() )
	qDebug("Number of records (should be " + QString::number(TEST_RECS) + "+1):" + recCount[0].toString() );
    else
	qDebug("FAILED counting records: " + recCount.lastError().databaseText());

    QString selQuery( "select * from " + TABLE_NAME + ";" );
    QSql selRecs( selQuery );
    if ( !selRecs.isActive() ) 
	qDebug("Query (" + selRecs.query() + ") failed:" + selRecs.lastError().databaseText());	

    for ( int i=0; i<100; ++i ){
	while ( selRecs.next() )
	    ; // go to end
	if ( selRecs.isValid() )
	    qDebug("ERROR going to end of query."); 
    
	while( selRecs.previous() ) 
	    ; // back to beginning
   
	selRecs.next();
	if ( selRecs.at() != 0 )
	    qDebug("ERROR going to first record."); 

	selRecs.last();
	if ( selRecs.at() != selRecs.size()-1 )
	    qDebug("ERROR going to last record."); 
   
	selRecs.first();
	if ( selRecs.at() != 0 )
	    qDebug("ERROR going to first record."); 

	selRecs.seek( TEST_RECS/2 );
	if ( selRecs.at() != TEST_RECS/2 )
	    qDebug("ERROR going to middle record.");        
	
	// change the query to something else
	selRecs.setQuery( "update " + TABLE_NAME + " set id=1 where id=1;");
	// now change it back
	selRecs.setQuery( selQuery );	
    }
    qDebug("Done.");	   
}

void TestDelete()
{
    QSqlDatabase* database = QSqlConnection::database();
    qDebug("Deleting records...");
    if ( database->hasTransactionSupport() )
	database->transaction();
    for ( int i=0; i<TEST_RECS; ++i ) {
	database->exec( "delete from " + TABLE_NAME + " where id=" + QString::number(i) + ";" );
    }
    if ( database->hasTransactionSupport() )
	database->commit();
    QSql recCount( "select count(1) from " + TABLE_NAME + ";");
    if ( !recCount.isActive() ) 
	qDebug("Query (" + recCount.query() + ") failed:" + recCount.lastError().databaseText());
    if ( recCount.next() )
	qDebug("After delete, number of records (should be 1):" + recCount[0].toString() );
    else
	qDebug("FAILED counting records: " + recCount.lastError().databaseText());
    qDebug("Done.");    
}

void TestTrans()
{
    QSqlDatabase* database = QSqlConnection::database();
    if ( database->hasTransactionSupport() ) {
	qDebug("Testing transaction...");
	database->transaction();
	for ( int i=0; i<TEST_RECS; ++i ) {
	    database->exec( "insert into " + TABLE_NAME + " (id) values (" + QString::number(i) + ");");
	}
	if ( database->rollback() )
	    qDebug("Transaction successful.");
	else {
	    QSql recsRemaining("select count(1) from " + TABLE_NAME + ";");
	    recsRemaining.next();
    	    qDebug("ERROR in Transaction.  Records remaining in table:" + recsRemaining[0].toString() );
    	}
    } else 
	  qDebug("Driver does NOT have transaction support, skipping test.");
    qDebug("Done.");    
}

int main( int argc, char** argv )
{
    QApplication a( argc, argv, FALSE );
    qDebug("Qt SQL Driver Test");
    qDebug("++++++++++++++");

    qDebug("Creating connection...");
    QSqlConnection::addDatabase( qApp->argv()[1],
				 qApp->argv()[2],
				 qApp->argv()[3],
				 qApp->argv()[4],
				 qApp->argv()[5]);
    qDebug("++++++++++++++");
    TestCreateTable();
    qDebug("++++++++++++++");    
    TestInsert();
    qDebug("++++++++++++++");    
    TestSelect();
    qDebug("++++++++++++++");    
    TestDelete();
    qDebug("++++++++++++++");    
    TestTrans();
    qDebug("++++++++++++++");
    
    qDebug("Dropping sample table...");
    QSqlConnection::database()->exec( "drop table " + TABLE_NAME + ";" );
    qDebug("++++++++++++++");    
    qDebug("Done.");
    return 0;
};
