#include <QtSql>

#include "editablesqlmodel.h"

EditableSqlModel::EditableSqlModel(QObject *parent)
    : QSqlQueryModel(parent)
{
}

QAbstractItemModel::ItemFlags EditableSqlModel::flags(
        const QModelIndex &index) const
{
    if (index.column() == 2)
        return ItemIsEditable | ItemIsEnabled;
    else
        return ItemIsEnabled;
}

bool EditableSqlModel::setData(const QModelIndex &index, int /* role */,
                               const QVariant &value)
{
    if (index.column() != 2)
        return false;

    QModelIndex primaryKeyIndex = QSqlQueryModel::index(index.row(), 0);
    int id = data(primaryKeyIndex).toInt();

    clear();
    bool ok = setLastName(id, value.toString());
    setQuery("select * from person"); // ###???
    return ok;
}

bool EditableSqlModel::setLastName(int personId, const QString &lastName)
{
    QSqlQuery query;
    query.prepare("update person set lastname = ? where id = ?");
    query.addBindValue(lastName);
    query.addBindValue(personId);
    return query.exec();
}
