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

// workaround for VC++ 6.0 linker bug (?)
typedef bool(*LessThan)(const QTreeWidgetItem *left, const QTreeWidgetItem *right);

class QTreeModel : public QAbstractItemModel
{
    friend class QTreeWidget;
    friend class QTreeWidgetItem;

public:
    QTreeModel(int columns = 0, QObject *parent = 0);
    ~QTreeModel();

    void clear();
    void setColumnCount(int columns);

    QTreeWidgetItem *item(const QModelIndex &index) const;

    QModelIndex index(QTreeWidgetItem *item, int column = 0) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex::Null) const;
    int columnCount(const QModelIndex &parent = QModelIndex::Null) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, int role, const QVariant &value);

    bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

    bool isSortable() const;
    void sort(int column, const QModelIndex &parent, Qt::SortOrder order);
    void sortAll(int column, Qt::SortOrder);

    static bool lessThan(const QTreeWidgetItem *left, const QTreeWidgetItem *right);
    static bool greaterThan(const QTreeWidgetItem *left, const QTreeWidgetItem *right);

    void emitDataChanged(QTreeWidgetItem *item);

protected:
    void append(QTreeWidgetItem *item);
    void remove(QTreeWidgetItem *item);
    void emitRowsInserted(QTreeWidgetItem *item);
    void emitRowsRemoved(QTreeWidgetItem *item);

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
    clear();
}

/*!
  \internal

  Removes all items in the model.
*/

void QTreeModel::clear()
{
    for (int i = 0; i < tree.count(); ++i) {
        QTreeWidgetItem *item = tree.at(i);
        item->par = 0;
        item->view = 0;
        item->model = 0;
        delete item;
    }
    tree.clear();
    emit reset();
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
    header->values.resize(c);
    for (int i = _c; i < c; ++i)
        header->setText(i, QString::number(i)); // FIXME: shouldn't save anything
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

  Returns the model index that refers to the
  tree view \a item and \a column.
*/

QModelIndex QTreeModel::index(QTreeWidgetItem *item, int column) const
{
    if (!item)
        return QModelIndex::Null;
    const QTreeWidgetItem *par = item->parent();
    int row = par ? par->children.indexOf(item) : tree.indexOf(item);
    return createIndex(row, column, item);
}

/*!
  \internal
  \reimp

  Returns the model index with the given \a row,
  \a column and \a parent.
*/

QModelIndex QTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    int c = header->columnCount();

    if (row < 0 || column < 0 || column >= c)
        return QModelIndex::Null;

    // toplevel items
    if (!parent.isValid()) {
        if (row >= tree.count())
            return QModelIndex::Null;
        QTreeWidgetItem *itm = tree.at(row);
        if (itm)
            return createIndex(row, column, itm);
        return QModelIndex::Null;
    }

    // children
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
  \reimp

  Returns the parent model index of the index given as
  the \a child.
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
  \reimp

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
  \reimp

  Returns the number of columns in the item referred to by
  the given \a index.
*/

int QTreeModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return header->columnCount();
}

/*!
  \internal
  \reimp

  Returns the data corresponding to the given model \a index
  and \a role.
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
  \reimp

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
  \reimp

  Returns the header data corresponding to the given header \a section,
  \a orientation and data \a role.
*/

QVariant QTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && header)
        return header->data(section, role);
    return QVariant();
}

/*!
  \internal
  \reimp

  Sets the header data for the item specified by the header \a section,
  \a orientation and data \a role to the given \a value.

  Returns true if successful; otherwise returns false.
*/

