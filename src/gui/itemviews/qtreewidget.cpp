/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtreewidget.h"
#include <qapplication.h>
#include <qheaderview.h>
#include <qpainter.h>
#include <qitemdelegate.h>
#include <private/qtreeview_p.h>

class QTreeItemDelegate : public QItemDelegate
{
public:
    QTreeItemDelegate(QObject *parent) : QItemDelegate(parent) {}
    ~QTreeItemDelegate() {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QAbstractItemModel *model, const QModelIndex &index) const;
    QSize sizeHint(const QFontMetrics &fontMetrics, const QStyleOptionViewItem &option,
                   const QAbstractItemModel *model, const QModelIndex &index) const;
};


void QTreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    // enabled
    if (model->flags(index) & QAbstractItemModel::ItemIsEnabled == 0)
        opt.state &= ~QStyle::Style_Enabled;
    // set font
    QVariant value = model->data(index, QAbstractItemModel::FontRole);
    if (value.isValid())
        opt.font = value.toFont();
    // set text color
    value = model->data(index, QAbstractItemModel::TextColorRole);
    if (value.isValid() && value.toColor().isValid())
        opt.palette.setColor(QPalette::Text, value.toColor());
    // draw the background color
    value = model->data(index, QAbstractItemModel::BackgroundColorRole);
    if (value.isValid() && value.toColor().isValid())
        painter->fillRect(option.rect, value.toColor());
    // draw the item
    QItemDelegate::paint(painter, opt, model, index);
}

QSize QTreeItemDelegate::sizeHint(const QFontMetrics &/*fontMetrics*/,
                                  const QStyleOptionViewItem &option,
                                  const QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    QVariant value = model->data(index, QAbstractItemModel::FontRole);
    QFont fnt = value.isValid() ? value.toFont() : option.font;
    return QItemDelegate::sizeHint(QFontMetrics(fnt), option, model, index);
}

class QTreeModel : public QAbstractItemModel
{
    friend class QTreeWidget;
    friend class QTreeWidgetItem;

public:
    QTreeModel(int columns = 0, QObject *parent = 0);
    ~QTreeModel();

    void setColumnCount(int columns);
    
    QTreeWidgetItem *item(const QModelIndex &index) const;

    QModelIndex index(QTreeWidgetItem *item) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex::Null) const;
    int columnCount(const QModelIndex &parent = QModelIndex::Null) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

    bool isSortable() const;
    void sort(int column, const QModelIndex &parent, Qt::SortOrder order);

protected:
    void append(QTreeWidgetItem *item);
    void remove(QTreeWidgetItem *item);
    void emitRowsInserted(QTreeWidgetItem *item);

private:
    QList<QTreeWidgetItem*> tree;
    QTreeWidgetItem *header;
    Qt::SortOrder sorting;
};

/*
  \class QTreeModel qtreewidget.h
 The QTreeModel class manages the items stored in a tree view.

  \ingroup model-view
    \mainclass
*/

/*!
  \internal

  Constructs a tree model with a \a parent object and the given
  number of \a columns.
*/

QTreeModel::QTreeModel(int columns, QObject *parent)
    : QAbstractItemModel(parent), header(new QTreeWidgetItem()),
      sorting(Qt::AscendingOrder)
{
    setColumnCount(columns);
}

/*!
  \internal

  Destroys this tree model.
*/

QTreeModel::~QTreeModel()
{
    for (int i = 0; i < tree.count(); ++i) {
        tree.at(i)->par = 0;
        tree.at(i)->view = 0;
        delete tree.at(i);
    }
    delete header;
}

/*!
  \internal

  Sets the number of \a columns in the tree model.
*/

void QTreeModel::setColumnCount(int columns)
{
    int c = header->columnCount();
    if (c == columns)
        return;
    int _c = c;
    c = columns;
    if (c < _c)
        emit columnsRemoved(QModelIndex::Null, qMax(_c - 1, 0), qMax(c - 1, 0));
//    header->setColumnCount(c);
    header->values.resize(c);
    for (int i = _c; i < c; ++i)
        header->setText(i, QString::number(i)); // FIXME: shoulnd't save anything
    if (c > _c)
        emit columnsInserted(QModelIndex::Null, qMax(_c - 1, 0), qMax(c - 1, 0));
}

/*!
  \internal

  Returns the tree view item corresponding to the \a index given.

  \sa QModelIndex
*/

QTreeWidgetItem *QTreeModel::item(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    return static_cast<QTreeWidgetItem*>(index.data());
}

/*!
  \internal

  Returns the model index that refers to the tree view \a item.
*/

QModelIndex QTreeModel::index(QTreeWidgetItem *item) const
{
    if (!item)
        return QModelIndex::Null;
    const QTreeWidgetItem *par = item->parent();
    int row = par ? par->children.indexOf(item) : tree.indexOf(item);
    return createIndex(row, 0, item);
}

/*!
  \internal

  Returns the model index with the given \a row, \a column, \a type,
  and \a parent.
*/

