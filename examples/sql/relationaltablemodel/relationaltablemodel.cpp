#include <QtGui>
#include <QtSql>

#include "../connection.h"

void initializeModel(QSqlRelationalTableModel *model)
{
    model->setTable("employees");

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setRelation(2, QSqlRelation("cities", "id", "name"));
    model->setRelation(3, QSqlRelation("countries", "id", "name"));

    model->setHeaderData(0, Qt::Horizontal, QAbstractItemModel::DisplayRole,
                         QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QAbstractItemModel::DisplayRole,
                         QObject::tr("Name"));
    model->setHeaderData(2, Qt::Horizontal, QAbstractItemModel::DisplayRole,
                         QObject::tr("City"));
    model->setHeaderData(3, Qt::Horizontal, QAbstractItemModel::DisplayRole,
                         QObject::tr("Country"));

    model->select();
}

QTableView *createView(const QString &title, QSqlTableModel *model)
{
    QTableView *view = new QTableView;
    view->setModel(model);
    view->setWindowTitle(title);
    return view;
}

void createRelationalTables()
{
    QSqlQuery query;
    query.exec("create table employees(id int, name varchar(20), city int, country int)");
    query.exec("insert into employees values(1, 'Espen', 5000, 47)");
    query.exec("insert into employees values(2, 'Harald', 80000, 49)");
    query.exec("insert into employees values(3, 'Sam', 100, 1)");

    query.exec("create table cities(id int, name varchar(20))");
    query.exec("insert into cities values(100, 'San Jose')");
    query.exec("insert into cities values(5000, 'Oslo')");
    query.exec("insert into cities values(80000, 'Munich')");

    query.exec("create table countries(id int, name varchar(20))");
    query.exec("insert into countries values(1, 'USA')");
    query.exec("insert into countries values(47, 'Norway')");
    query.exec("insert into countries values(49, 'Germany')");
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (!createConnection())
        return 1;
    createRelationalTables();

    QSqlRelationalTableModel model;

    initializeModel(&model);

    QTableView *view = createView(QObject::tr("Relational Table Model"), &model);
    view->show();

    app.setMainWidget(view);
    return app.exec();
}