bool QTreeModel::setHeaderData(int section, Qt::Orientation orientation,
                               int role, const QVariant &value)
{
    if (orientation == Qt::Horizontal && header) {
        header->setData(section, role, value);
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

/*!
  \internal

  Inserts a tree view item into the \a parent item at the given
  \a row. Returns true if successful; otherwise returns false.

  An empty item will be created by this function.
*/

bool QTreeModel::insertRows(int row, const QModelIndex &parent, int count)
{
    Q_UNUSED(count);

    if (parent.isValid()) {
        QTreeWidgetItem *p =  item(parent);
        if (p) {
            p->children.insert(row, new QTreeWidgetItem(p));
            emit rowsInserted(parent, row, row);
            return true;
        }
        return false;
    }
    tree.insert(row, new QTreeWidgetItem());
    emit rowsInserted(parent, row, row);
    return true;
}

/*!
  \internal

  Removes the given \a row from the \a parent item, and returns true
  if successful; otherwise false is returned.
*/

bool QTreeModel::removeRows(int row, const QModelIndex &parent, int count)
{
    Q_UNUSED(count);

    if (parent.isValid()) {
        QTreeWidgetItem *p = item(parent);
        if (p) {
            emit rowsRemoved(parent, row, row);
            p->children.removeAt(row);
            return true;
        }
        return false;
    }
    emit rowsRemoved(parent, row, row);
    tree.removeAt(row);
    return true;
}

/*!
  \internal
  \reimp

  Returns the flags for the item refered to the given \a index.
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
  \internal
  \reimp

  Returns if the model is sortable, otherwise returns false.
*/

bool QTreeModel::isSortable() const
{
    return true;
}

/*!
  \internal
  \reimp

  Sorts one level of the tree with the given \a parent
  in the given \a order by the values in the given \a column.
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

    LessThan compare = order == Qt::AscendingOrder ? &lessThan : &greaterThan;
    qHeapSort(begin, end, compare);

    emit reset(); // items with their subtrees may have been moved
}

/*!
  \internal

  Sorts the entire tree in the model in the given \a order,
  by the values in the given \a column.
*/

void QTreeModel::sortAll(int column, Qt::SortOrder order)
{
    // sort top level
    LessThan compare = order == Qt::AscendingOrder ? &lessThan : &greaterThan;
    qHeapSort(tree.begin(), tree.end(), compare);

    // sort the children
    QList<QTreeWidgetItem*>::iterator it = tree.begin();
    for (; it != tree.end(); ++it)
        (*it)->sortChildren(column, order, true);

    // everything has changed
    emit reset();
}

/*!
  \internal

  Returns true if the value of the \a left item is
  less than the value of the \a right item.

  Used by the sorting functions.
*/

bool QTreeModel::lessThan(const QTreeWidgetItem *left, const QTreeWidgetItem *right)
{
    return *left < *right;
}

/*!
  \internal

  Returns true if the value of the \a left item is
  greater than the value of the \a right item.

  Used by the sorting functions.
*/

bool QTreeModel::greaterThan(const QTreeWidgetItem *left, const QTreeWidgetItem *right)
{
    return !(*left < *right);
}

/*!
  \internal

  Emits the dataChanged() signal for the given \a item.
*/

void QTreeModel::emitDataChanged(QTreeWidgetItem *item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}

/*!
  \internal

  Appends the tree view \a item to the tree model and a toplevel item.
*/

void QTreeModel::append(QTreeWidgetItem *item)
{
    int r = tree.count();
    tree.push_back(item);
    emit rowsInserted(QModelIndex::Null, r, r);
}

/*!
  \internal

  Remove the treeview toplevel \a item from the tree model.
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

  Emits the rowsInserted() signal for the row containing the given \a item.

  \sa emitRowsRemoved()
*/

void QTreeModel::emitRowsInserted(QTreeWidgetItem *item)
{
    QModelIndex idx = index(item);
    QModelIndex parentIndex = parent(idx);
    emit rowsInserted(parentIndex, idx.row(), idx.row());
}

/*!
  \internal

  Emits the rowsRemoved() signal for the rows containing the given \a item.

  \sa emitRowsInserted()
*/

void QTreeModel::emitRowsRemoved(QTreeWidgetItem *item)
{
    QModelIndex idx = index(item);
    QModelIndex parentIndex = parent(idx);
    emit rowsRemoved(parentIndex, idx.row(), idx.row());
}

/*!
  \class QTreeWidgetItem qtreewidget.h

  \brief The QTreeWidgetItem class provides an item for use with the
  QTreeWidget convenience class.

  \ingroup model-view

  A tree widget item is used to display a row of information in a tree widget.
  Rows usually contain several columns of data, each of which can contain
  a text label and an icon.

  The QTreeWidgetItem class is a convenience class that replaces the
  \c QListViewItem class in Qt 3. It provides an item for use with
  the QTreeWidget class.

  Items are usually constructed with a parent that is either a QTreeWidget
  (for top-level items) or a QTreeWidgetItem (for items on lower levels of
  the tree).

  Each column in an item can have its own background color which is set with
  the setBackgroundColor() function. The current background color can be
  found with backgroundColor().
  The text label for each column can be rendered with its own font and text
  color. These are specified with the setFont() and setTextColor() functions,
  and read with font() and textColor().

*/

/*!
    \fn QAbstractItemModel::ItemFlags QTreeWidgetItem::flags() const
*/

/*!
    \fn void QTreeWidgetItem::setFlags(QAbstractItemModel::ItemFlags flags)

    Sets the \a flags for this item. These determine whether the item can be
    selected or modified.

    \sa flags()
*/

/*!
    \fn QString QTreeWidgetItem::text(int column) const

    Returns the text in the specified \a column.

    \sa setText()
*/

/*!
    \fn void QTreeWidgetItem::setText(int column, const QString &text)

    Sets the \a text to be displayed in the given \a column.

    \sa text() setFont() setTextColor()
*/

/*!
    \fn QIcon QTreeWidgetItem::icon(int column) const

    Returns the icon that is displayed in the specified \a column.

    \sa setIcon()
*/

/*!
    \fn void QTreeWidgetItem::setIcon(int column, const QIcon &icon)

    Sets the \a icon to be displayed in the given \a column.

    \sa icon() setText()
*/

/*!
    \fn QString QTreeWidgetItem::statusTip(int column) const

    Returns the status tip for the contents of the given \a column.

    \sa setStatusTip()
*/

/*!
    \fn void QTreeWidgetItem::setStatusTip(int column, const QString &statusTip)

    Sets the \a statusTip for the given \a column.

    \sa statusTip() setToolTip() setWhatsThis()
*/

/*!
    \fn QString QTreeWidgetItem::toolTip(int column) const

    Returns the tool tip for the given \a column.

    \sa setToolTip()
*/

/*!
    \fn void QTreeWidgetItem::setToolTip(int column, const QString &toolTip)

    Sets the \a toolTip for the given \a column.

    \sa toolTip() setStatusTip() setWhatsThis()
*/

/*!
    \fn QString QTreeWidgetItem::whatsThis(int column) const

    Returns the "What's This?" help for the contents of the given \a column.

    \sa setWhatsThis()
*/

/*!
    \fn void QTreeWidgetItem::setWhatsThis(int column, const QString &whatsThis)

    Sets the \a whatsThis help for the given \a column.

    \sa whatsThis() setStatusTip() setToolTip()
*/

/*!
    \fn QFont QTreeWidgetItem::font(int column) const

    Returns the font used to render the text in the specified \a column.

    \sa setFont()
*/

/*!
    \fn void QTreeWidgetItem::setFont(int column, const QFont &font)

    Sets the \a font used to display the text in the given \a column.

    \sa font() setText() setTextColor()
*/

/*!
    \fn QColor QTreeWidgetItem::backgroundColor(int column) const

    Returns the color used to render the background of the specified \a column.

    \sa textColor() setBackgroundColor()
*/

/*!
    \fn void QTreeWidgetItem::setBackgroundColor(int column, const QColor &color)

    Sets the background color of the label in the given \a column to the
    specified \a color.

    \sa backgroundColor() setTextColor()
*/

/*!
    \fn QColor QTreeWidgetItem::textColor(int column) const

    Returns the color used to render the text in the specified \a column.

    \sa backgroundColor() setTextColor()
*/

/*!
    \fn void QTreeWidgetItem::setTextColor(int column, const QColor &color)

    Sets the \a color used to display the text in the given \a column.

    \sa textColor() setFont() setText()
*/

/*!
    \fn int QTreeWidgetItem::checkedState(int column) const
*/

/*!
    \fn void QTreeWidgetItem::setCheckedState(int column, bool state)

    If \a state is true, the label in the given \a column is checked; otherwise
    it is unchecked. The item must be checkable for this function to have an
    effect.

    \sa checkedState() QAbstractItemModel::ItemFlag
*/

/*!
    \fn QTreeWidgetItem *QTreeWidgetItem::parent() const

    Returns this item's parent.

    \sa child()
*/

/*!
    \fn QTreeWidgetItem *QTreeWidgetItem::child(int index) const

    Returns the item at the given \a index in the list of this item's children.

    \sa parent()
*/

/*!
    \fn int QTreeWidgetItem::childCount() const
*/

/*!
    \fn int QTreeWidgetItem::columnCount() const
*/


/*!
  \internal
  Constructs a tree widget item. The item must be inserted into a tree view.

  \sa QTreeModel::append() QTreeWidget::append()
*/

QTreeWidgetItem::QTreeWidgetItem()
    : view(0), model(0), par(0),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled)
{
}

/*!
    \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view)

    Constructs a tree widget item and inserts it into the given tree
    \a view.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view)
    : view(view), model(0), par(0),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled)
{
    if (view) {
        model = ::qt_cast<QTreeModel*>(view->model());
        if (model) {
            model->tree.append(this);
            model->emitRowsInserted(this);
        }
    }
}

/*!
  Constructs a tree widget item and inserts it into the given
  tree \a view after the \a after item.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, QTreeWidgetItem *after)
    : view(0), model(0), par(0),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled)
{
    if (view) {
        model = ::qt_cast<QTreeModel*>(view->model());
        if (model) {
            int i = model->tree.indexOf(after);
            model->tree.insert(i, this);
            model->emitRowsInserted(this);
        }
    }
}

/*!
    Constructs a tree widget item with the given \a parent.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent)
    : view(0), model(0), par(parent),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled)
{
    if (parent)
        parent->appendChild(this);
}

/*!
  Constructs a tree widget item with the given \a parent and,
  and inserted after the \a after child.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *after)
    : view(0), model(0), par(parent),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled)
{
    if (parent) {
        int i = parent->indexOfChild(after);
        parent->insertChild(i, this);
    }
}

/*!
  Destroys this tree widget item.
*/

QTreeWidgetItem::~QTreeWidgetItem()
{
    for (int i = 0; i < children.count(); ++i) {
        QTreeWidgetItem *child = children.at(i);
        child->par = 0;
        child->view = 0;
        child->model = 0;
        delete child;
    }

    children.clear();

    if (par) {
        par->children.removeAll(this);
        return;
    }

    if (model)
        model->remove(this);
}

/*!
  Returns true if the text in the item is less than the text in the
  \a other item, otherwise returns false.
*/

bool QTreeWidgetItem::operator<(const QTreeWidgetItem &other) const
{
    int column = view->header()->sortIndicatorSection();
    return text(column) < other.text(column);
}

/*!
    Sets the value for the item's \a column and \a role to the given
    \a value.
*/
void QTreeWidgetItem::setData(int column, int role, const QVariant &value)
{
    role = (role == QAbstractItemModel::EditRole
            ? QAbstractItemModel::DisplayRole : role);
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
    if (model)
        model->emitDataChanged(this);
}

/*!
    Returns the value for the item's \a column and \a role.
*/
QVariant QTreeWidgetItem::data(int column, int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    if (column < values.size()) {
        const QVector<Data> column_values = values.at(column);
        for (int i = 0; i < column_values.count(); ++i)
            if (column_values.at(i).role == role)
                return column_values.at(i).value;
    }
    return QVariant();
}

/*!
  Removes all item data.
*/
void QTreeWidgetItem::clear()
{
    values.clear();
    if (model)
        model->emitDataChanged(this);
}

/*!
  Appends the \a child item to the list of children.

  \sa insertChild() takeChild()
*/
void QTreeWidgetItem::appendChild(QTreeWidgetItem *child)
{
    insertChild(children.count(), child);
}

/*!
  Inserts the \a child item at \a index in the list of children.
*/
void QTreeWidgetItem::insertChild(int index, QTreeWidgetItem *child)
{
    // FIXME: here we have a problem;
    // the user could build up a tree and then insert the root in the view
    children.insert(index, child);
    child->view = view;
    child->model = model;
    if (model)
        model->emitRowsInserted(child);
}

/*!
  Removes the item at \a index and returns it.
*/
QTreeWidgetItem *QTreeWidgetItem::takeChild(int index)
{
    if (model)
        model->emitRowsRemoved(children.at(index));
    return children.takeAt(index);
}

/*!
  Returns true if the item is explicitly hidden, otherwise returns false.
*/
bool QTreeWidgetItem::isHidden() const
{
    if (view && model) {
        if (this == model->header)
            return view->header()->isHidden();
        QModelIndex index = model->index(const_cast<QTreeWidgetItem*>(this));
        QModelIndex parent = model->parent(index);
        return view->isRowHidden(index.row(), parent);
    }
    return false;
}

/*!
  Hides the item if \a hide is true, otherwise shows the item.
*/
void QTreeWidgetItem::setHidden(bool hide)
{
    if (view && model) {
        if (this == model->header) {
            view->header()->setHidden(hide);
        } else {
            QModelIndex index = model->index(this);
            QModelIndex parent = model->parent(index);
            view->setRowHidden(index.row(), parent, hide);
        }
    }
}

/*!
  Sorts the children by the value in the given \a column, in the \a order
  specified.
*/
void QTreeWidgetItem::sortChildren(int column, Qt::SortOrder order, bool climb)
{
    LessThan compare = (order == Qt::AscendingOrder
                        ? &QTreeModel::lessThan : &QTreeModel::greaterThan);
    qHeapSort(children.begin(), children.end(), compare);
    if (!climb)
        return;
    QList<QTreeWidgetItem*>::iterator it = children.begin();
    for (; it != children.end(); ++it)
        (*it)->sortChildren(column, order, climb);
}

#define d d_func()
#define q q_func()

class QTreeWidgetPrivate : public QTreeViewPrivate
{
    Q_DECLARE_PUBLIC(QTreeWidget)
public:
    QTreeWidgetPrivate() : QTreeViewPrivate(), sortingEnabled(false) {}
    inline QTreeModel *model() const { return ::qt_cast<QTreeModel*>(q_func()->model()); }
    void emitPressed(const QModelIndex &index, Qt::ButtonState button);
    void emitClicked(const QModelIndex &index, Qt::ButtonState button);
    void emitDoubleClicked(const QModelIndex &index, Qt::ButtonState button);
    void emitKeyPressed(const QModelIndex &index, Qt::Key key, Qt::ButtonState state);
    void emitReturnPressed(const QModelIndex &index);
    void emitExpanded(const QModelIndex &index);
    void emitCollapsed(const QModelIndex &index);
    void emitCurrentChanged(const QModelIndex &previous, const QModelIndex &current);
    void emitItemEntered(const QModelIndex &index, Qt::ButtonState state);
    void emitAboutToShowContextMenu(QMenu *menu, const QModelIndex &index);
    void emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    bool sortingEnabled;
};

void QTreeWidgetPrivate::emitPressed(const QModelIndex &index, Qt::ButtonState button)
{
    emit q->pressed(model()->item(index), index.column(), button);
}

void QTreeWidgetPrivate::emitClicked(const QModelIndex &index, Qt::ButtonState button)
{
    emit q->clicked(model()->item(index), index.column(), button);
}

void QTreeWidgetPrivate::emitDoubleClicked(const QModelIndex &index, Qt::ButtonState button)
{
    emit q->doubleClicked(model()->item(index), index.column(), button);
}

void QTreeWidgetPrivate::emitKeyPressed(const QModelIndex &index, Qt::Key key,
                                        Qt::ButtonState state)
{
    emit q->keyPressed(model()->item(index), index.column(), key, state);
}

void QTreeWidgetPrivate::emitReturnPressed(const QModelIndex &index)
{
    emit q->returnPressed(model()->item(index), index.column());
}

void QTreeWidgetPrivate::emitExpanded(const QModelIndex &index)
{
    emit q->expanded(model()->item(index));
}

void QTreeWidgetPrivate::emitCollapsed(const QModelIndex &index)
{
    emit q->collapsed(model()->item(index));
}

void QTreeWidgetPrivate::emitCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    emit q->currentChanged(model()->item(current), model()->item(previous));
}

