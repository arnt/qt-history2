#include "qlistview_.h"
#include <private/qgenericlistview_p.h>

class QListModel : public QGenericItemModel
{
public:
    QListModel(QObject *parent = 0);

    virtual void setText(int row, const QString &text);
    virtual void setIconSet(int row, const QIconSet &iconSet);
    QString text(int row) const;
    QIconSet iconSet(int row) const;

    QListModelItem item(int row) const;
    void setItem(int row, const QListModelItem &item);
    void append(const QListModelItem &item);

private:
    QModelIndex index(int row, int column, const QModelIndex &parent = 0,
                      QModelIndex::Type type = QModelIndex::View) const;

    int rowCount(const QModelIndex &parent = 0) const;
    int columnCount(const QModelIndex &parent = 0) const;

    QVariant data(const QModelIndex &index, int role) const;
    void setData(const QModelIndex &index, int role, const QVariant &value);

    QModelIndex insertItem(const QModelIndex &index);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

private:
    QList<QListModelItem> lst;
};

QListModel::QListModel(QObject *parent)
    : QGenericItemModel(parent)
{
}

void QListModel::setText(int row, const QString &text)
{
    if (row >= 0 && row < (int)lst.count())
	lst[row].setText(text);
}

void QListModel::setIconSet(int row, const QIconSet &iconSet)
{
    if (row >= 0 && row < (int)lst.count())
	lst[row].setIconSet(iconSet);
}

QString QListModel::text(int row) const
{
    if (row >= 0 && row < (int)lst.count())
	return lst[row].text();
    return QString();
}

QIconSet QListModel::iconSet(int row) const
{
    if (row >= 0 && row < (int)lst.count())
	return lst[row].iconSet();
    return QIconSet();
}

QListModelItem QListModel::item(int row) const
{
    if (row >= 0 && row < (int)lst.count())
	return lst[row];
    else
	return QListModelItem(); // FIXME we need invalid?
}

void QListModel::setItem(int row, const QListModelItem &item)
{
    if (row >= 0 && row < (int)lst.count())
	lst[row] = item;
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
    return lst[index.row()].data(role);
}

void QListModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid() || index.row() >= (int)lst.count())
	return;
    lst[index.row()].setData(role, value);
    emit contentsChanged(index, index);
}

QModelIndex QListModel::insertItem(const QModelIndex &index)
{
    QListModelItem item;
    QModelIndex insert = index;
    if (insert.isValid() && insert.row() < rowCount()) {
 	lst.insert(insert.row(), item);
    } else {
 	lst.append(item);
	insert = QModelIndex(lst.count() - 1, 0);
    }
    return insert;
}

bool QListModel::isSelectable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
	return false;
    return lst[index.row()].isSelectable();
}

bool QListModel::isEditable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
	return false;
    return lst[index.row()].isEditable();
}

void QListModel::append(const QListModelItem &item)
{
    lst.append(item);
    QModelIndex idx(lst.count() - 1, 0);
    emit contentsInserted(idx, idx);
}

class QListView_Private : public QGenericListViewPrivate
{
    Q_DECLARE_PUBLIC(QListView_);
public:
    QListView_Private() : QGenericListViewPrivate() {}
    inline QListModel *model() const { return ::qt_cast<QListModel*>(q->model()); }
};

#define d d_func()
#define q q_func()

QListView_::QListView_(QWidget *parent)
    : QGenericListView(*new QListView_Private(), new QListModel(), parent)
{
    model()->setParent(this);
}

QListView_::~QListView_()
{
}

void QListView_::setText(int row, const QString &text)
{
    d->model()->setText(row, text);
}

void QListView_::setIconSet(int row, const QIconSet &iconSet)
{
    d->model()->setIconSet(row, iconSet);
}

QString QListView_::text(int row) const
{
    return d->model()->text(row);
}

QIconSet QListView_::iconSet(int row) const
{
    return d->model()->iconSet(row);
}

QListModelItem QListView_::item(int row) const
{
    return d->model()->item(row);
}

void QListView_::setItem(int row, const QListModelItem &item) {
    d->model()->setItem(row, item);
}

void QListView_::appendItem(const QListModelItem &item) {
    d->model()->append(item);
}
