/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtSql/QtSql>

#include "../qsqldatabase/tst_databases.h"



//TESTED_CLASS=
//TESTED_FILES=sql/gui/qsqlrelationaltablemodel.h sql/gui/qsqlrelationaltablemodel.cpp

class tst_QSqlRelationalTableModel : public QObject
{
    Q_OBJECT

public:
    void recreateTestTables(QSqlDatabase);

    tst_Databases dbs;

public slots:
    void initTestCase_data();
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void data();
    void setData();
    void multipleRelation();
    void insertRecord();
    void insertWithStrategies();
    void removeColumn();
    void filter();
    void sort();
    void revert();

    void clearDisplayValuesCache();
};


void tst_QSqlRelationalTableModel::initTestCase_data()
{
    dbs.open();
    if (dbs.fillTestTable() == 0) {
        qWarning("NO DATABASES");
        QSKIP("No database drivers are available in this Qt configuration", SkipAll);
    }
}

void tst_QSqlRelationalTableModel::recreateTestTables(QSqlDatabase db)
{
    QSqlQuery q(db);

    tst_Databases::safeDropTable(db, qTableName("reltest1"));
    QVERIFY2(q.exec("create table " + qTableName("reltest1", 
            db.driver()) + " (id int not null primary key, name varchar(20), title_key int, another_title_key int)"),
            q.lastError().text().toLatin1());
    QVERIFY2(q.exec("insert into " + qTableName("reltest1", db.driver()) + " values(1, 'harry', 1, 2)"), q.lastError().text().toLatin1());
    QVERIFY2(q.exec("insert into " + qTableName("reltest1", db.driver()) + " values(2, 'trond', 2, 1)"), q.lastError().text().toLatin1());
    QVERIFY2(q.exec("insert into " + qTableName("reltest1", db.driver()) + " values(3, 'vohi', 1, 2)"), q.lastError().text().toLatin1());
    QVERIFY2(q.exec("insert into " + qTableName("reltest1", db.driver()) + " values(4, 'boris', 2, 2)"), q.lastError().text().toLatin1());

    QString tidField = db.driver()->escapeIdentifier("tid", QSqlDriver::FieldName);
    QString titleField = db.driver()->escapeIdentifier("title", QSqlDriver::FieldName);
    tst_Databases::safeDropTable(db, qTableName("reltest2"));
    QVERIFY2(q.exec(QString("create table " + qTableName("reltest2", 
            db.driver()) + " (%1 int not null primary key, %2 varchar(20))").arg(tidField).arg(titleField)),
            q.lastError().text().toLatin1());
    QVERIFY2(q.exec("insert into " + qTableName("reltest2", db.driver()) + " values(1, 'herr')"), q.lastError().text().toLatin1());
    QVERIFY2(q.exec("insert into " + qTableName("reltest2", db.driver()) + " values(2, 'mister')"), q.lastError().text().toLatin1());
}

void tst_QSqlRelationalTableModel::initTestCase()
{
    foreach (QString dbname, dbs.dbNames)
        recreateTestTables(QSqlDatabase::database(dbname));
}

void tst_QSqlRelationalTableModel::cleanupTestCase()
{
    foreach (QString dbName, dbs.dbNames) {
        QSqlDatabase db = QSqlDatabase::database(dbName);
        tst_Databases::safeDropTable(db, qTableName("reltest1"));
        tst_Databases::safeDropTable(db, qTableName("reltest2"));
    }

    dbs.close();
}

void tst_QSqlRelationalTableModel::init()
{
}

void tst_QSqlRelationalTableModel::cleanup()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    recreateTestTables(db);
}

void tst_QSqlRelationalTableModel::data()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlRelationalTableModel model(0, db);

    model.setTable(qTableName("reltest1"));
    model.setRelation(2, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
    QCOMPARE(model.data(model.index(0, 2)).toString(), QString("herr"));
}

void tst_QSqlRelationalTableModel::setData()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    // set the values
    {
        QSqlRelationalTableModel model(0, db);

        model.setTable(qTableName("reltest1"));
        model.setRelation(2, QSqlRelation(qTableName("reltest2"), "tid", "title"));
        QVERIFY2(model.select(), model.lastError().text().toLatin1());

        QVERIFY(model.setData(model.index(0, 1), QString("harry2")));
        QVERIFY(model.setData(model.index(0, 2), 2));

        QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry2"));
        QCOMPARE(model.data(model.index(0, 2)).toInt(), 2);

        model.submitAll();
    }

    // verify the values
    {
        QSqlTableModel model(0, db);
        model.setTable(qTableName("reltest1"));
        model.setSort(0, Qt::AscendingOrder);
        QVERIFY2(model.select(), model.lastError().text().toLatin1());

        QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry2"));
        QCOMPARE(model.data(model.index(0, 2)).toInt(), 2);
    }
}