void QTreeWidgetPrivate::emitItemEntered(const QModelIndex &index, Qt::ButtonState state)
{
    emit q->itemEntered(model()->item(index), index.column(), state);
}

void QTreeWidgetPrivate::emitAboutToShowContextMenu(QMenu *menu, const QModelIndex &index)
{
    emit q->aboutToShowContextMenu(menu, model()->item(index), index.column());
}

void QTreeWidgetPrivate::emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft == bottomRight) // this should always be true, unless we sort
        emit q->itemChanged(model()->item(topLeft), topLeft.column());
}

/*!
  \class QTreeWidget qtreewidget.h

  \brief The QTreeWidget class provides a tree view that uses a predefined
  tree model.

  \ingroup model-view

  The QTreeWidget class is a convenience class that provides a standard
  tree view based on Qt's Model/View architecture with a classic item-based
  interface similar to that used by the \c QListView class in Qt 3.
  This class uses a default model to hold items, each of which is a
  QTreeWidgetItem.

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
    \fn void QTreeWidget::clicked(QTreeWidgetItem *item, int column, Qt::ButtonState button)

    This signal is emitted when the user clicks inside the widget.
    The \a item given is the item that was clicked, or 0 if no item was clicked.
    The \a column is the column that contained the item, or -1 if no item was
    clicked.
    The button clicked is specified by \a button (see \l{Qt::ButtonState}).
*/

