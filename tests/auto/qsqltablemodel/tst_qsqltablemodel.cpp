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
#include <QtSql>
#include <QSignalSpy>

//TESTED_CLASS=
//TESTED_FILES=sql/gui/qsqltablemodel.h sql/gui/qsqltablemodel.cpp

Q_DECLARE_METATYPE(QModelIndex)

class tst_QSqlTableModel : public QObject
{
    Q_OBJECT

public:
    tst_QSqlTableModel();
    virtual ~tst_QSqlTableModel();


    void dropTestTables();
    void createTestTables();
    void recreateTestTables();
    void repopulateTestTables();

    tst_Databases dbs;

public slots:
    void initTestCase_data();
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:

    void select();
    void submitAll();
    void setRecord();
    void insertRow();
    void insertRecord();
    void insertMultiRecords();
    void removeRow();
    void removeRows();
    void removeInsertedRow();
    void setFilter();
    void setInvalidFilter();

    void emptyTable();
    void tablesAndSchemas();
    void whitespaceInIdentifiers();
    void primaryKeyOrder();

    void sqlite_bigTable();

    // bug specific tests
    void insertRecordBeforeSelect();
    void submitAllOnInvalidTable();
    void insertRecordsInLoop();
    void sqlite_attachedDatabase(); // For task 130799
};

tst_QSqlTableModel::tst_QSqlTableModel()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
    dbs.open();
}

tst_QSqlTableModel::~tst_QSqlTableModel()
{
}

void tst_QSqlTableModel::dropTestTables()
{
    for (int i = 0; i < dbs.dbNames.count(); ++i) {
        QSqlDatabase db = QSqlDatabase::database(dbs.dbNames.at(i));
        QSqlQuery q(db);

        tst_Databases::safeDropTable(db, qTableName("test"));
        tst_Databases::safeDropTable(db, qTableName("test2"));
        tst_Databases::safeDropTable(db, qTableName("emptytable"));

        if (db.driverName().startsWith("QPSQL")) {
            q.exec("DROP SCHEMA " + qTableName("testschema") + " CASCADE");
        }
        if (testWhiteSpaceNames(db.driverName()))
            q.exec("DROP TABLE \"" + qTableName("qtestw") + " hitespace\"");
    }
}

void tst_QSqlTableModel::createTestTables()
{
    for (int i = 0; i < dbs.dbNames.count(); ++i) {
        QSqlDatabase db = QSqlDatabase::database(dbs.dbNames.at(i));
        QSqlQuery q(db);

        QVERIFY2(q.exec("create table " + qTableName("test", db.driver())
                       + "(id int, name varchar(20), title int)"),
                q.lastError().text().toLatin1());

        QVERIFY2(q.exec("create table " + qTableName("test2", db.driver())
                       + "(id int, title varchar(20))"),
                q.lastError().text().toLatin1());

        QVERIFY2(q.exec("create table " + qTableName("emptytable", db.driver())
                       + "(id int)"),
                q.lastError().text().toLatin1());

        if (testWhiteSpaceNames(db.driverName())) {
            QString qry = "create table \"" + qTableName("qtestw") + " hitespace\" (\"a field\" int)";
            QVERIFY2(q.exec(qry), q.lastError().text().toLatin1());
        }
    }
}

void tst_QSqlTableModel::repopulateTestTables()
{
    for (int i = 0; i < dbs.dbNames.count(); ++i) {
        QSqlDatabase db = QSqlDatabase::database(dbs.dbNames.at(i));
        QSqlQuery q(db);

        q.exec("delete from " + qTableName("test", db.driver())), q.lastError().text().toLatin1();
        QVERIFY2(q.exec("insert into " + qTableName("test", db.driver())
                       + " values(1, 'harry', 1)"), q.lastError().text().toLatin1());
        QVERIFY2(q.exec("insert into " + qTableName("test", db.driver())
                       + " values(2, 'trond', 2)"), q.lastError().text().toLatin1());
        QVERIFY2(q.exec("insert into " + qTableName("test", db.driver())
                       + " values(3, 'vohi', 3)"), q.lastError().text().toLatin1());

        q.exec("delete from " + qTableName("test2", db.driver())), q.lastError().text().toLatin1();
        QVERIFY2(q.exec("insert into " + qTableName("test2", db.driver())
                       + " values(1, 'herr')"), q.lastError().text().toLatin1());
        QVERIFY2(q.exec("insert into " + qTableName("test2", db.driver())
                       + " values(2, 'mister')"), q.lastError().text().toLatin1());
    }
}

