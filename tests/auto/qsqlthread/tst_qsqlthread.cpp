/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include "../qsqldatabase/tst_databases.h"

#include <QtCore>
#include <QtSql>
#include "qdebug.h"

#ifdef Q_OS_LINUX
#include <pthread.h>
#endif

// set this define if Oracle is built with threading support
//#define QOCI_THREADED

class tst_QSqlThread : public QObject
{
    Q_OBJECT

public:
    tst_QSqlThread();
    virtual ~tst_QSqlThread();


    void dropTestTables();
    void createTestTables();
    void recreateTestTables();
    void repopulateTestTables();

    void generic_data();
    tst_Databases dbs;

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

protected slots:
    void threadFinished() { ++threadFinishedCount; }

private slots:
    void simpleThreading_data() { generic_data(); }
    void simpleThreading();
    void readWriteThreading_data() { generic_data(); }
    void readWriteThreading();
    void readFromSingleConnection_data() { generic_data(); }
    void readFromSingleConnection();
    void readWriteFromSingleConnection_data() { generic_data(); }
    void readWriteFromSingleConnection();
    void preparedReadWriteFromSingleConnection_data() { generic_data(); }
    void preparedReadWriteFromSingleConnection();
    void transactionsFromSingleConnection_data() { generic_data(); }
    void transactionsFromSingleConnection();

private:
    int threadFinishedCount;
};

static QBasicAtomicInt counter;

class QtTestSqlThread : public QThread
{
    Q_OBJECT
public:
    QtTestSqlThread(const QSqlDatabase &aDb, QObject *parent = 0)
        : QThread(parent), sourceDb(aDb) {}

    void runHelper(const QString &dbName)
    {
        QSqlDatabase db = QSqlDatabase::cloneDatabase(sourceDb, dbName);
        QVERIFY2(db.open(), db.lastError().text().toLocal8Bit());

        int sum = 0;
        QSqlQuery q("select id from " + qTableName("test"), db);
        QVERIFY2(q.isActive(), q.lastError().text().toLocal8Bit());
        while (q.next())
            sum += q.value(0).toInt();
        QCOMPARE(sum, 6);
        q.clear();
    }

    void run()
    {
        QString dbName = QString("QThreadDb%1").arg((size_t)currentThreadId());
        runHelper(dbName);

        QSqlDatabase::database(dbName).close();
        QSqlDatabase::removeDatabase(dbName);
    }

private:
    QSqlDatabase sourceDb;
};

enum { ProdConIterations = 10 };

class SqlProducer: public QThread
{
    Q_OBJECT
public:
    SqlProducer(const QSqlDatabase &aDb, QObject *parent = 0)
        : QThread(parent), sourceDb(aDb) {}

    void runHelper(const QString &dbName)
    {
        QSqlDatabase db = QSqlDatabase::cloneDatabase(sourceDb, dbName);
        QVERIFY2(db.open(), db.lastError().text().toLocal8Bit());
        QSqlQuery q(db);
        QVERIFY2(q.prepare("insert into " + qTableName("test") + " values (?, ?, ?)"),
                q.lastError().text().toLocal8Bit());
        int id = 10;
        for (int i = 0; i < ProdConIterations; ++i) {
            q.bindValue(0, ++id);
            q.bindValue(1, "threaddy");
            q.bindValue(2, 10);
            QVERIFY2(q.exec(), q.lastError().text().toLocal8Bit());
#ifdef Q_OS_LINUX
            pthread_yield();
#endif
        }
    }

    void run()
    {
        QString dbName = QString("Producer%1").arg((size_t)currentThreadId());
        runHelper(dbName);
        QSqlDatabase::database(dbName).close();
        QSqlDatabase::removeDatabase(dbName);
    }
private:
    QSqlDatabase sourceDb;
};

class SqlConsumer: public QThread
{
    Q_OBJECT

public:
    SqlConsumer(const QSqlDatabase &aDb, QObject *parent = 0)
        : QThread(parent), sourceDb(aDb) {}

    void runHelper(const QString &dbName)
    {
        QSqlDatabase db = QSqlDatabase::cloneDatabase(sourceDb, dbName);
        QVERIFY2(db.open(), db.lastError().text().toLocal8Bit());
        QSqlQuery q(db);
        QVERIFY2(q.prepare("delete from " + qTableName("test") +
                          " where id = (select max(id) from " + qTableName("test") + ")"),
                    q.lastError().text().toLocal8Bit());

        for (int i = 0; i < ProdConIterations; ++i) {
            QVERIFY2(q.exec(), q.lastError().text().toLocal8Bit());
#ifdef Q_OS_LINUX
            pthread_yield();
#endif
        }
    }

    void run()
    {
        QString dbName = QString("Consumer%1").arg((size_t)currentThreadId());
        runHelper(dbName);
        QSqlDatabase::database(dbName).close();
        QSqlDatabase::removeDatabase(dbName);
    }

private:
    QSqlDatabase sourceDb;
};

class SqlThread: public QThread
{
    Q_OBJECT

public:
    enum Mode { SimpleReading, PreparedReading, SimpleWriting, PreparedWriting };