/*!
    \fn void QTreeWidget::doubleClicked(QTreeWidgetItem *item, int column, Qt::ButtonState button)

    This signal is emitted when the user double clicks inside the widget.
    The \a item given is the item that was clicked, or 0 if no item was clicked.
    The \a column is the column that contained the item, or -1 if no item was
    clicked.
    The button clicked is specified by \a button (see \l{Qt::ButtonState}).
*/

/*!
  Constructs a tree view with the given \a parent widget.
*/

QTreeWidget::QTreeWidget(QWidget *parent)
    : QTreeView(*new QTreeWidgetPrivate(), parent)
{
    setModel(new QTreeModel(1, this)); // default is 1 column
    connect(this, SIGNAL(pressed(QModelIndex,ButtonState)),
            SLOT(emitPressed(QModelIndex,ButtonState)));
    connect(this, SIGNAL(clicked(QModelIndex,ButtonState)),
            SLOT(emitClicked(QModelIndex,ButtonState)));
    connect(this, SIGNAL(doubleClicked(QModelIndex,ButtonState)),
            SLOT(emitDoubleClicked(QModelIndex,ButtonState)));
    connect(this, SIGNAL(keyPressed(QModelIndex,Key,ButtonState)),
            SLOT(emitKeyPressed(QModelIndex,Key,ButtonState)));
    connect(this, SIGNAL(returnPressed(QModelIndex)),
            SLOT(emitReturnPressed(QModelIndex)));
    connect(this, SIGNAL(expanded(QModelIndex)),
            SLOT(emitExpanded(QModelIndex)));
    connect(this, SIGNAL(collapsed(QModelIndex)),
            SLOT(emitCollapsed(QModelIndex)));
    connect(this, SIGNAL(itemEntered(QModelIndex,ButtonState)),
            SLOT(emitItemEntered(QModelIndex,ButtonState)));
    connect(this, SIGNAL(aboutToShowContextMenu(QMenu*,QModelIndex)),
            SLOT(emitAboutToShowContextMenu(QMenu*,QModelIndex)));
    connect(selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(emitCurrentChanged(QModelIndex,QModelIndex)));
    connect(selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SIGNAL(selectionChanged()));
    connect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(emitItemChanged(QModelIndex,QModelIndex)));
    connect(header(), SIGNAL(sectionPressed(int,ButtonState)), this, SLOT(sortItems(int)));
}

