#include "qlistmodel.h"
#include <qalgorithms.h>

QListModelItem::QListModelItem(QListModel *model)
    : edit(true), select(true)
{
    if (model)
	model->append(this);
}

QListModel::QListModel(QObject *parent)
    : QGenericItemModel(parent)
{
}

QListModel::~QListModel()
{
    qDeleteAll(lst);
}

void QListModel::setText(int row, const QString &text)
{
    if (row >= 0 && row < (int)lst.count())
	lst[row]->setText(text);
}

void QListModel::setIconSet(int row, const QIconSet &iconSet)
{
    if (row >= 0 && row < (int)lst.count())
	lst[row]->setIconSet(iconSet);
}

QString QListModel::text(int row) const
{
    if (row >= 0 && row < (int)lst.count())
	return lst[row]->text();
    return QString();
}

QIconSet QListModel::iconSet(int row) const
{
    if (row >= 0 && row < (int)lst.count())
	return lst[row]->iconSet();
    return QIconSet();
}

const QListModelItem *QListModel::item(const QModelIndex &index) const
{
    if (!index.isValid())
	return 0;
    if (index.row() < (int)lst.count())
	return lst.at(index.row());
    return 0;
}

QModelIndex QListModel::index(QListModelItem *item) const
{
    if (!item)
	return QModelIndex();
    int row = lst.indexOf(item);
    return QModelIndex(row, 0, 0);
}

QModelIndex QListModel::index(int row, int, const QModelIndex &, QModelIndex::Type type) const
{
    if (row >= 0 && row < (int)lst.count())
	return QModelIndex(row, 0, 0, type);
    return QModelIndex();
}

int QListModel::rowCount(const QModelIndex &) const
{
    return lst.count();
}

int QListModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant QListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
	return QVariant();
    if (role == QGenericItemModel::Display)
	return lst[index.row()]->text();
    if (role == QGenericItemModel::Decoration)
	return lst[index.row()]->iconSet();
    return QVariant();
}

void QListModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid() || index.row() >= (int)lst.count())
	return;
    if (role == QGenericItemModel::Display)
	lst[index.row()]->setText(value.toString());
    else if (role == QGenericItemModel::Decoration)
	lst[index.row()]->setIconSet(value.toIconSet());
    emit contentsChanged(index, index);
}

QModelIndex QListModel::insertItem(const QModelIndex &index)
{
    QListModelItem *item = new QListModelItem();
    if (index.isValid() && index.row() < rowCount())
	lst.insert(index.row(), item);
    else
	lst.append(item);
    return QListModel::index(item);
}

bool QListModel::isSelectable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
	return false;
    return lst[index.row()]->isSelectable();
}

bool QListModel::isEditable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
	return false;
    return lst[index.row()]->isEditable();
}

void QListModel::append(QListModelItem *item)
{
    lst.append(item);
    QModelIndex idx = index(qMax(lst.count() - 1, 0), 0, 0);
    emit contentsInserted(idx, idx);
}