void tst_QSqlTableModel::recreateTestTables()
{
    dropTestTables();
    createTestTables();
    repopulateTestTables();
}

void tst_QSqlTableModel::initTestCase_data()
{
    if (dbs.fillTestTable() == 0) {
        qWarning("NO DATABASES");
        QSKIP("No database drivers are available in this Qt configuration", SkipAll);
    }
}

void tst_QSqlTableModel::initTestCase()
{
    recreateTestTables();
}

void tst_QSqlTableModel::cleanupTestCase()
{
    dropTestTables();
    dbs.close();
}

void tst_QSqlTableModel::init()
{
}

void tst_QSqlTableModel::cleanup()
{
    repopulateTestTables();
}

void tst_QSqlTableModel::select()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.columnCount(), 3);

    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
    QCOMPARE(model.data(model.index(0, 2)).toInt(), 1);
    QCOMPARE(model.data(model.index(0, 3)), QVariant());

    QCOMPARE(model.data(model.index(1, 0)).toInt(), 2);
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond"));
    QCOMPARE(model.data(model.index(1, 2)).toInt(), 2);
    QCOMPARE(model.data(model.index(1, 3)), QVariant());

    QCOMPARE(model.data(model.index(2, 0)).toInt(), 3);
    QCOMPARE(model.data(model.index(2, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(2, 2)).toInt(), 3);
    QCOMPARE(model.data(model.index(2, 3)), QVariant());

    QCOMPARE(model.data(model.index(3, 0)), QVariant());
    QCOMPARE(model.data(model.index(3, 1)), QVariant());
    QCOMPARE(model.data(model.index(3, 2)), QVariant());
    QCOMPARE(model.data(model.index(3, 3)), QVariant());
}

void tst_QSqlTableModel::setRecord()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    for (int i = 0; i < model.rowCount(); ++i) {
        QSignalSpy spy(&model, SIGNAL(dataChanged(QModelIndex,QModelIndex)));

        QSqlRecord rec = model.record(i);
        rec.setValue(1, rec.value(1).toString() + "X");
        QVERIFY(model.setRecord(i, rec));

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).count(), 2);
        QCOMPARE(qvariant_cast<QModelIndex>(spy.at(0).at(0)), model.index(i, 1));
        QCOMPARE(qvariant_cast<QModelIndex>(spy.at(0).at(1)), model.index(i, 1));
    }

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harryX"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trondX"));
    QCOMPARE(model.data(model.index(2, 1)).toString(), QString("vohiX"));
}

void tst_QSqlTableModel::insertRow()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setEditStrategy(QSqlTableModel::OnRowChange);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QVERIFY(model.insertRow(2));
    QSqlRecord rec = model.record(1);
    rec.setValue(0, 42);
    rec.setValue(1, QString("vohi"));
    QVERIFY(model.setRecord(2, rec));

    QCOMPARE(model.data(model.index(2, 0)).toInt(), 42);
    QCOMPARE(model.data(model.index(2, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(2, 2)).toInt(), 2);

    QVERIFY(model.submitAll());
}

void tst_QSqlTableModel::insertRecord()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QSqlRecord rec = model.record();
    rec.setValue(0, 42);
    rec.setValue(1, QString("vohi"));
    rec.setValue(2, 1);
    QVERIFY(model.insertRecord(1, rec));
    QCOMPARE(model.rowCount(), 4);

    QCOMPARE(model.data(model.index(1, 0)).toInt(), 42);
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(1, 2)).toInt(), 1);

    model.revertAll();
    model.setEditStrategy(QSqlTableModel::OnRowChange);

    QVERIFY(model.insertRecord(-1, rec));

    QCOMPARE(model.data(model.index(3, 0)).toInt(), 42);
    QCOMPARE(model.data(model.index(3, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(3, 2)).toInt(), 1);
}