/*!
    Destroys the tree widget and all its items.
*/

QTreeWidget::~QTreeWidget()
{
}

/*!
  Retuns the number of header columns in the view.
*/

int QTreeWidget::columnCount() const
{
    return d->model()->columnCount();
}

/*!
  Sets the number of header \a columns in the tree widget.
*/

void QTreeWidget::setColumnCount(int columns)
{
    d->model()->setColumnCount(columns);
}

/*!
  Returns the top level item at the given \a index, or 0 if the item does
  not exist.
*/

QTreeWidgetItem *QTreeWidget::topLevelItem(int index) const
{
    return d->model()->tree.value(index);
}

/*!
  Returns the number of top level items.
*/

int QTreeWidget::topLevelItemCount() const
{
    return d->model()->tree.count();
}

/*!
  Inserts the \a item at \a index in the top level in the view.

  \sa appendToplevelItem()
*/

void QTreeWidget::insertTopLevelItem(int index, QTreeWidgetItem *item)
{
    d->model()->tree.insert(index, item);
}

/*!
  Appends the \a item as a top-level item in the widget.

  \sa insertToplevelItem()
*/

void QTreeWidget::appendTopLevelItem(QTreeWidgetItem *item)
{
    d->model()->tree.append(item);
}

/*!
  Returns the index of the top-level item or -1 if the item
  cannot be found.
 */
