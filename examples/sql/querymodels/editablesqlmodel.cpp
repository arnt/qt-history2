#include <QtSql>

#include "editablesqlmodel.h"

EditableSqlModel::EditableSqlModel(QObject *parent)
    : QSqlQueryModel(parent)
{
}

bool EditableSqlModel::isEditable(const QModelIndex &index) const
{
    return (index.column() == 2);
}

bool EditableSqlModel::setData(const QModelIndex &index, int role,
                               const QVariant &value)
{
    if (index.column() != 2)
        return false;

    QModelIndex primaryKeyIndex = QSqlQueryModel::index(index.row(), 0);
    int id = data(primaryKeyIndex).toInt();

    bool ok = editLastName(id, value.toString());

    setQuery("select * from persons"); // ###???
    return ok;
}

bool EditableSqlModel::editLastName(int primaryKey, const QString &newValue)
{
    clear();

    QSqlQuery query;
    query.prepare("update persons set lastname = ? where id = ?");
    query.addBindValue(newValue);
    query.addBindValue(primaryKey);
    return query.exec();
}
