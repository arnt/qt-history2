#include "customsqlmodel.h"

#include <qcolor.h>

CustomSqlModel::CustomSqlModel(QObject *parent)
    : QSqlQueryModel(parent)
{
}

QVariant CustomSqlModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlQueryModel::data(index, role);
    if (value.isValid() && role == Qt::DisplayRole) {
        if (index.column() == 0)
            return value.toString().prepend("#");
        else if (index.column() == 2)
            return value.toString().toUpper();
    }
    if (role == Qt::TextColorRole && index.column() == 1)
        return qVariantFromValue(QColor(Qt::blue));
    return value;
}
