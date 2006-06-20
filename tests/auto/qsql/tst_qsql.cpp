/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <qsqlquery.h>
#include <q3sqlcursor.h>
#include <qsqlrecord.h>
#include <qsql.h>
#include <qsqlresult.h>
#include <qsqldriver.h>

#include <private/qsqlnulldriver_p.h>

#include "../qsqldatabase/tst_databases.h"

//TESTED_FILES=

class tst_QSql : public QObject
{
Q_OBJECT

public:
    tst_QSql();
    virtual ~tst_QSql();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void open();
    void openInvalid();
    void registerSqlDriver();

    // problem specific tests
    void openErrorRecovery();
    void concurrentAccess();
    void basicDriverTest();
};

/****************** General Qt SQL Module tests *****************/

tst_QSql::tst_QSql()
{
}

tst_QSql::~tst_QSql()
{
}

void tst_QSql::initTestCase()
{
}

void tst_QSql::cleanupTestCase()
{
}

void tst_QSql::init()
{
}

void tst_QSql::cleanup()
{
}


// this is a very basic test for drivers that cannot create/delete tables
// it can be used while developing new drivers,
// it's original purpose is to test ODBC Text datasources that are basically
// to stupid to do anything more advanced than SELECT/INSERT/UPDATE/DELETE
// the datasource has to have a table called "qtest_basictest" consisting
// of a field "id"(integer) and "name"(char/varchar).
void tst_QSql::basicDriverTest()
{
    int argc = 0;
    QApplication app( argc, 0, FALSE );
    tst_Databases dbs;
    dbs.open();

    for ( QStringList::Iterator it = dbs.dbNames.begin(); it != dbs.dbNames.end(); ++it ) {
	QSqlDatabase db = QSqlDatabase::database( *it );
	Q_ASSERT( db.isValid() );

	QStringList tables = db.tables();
	QString tableName;
	if ( tables.contains( "qtest_basictest.txt" ) ) {
	    tableName = "qtest_basictest.txt";
	} else if ( tables.contains( "qtest_basictest" ) ) {
	    tableName = "qtest_basictest";
	} else if ( tables.contains( "QTEST_BASICTEST" ) ) {
	    tableName = "QTEST_BASICTEST";
	} else {
	    QVERIFY( 1 );
	    continue;
	}
	qDebug( "Testing: " + tst_Databases::dbToString( db ) );
	QSqlRecord* rec = 0;

	QSqlRecord rInf = db.record( tableName );
	QCOMPARE( (int)rInf.count(), 2 );
	QCOMPARE( rInf.fieldName(0).lower(), QString( "id" ) );
	QCOMPARE( rInf.fieldName(1).lower(), QString( "name" ) );

	Q3SqlCursor cur( tableName, TRUE, db );
	QVERIFY2( cur.select(), tst_Databases::printError( cur.lastError() ) );
	QCOMPARE( (int)cur.count(), 2 );
	QCOMPARE( cur.fieldName( 0 ).lower(), QString( "id" ) );
	QCOMPARE( cur.fieldName( 1 ).lower(), QString( "name" ) );

	rec = cur.primeDelete();
	rec->setGenerated( 0, FALSE );
	rec->setGenerated( 1, FALSE );
	QVERIFY2( cur.del(), tst_Databases::printError( cur.lastError() ) );
	QVERIFY2( cur.select(), tst_Databases::printError( cur.lastError() ) );
	QCOMPARE( cur.at(), int(QSql::BeforeFirst) );
	QVERIFY( !cur.next() );
	rec = cur.primeInsert();
	rec->setValue( 0, 1 );
	rec->setValue( 1, QString( "Harry" ) );
	QVERIFY2( cur.insert( FALSE ), tst_Databases::printError( cur.lastError() ) );
	rec = cur.primeInsert();
	rec->setValue( 0, 2 );
	rec->setValue( 1, QString( "Trond" ) );
	QVERIFY2( cur.insert( TRUE ), tst_Databases::printError( cur.lastError() ) );
	QVERIFY2( cur.select( cur.index( QString("id") ) ), tst_Databases::printError( cur.lastError() ) );
	QVERIFY2( cur.next(), tst_Databases::printError( cur.lastError() ) );
	QCOMPARE( cur.value( 0 ).toInt(), 1 );
	QCOMPARE( cur.value( 1 ).toString().stripWhiteSpace(), QString( "Harry" ) );
	QVERIFY2( cur.next(), tst_Databases::printError( cur.lastError() ) );
	QCOMPARE( cur.value( 0 ).toInt(), 2 );
	QCOMPARE( cur.value( 1 ).toString().stripWhiteSpace(), QString( "Trond" ) );
	QVERIFY( !cur.next() );
	QVERIFY2( cur.first(), tst_Databases::printError( cur.lastError() ) );
	rec = cur.primeUpdate();
	rec->setValue( 1, QString( "Vohi" ) );
	QVERIFY2( cur.update( TRUE ), tst_Databases::printError( cur.lastError() ) );
	QVERIFY2( cur.select( "id = 1" ), tst_Databases::printError( cur.lastError() ) );
	QVERIFY2( cur.next(), tst_Databases::printError( cur.lastError() ) );
	QCOMPARE( cur.value( 0 ).toInt(), 1 );
	QCOMPARE( cur.value( 1 ).toString().stripWhiteSpace(), QString( "Vohi" ) );
    }
    dbs.close();
    QVERIFY(1); // make sure the test doesn't fail if no database drivers are there
}