void tst_QSqlTableModel::insertMultiRecords()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.rowCount(), 3);

    QVERIFY(model.insertRow(2));

    QCOMPARE(model.data(model.index(2, 0)), QVariant());
    QCOMPARE(model.data(model.index(2, 1)), QVariant());
    QCOMPARE(model.data(model.index(2, 2)), QVariant());

    QVERIFY(model.insertRow(3));
    QVERIFY(model.insertRow(0));

    QCOMPARE(model.data(model.index(5, 0)).toInt(), 3);
    QCOMPARE(model.data(model.index(5, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(5, 2)).toInt(), 3);

    QVERIFY(model.setData(model.index(0, 0), QVariant(42)));
    QVERIFY(model.setData(model.index(3, 0), QVariant(43)));
    QVERIFY(model.setData(model.index(4, 0), QVariant(44)));
    QVERIFY(model.setData(model.index(4, 1), QVariant(QLatin1String("gunnar"))));
    QVERIFY(model.setData(model.index(4, 2), QVariant(1)));

    QVERIFY(model.submitAll());
    model.clear();
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
    QCOMPARE(model.data(model.index(1, 0)).toInt(), 2);
    QCOMPARE(model.data(model.index(2, 0)).toInt(), 3);
    QCOMPARE(model.data(model.index(3, 0)).toInt(), 42);
    QCOMPARE(model.data(model.index(4, 0)).toInt(), 43);
    QCOMPARE(model.data(model.index(5, 0)).toInt(), 44);
    QCOMPARE(model.data(model.index(5, 1)).toString(), QString("gunnar"));
    QCOMPARE(model.data(model.index(5, 2)).toInt(), 1);
}

void tst_QSqlTableModel::submitAll()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QVERIFY(model.setData(model.index(0, 1), "harry2", Qt::EditRole));
    QVERIFY(model.setData(model.index(1, 1), "trond2", Qt::EditRole));

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry2"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond2"));

    QVERIFY2(model.submitAll(), model.lastError().text().toLatin1());

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry2"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond2"));

    QVERIFY(model.setData(model.index(0, 1), "harry", Qt::EditRole));
    QVERIFY(model.setData(model.index(1, 1), "trond", Qt::EditRole));

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond"));

    QVERIFY2(model.submitAll(), model.lastError().text().toLatin1());

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond"));
}

void tst_QSqlTableModel::removeRow()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 3);

    QVERIFY(model.removeRow(1));
    QVERIFY(model.submitAll());
    QCOMPARE(model.rowCount(), 2);

    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
    QCOMPARE(model.data(model.index(1, 0)).toInt(), 3);
    model.clear();

    recreateTestTables();

    model.setTable(qTableName("test"));
    model.setEditStrategy(QSqlTableModel::OnRowChange);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 3);

    QVERIFY(model.removeRow(1));
    QCOMPARE(model.rowCount(), 2);

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("vohi"));
}

void tst_QSqlTableModel::removeRows()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    model.setEditStrategy(QSqlTableModel::OnFieldChange);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 3);
    
    QSignalSpy beforeDeleteSpy(&model, SIGNAL(beforeDelete(int)));
    QVERIFY2(model.removeRows(0, 2), model.lastError().text().toLatin1());
    QVERIFY(beforeDeleteSpy.count() == 2);
    QVERIFY(beforeDeleteSpy.at(0).at(0).toInt() == 0);
    QVERIFY(beforeDeleteSpy.at(1).at(0).toInt() == 1);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("vohi"));
    model.clear();

    recreateTestTables();
    model.setTable(qTableName("test"));
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 3);
    beforeDeleteSpy.clear();

    // When the edit strategy is OnManualSubmit the beforeDelete() signal 
    // isn't emitted until submitAll() is called 
    QVERIFY(model.removeRows(0, 2, QModelIndex()));
    QCOMPARE(model.rowCount(), 3);
    QVERIFY(beforeDeleteSpy.count() == 0);
    QVERIFY(model.submitAll()); 
    QVERIFY(beforeDeleteSpy.count() == 2);
    QVERIFY(beforeDeleteSpy.at(0).at(0).toInt() == 0);
    QVERIFY(beforeDeleteSpy.at(1).at(0).toInt() == 1);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("vohi"));
}