int QTreeWidget::indexOfTopLevelItem(QTreeWidgetItem *item)
{
    return d->model()->tree.indexOf(item);
}

/*!
    Returns the item used for the tree widget's header.

    \sa setHeaderItem()
*/

QTreeWidgetItem *QTreeWidget::headerItem()
{
    return d->model()->header;
}

/*!
    Sets the header \a item for the tree widget. The contents of each of the
    header's columns are supplied by the labels from each of the columns in the
    item.

    \sa headerItem()
*/

void QTreeWidget::setHeaderItem(QTreeWidgetItem *item)
{
    delete d->model()->header;
    d->model()->header = item;
}

/*!
  Adds a column in the header for each item in the \a labels list, and sets
  the label for each column.
*/
void QTreeWidget::setHeaderLabels(const QStringList &labels)
{
    QTreeModel *model = d->model();
    QTreeWidgetItem *item = model->header;
    for (int i = 0; i < labels.count(); ++i)
        item->setText(i, labels.at(i));
}

/*!
    Returns the current item in the tree widget. This is usually highlighted.

    \sa setCurrentItem()
*/

QTreeWidgetItem *QTreeWidget::currentItem() const
{
    return d->model()->item(currentIndex());
}

/*!
  Sets the current \a item in the tree widget.

  \sa currentItem()
*/
void QTreeWidget::setCurrentItem(QTreeWidgetItem *item)
{
    if (item)
        setCurrentIndex(d->model()->index(item));
    else
        setCurrentIndex(QModelIndex());
}