// make sure that the static stuff will be deleted
// when using multiple QApplication objects
void tst_QSql::open()
{
    int i;
    int argc = 0;
    int count = -1;
    for ( i = 0; i < 10; ++i ) {

	QApplication app( argc, 0, FALSE );
	tst_Databases dbs;

	dbs.open();
	if ( count == -1 )
	    // first iteration: see how many dbs are open
	    count = (int) dbs.dbNames.count();
	else
	    // next iterations: make sure all are opened again
	    QCOMPARE( count, (int)dbs.dbNames.count() );
	dbs.close();
    }
}

void tst_QSql::openInvalid()
{
    QSqlDatabase db;
    QVERIFY(!db.open());

    QSqlDatabase db2 = QSqlDatabase::addDatabase("doesnt_exist_will_never_exist", "blah");
    QVERIFY(!db2.open());
}

void tst_QSql::concurrentAccess()
{
    int argc = 0;
    QApplication app( argc, 0, FALSE );
    tst_Databases dbs;

    dbs.open();
    for ( QStringList::Iterator it = dbs.dbNames.begin(); it != dbs.dbNames.end(); ++it ) {
	QSqlDatabase db = QSqlDatabase::database( *it );
	QVERIFY( db.isValid() );
        if (tst_Databases::isMSAccess(db))
            continue;

	QSqlDatabase ndb = QSqlDatabase::addDatabase( db.driverName(), "tst_QSql::concurrentAccess" );
	ndb.setDatabaseName( db.databaseName() );
	ndb.setHostName( db.hostName() );
	ndb.setPort( db.port() );
	ndb.setUserName( db.userName() );
	ndb.setPassword( db.password() );
	QVERIFY2( ndb.open(),
		tst_Databases::printError( ndb.lastError() ) );

	QCOMPARE( db.tables(), ndb.tables() );
    }
    // no database servers installed - don't fail
    QVERIFY(1);
    dbs.close();
}

void tst_QSql::openErrorRecovery()
{
    int argc = 0;
    QApplication app( argc, 0, FALSE );
    tst_Databases dbs;

    dbs.addDbs();
    if (dbs.dbNames.isEmpty())
        QSKIP("No database drivers installed", SkipAll);
    for ( QStringList::Iterator it = dbs.dbNames.begin(); it != dbs.dbNames.end(); ++it ) {
	QSqlDatabase db = QSqlDatabase::database( (*it), FALSE );
	CHECK_DATABASE( db );

	QString userName = db.userName();
	QString password = db.password();

	// force an open error
	if ( db.open( "dummy130977", "doesnt_exist" ) ) {
	    qDebug( "Promiscuous database server without access control - test skipped for " +
		    tst_Databases::dbToString( db ) );
	    QVERIFY(1);
	    continue;
	}

	QVERIFY2( !db.isOpen(), (*it) );
	QVERIFY2( db.isOpenError(), (*it) );

	// now open it
	if ( !db.open( userName, password ) ) {
	    qDebug( "Could not open Database " + tst_Databases::dbToString( db ) +
		    ". Assuming DB is down, skipping... (Error: " + 
		    tst_Databases::printError( db.lastError() ) + ")" );
	    continue;
	}
	QVERIFY2( db.open( userName, password ), (*it) );
	QVERIFY2( db.isOpen(), (*it) );
	QVERIFY2( !db.isOpenError(), (*it) );
	db.close();
	QVERIFY2( !db.isOpen(), (*it) );

	// force another open error
	QVERIFY2( !db.open( "dummy130977", "doesnt_exist" ), (*it) );
	QVERIFY2( !db.isOpen(), (*it) );
	QVERIFY2( db.isOpenError(), (*it) );
    }
}

void tst_QSql::registerSqlDriver()
{
    int argc = 0;
    QApplication app( argc, 0, FALSE );

    QSqlDatabase::registerSqlDriver( "QSQLTESTDRIVER", new QSqlDriverCreator<QSqlNullDriver> );
    QVERIFY( QSqlDatabase::drivers().contains( "QSQLTESTDRIVER" ) );

    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLTESTDRIVER" );
    QVERIFY( db.isValid() );

    QCOMPARE( db.tables(), QStringList() );
}

QTEST_APPLESS_MAIN(tst_QSql)
#include "tst_qsql.moc"