QModelIndex QTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    int r = tree.count();
    int c = header->columnCount();
    if (row < 0 || row >= r || column < 0 || column >= c)
        return QModelIndex::Null;
    if (!parent.isValid()) {// toplevel
        QTreeWidgetItem *itm = tree.at(row);
        if (itm)
            return createIndex(row, column, itm);
        return QModelIndex::Null;
    }
    QTreeWidgetItem *parentItem = item(parent);
    if (parentItem && row < parentItem->childCount()) {
        QTreeWidgetItem *itm = static_cast<QTreeWidgetItem*>(parentItem->child(row));
        if (itm)
            return createIndex(row, column, itm);
        return QModelIndex::Null;
    }
    return QModelIndex::Null;
}

/*!
  \internal

  Returns the parent model index of the index given as the \a child.
*/

QModelIndex QTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex::Null;
    QTreeWidgetItem *itm = reinterpret_cast<QTreeWidgetItem *>(child.data());
    if (!itm)
        return QModelIndex::Null;
    QTreeWidgetItem *parent = itm->parent();
    return index(parent);
}

/*!
  \internal

  Returns the number of rows in the \a parent model index.
*/

int QTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        QTreeWidgetItem *parentItem = item(parent);
        if (parentItem)
            return parentItem->childCount();
    }
    return tree.count();
}

/*!
  \internal

  Returns the number of columns in the item referred to by the given
  \a index.
*/

int QTreeModel::columnCount(const QModelIndex &) const
{
    return header->columnCount();
}

/*!
  \internal

  Returns the data corresponding to the given model \a index and
  \a role.
*/

QVariant QTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    QTreeWidgetItem *itm = item(index);
    if (itm)
        return itm->data(index.column(), role);
    return QVariant();
}

/*!
  \internal

  Sets the data for the item specified by the \a index and \a role
  to that referred to by the \a value.

  Returns true if successful; otherwise returns false.
*/

bool QTreeModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid())
        return false;
    QTreeWidgetItem *itm = item(index);
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
  tree model at the row given.
*/

bool QTreeModel::insertRows(int row, const QModelIndex &parent, int)
{
    if (parent.isValid()) {
        QTreeWidgetItem *p =  item(parent);
        if (p) {
            p->children.insert(row, new QTreeWidgetItem(p));
            return true;
        }
        return false;
    }
    tree.insert(row, new QTreeWidgetItem());
    return true;
}

/*!
  \internal

  Removes the given \a row from the \a parent item, and returns true
  if successful; otherwise false is returned.
*/

