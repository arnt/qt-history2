#include <QtGui>
#include "historymodel.h"

HistoryCompleter::HistoryCompleter(QObject *parent)
: QCompleter(parent)
{
}

QStringList HistoryCompleter::splitPath(const QString& filter) const
{
    if (QDir(filter).isAbsolute())
        return QCompleter::splitPath(filter);
    return QStringList(filter);
}

HistoryModel::HistoryModel(QObject *parent) 
: QDirModel(parent)
{  
}

QModelIndex HistoryModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row < QDirModel::rowCount())
            return QDirModel::index(row, column, parent);
        return createIndex(row, column);
    }

    if (!hasChildren(parent))
        return QModelIndex();

    return QDirModel::index(row, column, parent);
}

QModelIndex HistoryModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    if (child.internalPointer() == 0)
        return QModelIndex();

    return QDirModel::parent(child);
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return QDirModel::rowCount() + history.count();
    if (parent.internalPointer())
        return QDirModel::rowCount(parent);
    return 0;
}

int HistoryModel::columnCount(const QModelIndex &parent) const 
{
    return QDirModel::columnCount(parent);
}

bool HistoryModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;
    if (parent.internalPointer())
        return QDirModel::hasChildren(parent);
    return false; // our item
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const 
{
    if (!index.isValid())
        return QVariant();

    if (index.internalPointer())
        return QDirModel::data(index, role);

    if (role == Qt::DisplayRole && index.column() == 0)
        return history[index.row() - QDirModel::rowCount()].first;

    if (role == Qt::EditRole && index.column() == 0)
        return history[index.row() - QDirModel::rowCount()].second;

    return QVariant();
}

void HistoryModel::addToHistory(const QString& display, QString edit)
{
    if (edit.isNull())
        edit = display;
    qDebug() << "Adding " << display << " to history";
    history.append(QPair<QString, QString>(display, edit));
    reset();
}

void HistoryModel::addToHistory()
{
    if (!sender())
        return;
    QLineEdit *le = qobject_cast<QLineEdit *>(sender());
    if (!le)
        return;
    addToHistory(le->text());
}

