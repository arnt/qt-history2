#include "qlistmodel.h"
#include <qalgorithms.h>
#include <private/qobject_p.h>

QListModelItem::QListModelItem(QListModel *model)
    : edit(true), select(true)
{
    if (model)
	model->append(this);
}




class QListModelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QListModel);
public:
    QList<QListModelItem*> lst;
};
#define d d_func()
#define q q_func()

QListModel::QListModel(QObject *parent)
    : QGenericItemModel(*new QListModelPrivate, parent)
{
}

QListModel::~QListModel()
{
    qDeleteAll(d->lst);
}

void QListModel::setText(int row, const QString &text)
{
    if (row >= 0 && row < (int)d->lst.count())
	d->lst[row]->setText(text);
}

void QListModel::setIconSet(int row, const QIconSet &iconSet)
{
    if (row >= 0 && row < (int)d->lst.count())
	d->lst[row]->setIconSet(iconSet);
}

QString QListModel::text(int row) const
{
    if (row >= 0 && row < (int)d->lst.count())
	return d->lst[row]->text();
    return QString();
}

QIconSet QListModel::iconSet(int row) const
{
    if (row >= 0 && row < (int)d->lst.count())
	return d->lst[row]->iconSet();
    return QIconSet();
}

const QListModelItem *QListModel::item(const QModelIndex &index) const
{
    if (!index.isValid())
	return 0;
    if (index.row() < (int)d->lst.count())
	return d->lst.at(index.row());
    return 0;
}

QModelIndex QListModel::index(QListModelItem *item) const
{
    if (!item)
	return QModelIndex();
    int row = d->lst.indexOf(item);
    return QModelIndex(row, 0, 0);
}

QModelIndex QListModel::index(int row, int, const QModelIndex &, QModelIndex::Type type) const
{
    if (row >= 0 && row < (int)d->lst.count())
	return QModelIndex(row, 0, 0, type);
    return QModelIndex();
}

int QListModel::rowCount(const QModelIndex &) const
{
    return d->lst.count();
}

int QListModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant QListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= (int)d->lst.count())
	return QVariant();
    if (role == QGenericItemModel::Display)
	return d->lst[index.row()]->text();
    if (role == QGenericItemModel::Decoration)
	return d->lst[index.row()]->iconSet();
    return QVariant();
}

void QListModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid() || index.row() >= (int)d->lst.count())
	return;
    if (role == QGenericItemModel::Display)
	d->lst[index.row()]->setText(value.toString());
    else if (role == QGenericItemModel::Decoration)
	d->lst[index.row()]->setIconSet(value.toIconSet());
    emit contentsChanged(index, index);
}

QModelIndex QListModel::insertItem(const QModelIndex &index)
{
    QListModelItem *item = new QListModelItem();
    if (index.isValid() && index.row() < rowCount())
	d->lst.insert(index.row(), item);
    else
	d->lst.append(item);
    return QListModel::index(item);
}

bool QListModel::isSelectable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)d->lst.count())
	return false;
    return d->lst[index.row()]->isSelectable();
}

bool QListModel::isEditable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)d->lst.count())
	return false;
    return d->lst[index.row()]->isEditable();
}

void QListModel::append(QListModelItem *item)
{
    d->lst.append(item);
    QModelIndex idx = index(qMax(d->lst.count() - 1, 0), 0, 0);
    emit contentsInserted(idx, idx);
}