/*!
  Sorts the items in the widget in the specified \a order by the values in
  the given \a column.
*/

void QTreeWidget::sortItems(int column, Qt::SortOrder order)
{
    d->model()->sortAll(column, order);
    header()->setSortIndicator(column, order);
}

/*!
  If \a enable is true, the items in the widget will be sorted if the user
  clicks on a header section; otherwise sorting is disabled.
*/

void QTreeWidget::setSortingEnabled(bool enable)
{
    d->sortingEnabled = enable;
    if (!enable && header()->isSortIndicatorShown())
        header()->setSortIndicatorShown(false);
}

/*!
  Returns if sorting is enabled; otherwise returns false.
  Sorting is enabled when the user clicks on a header section.
*/

bool QTreeWidget::isSortingEnabled() const
{
    return d->sortingEnabled;
}

/*!
  Opens a persistent editor for the \a item in the given \a column.
*/

void QTreeWidget::openPersistentEditor(QTreeWidgetItem *item, int column)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item, column);
    QAbstractItemView::openPersistentEditor(index);
}

/*!
  Closes the persistent editor for the \a item in the given \a column.

  This function has no effect if no persistent editor is open for this
  combination of item and column.
*/

void QTreeWidget::closePersistentEditor(QTreeWidgetItem *item, int column)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item, column);
    QAbstractItemView::closePersistentEditor(index);
}

