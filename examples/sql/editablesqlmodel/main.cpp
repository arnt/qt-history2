/****************************************************************************
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is an example program for the Qt SQL module.
 ** EDITIONS: NOLIMITS
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

/* This examples demonstrates how to subclass QSqlModel to allow
   editing of database values.
 */

#include <qapplication.h>
#include <qgenerictableview.h>
#include <qsqldatabase.h>
#include <qsqldriver.h>
#include <qsqlmodel.h>
#include <qsqlquery.h>

static void createConnection()
{
    // create the database connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        qWarning("This example requires the QSQLITE driver");
        return;
    }

    // create and populate a test table
    QSqlQuery q;
    q.exec("create table persons (id int primary key, firstname varchar(20), "
           "lastname varchar(20)");
    q.exec("insert into persons values(1, 'Arthur', 'Tulip')");
    q.exec("insert into persons values(2, 'Scribe', 'Dent')");
    q.exec("insert into persons values(3, 'Peter', 'Arthurson')");
}

class EditableSqlModel: public QSqlModel
{
public:
    EditableSqlModel(QObject *parent = 0): QSqlModel(parent) {}

    bool isEditable(const QModelIndex &index) const;
    bool setData(const QModelIndex &idx, int role, const QVariant &value);

private:
    bool editLastName(int pkey, const QString &newValue);
};

bool EditableSqlModel::isEditable(const QModelIndex &index) const
{
    if (index.type() != QModelIndex::View)
        return QSqlModel::isEditable(index);
    if (index.column() != 2)
        return false;
    return true;
}

bool EditableSqlModel::setData(const QModelIndex &idx, int role, const QVariant &value)
{
    if (idx.type() != QModelIndex::View)
        return QSqlModel::setData(idx, role, value);
    if (idx.column() != 2)
        return false;

    QModelIndex primaryKeyIndex = index(idx.row(), 0);
    int id = data(primaryKeyIndex).toInt();

    if (!editLastName(id, value.toString()))
        return false;

    setQuery("select * from persons");
    return true;
}

bool EditableSqlModel::editLastName(int pkey, const QString &newValue)
{
    QSqlQuery q;
    q.prepare("update table persons set lastname = ? where id = ?");
    q.addBindValue(newValue);
    q.addBindValue(pkey);
    return q.exec();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    createConnection();

    EditableSqlModel *model = new EditableSqlModel(&app);
    model->setQuery("select * from persons");

    QGenericTableView view(model, 0);
    app.setMainWidget(&view);
    view.show();

    return app.exec();
}