void tst_QSqlTableModel::removeInsertedRow()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    for (int i = 0; i <= 1; ++i) {

        QSqlTableModel model(0, db);
        model.setTable(qTableName("test"));

        model.setEditStrategy(i == 0
                ? QSqlTableModel::OnRowChange : QSqlTableModel::OnManualSubmit);
        QVERIFY2(model.select(), model.lastError().text().toLatin1());
        QCOMPARE(model.rowCount(), 3);

        QVERIFY(model.insertRow(1));
        QCOMPARE(model.rowCount(), 4);

        QVERIFY(model.removeRow(1));
        QCOMPARE(model.rowCount(), 3);

        QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
        QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond"));
        QCOMPARE(model.data(model.index(2, 1)).toString(), QString("vohi"));
        model.clear();

        recreateTestTables();
    }
}

void tst_QSqlTableModel::emptyTable()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 0);

    model.setTable(qTableName("emptytable"));
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 1);

    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 1);
}

void tst_QSqlTableModel::tablesAndSchemas()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("QPSQL"))
        QSKIP("Postgres specific test", SkipSingle);

    QSqlQuery q(db);
    q.exec("DROP SCHEMA " + qTableName("testschema") + " CASCADE");
    QVERIFY2(q.exec("create schema " + qTableName("testschema")), q.lastError().text().toLatin1());
    QString tableName = qTableName("testschema") + "." + qTableName("testtable");
    QVERIFY2(q.exec("create table " + tableName + "(id int)"), q.lastError().text().toLatin1());
    QVERIFY2(q.exec("insert into " + tableName + " values(1)"), q.lastError().text().toLatin1());
    QVERIFY2(q.exec("insert into " + tableName + " values(2)"), q.lastError().text().toLatin1());

    QSqlTableModel model(0, db);
    model.setTable(tableName);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.columnCount(), 1);
}

void tst_QSqlTableModel::whitespaceInIdentifiers()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!testWhiteSpaceNames(db.driverName()))
        QSKIP("DBMS doesn't support whitespaces in identifiers", SkipSingle);

    QString tableName = qTableName("qtestw") + " hitespace";

    QSqlTableModel model(0, db);
    model.setTable(tableName);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
}

void tst_QSqlTableModel::primaryKeyOrder()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "primaryKeyOrderTest");
    db.setDatabaseName(":memory:");
    QVERIFY2(db.open(), db.lastError().text().toLatin1());

    QSqlQuery q(db);

    QVERIFY2(q.exec("create table foo(a varchar(20), id int primary key, b varchar(20))"),
            q.lastError().text().toLatin1());

    QSqlTableModel model(0, db);
    model.setTable("foo");

    QSqlIndex pk = model.primaryKey();
    QCOMPARE(pk.count(), 1);
    QCOMPARE(pk.fieldName(0), QString::fromLatin1("id"));

    QVERIFY(model.insertRow(0));
    QVERIFY(model.setData(model.index(0, 0), "hello"));
    QVERIFY(model.setData(model.index(0, 1), 42));
    QVERIFY(model.setData(model.index(0, 2), "blah"));
    QVERIFY2(model.submitAll(), model.lastError().text().toLatin1());

    QVERIFY(model.setData(model.index(0, 1), 43));
    QVERIFY2(model.submitAll(), model.lastError().text().toLatin1());

    QCOMPARE(model.data(model.index(0, 1)).toInt(), 43);
}

void tst_QSqlTableModel::setInvalidFilter()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    // set an invalid filter, make sure it fails
    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setFilter("blahfahsel");

    QCOMPARE(model.filter(), QString("blahfahsel"));
    QVERIFY(!model.select());

    // set a valid filter later, make sure if passes
    model.setFilter("id = 1");
    QVERIFY2(model.select(), qPrintable(model.lastError().text()));
}

