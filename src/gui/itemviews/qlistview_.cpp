#include "qlistview_.h"
#include <private/qgenericlistview_p.h>

class QListModel : public QAbstractItemModel
{
public:
    QListModel(QObject *parent = 0);

    QListView_Item item(int row) const;
    void setItem(int row, const QListView_Item &item);
    void append(const QListView_Item &item);

    int rowCount(const QModelIndex &parent = 0) const;
    int columnCount(const QModelIndex &parent = 0) const;

    QVariant data(const QModelIndex &index, int role) const;
    void setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRow(const QModelIndex &parent, int row);
    bool removeRow(const QModelIndex &parent,int row);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

private:
    QList<QListView_Item> lst;
};

QListModel::QListModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QListView_Item QListModel::item(int row) const
{
    if (row >= 0 && row < (int)lst.count())
        return lst.at(row);
    else
        return QListView_Item(); // FIXME we need invalid?
}

void QListModel::setItem(int row, const QListView_Item &item)
{
    if (row >= 0 && row < (int)lst.count())
        lst[row] = item;
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
    return lst.at(index.row()).data(role);
}

void QListModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid() || index.row() >= (int)lst.count())
        return;
    lst[index.row()].setData(role, value);
    emit contentsChanged(index, index);
}

bool QListModel::insertRow(const QModelIndex &parent, int row)
{
    QListView_Item item;
    if (row < rowCount(parent))
        lst.insert(row, item);
    else
        lst.append(item);
    return true;
}

bool QListModel::removeRow(const QModelIndex &parent, int row)
{
    if (row < rowCount(parent)) {
        lst.removeAt(row);
        return true;
    }
    return false;
}

bool QListModel::isSelectable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
        return false;
    return lst.at(index.row()).isSelectable();
}

bool QListModel::isEditable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
        return false;
    return lst.at(index.row()).isEditable();
}

void QListModel::append(const QListView_Item &item)
{
    lst.append(item);
    QModelIndex idx(lst.count() - 1, 0);
    emit contentsInserted(idx, idx);
}

bool QListView_Item::operator ==(const QListView_Item &other) const
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

QVariant QListView_Item::data(int role) const
{
    role = (role == QAbstractItemModel::Edit ? QAbstractItemModel::Display : role);
    for (int i = 0; i < values.count(); ++i) {
        if (values.at(i).role == role)
            return values.at(i).value;
    }
    return QVariant();
}

void QListView_Item::setData(int role, const QVariant &value)
{
    role = (role == QAbstractItemModel::Edit ? QAbstractItemModel::Display : role);
    for (int i = 0; i < values.count(); ++i) {
        if (values.at(i).role == role) {
            values[i].value = value;
            return;
        }
    }
    values.append(Data(role, value));
}

class QListView_Private : public QGenericListViewPrivate
{
    Q_DECLARE_PUBLIC(QListView_)
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
    setSpacing(0);
}

QListView_::~QListView_()
{
}

void QListView_::setText(int row, const QString &text)
{
    model()->setData(model()->index(row,0), QAbstractItemModel::Display, text);
}

void QListView_::setIconSet(int row, const QIconSet &iconSet)
{
    model()->setData(model()->index(row,0), QAbstractItemModel::Decoration, iconSet);
}

QString QListView_::text(int row) const
{
    return model()->data(model()->index(row,0),
                         QAbstractItemModel::Display).toString();
}

QIconSet QListView_::iconSet(int row) const
{
    return model()->data(model()->index(row,0),
                         QAbstractItemModel::Decoration).toIconSet();
}

QListView_Item QListView_::item(int row) const
{
    return d->model()->item(row);
}

void QListView_::setItem(int row, const QListView_Item &item)
{
    d->model()->setItem(row, item);
}

void QListView_::appendItem(const QListView_Item &item)
{
    d->model()->append(item);
}
