#include <qstring.h>
#include <qsqlfield.h>
#include <qsqldatabase.h>
#include <qsqldriver.h>
#include <qsqlcursor.h>
#include <qapplication.h>
#include <qimage.h>
#include <qcstring.h>
#include <stdio.h>

#include <time.h>

#define TEST_RECS 10
#define TABLE_NAME QString("qsql_test")

void TestCreateTable()
{
    QString stmt(  "create table " + TABLE_NAME + \
		    "(id numeric(10) primary key,"
		    "field1 char(10),"
		    "field2 varchar(200),"
		    "field3 decimal(15,2),"
		    "field4 date);");
    QSqlDatabase* db = QSqlDatabase::database();
    qDebug("Creating sample table...");
    db->exec( stmt );
    if ( db->lastError().type() != QSqlError::None  )
	qDebug("ERROR:" + db->lastError().databaseText());
    qDebug("Table field list:" );
    QSqlRecord fl = QSqlDatabase::database()->record( TABLE_NAME );
    for ( uint i = 0; i <fl.count(); ++i )
	qDebug( fl.field(i)->name() + (fl.field(i)->isPrimaryIndex() ? QString(" primary index") : QString::null ) );
    qDebug("Done.");
}

void TestInsert()
{
    QSqlDatabase* database = QSqlDatabase::database();
    qDebug("Inserting records...");
    if ( database->driver()->hasTransactionSupport() )
	database->transaction();
    for ( int i=0; i<TEST_RECS; ++i ) {
	QSqlQuery r = database->exec( "insert into " + TABLE_NAME + " values (" + QString::number(i) + ",'foo:" + QString::number(i) + "','blarg " + QString::number(i*10) + " blarg'," + QString::number(i*12.345) + ", '18-APR-1972');");
	Q_ASSERT( r.numRowsAffected() == 1 );
    }
    QSqlQuery r = database->exec( "insert into " + TABLE_NAME + " values (9999,NULL,'should have nulls',NULL,NULL);");
    if ( r.numRowsAffected() != 1)
	qDebug("ERROR:" + database->lastError().databaseText());
    Q_ASSERT( r.numRowsAffected() == 1);
    if ( database->driver()->hasTransactionSupport() )
	database->commit();
    qDebug("Done.");
}

void TestSelect()
{
    qDebug("Selecting records...");
    QSqlDatabase* database = QSqlDatabase::database();
    QSqlQuery recCount( "select count(1) from " + TABLE_NAME + ";");
    if ( !recCount.isActive() )
	qDebug("Query (" + recCount.lastQuery() + ") failed:" + recCount.lastError().databaseText());
    if ( recCount.next() )
	qDebug("Number of records (should be " + QString::number(TEST_RECS) + "+1):" + recCount.value(0).toString() );
    else
	qDebug("FAILED counting records: " + recCount.lastError().databaseText());

    QString selQuery( "select * from " + TABLE_NAME + ";" );
    QSqlQuery selRecs( selQuery );
    if ( !selRecs.isActive() )
	qDebug("Query (" + selRecs.lastQuery() + ") failed:" + selRecs.lastError().databaseText());

    qDebug("Random selection test..." + selRecs.lastQuery() );
    for ( int i=0; i<TEST_RECS; ++i ){
	while ( selRecs.next() )
	    ; // go to end
	if ( selRecs.isValid() )
	    qDebug("ERROR going to end of query.");
	qDebug("...after going to end");

	while( selRecs.prev() )
	    ; // back to beginning
	qDebug("...after going back to beginning");

	selRecs.next();
	if ( selRecs.at() != 0 )
	    qDebug("ERROR going to first record.");
	qDebug("...back to first record");

	selRecs.last();
	if ( database->driver()->hasQuerySizeSupport() && selRecs.at() != selRecs.size()-1 )
	    qDebug("ERROR going to last record.");
	qDebug("...back to end");

	selRecs.first();
	if ( selRecs.at() != 0 )
	    qDebug("ERROR going to first record.");
	qDebug("...back to first");

	selRecs.seek( TEST_RECS/2 );
	if ( selRecs.at() != TEST_RECS/2 )
	    qDebug("ERROR going to middle record.");
	qDebug("...to the middle");

	// change the query to something else
	selRecs.exec( "update " + TABLE_NAME + " set id=1 where id=1;");
	qDebug("...after changed query");

	// now change it back
	selRecs.exec( selQuery );
	qDebug("...after restoring original query");
    }
    qDebug("Testing custom cursor...");
    QSqlCursor v( TABLE_NAME, FALSE );
    QSqlField f1("field1", 1, QVariant::String );
    v.append( f1 );
    QSqlField f2("field3", 2, QVariant::Double );
    v.append( f2 );
    v.select();
    while ( v.next() )
	;

    qDebug("Done.");
}

