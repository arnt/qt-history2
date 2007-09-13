/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <q3sqlselectcursor.h>

#include <qsqldriver.h>

#define NODATABASE_SKIP "No database drivers are available in this Qt configuration"


#include "../qsqldatabase/tst_databases.h"

//TESTED_FILES=

QT_DECLARE_CLASS(QSqlDatabase)

class tst_Q3SqlSelectCursor : public QObject
{
Q_OBJECT

public:
    tst_Q3SqlSelectCursor();
    virtual ~tst_Q3SqlSelectCursor();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void value_data() { generic_data(); }
    void value();
    void _exec_data() { generic_data(); }
    void _exec();

private:
    void generic_data();
    void createTestTables( QSqlDatabase db );
    void dropTestTables( QSqlDatabase db );
    void populateTestTables( QSqlDatabase db );

    tst_Databases dbs;
};

tst_Q3SqlSelectCursor::tst_Q3SqlSelectCursor()
{
}

tst_Q3SqlSelectCursor::~tst_Q3SqlSelectCursor()
{
}

void tst_Q3SqlSelectCursor::generic_data()
{
    if ( dbs.fillTestTable() == 0 )
        QSKIP( "No database drivers are available in this Qt configuration", SkipAll );
}

void tst_Q3SqlSelectCursor::createTestTables( QSqlDatabase db )
{
    if ( !db.isValid() )
	return;
    QSqlQuery q( QString::null, db );
    // please never ever change this table; otherwise fix all tests ;)
    QVERIFY2( q.exec( "create table " + qTableName( "qtest" ) + " ( id int not null, t_varchar varchar(40) not null,"
	    "t_char char(40), t_numeric numeric(6, 3), primary key (id, t_varchar) )" ),
	    tst_Databases::printError( q.lastError() ) );
}

void tst_Q3SqlSelectCursor::dropTestTables( QSqlDatabase db )
{
    tst_Databases::safeDropTable( db, qTableName( "qtest" ) );
}

void tst_Q3SqlSelectCursor::populateTestTables( QSqlDatabase db )
{
    if ( !db.isValid() )
	return;
    QSqlQuery q( QString::null, db );

    q.exec( "delete from " + qTableName( "qtest" ) ); //non-fatal
    QVERIFY2( q.exec( "insert into " + qTableName( "qtest" ) + " (id, t_varchar, t_char, t_numeric) values ( 0, 'VarChar0', 'Char0', 1.1 )" ),
	    tst_Databases::printError( q.lastError() ) );
    QVERIFY2( q.exec( "insert into " + qTableName( "qtest" ) + " (id, t_varchar, t_char, t_numeric) values ( 1, 'VarChar1', 'Char1', 2.2 )" ),
	    tst_Databases::printError( q.lastError() ) );
    QVERIFY2( q.exec( "insert into " + qTableName( "qtest" ) + " (id, t_varchar, t_char, t_numeric) values ( 2, 'VarChar2', 'Char2', 3.3 )" ),
	    tst_Databases::printError( q.lastError() ) );
    QVERIFY2( q.exec( "insert into " + qTableName( "qtest" ) + " (id, t_varchar, t_char, t_numeric) values ( 3, 'VarChar3', 'Char3', 4.4 )" ),
	    tst_Databases::printError( q.lastError() ) );
}

void tst_Q3SqlSelectCursor::initTestCase()
{
    dbs.open();

    for ( QStringList::ConstIterator it = dbs.dbNames.begin(); it != dbs.dbNames.end(); ++it ) {
	QSqlDatabase db = QSqlDatabase::database( (*it) );
	CHECK_DATABASE( db );

	dropTestTables( db ); //in case of leftovers
	createTestTables( db );
	populateTestTables( db );
    }
}

void tst_Q3SqlSelectCursor::cleanupTestCase()
{
    for ( QStringList::ConstIterator it = dbs.dbNames.begin(); it != dbs.dbNames.end(); ++it ) {
	QSqlDatabase db = QSqlDatabase::database( (*it) );
	CHECK_DATABASE( db );
	dropTestTables( db );
    }

    dbs.close();
}

void tst_Q3SqlSelectCursor::init()
{
}

void tst_Q3SqlSelectCursor::cleanup()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    if ( QTest::currentTestFailed() ) {
	//since Oracle ODBC totally craps out on error, we init again
	db.close();
	db.open();
    }
}

void tst_Q3SqlSelectCursor::value()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    Q3SqlSelectCursor cur( "select * from " + qTableName( "qtest" ) + " order by id", db );
    QVERIFY( cur.select() );
    QVERIFY2 ( cur.isActive(), tst_Databases::printError( cur.lastError() ) );
    int i = 0;
    while ( cur.next() ) {
	QVERIFY( cur.value( "id" ).toInt() == i );
	i++;
    }
}

void tst_Q3SqlSelectCursor::_exec()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );
    
    Q3SqlSelectCursor cur( QString::null, db );
    QVERIFY2 ( cur.isActive() == FALSE, tst_Databases::printError( cur.lastError() ) );

    cur.exec( "select * from " + qTableName( "qtest" ) ); //nothing should happen
    QVERIFY2 ( cur.isActive(), tst_Databases::printError( cur.lastError() ) );
    int i = 0;
    while ( cur.next() ) {
	QVERIFY( cur.value( "id" ).toInt() == i );
	i++;
    }
}


QTEST_MAIN(tst_Q3SqlSelectCursor)
#include "tst_qsqlselectcursor.moc"
