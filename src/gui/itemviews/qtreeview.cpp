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
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex(),
                      QModelIndex::Type type = QModelIndex::View) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::Role_Display) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRows(int row, const QModelIndex &parent = QModelIndex(), int count = 1);
    bool removeRows(int row, const QModelIndex &parent = QModelIndex(), int count = 1);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

protected:
    void append(QTreeViewItem *item);
    void emitRowsInserted(QTreeViewItem *item);

private:
    int c;
    QList<QTreeViewItem*> tree;
    mutable QTreeViewItem topHeader;
};

/*
  \class QTreeModel qtreeview.h

  \brief The QTreeModel class manages the items stored in a tree view.

  \ingroup model-view


*/

/*!
  \internal

  Constructs a tree model with a \a parent object and the given
  number of \a columns.
*/

QTreeModel::QTreeModel(int columns, QObject *parent)
    : QAbstractItemModel(parent), c(0)
{
    setColumnCount(columns);
}

/*!
  \internal

  Destroys this tree model.
*/

QTreeModel::~QTreeModel()
{
    for (int i = 0; i < tree.count(); ++i)
        delete tree.at(i);
}

/*!
  \internal

  Sets the number of \a columns in the tree model.
*/

void QTreeModel::setColumnCount(int columns)
{
    if (c == columns)
        return;
    int _c = c;
    c = columns;
    if (c < _c)
        emit columnsRemoved(QModelIndex(), qMax(_c - 1, 0), qMax(c - 1, 0));
    topHeader.setColumnCount(c);
    for (int i = _c; i < c; ++i)
        topHeader.setText(i, QString::number(i));
    if (c > _c)
        emit columnsInserted(QModelIndex(), qMax(_c - 1, 0), qMax(c - 1, 0));
}

/*!
  \internal

  Returns the number of columns in the tree model.*/

int QTreeModel::columnCount() const
{
    return c;
}

/*!
  \internal

  Sets the column text for the \a column to the given \a text.*/

void QTreeModel::setColumnText(int column, const QString &text)
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QAbstractItemModel::Role_Display, text);
}

/*!
  \internal

  Sets the icon set for the \a column to the icon set specified by
  \a iconSet.*/

void QTreeModel::setColumnIconSet(int column, const QIconSet &iconSet)
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QAbstractItemModel::Role_Decoration, iconSet);
}

/*!
  \internal

  Returns the text for the given \a column in the tree model.*/

QString QTreeModel::columnText(int column) const
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    return data(index, QAbstractItemModel::Role_Display).toString();
}

/*!
  \internal

  Returns the icon set for the given \a column.*/

QIconSet QTreeModel::columnIconSet(int column) const
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    return data(index, QAbstractItemModel::Role_Decoration).toIconSet();
}

/*!
  \internal

  Returns the tree view item corresponding to the \a index given.

  \sa QModelIndex*/

QTreeViewItem *QTreeModel::item(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    if (index.type() != QModelIndex::View)
        return &topHeader;
    return static_cast<QTreeViewItem *>(index.data());
}

/*!
  \internal

  Returns the model index that refers to the tree view \a item.*/

QModelIndex QTreeModel::index(QTreeViewItem *item) const
{
    if (!item)
        return QModelIndex();
    const QTreeViewItem *par = item->parent();
    int row = par ? par->children.indexOf(item) : tree.indexOf(item);
    return createIndex(row, 0, item);
}

/*!
  \internal

  Returns the model index with the given \a row, \a column, \a type,
  and \a parent.*/

QModelIndex QTreeModel::index(int row, int column, const QModelIndex &parent,
                              QModelIndex::Type type) const
{
    int r = tree.count();
    if (row < 0 || row >= r || column < 0 || column >= c)
        return QModelIndex();
    if (!parent.isValid()) {// toplevel
        QTreeViewItem *itm = const_cast<QTreeModel*>(this)->tree.at(row);
        if (itm)
            return createIndex(row, column, itm, type);
        return QModelIndex();
    }
    QTreeViewItem *parentItem = item(parent);
    if (parentItem && row < parentItem->childCount()) {
        QTreeViewItem *itm = static_cast<QTreeViewItem *>(parentItem->child(row));
        if (itm)
            return createIndex(row, column, itm, type);
        return QModelIndex();
    }
    return QModelIndex();
}