void TestDelete()
{
    QSqlDatabase* database = QSqlDatabase::database();
    qDebug("Deleting records...");
    if ( database->driver()->hasTransactionSupport() )
	database->transaction();
    for ( int i=0; i<TEST_RECS; ++i ) {
	database->exec( "delete from " + TABLE_NAME + " where id=" + QString::number(i) + ";" );
    }
    if ( database->driver()->hasTransactionSupport() )
	database->commit();
    QSqlQuery recCount( "select count(1) from " + TABLE_NAME + ";");
    if ( !recCount.isActive() )
	qDebug("Query (" + recCount.lastQuery() + ") failed:" + recCount.lastError().databaseText());
    if ( recCount.next() )
	qDebug("After delete, number of records (should be 1):" + recCount.value(0).toString() );
    else
	qDebug("FAILED counting records: " + recCount.lastError().databaseText());
    qDebug("Done.");
}

void TestTrans()
{
    QSqlDatabase* database = QSqlDatabase::database();
    if ( database->driver()->hasTransactionSupport() ) {
	qDebug("Testing transaction...");
	QSqlQuery recsRemaining("select count(1) from " + TABLE_NAME + ";");
	recsRemaining.next();
	qDebug("Records in table:" + recsRemaining.value(0).toString() );

	database->transaction();
	for ( int i=0; i<TEST_RECS; ++i ) {
	    database->exec( "insert into " + TABLE_NAME + " (id) values (" + QString::number(i) + ");");
	}
	if ( database->rollback() ) {
	    qDebug("Transaction successful.");
	    QSqlQuery recsRemaining("select count(1) from " + TABLE_NAME + ";");
	    recsRemaining.next();
    	    qDebug("Records remaining in table:" + recsRemaining.value(0).toString() );
	}
	else {
	    QSqlQuery recsRemaining("select count(1) from " + TABLE_NAME + ";");
	    recsRemaining.next();
    	    qDebug("ERROR in Transaction.  Records remaining in table:" + recsRemaining.value(0).toString() );
    	}
    } else
	  qDebug("Driver does NOT have transaction support, skipping test.");
    qDebug("Done.");
}

void qt_debug_buffer( const QString& msg, QSqlCursor* cursor )
{
    qDebug("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    qDebug(msg);
    for ( uint j = 0; j < cursor->count(); ++j ) {
	qDebug(cursor->field(j)->name() + " type:" + QString(cursor->field(j)->value().typeName()) + " value:" + cursor->field(j)->value().toString() );
    }
}


int main( int argc, char** argv )
{
    QApplication a( argc, argv, FALSE );
    qDebug("Qt SQL Driver Test");
    qDebug("++++++++++++++");

    qDebug("Creating connection...");
    QSqlDatabase* db = QSqlDatabase::addDatabase( qApp->argv()[1] );
    db->setDatabaseName( qApp->argv()[2] );
    db->setUserName( qApp->argv()[3] );
    db->setPassword( qApp->argv()[4] );
    db->setHostName( qApp->argv()[5] );
    db->open();

    
    db->exec("create table b"
	     "(id number primary key,"
	     "l long raw);");
    
    QSqlCursor cursor( "b" );
    
    cursor["ID"] = 1;
    QImage img("/tmp/radio_t.xpm", "XPM" );
    QByteArray b;
    b.duplicate( (char*)img.bits(), img.numBytes() );
    cursor["L"] = b;
    cursor.insert();
    
    cursor.select();
    while( cursor.next() ) {
	qt_debug_buffer("", &cursor );
	QByteArray ba = cursor["L"].toByteArray();
	qDebug("Binary data size:" + QString::number(ba.size()));
	QString res;
	static const char hexchars[] = "0123456789abcdef";
	for ( uint i = 0; i < ba.size(); ++i ) {
	    uchar s = (uchar) ba[i];
	    res += hexchars[s >> 4];
	    res += hexchars[s & 0x0f];
	}
	qDebug("hex data:" +  res );
    }
    
    QSqlQuery q("select l from b;");
    while ( q.next() )
	qDebug("rawtohex:" + q.value(0).toString() );
        
    db->exec("drop table b;");
    return 0;
    
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
    db->exec( "drop table " + TABLE_NAME + ";" );
    qDebug("++++++++++++++");
    qDebug("Done.");
    return 0;
};