/*!
  Returns true if the \a item is selected; otherwise returns false.
*/

bool QTreeWidget::isSelected(const QTreeWidgetItem *item) const
{
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item));
    return selectionModel()->isSelected(index);
}

/*!
  If \a select is true, the given \a item is selected; otherwise it is
  deselected.
*/

void QTreeWidget::setSelected(const QTreeWidgetItem *item, bool select)
{
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item));
    selectionModel()->select(index, (select ? QItemSelectionModel::Select
                                     : QItemSelectionModel::Deselect)
                             |QItemSelectionModel::Rows);
}

/*!
  Returns a list of all selected items.
*/

QList<QTreeWidgetItem*> QTreeWidget::selectedItems() const
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        items.append(d->model()->item(indexes.at(i)));
    return items;
}

/*!
  Returns a list of items that match the \a text, using the criteria given
  in the \a flags.
*/

QList<QTreeWidgetItem*> QTreeWidget::findItems(const QString &text,
                                               QAbstractItemModel::MatchFlags flags) const
{
    QModelIndex topLeft = d->model()->index(0, 0);
    int role = QAbstractItemModel::DisplayRole;
    int hits = d->model()->rowCount();
    QModelIndexList indexes = d->model()->match(topLeft, role, text,hits, flags);
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        items << d->model()->item(indexes.at(i));
    return items;
}

/*!
  Returns true if the \a item is in the viewport; otherwise returns false.
*/

bool QTreeWidget::isItemVisible(const QTreeWidgetItem *item) const
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item));
    QRect rect = itemViewportRect(index);
    return d->viewport->rect().contains(rect);
}

/*!
  Returns true if the given \a item is open; otherwise returns false.
*/

bool QTreeWidget::isItemOpen(const QTreeWidgetItem *item) const
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item));
    return isOpen(index);
}

/*!
  Ensures that the \a item is visible, scrolling the view if necessary.
*/

void QTreeWidget::ensureItemVisible(const QTreeWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item));
    QTreeView::ensureItemVisible(index);
}

/*!
    Opens the \a item. This causes the tree containing the item's children
    to be expanded.
*/

void QTreeWidget::openItem(const QTreeWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item));
    open(index);
}

/*!
    Closes the \a item. This causes the tree containing the item's children
    to be collapsed.
*/

void QTreeWidget::closeItem(const QTreeWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item));
    close(index);
}

/*!
  Sorts all items in the widget by the values in the given \a column.
  The items are sorted in the order given by the sort indicator in the header.
*/

void QTreeWidget::sortItems(int column)
{
    if (d->sortingEnabled) {
        Qt::SortOrder order = header()->sortIndicatorOrder();
        int section = header()->sortIndicatorSection();
        order = (order == Qt::AscendingOrder && column == section
                 ? Qt::DescendingOrder : Qt::AscendingOrder);
        header()->setSortIndicator(column, order);
        d->model()->sortAll(column, order);
        if (!header()->isSortIndicatorShown())
            header()->setSortIndicatorShown(true);
    }
}

/*!
  Clears the tree widget by removing all of its items.
*/

void QTreeWidget::clear()
{
    d->model()->clear();
}

/*!
  Appends an \a item to the tree widget.
*/

void QTreeWidget::appendItem(QTreeWidgetItem *item)
{
    d->model()->append(item);
}

/*!
  Removes an \a item from the tree widget.
*/

void QTreeWidget::removeItem(QTreeWidgetItem *item)
{
    d->model()->remove(item);
}

/*!
  \reimp
*/

void QTreeWidget::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
}

#include "moc_qtreewidget.cpp"
