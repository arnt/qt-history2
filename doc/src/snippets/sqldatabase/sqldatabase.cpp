#include <QtGui>
#include <QtSql>

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
    {
    QSqlQueryModel *model = new QSqlQueryModel;
    model->setQuery("SELECT name, salary FROM employee");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"),
                         QAbstractItemModel::DisplayRole);
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Salary"),
                         QAbstractItemModel::DisplayRole);

    QTableView *view = new QTableView;
    view->setModel(model);
    view->show();
    }

    QSqlQueryModel model;
    model.setQuery("SELECT * FROM employee");
    int salary = model.record(4).value("salary").toInt();

    {
    int salary = model.data(model.index(4, 2)).toInt();
    }

    for (int row = 0; row < model.rowCount(); ++row) {
        for (int col = 0; col < model.columnCount(); ++col) {
            qDebug() << model.data(model.index(row, col));
        }
    }
}

class MyModel : public QSqlQueryModel
{
public:
    QVariant data(const QModelIndex &item, int role) const;

    int m_specialColumnNo;
};

QVariant MyModel::data(const QModelIndex &item, int role) const
{
    if (item.column() == m_specialColumnNo) {
        // handle column separately
    }
    return QSqlQueryModel::data(item, role);
}

void QSqlTableModel_snippets()
{
    QSqlTableModel *model = new QSqlTableModel;
    model->setTable("employee");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    model->removeColumn(0); // don't show the ID
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"),
                         QAbstractItemModel::DisplayRole);
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Salary"),
                         QAbstractItemModel::DisplayRole);

    QTableView *view = new QTableView;
    view->setModel(model);
    view->show();

    {
    QSqlTableModel model;
    model.setTable("employee");
    QString name = model.record(4).value("name").toString();
    }
}

class XyzResult : public QSqlResult
{
public:
    XyzResult(const QSqlDriver *driver)
        : QSqlResult(driver) {}
    ~XyzResult() {}

protected:
    QCoreVariant data(int /* index */) { return QCoreVariant(); }
    bool isNull(int /* index */) { return false; }
    bool reset(const QString & /* query */) { return false; }
    bool fetch(int /* index */) { return false; }
    bool fetchFirst() { return false; }
    bool fetchLast() { return false; }
    int size() { return 0; }
    int numRowsAffected() { return 0; }
    QSqlRecord record() { return QSqlRecord(); }
};

class XyzDriver : public QSqlDriver
{
public:
    XyzDriver() {}
    ~XyzDriver() {}

    bool hasFeature(DriverFeature /* feature */) const { return false; }
    bool open(const QString & /* db */, const QString & /* user */,
              const QString & /* password */, const QString & /* host */,
              int /* port */, const QString & /* options */)
        { return false; }
    void close() {}
    QSqlResult *createResult() const { return new XyzResult(this); }
};

int main()
{
    QSqlDatabase_snippets();
    QSqlField_snippets();
    QSqlQuery_snippets();
    QSqlQueryModel_snippets();
    QSqlTableModel_snippets();

    XyzDriver driver;
    XyzResult result(&driver);
}