    SqlThread(Mode m, const QSqlDatabase &db, QObject *parent = 0)
        : QThread(parent), sourceDb(db), mode(m) {}

    void run()
    {
        switch (mode) {
        case SimpleReading: {
            // Executes a Query for reading, iterates over the first 4 results
            QSqlQuery q(sourceDb);
            for (int j = 0; j < ProdConIterations; ++j) {
                QVERIFY_SQL(q, q.exec("select id,name from " + qTableName("test") + " order by id"));
                for (int i = 1; i < 4; ++i) {
                    QVERIFY_SQL(q, q.next());
                    QCOMPARE(q.value(0).toInt(), i);
                }
            }
            break; }
        case SimpleWriting: {
            // Executes a query for writing (appends a new row)
            QSqlQuery q(sourceDb);
            for (int j = 0; j < ProdConIterations; ++j) {
                QVERIFY_SQL(q, q.exec(QString("insert into " + qTableName("test")
                                + " (id, name) values(%1, '%2')")
                                      .arg(counter.fetchAndAddRelaxed(1)).arg("Robert")));
            }
            break; }
        case PreparedReading: {
            // Prepares a query for reading and iterates over the results
            QSqlQuery q(sourceDb);
            QVERIFY_SQL(q, q.prepare("select id, name from " + qTableName("test") + " where id = ?"));
            for (int j = 0; j < ProdConIterations; ++j) {
                q.addBindValue(j % 3 + 1);
                QVERIFY_SQL(q, q.exec());
                QVERIFY_SQL(q, q.next());
                QCOMPARE(q.value(0).toInt(), j % 3 + 1);
            }
            break; }
        case PreparedWriting: {
            QSqlQuery q(sourceDb);
            QVERIFY_SQL(q, q.prepare("insert into " + qTableName("test") + " (id, name) "
                                     "values(?, ?)"));
            for (int i = 0; i < ProdConIterations; ++i) {
                q.addBindValue(counter.fetchAndAddRelaxed(1));
                q.addBindValue("Robert");
                QVERIFY_SQL(q, q.exec());
            }
            break; }
        }
    }

private:
    QSqlDatabase sourceDb;
    Mode mode;
};


tst_QSqlThread::tst_QSqlThread()
    : threadFinishedCount(0)
{
}

tst_QSqlThread::~tst_QSqlThread()
{
}

void tst_QSqlThread::generic_data()
{
    if (dbs.fillTestTable() == 0)
        QSKIP("No database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlThread::dropTestTables()
{
    for (int i = 0; i < dbs.dbNames.count(); ++i) {
        QSqlDatabase db = QSqlDatabase::database(dbs.dbNames.at(i));
        QSqlQuery q(db);

        tst_Databases::safeDropTable(db, qTableName("test"));
        tst_Databases::safeDropTable(db, qTableName("test2"));
        tst_Databases::safeDropTable(db, qTableName("emptytable"));
    }
}

void tst_QSqlThread::createTestTables()
{
    for (int i = 0; i < dbs.dbNames.count(); ++i) {
        QSqlDatabase db = QSqlDatabase::database(dbs.dbNames.at(i));
        QSqlQuery q(db);

        QVERIFY2(q.exec("create table " + qTableName("test")
                       + "(id int primary key, name varchar(20), title int)"),
                q.lastError().text().toAscii());

        QVERIFY2(q.exec("create table " + qTableName("test2")
                       + "(id int primary key, title varchar(20))"),
                q.lastError().text().toAscii());

        QVERIFY2(q.exec("create table " + qTableName("emptytable")
                       + "(id int primary key)"),
                q.lastError().text().toAscii());
    }
}

void tst_QSqlThread::repopulateTestTables()
{
    for (int i = 0; i < dbs.dbNames.count(); ++i) {
        QSqlDatabase db = QSqlDatabase::database(dbs.dbNames.at(i));
        QSqlQuery q(db);

        QVERIFY2(q.exec("delete from " + qTableName("test")), q.lastError().text().toAscii());
        QVERIFY2(q.exec("insert into " + qTableName("test")
                       + " values(1, 'harry', 1)"), q.lastError().text().toAscii());
        QVERIFY2(q.exec("insert into " + qTableName("test")
                       + " values(2, 'trond', 2)"), q.lastError().text().toAscii());
        QVERIFY2(q.exec("insert into " + qTableName("test")
                       + " values(3, 'vohi', 3)"), q.lastError().text().toAscii());

        QVERIFY2(q.exec("delete from " + qTableName("test2")), q.lastError().text().toAscii());
        QVERIFY2(q.exec("insert into " + qTableName("test2")
                       + " values(1, 'herr')"), q.lastError().text().toAscii());
        QVERIFY2(q.exec("insert into " + qTableName("test2")
                       + " values(2, 'mister')"), q.lastError().text().toAscii());
    }
}

void tst_QSqlThread::recreateTestTables()
{
    dropTestTables();
    createTestTables();
    repopulateTestTables();
}

void tst_QSqlThread::initTestCase()
{
    dbs.open();
    recreateTestTables();
}

void tst_QSqlThread::cleanupTestCase()
{
    dropTestTables();
    dbs.close();
}

void tst_QSqlThread::init()
{
    threadFinishedCount = 0;
    counter = 4;
}

void tst_QSqlThread::cleanup()
{
    repopulateTestTables();
}

// This test creates two threads that clone their db connection and read
// from it
void tst_QSqlThread::simpleThreading()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (db.databaseName() == ":memory:")
        QSKIP("does not work with in-memory databases", SkipSingle);

    QtTestSqlThread t1(db);
    QtTestSqlThread t2(db);

    connect(&t1, SIGNAL(finished()), this, SLOT(threadFinished()), Qt::QueuedConnection);
    connect(&t2, SIGNAL(finished()), this, SLOT(threadFinished()), Qt::QueuedConnection);

    t1.start();
    t2.start();

    while (threadFinishedCount < 2)
        QTest::qWait(100);
}

// This test creates two threads that clone their db connection and read
// or write
void tst_QSqlThread::readWriteThreading()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (db.databaseName() == ":memory:")
        QSKIP("does not work with in-memory databases", SkipSingle);

    SqlProducer producer(db);
    SqlConsumer consumer(db);

    connect(&producer, SIGNAL(finished()), this, SLOT(threadFinished()), Qt::QueuedConnection);
    connect(&consumer, SIGNAL(finished()), this, SLOT(threadFinished()), Qt::QueuedConnection);

    producer.start();
    consumer.start();

    while (threadFinishedCount < 2)
        QTest::qWait(100);
}