void tst_QSqlRelationalTableModel::multipleRelation()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlRelationalTableModel model(0, db);

    model.setTable(qTableName("reltest1"));
    model.setRelation(2, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    model.setRelation(3, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.data(model.index(2, 0)).toInt(), 3);

    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
    QCOMPARE(model.data(model.index(0, 2)).toString(), QString("herr"));
    QCOMPARE(model.data(model.index(0, 3)).toString(), QString("mister"));
}

void tst_QSqlRelationalTableModel::insertRecord()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlRelationalTableModel model(0, db);

    model.setTable(qTableName("reltest1"));
    model.setRelation(2, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QSqlRecord rec;
    QSqlField f1("id", QVariant::Int);
    QSqlField f2("name", QVariant::String);
    QSqlField f3("title_key", QVariant::Int);
    QSqlField f4("another_title_key", QVariant::Int);

    f1.setValue(5);
    f2.setValue("test");
    f3.setValue(1);
    f4.setValue(2);

    f1.setGenerated(true);
    f2.setGenerated(true);
    f3.setGenerated(true);
    f4.setGenerated(true);

    rec.append(f1);
    rec.append(f2);
    rec.append(f3);
    rec.append(f4);

    QVERIFY_SQL(model, model.insertRecord(-1, rec));

    QCOMPARE(model.data(model.index(4, 0)).toInt(), 5);
    QCOMPARE(model.data(model.index(4, 1)).toString(), QString("test"));
}

void tst_QSqlRelationalTableModel::insertWithStrategies()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlRelationalTableModel model(0, db);

    model.setTable(qTableName("reltest1"));
    model.setRelation(2, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    model.setRelation(3, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    model.insertRows(0, 1);
    model.setData(model.index(0, 0), 1011);
    model.setData(model.index(0, 1), "test");
    model.setData(model.index(0, 2), 1);
    model.setData(model.index(0, 3), 2);
    QVERIFY2(model.submitAll(), qPrintable(model.lastError().text()));

    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.insertRows(0, 1);
    model.setData(model.index(0, 0), 1012);
    model.setData(model.index(0, 1), "test");
    model.setData(model.index(0, 2), 1);
    model.setData(model.index(0, 3), 2);

    QVERIFY2(model.submitAll(), qPrintable(model.lastError().text()));
}

void tst_QSqlRelationalTableModel::removeColumn()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlRelationalTableModel model(0, db);

    model.setTable(qTableName("reltest1"));
    model.setRelation(2, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QVERIFY_SQL(model, model.removeColumn(3));
    QVERIFY_SQL(model, model.select());

    QCOMPARE(model.columnCount(), 3);

    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
    QCOMPARE(model.data(model.index(0, 2)).toString(), QString("herr"));
    QCOMPARE(model.data(model.index(0, 3)), QVariant());

    // try removing more than one column
    QVERIFY_SQL(model, model.removeColumns(1, 2));
    QCOMPARE(model.columnCount(), 1);
    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
    QCOMPARE(model.data(model.index(0, 1)), QVariant());
}

void tst_QSqlRelationalTableModel::filter()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlRelationalTableModel model(0, db);

    model.setTable(qTableName("reltest1"));
    model.setRelation(2, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    QString titleField = db.driver()->escapeIdentifier("title", QSqlDriver::FieldName);
    model.setFilter(QString("%1 = 'herr'").arg(titleField));
    
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0, 2)).toString(), QString("herr"));
    QCOMPARE(model.data(model.index(1, 2)).toString(), QString("herr"));
}

void tst_QSqlRelationalTableModel::sort()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlRelationalTableModel model(0, db);

    model.setTable(qTableName("reltest1"));
    model.setRelation(2, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    model.setRelation(3, QSqlRelation(qTableName("reltest2"), "tid", "title"));

    model.setSort(2, Qt::DescendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.data(model.index(0, 2)).toString(), QString("mister"));
    QCOMPARE(model.data(model.index(1, 2)).toString(), QString("mister"));
    QCOMPARE(model.data(model.index(2, 2)).toString(), QString("herr"));
    QCOMPARE(model.data(model.index(3, 2)).toString(), QString("herr"));


    model.setSort(3, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.data(model.index(0, 3)).toString(), QString("herr"));
    QCOMPARE(model.data(model.index(1, 3)).toString(), QString("mister"));
    QCOMPARE(model.data(model.index(2, 3)).toString(), QString("mister"));
    QCOMPARE(model.data(model.index(3, 3)).toString(), QString("mister"));
}

