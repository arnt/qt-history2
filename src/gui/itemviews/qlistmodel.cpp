#include "qlistmodel.h"
#include <qtl.h>

QListModelItem::QListModelItem(QListModel *model)
    : edit(true), select(true)
{
    if (model)
	model->append(this);
}

QListModelItem::QListModelItem(const QVariant &values)
    : edit(true), select(true)
{
    if (values.type() != QVariant::List)
	return;
    QList<QVariant> elementList = *reinterpret_cast< QList<QVariant> *>(&values.toList());
    QList<QVariant>::ConstIterator it = elementList.begin();
    for (int e = 0; e < elementList.count(); ++it, ++e) {
	if ((*it).type() == QVariant::String)
	    setText((*it).toString());
	else if ((*it).type() == QVariant::IconSet)
	    setIconSet((*it).toIconSet());
    }
}

QListModel::QListModel(QObject *parent, const char *name)
    : QGenericItemModel(parent, name)
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

QModelIndex QListModel::index(int row, int, const QModelIndex &) const
{
    if (row >= 0 && row < (int)lst.count())
	return QModelIndex(row, 0, 0);
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

QVariant QListModel::data(const QModelIndex &index, int element) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
	return QVariant();
    if (element == 0)
	return lst[index.row()]->text();
    if (element == 1)
	return lst[index.row()]->iconSet();
    return QVariant();
}

void QListModel::setData(const QModelIndex &index, int element, const QVariant &variant)
{
    if (!index.isValid() || index.row() >= (int)lst.count())
	return;
    if (element == 0)
	lst[index.row()]->setText(variant.toString());
    else if (element == 1)
	lst[index.row()]->setIconSet(variant.toIconSet());
    emit contentsChanged(index, index);
}

void QListModel::insertDataList(const QModelIndex &index, const QVariant &variant)
{
    lst.insert(index.row(), new QListModelItem(variant));
}

void QListModel::appendDataList(const QVariant &variant)
{
    (void)new QListModelItem(this);
    setDataList(index(rowCount() - 1, 0, 0), variant);
}

QVariant::Type QListModel::type(const QModelIndex &index, int element) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
	return QVariant::Invalid;
    if (element == 0)
	return QVariant::String;
    if (element == 1)
	return QVariant::IconSet;
    return QVariant::Invalid;
}

int QListModel::element(const QModelIndex &index, QVariant::Type type) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
	return -1;
    if (type == QVariant::String)
	return 0;
    if (type == QVariant::IconSet)
	return 1;
    return -1;
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
    lst.push_back(item);
    int r = lst.count();
    int bottom = qMax(r - 1, 0);
    QModelIndex idx = index(bottom, 0, 0);
    emit contentsInserted(idx, idx);
}
