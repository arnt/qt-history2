#ifndef QSTANDARDITEMMODEL_H
#define QSTANDARDITEMMODEL_H

#include <qvariant.h>
#include <qvector.h>
#include <qdebug.h>
#include <qabstractitemmodel.h>

class ModelItem;
class ModelRow;

class QStandardItemModel : public QAbstractItemModel
{
public:
    QStandardItemModel(QObject *parent = 0) : QAbstractItemModel(parent), topLevelColumns(0) {}
    ~QStandardItemModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &) const;

    bool hasChildren(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRows(int row, const QModelIndex &parent, int count);
    bool insertColumns(int column, const QModelIndex &parent, int count);
    bool removeRows(int row, const QModelIndex &parent, int count);
    bool removeColumns(int column, const QModelIndex &parent, int count);

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const
        {
            return QAbstractItemModel::flags(index) | QAbstractItemModel::ItemIsEditable;
        }

private:
    ModelRow *containedRow(const QModelIndex &index, bool createIfMissing) const;

    QVector<ModelRow*> topLevelRows;
    int topLevelColumns;
};

#endif //QSTANDARDITEMMODEL_H
