#include <QtSql>
#include <QPixmap>

#include <iostream>

using namespace std;

void QSqlDatabase_snippets()
{
    {
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("customdb");
    db.setUserName("mojito");
    db.setPassword("J0a1m8");
    bool ok = db.open();
    Q_UNUSED(ok);
    }

    {
    QSqlDatabase db = QSqlDatabase::database();
    }
}

void QSqlField_snippets()
{
#if 0
    {
    QSqlField field("age", QCoreVariant::Int);
    field.setValue(QPixmap());  // WRONG
    }
#endif

    {
    QSqlField field("age", QCoreVariant::Int);
    field.setValue(QString("123"));  // casts QString to int
    }

    {
    QSqlQuery query;
    QSqlRecord record = query.record();
    QSqlField field = record.field("country");
    }
}

void doSomething(const QString &)
{
}

void QSqlQuery_snippets()
{
    {
    // typical loop
    QSqlQuery query("SELECT country FROM artist");
    while (query.next()) {
        QString country = query.value(0).toString();
        doSomething(country);
    }
    }

    {
    // field index lookup
    QSqlQuery query("SELECT * FROM artist");
    int fieldNo = query.record().indexOf("country");
    while (query.next()) {
        QString country = query.value(fieldNo).toString();
        doSomething(country);
    }
    }

    {
    // named with named
    QSqlQuery query;
    query.prepare("INSERT INTO person (id, forename, surname) "
                  "VALUES (:id, :forename, :surname)");
    query.bindValue(":id", 1001);
    query.bindValue(":forename", "Bart");
    query.bindValue(":surname", "Simpson");
    query.exec();
    }

    {
    // positional with named
    QSqlQuery query;
    query.prepare("INSERT INTO person (id, forename, surname) "
                  "VALUES (:id, :forename, :surname)");
    query.bindValue(0, 1001);
    query.bindValue(1, "Bart");
    query.bindValue(2, "Simpson");
    query.exec();
    }

    {
    // positional 1
    QSqlQuery query;
    query.prepare("INSERT INTO person (id, forename, surname) "
                  "VALUES (?, ?, ?)");
    query.bindValue(0, 1001);
    query.bindValue(1, "Bart");
    query.bindValue(2, "Simpson");
    query.exec();
    }

    {
    // positional 2
    QSqlQuery query;
    query.prepare("INSERT INTO person (id, forename, surname) "
                  "VALUES (?, ?, ?)");
    query.addBindValue(1001);
    query.addBindValue("Bart");
    query.addBindValue("Simpson");
    query.exec();
    }

    {
    // stored
    QSqlQuery query;
    query.prepare("CALL AsciiToInt(?, ?)");
    query.bindValue(0, "A");
    query.bindValue(1, 0, QSql::Out);
    query.exec();
    int i = query.boundValue(1).toInt(); // i is 65
    Q_UNUSED(i);
    }

    QSqlQuery query;

    {
    // examine with named binding
    QMapIterator<QString, QCoreVariant> i(query.boundValues());
    while (i.hasNext()) {
        i.next();
        cout << i.key().ascii() << ": " << i.value().toString().ascii() << endl;
    }
    }

    {
    // examine with positional binding
    QList<QCoreVariant> list = query.boundValues().values();
    for (int i = 0; i < list.size(); ++i)
        cout << i << ": " << list.at(i).toString().ascii() << endl;
    }
}

void QSqlQueryModel_snippets()
{
    QSqlQueryModel model;
    model.setQuery("SELECT * FROM employee");
    int age = model.record(4).value("age").toInt();

    {
    int age = model.data(model.index(4, 3)).toInt();
    }

    for (int row = 0; row < model.rowCount(); ++row) {
        for (int col = 0; col < model.columnCount(); ++col) {
            qDebug() << model.data(model.index(row, col));
        }
    }

    model.insertRow(4);
    model.setData(model.index(4, 1), "Noah");

    model.removeRow(4);
}

int main()
{
    QSqlDatabase_snippets();
    QSqlField_snippets();
    QSqlQuery_snippets();
    QSqlQueryModel_snippets();
}
