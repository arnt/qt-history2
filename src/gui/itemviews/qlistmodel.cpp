#include "qlistmodel.h"
#include <private/qobject_p.h>

bool QListModelItem::operator ==(const QListModelItem &other) const
{
    if (values.count() != other.values.count()
	|| edit != other.edit
	|| select != other.select)
	return false;

    for (int i=0; values.count(); ++i)
	if (values.at(i).role != other.values.at(i).role
	    || values.at(i).value != other.values.at(i).value)
	    return false;

    return true;
}

QVariant QListModelItem::data(int role) const
{
    for (int i=0; i<values.count(); ++i) {
	if (values.at(i).role == role)
	    return values.at(i).value;
    }
    return QVariant();
}

void QListModelItem::setData(int role, const QVariant &value)
{
    for (int i=0; i<values.count(); ++i) {
	if (values.at(i).role == role) {
	    values[i].value = value;
	    return;
	}
    }
    values.append(Data(role, value));
    return;
}

class QListModelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QListModel);
public:
    QList<QListModelItem> lst;
};

#define d d_func()
#define q q_func()

QListModel::QListModel(QObject *parent)
    : QGenericItemModel(*new QListModelPrivate, parent)
{
}

QListModel::~QListModel()
{
}

void QListModel::setText(int row, const QString &text)
{
    if (row >= 0 && row < (int)d->lst.count())
	d->lst[row].setText(text);
}

void QListModel::setIconSet(int row, const QIconSet &iconSet)
{
    if (row >= 0 && row < (int)d->lst.count())
	d->lst[row].setIconSet(iconSet);
}

QString QListModel::text(int row) const
{
    if (row >= 0 && row < (int)d->lst.count())
	return d->lst[row].text();
    return QString();
}

QIconSet QListModel::iconSet(int row) const
{
    if (row >= 0 && row < (int)d->lst.count())
	return d->lst[row].iconSet();
    return QIconSet();
}

QListModelItem QListModel::item(int row) const
{
    if (row >= 0 && row < (int)d->lst.count())
	return d->lst[row];
    else
	return QListModelItem(); // FIXME we need invalid?
}

void QListModel::setItem(int row, const QListModelItem &item)
{
    if (row >= 0 && row < (int)d->lst.count())
	d->lst[row] = item;
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
    return d->lst[index.row()].data(role);
}

void QListModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid() || index.row() >= (int)d->lst.count())
	return;
    d->lst[index.row()].setData(role, value);
    emit contentsChanged(index, index);
}

QModelIndex QListModel::insertItem(const QModelIndex &index)
{
    QListModelItem item;
    QModelIndex insert = index;
    if (insert.isValid() && insert.row() < rowCount()) {
 	d->lst.insert(insert.row(), item);
    } else {
 	d->lst.append(item);
	insert = QModelIndex(d->lst.count() - 1, 0);
    }
    return insert;
}

bool QListModel::isSelectable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)d->lst.count())
	return false;
    return d->lst[index.row()].isSelectable();
}

bool QListModel::isEditable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)d->lst.count())
	return false;
    return d->lst[index.row()].isEditable();
}

void QListModel::append(const QListModelItem &item)
{
    d->lst.append(item);
    QModelIndex idx(d->lst.count() - 1, 0);
    emit contentsInserted(idx, idx);
}