void tst_QSqlTableModel::setFilter()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setFilter("id = 1");
    QCOMPARE(model.filter(), QString("id = 1"));
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);

    QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    QSignalSpy rowsAboutToBeRemovedSpy(&model,
            SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy rowsAboutToBeInsertedSpy(&model,
            SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    model.setFilter("id = 2");

    // check the signals
    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QList<QVariant> args = rowsAboutToBeRemovedSpy.takeFirst();
    QCOMPARE(args.count(), 3);
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), QModelIndex());
    QCOMPARE(args.at(1).toInt(), 0);
    QCOMPARE(args.at(2).toInt(), 0);
    args = rowsRemovedSpy.takeFirst();
    QCOMPARE(args.count(), 3);
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), QModelIndex());
    QCOMPARE(args.at(1).toInt(), 0);
    QCOMPARE(args.at(2).toInt(), 0);
    args = rowsInsertedSpy.takeFirst();
    QCOMPARE(args.count(), 3);
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), QModelIndex());
    QCOMPARE(args.at(1).toInt(), 0);
    QCOMPARE(args.at(2).toInt(), 0);
    args = rowsAboutToBeInsertedSpy.takeFirst();
    QCOMPARE(args.count(), 3);
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), QModelIndex());
    QCOMPARE(args.at(1).toInt(), 0);
    QCOMPARE(args.at(2).toInt(), 0);

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0)).toInt(), 2);
}

void tst_QSqlTableModel::sqlite_bigTable()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("QSQLITE"))
        QSKIP("SQLite specific test", SkipSingle);

    QSqlQuery q(db);
    QVERIFY_SQL(q, q.exec("create table foo(id int primary key, name varchar)"));
    QVERIFY_SQL(q, q.prepare("insert into foo(id, name) values (?, ?)"));
    for (int i = 0; i < 10000; ++i) {
        q.addBindValue(i);
        q.addBindValue(QString::number(i));
        QVERIFY_SQL(q, q.exec());
    }
    q.clear();

    QSqlTableModel model(0, db);
    model.setTable("foo");
    QVERIFY_SQL(model, model.select());

    QSqlRecord rec = model.record();
    rec.setValue("id", 424242);
    rec.setValue("name", "Guillaume");
    QVERIFY_SQL(model, model.insertRecord(-1, rec));

    model.clear();

    QVERIFY_SQL(q, q.exec("drop table foo"));
}

// For task 118547: couldn't insert records unless select() 
// had first been called. 
void tst_QSqlTableModel::insertRecordBeforeSelect()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    QCOMPARE(model.lastError().type(), QSqlError::NoError);

    QSqlRecord buffer = model.record();
    buffer.setValue("id", 13);
    buffer.setValue("name", QString("The Lion King"));
    buffer.setValue("title", 0);
    QVERIFY2(model.insertRecord(-1, buffer),
        model.lastError().text().toLatin1());

    buffer.setValue("id", 26);
    buffer.setValue("name", QString("T. Leary"));
    buffer.setValue("title", 0);
    QVERIFY2(model.insertRecord(1, buffer),
        model.lastError().text().toLatin1());

    int rowCount = model.rowCount();
    model.clear();
    QCOMPARE(model.rowCount(), 0);
    
    QSqlTableModel model2(0, db);
    model2.setTable(qTableName("test"));
    QVERIFY2(model2.select(), model2.lastError().text().toLatin1());
    QCOMPARE(model2.rowCount(), rowCount);
}

// For task 118547: set errors if table doesn't exist and if records
// are inserted and submitted on a non-existing table.
void tst_QSqlTableModel::submitAllOnInvalidTable()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    
    // setTable returns a void, so the error can only be caught by
    // manually checking lastError(). This should be changed for Qt5.
    model.setTable(qTableName("invalidTable"));
    QCOMPARE(model.lastError().type(), QSqlError::StatementError);

    // This will give us an empty record which is expected behavior
    QSqlRecord buffer = model.record();
    buffer.setValue("bogus", 1000);
    buffer.setValue("bogus2", QString("I will go nowhere!"));

    // Inserting the record into the *model* will work (OnManualSubmit)
    QVERIFY2(model.insertRecord(-1, buffer), 
        model.lastError().text().toLatin1());

    // The submit and select shall fail because the table doesn't exist
    QEXPECT_FAIL("", "The table doesn't exist: submitAll() shall fail",
        Continue);
    QVERIFY2(model.submitAll(), model.lastError().text().toLatin1());
    QEXPECT_FAIL("", "The table doesn't exist: select() shall fail",
        Continue);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
}