bool QTreeModel::removeRows(int row, const QModelIndex &parent, int count)
{
    // FIXME: !!!!!!!
    if (parent.isValid()) {
        QTreeWidgetItem *p = item(parent);
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

*/

QAbstractItemModel::ItemFlags QTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    QTreeWidgetItem *itm = item(index);
    if (itm)
        return itm->flags();
    return 0;
}

/*!

*/

bool QTreeModel::isSortable() const
{
    return true;
}

/*!

*/

void QTreeModel::sort(int column, const QModelIndex &parent, Qt::SortOrder order)
{
    if (column == -1)
        return;

    int count = 0;
    QList<QTreeWidgetItem*>::iterator begin;
    QList<QTreeWidgetItem*>::iterator end;
    QTreeWidgetItem *par = static_cast<QTreeWidgetItem*>(parent.data());

    if (!par) {
        count = tree.count();
        begin = tree.begin();
        end = tree.end();
    } else {
        count = par->children.count();
        begin = par->children.begin();
        end = par->children.end();
    }

    if (order == Qt::AscendingOrder)
        qHeapSort(begin, end);
    else
        qHeapSort(end, begin);

    emit dataChanged(index(0, 0, parent), index(count - 1, columnCount() - 1, parent));
}

/*!
  \internal

  Appends the tree view \a item to the tree model.*/

void QTreeModel::append(QTreeWidgetItem *item)
{
    int r = tree.count();
    tree.push_back(item);
    emit rowsInserted(QModelIndex::Null, r, r);
}

/*!
  \internal

  Remove the treeview \a item to the tree model.
*/

void QTreeModel::remove(QTreeWidgetItem *item)
{
    int r = tree.indexOf(item);
    if (r != -1) {
        tree.removeAt(r);
        emit rowsRemoved(QModelIndex::Null, r, r);
    }
}

/*!
\internal

Emits the rowsInserted() signal for the rows containing the given \a item.

\sa rowsInserted()*/

void QTreeModel::emitRowsInserted(QTreeWidgetItem *item)
{
    QModelIndex idx = index(item);
    QModelIndex parentIndex = parent(idx);
    emit rowsInserted(parentIndex, idx.row(), idx.row());
}

/*!
  \class QTreeWidgetItem qtreewidget.h

  \brief The QTreeWidgetItem class provides a tree view item for use with the
  QTreeWidget convenience class.

  \ingroup model-view

  The QTreeWidgetItem class is a convenience class that replaces the
  \c QListViewItem class in Qt 3. It provides a list view item for use with
  the QTreeWidget class.

*/

/*!
  Constructs a tree widget item. The item must be inserted
  into a tree view.

  \sa QTreeModel::append() QTreeWidget::append()
*/

QTreeWidgetItem::QTreeWidgetItem()
    : view(0), par(0)
{
}

/*!
    \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view)

    Constructs a tree widget item and inserts it into the given tree
    \a view.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view)
    : view(view), par(0)
{
    if (view)
        view->appendItem(this);
}

/*!
    Constructs a tree widget item with the given \a parent.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent)
    : view(parent->view), par(parent),
      itemFlags(QAbstractItemModel::ItemIsEditable
                |QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsCheckable
                |QAbstractItemModel::ItemIsEnabled)
{
    if (parent)
        parent->children.push_back(this);
    QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
    model->emitRowsInserted(this);
}

/*!
  Destroys this tree widget item.
*/

QTreeWidgetItem::~QTreeWidgetItem()
{
    for (int i = 0; i < children.count(); ++i) {
        children.at(i)->par = 0;
        children.at(i)->view = 0;
        delete children.at(i);
    }

    if (par)
        par->children.remove(this);
    else if (view)
        view->removeItem(this);
}

/*!
  ###
*/

bool QTreeWidgetItem::operator<(const QTreeWidgetItem &other) const
{
    int column = view->header()->sortIndicatorSection();
    return text(column) < other.text(column);
}

/*!
    Sets the value for the item's \a column and \a role to the given
    \a value.

    \sa store()
*/
void QTreeWidgetItem::setData(int column, int role, const QVariant &value)
{
    if (column >= values.count())
        values.resize(column + 1);
    QVector<Data> column_values = values.at(column);
    for (int i = 0; i < column_values.count(); ++i) {
        if (column_values.at(i).role == role) {
            values[column][i].value = value;
            return;
        }
    }
    values[column].append(Data(role, value));
}

/*!
    Returns the value for the item's \a column and \a role.

    \sa store()
*/
QVariant QTreeWidgetItem::data(int column, int role) const
{
    const QVector<Data> column_values = values.at(column);
    for (int i = 0; i < column_values.count(); ++i)
        if (column_values.at(i).role == role)
            return column_values.at(i).value;
    return QVariant();
}

class QTreeWidgetPrivate : public QTreeViewPrivate
{
    Q_DECLARE_PUBLIC(QTreeWidget)
public:
    QTreeWidgetPrivate() : QTreeViewPrivate() {}
    inline QTreeModel *model() const { return ::qt_cast<QTreeModel*>(q_func()->model()); }
};

#define d d_func()
#define q q_func()

/*!
  \class QTreeWidget qtreewidget.h

  \brief The QTreeWidget class provides a tree view that uses a predefined
  tree model.

  \ingroup model-view

  The QTreeWidget class is a convenience class that provides a standard
  tree view based on Qt's Model/View architecture, but which presents the
  classic item-based interface used by the \c QListView class in Qt 3.
  This class uses a default model to hold items, each of which is an instance
  of the QTreeWidgetItem class.

  Developers who do not need the flexibility of the Model/View framework
  can use this class to create simple hierarchical lists very easily. A more
  flexible approach involves combining a QTreeView with a standard item model.
  This allows the storage of data to be separated from its representation.

  \omit
  In its simplest form, a tree widget can be constructed and populated in
  the familiar way:

  \code
    QTreeWidget *view = new QTreeWidget(parent);

  \endcode
  \endomit

  \sa \link model-view-programming.html Model/View Programming\endlink QTreeModel QTreeWidgetItem
*/

/*!
  Constructs a tree view with the given \a parent widget, using the default
  model
*/

QTreeWidget::QTreeWidget(QWidget *parent)
    : QTreeView(*new QTreeViewPrivate(), parent)
{
    setModel(new QTreeModel(0, this));
    setItemDelegate(new QTreeItemDelegate(this));
    header()->setItemDelegate(new QTreeItemDelegate(header()));
}

/*!
  Retuns the number of header columns in the view.
*/

int QTreeWidget::columnCount() const
{
    return d->model()->columnCount();
}

/*!
  Sets the number of header \a columns in the tree view.
*/

void QTreeWidget::setColumnCount(int columns)
{
    d->model()->setColumnCount(columns);
}

QTreeWidgetItem *QTreeWidget::headerItem()
{
    return d->model()->header;
}

void QTreeWidget::setHeaderItem(QTreeWidgetItem *item)
{
    delete d->model()->header;
    d->model()->header = item;
}

/*!
  Appends a tree view \a item to the tree view.
*/

void QTreeWidget::appendItem(QTreeWidgetItem *item)
{
    d->model()->append(item);
}

/*!
  Removes a tree view \a item from tree view.
*/

void QTreeWidget::removeItem(QTreeWidgetItem *item)
{
    d->model()->remove(item);
}

/*!
  Returns true if the \a item is selected, otherwise returns false.
*/

bool QTreeWidget::isSelected(QTreeWidgetItem *item) const
{
    QModelIndex index = d->model()->index(item);
    return selectionModel()->isSelected(index);
}

void QTreeWidget::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
}