static void testRevert(QSqlRelationalTableModel &model)
{
    /* revert single row */
    QCOMPARE(model.data(model.index(0, 2)).toString(), QString("herr"));
    QVERIFY(model.setData(model.index(0, 2), 2, Qt::EditRole));
    QVERIFY(model.setData(model.index(0, 2), "mister", Qt::DisplayRole));

    QCOMPARE(model.data(model.index(0, 2)).toString(), QString("mister"));
    model.revertRow(0);
    QCOMPARE(model.data(model.index(0, 2)).toString(), QString("herr"));

    /* revert all */
    QVERIFY(model.setData(model.index(0, 2), 2, Qt::EditRole));
    QVERIFY(model.setData(model.index(0, 2), "mister", Qt::DisplayRole));

    QCOMPARE(model.data(model.index(0, 2)).toString(), QString("mister"));
    model.revertAll();
    QCOMPARE(model.data(model.index(0, 2)).toString(), QString("herr"));

    // the following only works for OnManualSubmit
    if (model.editStrategy() != QSqlTableModel::OnManualSubmit)
        return;

    /* revert inserted row */
    QVERIFY(model.insertRows(4, 4));

    /* make sure the new rows are initialized to nothing */
    QVERIFY(model.data(model.index(4, 2)).toString().isEmpty());
    QVERIFY(model.data(model.index(5, 2)).toString().isEmpty());
    QVERIFY(model.data(model.index(6, 2)).toString().isEmpty());
    QVERIFY(model.data(model.index(7, 2)).toString().isEmpty());

    /* Set some values */
    QVERIFY(model.setData(model.index(4, 0), 42, Qt::EditRole));
    QVERIFY(model.setData(model.index(5, 0), 43, Qt::EditRole));
    QVERIFY(model.setData(model.index(6, 0), 44, Qt::EditRole));
    QVERIFY(model.setData(model.index(7, 0), 45, Qt::EditRole));

    QVERIFY(model.setData(model.index(4, 2), 2, Qt::EditRole));
    QVERIFY(model.setData(model.index(4, 2), "mister", Qt::DisplayRole));
    QVERIFY(model.setData(model.index(5, 2), 2, Qt::EditRole));
    QVERIFY(model.setData(model.index(5, 2), "mister", Qt::DisplayRole));
    QVERIFY(model.setData(model.index(6, 2), 1, Qt::EditRole));
    QVERIFY(model.setData(model.index(6, 2), "herr", Qt::DisplayRole));
    QVERIFY(model.setData(model.index(7, 2), 2, Qt::EditRole));
    QVERIFY(model.setData(model.index(7, 2), "mister", Qt::DisplayRole));

    /* Now revert the newly inserted rows */
    model.revertAll();

    /* Insert rows again */
    QVERIFY(model.insertRows(4, 4));

    /* make sure the new rows are initialized to nothing */
    QVERIFY(model.data(model.index(4, 2)).toString().isEmpty());
    QVERIFY(model.data(model.index(5, 2)).toString().isEmpty());
    QVERIFY(model.data(model.index(6, 2)).toString().isEmpty());
    QVERIFY(model.data(model.index(7, 2)).toString().isEmpty());
}

void tst_QSqlRelationalTableModel::revert()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlRelationalTableModel model(0, db);

    model.setTable(qTableName("reltest1"));
    model.setRelation(2, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    model.setRelation(3, QSqlRelation(qTableName("reltest2"), "tid", "title"));

    model.setSort(0, Qt::AscendingOrder);

    QVERIFY_SQL(model, model.select());
    QCOMPARE(model.data(model.index(0, 0)).toString(), QString("1"));

    testRevert(model);
    if (QTest::currentTestFailed())
        return;

    /* and again with OnManualSubmit */
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    testRevert(model);
}

void tst_QSqlRelationalTableModel::clearDisplayValuesCache()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlRelationalTableModel model(0, db);

    model.setTable(qTableName("reltest1"));
    model.setRelation(2, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    model.setRelation(3, QSqlRelation(qTableName("reltest2"), "tid", "title"));
    model.setSort(1, Qt::AscendingOrder);
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    
    QVERIFY_SQL(model, model.select());

    QCOMPARE(model.data(model.index(3, 0)).toInt(), 3);
    QCOMPARE(model.data(model.index(3, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(3, 2)).toString(), QString("herr"));
    QCOMPARE(model.data(model.index(3, 3)).toString(), QString("mister"));

    model.insertRow(model.rowCount());
    QVERIFY(model.setData(model.index(4, 0), 5, Qt::EditRole));
    QVERIFY(model.setData(model.index(4, 1), "anders", Qt::EditRole));
    QVERIFY(model.setData(model.index(4, 2), 1, Qt::EditRole));
    QVERIFY(model.setData(model.index(4, 2), "herr", Qt::DisplayRole));
    QVERIFY(model.setData(model.index(4, 3), 1, Qt::EditRole));
    QVERIFY(model.setData(model.index(4, 3), "herr", Qt::DisplayRole));
    model.submitAll();

    QCOMPARE(model.data(model.index(0, 0)).toInt(), 5);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("anders"));
    QCOMPARE(model.data(model.index(0, 2)).toString(), QString("herr"));
    QCOMPARE(model.data(model.index(0, 3)).toString(), QString("herr"));
    QCOMPARE(model.data(model.index(4, 0)).toInt(), 3);
    QCOMPARE(model.data(model.index(4, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(4, 2)).toString(), QString("herr"));
    QCOMPARE(model.data(model.index(4, 3)).toString(), QString("mister"));
}

QTEST_MAIN(tst_QSqlRelationalTableModel)
#include "tst_qsqlrelationaltablemodel.moc"
