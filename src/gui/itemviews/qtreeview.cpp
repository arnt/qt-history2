#include "qtreeview.h"
#include <private/qgenerictreeview_p.h>

class QTreeModel : public QAbstractItemModel
{
    friend class QTreeView;
    friend class QTreeViewItem;
public:
    QTreeModel(int columns = 0, QObject *parent = 0);
    ~QTreeModel();

    virtual void setColumnCount(int columns);
    int columnCount() const;

    virtual void setColumnText(int column, const QString &text);
    virtual void setColumnIconSet(int column, const QIconSet &iconSet);
    QString columnText(int column) const;
    QIconSet columnIconSet(int column) const;

    QTreeViewItem *item(const QModelIndex &index) const;

    QModelIndex index(QTreeViewItem *item) const;
    QModelIndex index(int row, int column, const QModelIndex &parent,
                      QModelIndex::Type type = QModelIndex::View) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    void setData(const QModelIndex &index, int role, const QVariant &value);
    QModelIndex insertItem(const QModelIndex &index);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

protected:
    void append(QTreeViewItem *item);
    void emitContentsInserted(QTreeViewItem *item);

private:
    int c;
    QList<QTreeViewItem*> tree;
    mutable QTreeViewItem topHeader;
};

QTreeModel::QTreeModel(int columns, QObject *parent)
    : QAbstractItemModel(parent), c(0)
{
    setColumnCount(columns);
}

QTreeModel::~QTreeModel()
{
    for (int i = 0; i < tree.count(); ++i)
	delete tree.at(i);
}

void QTreeModel::setColumnCount(int columns)
{
    if (c == columns)
	return;
    int _c = c;
    c = columns;
    topHeader.setColumnCount(c);
    for (int i = _c; i < c; ++i)
	topHeader.setText(i, QString::number(i));
    int r = rowCount(0);
    if (c > _c)
	emit contentsInserted(index(0, _c - 1, 0), index(r - 1, c - 1, 0));
    else
	emit contentsRemoved(0, index(0, c - 1, 0), index(r - 1, _c - 1, 0));
}

int QTreeModel::columnCount() const
{
    return c;
}

void QTreeModel::setColumnText(int column, const QString &text)
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QAbstractItemModel::Display, text);
}

void QTreeModel::setColumnIconSet(int column, const QIconSet &iconSet)
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QAbstractItemModel::Decoration, iconSet);
}

QString QTreeModel::columnText(int column) const
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    return data(index, QAbstractItemModel::Display).toString();
}

QIconSet QTreeModel::columnIconSet(int column) const
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    return data(index, QAbstractItemModel::Decoration).toIconSet();
}

QTreeViewItem *QTreeModel::item(const QModelIndex &index) const
{
    if (!index.isValid())
	return 0;
    if (index.type() != QModelIndex::View)
	return &topHeader;
    return (QTreeViewItem *)index.data();
}

QModelIndex QTreeModel::index(QTreeViewItem *item) const
{
    if (!item)
	return QModelIndex();
    const QTreeViewItem *par = item->parent();
    int row = par ? par->children.indexOf(item) : tree.indexOf(item);
    return QModelIndex(row, 0, item);
}

QModelIndex QTreeModel::index(int row, int column, const QModelIndex &parent,
                              QModelIndex::Type type) const
{
    int r = tree.count();
    if (row < 0 || row >= r || column < 0 || column >= c)
	return QModelIndex();
    if (!parent.isValid() && row < r) {// toplevel
	QTreeViewItem *itm = ((QTreeModel*)this)->tree[row]; // FIXME
	return QModelIndex(row, column, itm, type);
    }
    QTreeViewItem *parentItem = item(parent);
    if (parentItem && row < parentItem->childCount()) {
	QTreeViewItem *itm = (QTreeViewItem*)parentItem->child(row); // FIXME
	return QModelIndex(row, column, itm, type);
    }
    return QModelIndex();
}

QModelIndex QTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
	return QModelIndex();
    const QTreeViewItem *itm = (const QTreeViewItem*)child.data();
    if (!itm)
	return QModelIndex();
    QTreeViewItem *parent = (QTreeViewItem*)itm->parent(); // FIXME
    return index(parent);
}

int QTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
	QTreeViewItem *parentItem = item(parent);
	if (parentItem)
	    return parentItem->childCount();
    }
    return tree.count();
}

int QTreeModel::columnCount(const QModelIndex &) const
{
    return c;
}