// For task 147575: the rowsRemoved signal emitted from the model was lying
void tst_QSqlTableModel::insertRecordsInLoop()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.select();

    QSqlRecord record = model.record();
    record.setValue(0, 10);
    record.setValue(1, "Testman");
    record.setValue(2, 1);
    
    QSignalSpy spyRowsRemoved(&model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
    QSignalSpy spyRowsInserted(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
    for (int i = 0; i < 10; i++) {
        QVERIFY(model.insertRecord(model.rowCount(), record));
        QCOMPARE(spyRowsInserted.at(i).at(1).toInt(), i+3); // The table already contains three rows
        QCOMPARE(spyRowsInserted.at(i).at(2).toInt(), i+3);
    }
    model.submitAll(); // submitAll() calls select() which clears and repopulates the table

    int firstRowIndex = 0, lastRowIndex = 12;
    QCOMPARE(spyRowsRemoved.count(), 1);
    QCOMPARE(spyRowsRemoved.at(0).at(1).toInt(), firstRowIndex);
    QCOMPARE(spyRowsRemoved.at(0).at(2).toInt(), lastRowIndex);

    QCOMPARE(spyRowsInserted.at(10).at(1).toInt(), firstRowIndex);
    QCOMPARE(spyRowsInserted.at(10).at(2).toInt(), lastRowIndex);
    QCOMPARE(spyRowsInserted.count(), 11);

    QCOMPARE(model.rowCount(), 13);
    QCOMPARE(model.columnCount(), 3);
}

void tst_QSqlTableModel::sqlite_attachedDatabase()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("QSQLITE")) {
        QSKIP("SQLite server specific test", SkipSingle);
        return;
    }

    QSqlDatabase attachedDb = QSqlDatabase::addDatabase("QSQLITE", "attached");
    attachedDb.setDatabaseName("attached.dat");
    QVERIFY2(attachedDb.open(), attachedDb.lastError().text().toLatin1());

    QSqlQuery q(attachedDb);
    q.exec("DROP TABLE atest"), q.lastError().text().toLatin1();
    q.exec("DROP TABLE atest1"), q.lastError().text().toLatin1();
    QVERIFY2(q.exec("CREATE TABLE atest(id int, text varchar(20))"), q.lastError().text().toLatin1());
    QVERIFY2(q.exec("CREATE TABLE atest2(id int, text varchar(20))"), q.lastError().text().toLatin1());
    QVERIFY2(q.exec("INSERT INTO atest VALUES(1, 'attached-atest')"), q.lastError().text().toLatin1());
    QVERIFY2(q.exec("INSERT INTO atest2 VALUES(2, 'attached-atest2')"), q.lastError().text().toLatin1());

    QSqlQuery q2(db);
    QVERIFY2(q2.exec("CREATE TABLE atest(id int, text varchar(20))"), q.lastError().text().toLatin1());
    QVERIFY2(q2.exec("INSERT INTO atest VALUES(3, 'main')"), q.lastError().text().toLatin1());
    q2.exec("ATTACH DATABASE \"attached.dat\" as adb");

    // This should query the table in the attached database (schema supplied)
    QSqlTableModel model(0, db);
    model.setTable("adb.atest");
    QVERIFY2(model.select(), q.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toInt(), 1);
    QCOMPARE(model.data(model.index(0, 1), Qt::DisplayRole).toString(), QLatin1String("attached-atest"));

    // This should query the table in the attached database (unique tablename)
    model.setTable("atest2");
    QVERIFY2(model.select(), q.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toInt(), 2);
    QCOMPARE(model.data(model.index(0, 1), Qt::DisplayRole).toString(), QLatin1String("attached-atest2"));

    // This should query the table in the main database (tables in main db has 1st priority)
    model.setTable("atest");
    QVERIFY2(model.select(), q.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toInt(), 3);
    QCOMPARE(model.data(model.index(0, 1), Qt::DisplayRole).toString(), QLatin1String("main"));

    QVERIFY2(q2.exec("DROP TABLE adb.atest"), q.lastError().text().toLatin1());
    QVERIFY2(q2.exec("DROP TABLE atest2"), q.lastError().text().toLatin1());
    QVERIFY2(q2.exec("DROP TABLE atest"), q.lastError().text().toLatin1());

    attachedDb.close();
}

QTEST_MAIN(tst_QSqlTableModel)
#include "tst_qsqltablemodel.moc"
