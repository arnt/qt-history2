#include <qpair.h>
#include "qstandarditemmodel.h"
#include <qdebug.h>

class  ModelItem {
public:
    ModelItem() {};
    QVariant value(int role)
        {
            for (int i=0; i<roles.count(); ++i)
                if (roles.at(i).first == role)
                    return roles.at(i).second;
            return QVariant::Invalid;
        }
    void setValue(int role, QVariant value)
        {
            for (int i=0; i<roles.count(); ++i)
                if (roles.at(i).first == role) {
                    roles[i].second = value;
                    return;
                }
            roles.append(QPair<int, QVariant>(role, value));
            return;
        }

private:
    QVector<QPair<int, QVariant> > roles;
};

class ModelRow {
public:
    ModelRow(ModelRow *parent) : p(parent), childrenColumns(0) {}
    ~ModelRow()
        {
            for (int i=0; i<items.count(); ++i)
                delete items.at(i);
            items.clear();
            for (int i=0; i<childrenRows.count(); ++i)
                delete childrenRows.at(i);
            childrenRows.clear();
        }

    ModelRow *p;
    QVector<ModelItem*> items;
    int childrenColumns;
    QVector<ModelRow*> childrenRows;
};


QStandardItemModel::~QStandardItemModel()
{
    for (int i=0; i<topLevelRows.count(); ++i)
        delete topLevelRows.at(i);
    topLevelRows.clear();
}

QModelIndex QStandardItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent)) {
        if (parent.isValid()) {
            ModelRow *parentRow = containedRow(parent, false);
            return createIndex(row, column, parentRow);
        } else {
            return createIndex(row, column, 0);
        }
    }
    return QModelIndex::Null;
}

QModelIndex QStandardItemModel::parent(const QModelIndex &child) const
{
    if (child.isValid() && child.data()) {
        ModelRow *parentRow = static_cast<ModelRow*>(child.data());
        ModelRow *grandParentRow = parentRow ? parentRow->p : 0;
        QVector<ModelRow*> grandParentChildren = topLevelRows;
        if (grandParentRow)
            grandParentChildren = grandParentRow->childrenRows;
        // ### slow, use ptr trick
        int row = grandParentChildren.indexOf(parentRow);
        return createIndex(row, 0, grandParentRow);
    }
    return QModelIndex::Null;
}

int QStandardItemModel::rowCount(const QModelIndex &parent) const
{
    ModelRow *modelRow = containedRow(parent, false);
    if (modelRow)
        return modelRow->childrenRows.count();

    return topLevelRows.count();
}

int QStandardItemModel::columnCount(const QModelIndex &parent) const
{
    ModelRow *modelRow = containedRow(parent, false);
    if (modelRow)
        return modelRow->childrenColumns;

    return topLevelColumns;
}

bool QStandardItemModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        ModelRow *modelRow = containedRow(parent, false);
        if (modelRow)
            return modelRow->childrenRows.count() && modelRow->childrenColumns;
    }
    return false;
}

QVariant QStandardItemModel::data(const QModelIndex &index, int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    if (index.isValid()) {
        ModelRow *modelRow = containedRow(index, false);
        if (modelRow && modelRow->items.count() > index.column()) {
            ModelItem *item = modelRow->items.at(index.column());
            if (item)
                return item->value(role);
        }
    }
    return QVariant::Invalid;
}

bool QStandardItemModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    if (index.isValid()) {
        ModelRow *modelRow = containedRow(index, true);
        int count = modelRow->items.count();
        // make room for enough items
        if (count <= index.column())
            modelRow->items.insert(count, index.column() + 1 - count, 0);
        // make sure we have a ModelItem at the position
        if (!modelRow->items.at(index.column()))
            modelRow->items[index.column()] = new ModelItem();
        modelRow->items.at(index.column())->setValue(role, value);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool QStandardItemModel::insertRows(int row, const QModelIndex &parent, int count)
{
    if (count < 1 || row < 0 || row > rowCount(parent))
        return false;

    QVector<ModelRow*> &rows = (parent.isValid()) ? containedRow(parent, true)->childrenRows
                               : topLevelRows;
    rows.insert(row, count, 0);
    emit rowsInserted(parent, row, row + count - 1);
    return true;
}

bool QStandardItemModel::insertColumns(int column, const QModelIndex &parent, int count)
{
    if (count < 0 || column < 0 || column > columnCount(parent))
        return false;

    QVector<ModelRow*> *rows = &topLevelRows;
    // update the column counters
    if (parent.isValid()) {
        ModelRow *modelRow = containedRow(parent, true);
        modelRow->childrenColumns += count;
        rows = &modelRow->childrenRows;
    } else {
        topLevelColumns += count;
    }

    // update any item vectors if needed
    for (int i=0; i<rows->count(); ++i) {
        ModelRow *modelRow = rows->at(i);
        // only insert if there is a ModelRow and ModelItems in it
        if (modelRow && modelRow->items.count()
              && modelRow->items.count() > column)
            modelRow->items.insert(column, count, 0);
    }
    emit columnsInserted(parent, column, column + count - 1);
    return true;
}

bool QStandardItemModel::removeRows(int row, const QModelIndex &parent, int count)
{
    if (count < 1 || row < 0 || (row + count) > rowCount(parent))
        return false;

    emit rowsAboutToBeRemoved(parent, row, row + count - 1);

    QVector<ModelRow*> &rows = (parent.isValid()) ? containedRow(parent, false)->childrenRows
                               : topLevelRows;
    // delete ModelRows
    for (int i=row; i<(row+count); ++i)
        delete rows.at(i);
    // resize row vector
    rows.remove(row, count);
    return true;
}

bool QStandardItemModel::removeColumns(int column, const QModelIndex &parent, int count)
{
    if (count < 1 || column < 0 || (column + count) > columnCount(parent))
        return false;

    emit columnsAboutToBeRemoved(parent, column, column + count - 1);

    QVector<ModelRow*> *rows = &topLevelRows;
    // update the column counters
    if (parent.isValid()) {
        ModelRow *modelRow = containedRow(parent, true);
        modelRow->childrenColumns -= count;
        rows = &modelRow->childrenRows;
    } else {
        topLevelColumns -= count;
    }

    // iterate over all child rows and remove if needed
    for (int i=0; i<rows->count(); ++i) {
        ModelRow *modelRow = rows->at(i);
        if (modelRow && column < modelRow->items.count()) {
            // delete the ModelItem if any
            for (int j=column; j<(column+count) && j<modelRow->items.count(); ++j)
                delete modelRow->items.at(j);
            // resize item vector
            modelRow->items.remove(column, qMin(count, modelRow->items.count() - column));
        }
    }
    return true;
}

ModelRow *QStandardItemModel::containedRow(const QModelIndex &index, bool createIfMissing) const
{
    if (!index.isValid())
        return 0;

    ModelRow *parentRow = static_cast<ModelRow*>(index.data());
    QVector<ModelRow*> *rowList = const_cast<QVector<ModelRow*> *>(&topLevelRows);
    if (parentRow) {
        rowList = const_cast<QVector<ModelRow*> *>(&parentRow->childrenRows);
    }

    if (createIfMissing && !rowList->at(index.row()))
        rowList->replace(index.row() , new ModelRow(parentRow));
    return rowList->at(index.row());
}
