#include "qlistview.h"
#include <private/qgenericlistview_p.h>

class QListModel : public QAbstractItemModel
{
public:
    QListModel(QObject *parent = 0);

    QListViewItem item(int row) const;
    void setItem(int row, const QListViewItem &item);
    void append(const QListViewItem &item);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::Display) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRows(int row, const QModelIndex &parent = QModelIndex(), int count = 1);
    bool removeRows(int row, const QModelIndex &parent = QModelIndex(), int count = 1);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

private:
    QList<QListViewItem> lst;
};

QListModel::QListModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QListViewItem QListModel::item(int row) const
{
    if (row >= 0 && row < (int)lst.count())
        return lst.at(row);
    else
        return QListViewItem(); // FIXME we need invalid?
}

void QListModel::setItem(int row, const QListViewItem &item)
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

bool QListModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid() || index.row() >= (int)lst.count())
        return false;
    lst[index.row()].setData(role, value);
    emit dataChanged(index, index);
    return true;
}

bool QListModel::insertRows(int row, const QModelIndex &parent, int)
{
    QListViewItem item;
    if (row < rowCount(parent))
        lst.insert(row, item);
    else
        lst.append(item);
    return true;
}

bool QListModel::removeRows(int row, const QModelIndex &parent, int)
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

void QListModel::append(const QListViewItem &item)
{
    lst.append(item);
    int row = lst.count() - 1;
    emit rowsInserted(QModelIndex(), row, row);
}

bool QListViewItem::operator ==(const QListViewItem &other) const
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

QVariant QListViewItem::data(int role) const
{
    role = (role == QAbstractItemModel::Edit ? QAbstractItemModel::Display : role);
    for (int i = 0; i < values.count(); ++i) {
        if (values.at(i).role == role)
            return values.at(i).value;
    }
    return QVariant();
}

void QListViewItem::setData(int role, const QVariant &value)
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

class QListViewPrivate : public QGenericListViewPrivate
{
    Q_DECLARE_PUBLIC(QListView)
public:
    QListViewPrivate() : QGenericListViewPrivate() {}
    inline QListModel *model() const { return ::qt_cast<QListModel*>(q_func()->model()); }
};

#define d d_func()
#define q q_func()

QListView::QListView(QWidget *parent)
    : QGenericListView(*new QListViewPrivate(), new QListModel(), parent)
{
    model()->setParent(this);
    setSpacing(0);
}

QListView::~QListView()
{
}

void QListView::setText(int row, const QString &text)
{
    model()->setData(model()->index(row,0), QAbstractItemModel::Display, text);
}

void QListView::setIconSet(int row, const QIconSet &iconSet)
{
    model()->setData(model()->index(row,0), QAbstractItemModel::Decoration, iconSet);
}

QString QListView::text(int row) const
{
    return model()->data(model()->index(row,0),
                         QAbstractItemModel::Display).toString();
}

QIconSet QListView::iconSet(int row) const
{
    return model()->data(model()->index(row,0),
                         QAbstractItemModel::Decoration).toIconSet();
}

QListViewItem QListView::item(int row) const
{
    return d->model()->item(row);
}

void QListView::setItem(int row, const QListViewItem &item)
{
    d->model()->setItem(row, item);
}

void QListView::appendItem(const QListViewItem &item)
{
    d->model()->append(item);
}