/*!
  \internal

  Returns the parent model index of the index given as the \a child.*/

QModelIndex QTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
    const QTreeViewItem *itm = reinterpret_cast<const QTreeViewItem *>(child.data());
    if (!itm)
        return QModelIndex();
    QTreeViewItem *parent = const_cast<QTreeViewItem *>(itm->parent()); // FIXME
    return index(parent);
}

/*!
  \internal

  Returns the number of rows in the \a parent model index.*/

int QTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        QTreeViewItem *parentItem = item(parent);
        if (parentItem)
            return parentItem->childCount();
    }
    return tree.count();
}

/*!
  \internal

  Returns the number of columns in the item referred to by the given
  \a index.*/

int QTreeModel::columnCount(const QModelIndex &) const
{
    return c;
}

/*!
  \internal

  Returns the data corresponding to the given model \a index and
  \a role.*/

QVariant QTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    QTreeViewItem *itm = item(index);
    if (itm)
        return itm->data(index.column(), role);
    return QVariant();
}

/*!
  \internal

  Sets the data for the item specified by the \a index and \a role
  to that referred to by the \a value.

  Returns true if successful; otherwise returns false.*/

bool QTreeModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid())
        return false;
    QTreeViewItem *itm = item(index);
    if (itm) {
        itm->setData(index.column(), role, value);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

/*!
  \internal

  Inserts a tree view item into the \a parent item at the given
  \a row. Returns true if successful; otherwise returns false.

  If no valid parent is given, the item will be inserted into this
  tree model at the row given.*/

bool QTreeModel::insertRows(int row, const QModelIndex &parent, int)
{
    if (parent.isValid()) {
        QTreeViewItem *p =  item(parent);
        if (p) {
            p->children.insert(row, new QTreeViewItem(p));
            return true;
        }
        return false;
    }
    tree.insert(row, new QTreeViewItem());
    return true;
}

/*!
  \internal

  Removes the given \a row from the \a parent item, and returns true
  if successful; otherwise false is returned. */

bool QTreeModel::removeRows(int row, const QModelIndex &parent, int)
{
    if (parent.isValid()) {
        QTreeViewItem *p = item(parent);
        if (p) {
            p->children.removeAt(row);
            return true;
        }
        return false;
    }
    tree.removeAt(row);
    return true;
}

/*!
  \internal

  Returns true if the item at the \a index given is selectable;
  otherwise returns false.
*/

bool QTreeModel::isSelectable(const QModelIndex &) const
{
    return true;
}

/*!
  \internal

  Returns true if the item at the \a index given is editable;
  otherwise returns false.
*/

bool QTreeModel::isEditable(const QModelIndex &) const
{
    return true;
}

/*!
  \internal

  Appends the tree view \a item to the tree model.*/

void QTreeModel::append(QTreeViewItem *item)
{
    int r = tree.count();
    tree.push_back(item);
    emit rowsInserted(QModelIndex(), r, r);
}

/*!
\internal

Emits the rowsInserted() signal for the rows containing the given \a item.

\sa rowsInserted()*/

void QTreeModel::emitRowsInserted(QTreeViewItem *item)
{
    QModelIndex idx = index(item);
    QModelIndex parentIndex = parent(idx);
    emit rowsInserted(parentIndex, idx.row(), idx.row());
}

// QTreeViewItem

/*!
  \class QTreeViewItem qtreeview.h

  \brief The QTreeViewItem class provides an item for use with the
  predefined QTreeView class.

  \ingroup model-view

  The QTreeViewItem class provides a familiar interface for items displayed
  in a QTreeView widget.

  \sa QTreeView QTreeModel
*/

/*!
  Constructs a tree view item. The item will need to be inserted
  into a tree view.

  \sa QTreeModel::append() QTreeView::append()*/

QTreeViewItem::QTreeViewItem()
    : par(0), view(0), c(0), edit(true), select(true)
{
}

/*!
  \fn QTreeViewItem::QTreeViewItem(QTreeView *view)

  Constructs a tree view item and inserts it into the tree \a view.
*/

QTreeViewItem::QTreeViewItem(QTreeView *v)
    : par(0), view(v), c(0), edit(true), select(true)
{
    if (view)
        view->append(this);
}

/*!
  Constructs a tree view item with a \a parent tree view item.
*/

QTreeViewItem::QTreeViewItem(QTreeViewItem *parent)
    : par(parent), view(parent->view), c(0), edit(true), select(true)
{
    if (parent)
        parent->children.push_back(this);
    QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
    model->emitRowsInserted(this);
}

/*!
  Destroys this tree view item.
*/

QTreeViewItem::~QTreeViewItem()
{
    for (int i = 0; i < children.count(); ++i)
        delete children.at(i);
}

/*!
  Sets the number of \a columns in the tree view item.
*/

void QTreeViewItem::setColumnCount(int columns)
{
    c = columns;
    values.resize(columns);
}

/*!
  Returns the data stored in the \a column with the given \a role.

  \sa QAbstractItemModel::Role*/

QVariant QTreeViewItem::data(int column, int role) const
{
    if (column < 0 || column >= c)
        return QVariant();
    const QVector<Data> column_values = values.at(column);
    role = (role == QAbstractItemModel::Role_Edit ? QAbstractItemModel::Role_Display : role);
    for (int i = 0; i < column_values.count(); ++i) {
        if (column_values.at(i).role == role)
            return column_values.at(i).value;
    }
    return QVariant();
}

/*!
  Sets the data for the item specified by the \a column and \a role to
  that referred to by the \a value.

  Returns true if successful; otherwise returns false.*/

void QTreeViewItem::setData(int column, int role, const QVariant &value)
{
    if (column >= c)
        setColumnCount(column + 1);
    QVector<Data> column_values = values.at(column);
    role = (role == QAbstractItemModel::Role_Edit ? QAbstractItemModel::Role_Display : role);
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
    Q_DECLARE_PUBLIC(QTreeView)
public:
    QTreeViewPrivate() : QGenericTreeViewPrivate() {}
    inline QTreeModel *model() const { return ::qt_cast<QTreeModel*>(q_func()->model()); }
};

#define d d_func()
#define q q_func()

/*!
  \class QTreeView qtreeview.h

  \brief The QTreeView class provides a tree view that uses a predefined
  tree model.

  \ingroup model-view

  The QTreeView class is a convenience class that replaces the \c QListView
  class. It provides a list view widget that takes advantage of Qt's
  model-view architecture.

  This class uses a default model to organize the data represented in the
  tree view, but also uses the QTreeViewItem class to provide a familiar
  interface for simple list structures.

  \omit
  In its simplest form, a tree view can be constructed and populated in
  the familiar way:

  \code
    QTreeView *view = new QTreeView(parent);

  \endcode
  \endomit

  \sa \link model-view-programming.html Model/View Programming\endlink
      QTreeModel QTreeViewItem
*/

/*!
  Constructs a tree view with the given \a parent widget, using the default
  model.*/

QTreeView::QTreeView(QWidget *parent)
    : QGenericTreeView(*new QGenericTreeViewPrivate(), parent)
{
    setModel(new QTreeModel(0, this));
}

/*!
  Sets the number of \a columns in the tree view.*/

void QTreeView::setColumnCount(int columns)
{
    d->model()->setColumnCount(columns);
}

/*!
  Returns the number of columns in the tree view.*/

int QTreeView::columnCount() const
{
    return model()->columnCount();
}

/*!
  Sets the text for the \a column to the \a text given.*/

void QTreeView::setColumnText(int column, const QString &text)
{
    d->model()->setColumnText(column, text);
}

/*!
  Sets the icon set for the \a column to that specified by \a iconSet.*/

void QTreeView::setColumnIconSet(int column, const QIconSet &iconSet)
{
    d->model()->setColumnIconSet(column, iconSet);
}

/*!
  Returns the text for the given \a column in the tree view.*/

QString QTreeView::columnText(int column) const
{
    return d->model()->columnText(column);
}

/*!
  Returns the icon set for the given \a column in the tree view.*/

QIconSet QTreeView::columnIconSet(int column) const
{
    return d->model()->columnIconSet(column);
}

/*!
  Appends a tree view \a item to the tree view.*/

void QTreeView::append(QTreeViewItem *item)
{
    d->model()->append(item);
}