QVariant QTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
	return QVariant();
    QTreeViewItem *itm = item(index);
    if (itm)
	return itm->data(index.column(), role);
    return QVariant();
}

void QTreeModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid())
	return;
    QTreeViewItem *itm = item(index);
    if (itm)
	itm->setData(index.column(), role, value);
    emit contentsChanged(index, index);
}

QModelIndex QTreeModel::insertItem(const QModelIndex &)
{
    // FIXME
    return QModelIndex();
}

bool QTreeModel::isSelectable(const QModelIndex &) const
{
    return true;
}

bool QTreeModel::isEditable(const QModelIndex &) const
{
    return true;
}

void QTreeModel::append(QTreeViewItem *item)
{
    tree.push_back(item);
    int r = tree.count();
    QModelIndex topLeft = index(r - 1, 0, 0);
    QModelIndex bottomRight = index(r - 1, c - 1, 0);
    emit contentsInserted(topLeft, bottomRight);
}

void QTreeModel::emitContentsInserted(QTreeViewItem *item)
{
    QModelIndex topLeft = index(item);
    QModelIndex parentIndex = parent(topLeft);
    QModelIndex bottomRight = index(topLeft.row(), columnCount(parentIndex) - 1, parentIndex);
    emit contentsInserted(topLeft, bottomRight);
}

// QTreeViewItem

QTreeViewItem::QTreeViewItem()
    : par(0), view(0), c(0), edit(true), select(true)
{
}

QTreeViewItem::QTreeViewItem(QTreeView *v)
    : par(0), view(v), c(0), edit(true), select(true)
{
    if (view)
	view->append(this);
}

QTreeViewItem::QTreeViewItem(QTreeViewItem *parent)
    : par(parent), view(parent->view), c(0), edit(true), select(true)
{
    if (parent)
	parent->children.push_back(this);
    QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
    model->emitContentsInserted(this);
}

QTreeViewItem::~QTreeViewItem()
{
    for (int i = 0; i < children.count(); ++i)
	delete children.at(i);
}

void QTreeViewItem::setColumnCount(int columns)
{
    c = columns;
    values.resize(columns);
}

QVariant QTreeViewItem::data(int column, int role) const
{
    if (column < 0 || column >= c)
	return QVariant();
    const QVector<Data> column_values = values.at(column);
    role = (role == QAbstractItemModel::Edit ? QAbstractItemModel::Display : role);
    for (int i = 0; i < column_values.count(); ++i) {
	if (column_values.at(i).role == role)
	    return column_values.at(i).value;
    }
    return QVariant();
}

void QTreeViewItem::setData(int column, int role, const QVariant &value)
{
    if (column >= c)
	setColumnCount(column + 1);
    QVector<Data> column_values = values.at(column);
    role = (role == QAbstractItemModel::Edit ? QAbstractItemModel::Display : role);
    for (int i = 0; i < column_values.count(); ++i) {
	if (column_values.at(i).role == role) {
	    values[column][i].value = value;
	    return;
	}
    }
    values[column].append(Data(role, value));
}

class QTreeViewPrivate : public QGenericTreeViewPrivate
{
    Q_DECLARE_PUBLIC(QTreeView);
public:
    QTreeViewPrivate() : QGenericTreeViewPrivate() {}
    inline QTreeModel *model() const { return ::qt_cast<QTreeModel*>(q->model()); }
};

#define d d_func()
#define q q_func()

/*!
  \class QTreeView qtreeview.h

  \brief Tree view implementation using the QTreeModel by default
*/


QTreeView::QTreeView(QWidget *parent)
    : QGenericTreeView(new QTreeModel, parent)
{
    model()->setParent(this); // make sure the model gets deleted
}

void QTreeView::setColumnCount(int columns)
{
    d->model()->setColumnCount(columns);
}

int QTreeView::columnCount() const
{
    return model()->columnCount();
}

void QTreeView::setColumnText(int column, const QString &text)
{
    d->model()->setColumnText(column, text);
}

void QTreeView::setColumnIconSet(int column, const QIconSet &iconSet)
{
    d->model()->setColumnIconSet(column, iconSet);
}

QString QTreeView::columnText(int column) const
{
    return d->model()->columnText(column);
}

QIconSet QTreeView::columnIconSet(int column) const
{
    return d->model()->columnIconSet(column);
}

void QTreeView::append(QTreeViewItem *item)
{
    d->model()->append(item);
}
