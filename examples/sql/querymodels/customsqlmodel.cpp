#include "customsqlmodel.h"

CustomSqlModel::CustomSqlModel(QObject *parent)
    : QSqlQueryModel(parent)
{
}

QVariant CustomSqlModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlQueryModel::data(index, role);
    if (value.isValid() && index.column() == 0
            && role == QAbstractItemModel::DisplayRole)
        return value.toString().prepend("#");
    return value;
}