// run with n threads in parallel. Change this constant to hammer the poor DB server even more
static const int maxThreadCount = 4;

void tst_QSqlThread::readFromSingleConnection()
{
#ifdef QOCI_THREADED
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (db.databaseName() == ":memory:")
        QSKIP("does not work with in-memory databases", SkipSingle);

    QObject cleanupHelper; // make sure the threads die when we exit the scope
    for (int i = 0; i < maxThreadCount; ++i) {
        SqlThread *reader = new SqlThread(SqlThread::SimpleReading, db, &cleanupHelper);
        connect(reader, SIGNAL(finished()), this, SLOT(threadFinished()), Qt::QueuedConnection);
        reader->start();
    }

    while (threadFinishedCount < maxThreadCount)
        QTest::qWait(100);
#endif
}

void tst_QSqlThread::readWriteFromSingleConnection()
{
#ifdef QOCI_THREADED
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (db.databaseName() == ":memory:")
        QSKIP("does not work with in-memory databases", SkipSingle);

    QObject cleanupHelper;
    for (int i = 0; i < maxThreadCount; ++i) {
        SqlThread *reader = new SqlThread(SqlThread::SimpleReading, db, &cleanupHelper);
        connect(reader, SIGNAL(finished()), this, SLOT(threadFinished()), Qt::QueuedConnection);
        reader->start();

        SqlThread *writer = new SqlThread(SqlThread::SimpleWriting, db, &cleanupHelper);
        connect(writer, SIGNAL(finished()), this, SLOT(threadFinished()), Qt::QueuedConnection);
        writer->start();
    }

    while (threadFinishedCount < maxThreadCount * 2)
        QTest::qWait(100);
#endif
}

void tst_QSqlThread::preparedReadWriteFromSingleConnection()
{
#ifdef QOCI_THREADED
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (db.databaseName() == ":memory:")
        QSKIP("does not work with in-memory databases", SkipSingle);

    QObject cleanupHelper;
    for (int i = 0; i < maxThreadCount; ++i) {
        SqlThread *reader = new SqlThread(SqlThread::PreparedReading, db, &cleanupHelper);
        connect(reader, SIGNAL(finished()), this, SLOT(threadFinished()), Qt::QueuedConnection);
        reader->start();

        SqlThread *writer = new SqlThread(SqlThread::PreparedWriting, db, &cleanupHelper);
        connect(writer, SIGNAL(finished()), this, SLOT(threadFinished()), Qt::QueuedConnection);
        writer->start();
    }

    while (threadFinishedCount < maxThreadCount * 2)
        QTest::qWait(100);
#endif
}

void tst_QSqlThread::transactionsFromSingleConnection()
{
#ifdef QOCI_THREADED
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (db.databaseName() == ":memory:")
        QSKIP("does not work with in-memory databases", SkipSingle);

    // start and commit a transaction
    QVERIFY_SQL(db, db.transaction());
    preparedReadWriteFromSingleConnection(); // read and write from multiple threads
    if (QTest::currentTestFailed())
        return;
    QVERIFY_SQL(db, db.commit());

    // reset test environment
    threadFinishedCount = 0;

    // start and roll back a transaction
    QVERIFY_SQL(db, db.transaction());
    preparedReadWriteFromSingleConnection(); // read and write from multiple threads
    if (QTest::currentTestFailed())
        return;
    QVERIFY_SQL(db, db.rollback());
#endif
}

QTEST_MAIN(tst_QSqlThread)
#include "tst_qsqlthread.moc"
