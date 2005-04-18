#include "pimmodel.h"

PimModel::PimModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

PimModel::~PimModel()
{

}

int PimModel::rowCount(const QModelIndex &) const
{
    return entries.count();
}

QVariant PimModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        switch (role) {
        case PhotoRole: return qVariantFromValue(entries.at(index.row()).photo);
        case FirstNameRole: return entries.at(index.row()).firstName;
        case LastNameRole: return entries.at(index.row()).lastName;
        case MiddleNameRole: return entries.at(index.row()).middleName;
        case CompanyRole: return entries.at(index.row()).company;
        case DepartmentRole: return entries.at(index.row()).department;
        case JobTitleRole: return entries.at(index.row()).jobTitle;
        default: break;
        }
    }
    return QVariant();
}

bool PimModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()) {
        switch (role) {
        case PhotoRole:
            entries[index.row()].photo = qvariant_cast<QPixmap>(value);
            break;
        case FirstNameRole:
            entries[index.row()].firstName = value.toString();
            break;
        case LastNameRole:
            entries[index.row()].lastName = value.toString();
            break;
        case MiddleNameRole:
            entries[index.row()].middleName = value.toString();
            break;
        case CompanyRole:
            entries[index.row()].company = value.toString();
            break;
        case DepartmentRole:
            entries[index.row()].department = value.toString();
            break;
        case JobTitleRole:
            entries[index.row()].jobTitle = value.toString();
            break;
        default:
            return false;
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool PimModel::insertRows(int row, int count, const QModelIndex &)
{
    beginInsertRows(QModelIndex(), row, row + count - 1);
    while (count--)
        entries.insert(row, PimEntry());
    endInsertRows();
    return true;
}

bool PimModel::removeRows(int row, int count, const QModelIndex &)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    while (count--)
        entries.removeAt(row);
    endRemoveRows();
    return true;
}

const PimEntry &PimModel::entry(int row) const
{
    return entries.at(row);
}

bool PimModel::appendEntry(const PimEntry &entry)
{
    int row = entries.count();
    beginInsertRows(QModelIndex(), row, row);
    entries.append(entry);
    endInsertRows();
    return true;
}

bool PimModel::setEntry(int row, const PimEntry &entry)
{
    entries[row] = entry;
    QModelIndex idx = index(row);
    emit dataChanged(idx, idx);
    return true;
}

bool PimModel::removeEntry(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    entries.removeAt(row);
    endRemoveRows();
    return true;
}
