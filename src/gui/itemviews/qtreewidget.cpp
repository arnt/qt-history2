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

    QModelIndex index(QTreeWidgetItem *item, int column) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent = QModelIndex::Null) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, int role, const QVariant &value);

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, QDrag::DropAction action,
                      int row, const QModelIndex &parent);
    QDrag::DropActions supportedDropActions() const;
    
    bool insertRows(int row, const QModelIndex &parent, int count);
    bool removeRows(int row, const QModelIndex &parent, int count);

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

    bool isSortable() const;
    void sort(int column, const QModelIndex &parent, Qt::SortOrder order);
    void sortAll(int column, Qt::SortOrder);

    static bool lessThan(const QTreeWidgetItem *left, const QTreeWidgetItem *right);
    static bool greaterThan(const QTreeWidgetItem *left, const QTreeWidgetItem *right);

    void emitDataChanged(QTreeWidgetItem *item, int column);

protected:
    void append(QTreeWidgetItem *item);
    void remove(QTreeWidgetItem *item);
    void emitRowsInserted(QTreeWidgetItem *item);
    void emitRowsAboutToBeRemoved(QTreeWidgetItem *item);

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
        emit columnsAboutToBeRemoved(QModelIndex::Null, qMax(_c - 1, 0), qMax(c - 1, 0));
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
    return index(parent, 0);
}

/*!
  \internal
  \reimp

  Returns the number of rows in the \a parent model index.
*/

int QTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (parent.column() != 0)
            return 0;
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
    if (orientation == Qt::Horizontal)
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

QStringList QTreeModel::mimeTypes() const
{
    return QAbstractItemModel::mimeTypes();
//     QStringList types;
//     types << "application/x-qtreemodeldatalist";
//     return types;
}

QMimeData *QTreeModel::mimeData(const QModelIndexList &indexes) const
{
    // FIXME: we need some way of finding the out if we are dnd'ing internally or not
    return QAbstractItemModel::mimeData(indexes);
}

bool QTreeModel::dropMimeData(const QMimeData *data, QDrag::DropAction action,
                              int row, const QModelIndex &parent)
{
    // FIXME: just do copy for now
    if (action == QDrag::MoveAction)
        qWarning("QDrag::MoveAction is not supported yet");
    return QAbstractItemModel::dropMimeData(data, action, row, parent);
}

QDrag::DropActions QTreeModel::supportedDropActions() const
{
    // the move action is only effective when dragging and dropping internally
    return QDrag::CopyAction;// | QDrag::MoveAction;
}

/*!
  \internal

  Inserts a tree view item into the \a parent item at the given
  \a row. Returns true if successful; otherwise returns false.

  An empty item will be created by this function.
*/

bool QTreeModel::insertRows(int row, const QModelIndex &parent, int count)
{
    QTreeWidgetItem *c = 0;
    if (parent.isValid()) {
        QTreeWidgetItem *p = item(parent);
        Q_ASSERT(p);
        for (int r = row; r < row + count; ++r) {
            c = new QTreeWidgetItem();
            c->view = p->view;
            c->model = p->model;
            c->par = p;
            p->children.insert(r, c);
        }
    } else {
        QTreeWidget *view = ::qt_cast<QTreeWidget*>(QObject::parent());
        for (int r = row; r < row + count; ++r) {
            c = new QTreeWidgetItem();
            c->view = view;
            c->model = this;
            c->par = 0;
            tree.insert(r, c);
        }
    }
    emit rowsInserted(parent, row, row + count - 1);
    return true;
}

/*!
  \internal

  Removes the given \a row from the \a parent item, and returns true
  if successful; otherwise false is returned.
*/

bool QTreeModel::removeRows(int row, const QModelIndex &parent, int count)
{
    emit rowsAboutToBeRemoved(parent, row, row + count - 1);
    QTreeWidgetItem *c = 0;
    if (parent.isValid()) {
        QTreeWidgetItem *p = item(parent);
        Q_ASSERT(p);
        for (int r = row; r < row + count; ++r) {
             c = p->children.takeAt(r);
             c->view = 0;
             c->model = 0;
             delete c;
        }
    } else {
        for (int r = row; r < row + count; ++r) {
            c = tree.takeAt(r);
            c->view = 0;
            c->model = 0;
            delete c;
        }
    }
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
        return QAbstractItemModel::ItemIsDropEnabled; // can drop on the viewport
    QTreeWidgetItem *itm = item(index);
    Q_ASSERT(itm);
    return itm->flags();
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

void QTreeModel::emitDataChanged(QTreeWidgetItem *item, int column)
{
    QModelIndex tl = index(item, (column != -1 ? column : 0));
    QModelIndex br = (column != -1 ? tl : index(item, columnCount() - 1));
    emit dataChanged(tl, br);
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
        emit rowsAboutToBeRemoved(QModelIndex::Null, r, r);
    }
}

/*!
  \internal

  Emits the rowsInserted() signal for the row containing the given \a item.

  \sa emitRowsRemoved()
*/

void QTreeModel::emitRowsInserted(QTreeWidgetItem *item)
{
    QModelIndex idx = index(item, 0);
    QModelIndex parentIndex = parent(idx);
    emit rowsInserted(parentIndex, idx.row(), idx.row());
}

/*!
  \internal

  Emits the rowsRemoved() signal for the rows containing the given \a item.

  \sa emitRowsInserted()
*/

void QTreeModel::emitRowsAboutToBeRemoved(QTreeWidgetItem *item)
{
    QModelIndex idx = index(item, 0);
    QModelIndex parentIndex = parent(idx);
    emit rowsAboutToBeRemoved(parentIndex, idx.row(), idx.row());
}

/*!
  \class QTreeWidgetItem qtreewidget.h

  \brief The QTreeWidgetItem class provides an item for use with the
  QTreeWidget convenience class.

  \ingroup model-view

  Tree widget items are used to hold rows of information for tree widgets.
  Rows usually contain several columns of data, each of which can contain
  a text label and an icon.

  The QTreeWidgetItem class is a convenience class that replaces the
  \c QListViewItem class in Qt 3. It provides an item for use with
  the QTreeWidget class.

  Items are usually constructed with a parent that is either a QTreeWidget
  (for top-level items) or a QTreeWidgetItem (for items on lower levels of
  the tree). For example, the following code constructs a top-level item
  to represent cities of the world, and adds a entry for Oslo as a child
  item:

  \quotefile snippets/qtreewidget-using/mainwindow.cpp
  \skipto QTreeWidgetItem *cities
  \printuntil osloItem->setText(1, tr("Yes"));

  Items can be added in a particular order by specifying the item they
  follow when they are constructed:

  \skipto QTreeWidgetItem *planets
  \printuntil planets->setText(0

  Each column in an item can have its own background color which is set with
  the setBackgroundColor() function. The current background color can be
  found with backgroundColor().
  The text label for each column can be rendered with its own font and text
  color. These are specified with the setFont() and setTextColor() functions,
  and read with font() and textColor().

  The main difference between top-level items and those in lower levels of
  the tree is that a top-level item has no parent(). This information
  can be used to tell the difference between items, and is useful to know
  when inserting and removing items from the tree.
  Children of an item can be removed with takeChild() and inserted at a
  given index in the list of children with the insertChild() function.

  \sa QTreeWidget
*/

/*!
    \fn QTreeWidget *QTreeWidgetItem::treeWidget() const

    Returns the tree widget that contains the item.
*/

/*!
    \fn QAbstractItemModel::ItemFlags QTreeWidgetItem::flags() const

    Returns the flags used to describe the item. These determine whether
    the item can be checked, edited, and selected.

    \sa setFlags()
*/

/*!
    \fn void QTreeWidgetItem::setFlags(QAbstractItemModel::ItemFlags flags)

    Sets the flags for the item to the given \a flags. These determine whether
    the item can be selected or modified.

    \sa flags()
*/

/*!
    \fn QString QTreeWidgetItem::text(int column) const

    Returns the text in the specified \a column.

    \sa setText()
*/

/*!
    \fn void QTreeWidgetItem::setText(int column, const QString &text)

    Sets the text to be displayed in the given \a column to the given \a text.

    \sa text() setFont() setTextColor()
*/

/*!
    \fn QIcon QTreeWidgetItem::icon(int column) const

    Returns the icon that is displayed in the specified \a column.

    \sa setIcon()
*/

/*!
    \fn void QTreeWidgetItem::setIcon(int column, const QIcon &icon)

    Sets the icon to be displayed in the given \a column to \a icon.

    \sa icon() setText()
*/

/*!
    \fn QString QTreeWidgetItem::statusTip(int column) const

    Returns the status tip for the contents of the given \a column.

    \sa setStatusTip()
*/

/*!
    \fn void QTreeWidgetItem::setStatusTip(int column, const QString &statusTip)

    Sets the status tip for the given \a column to the given \a statusTip.

    \sa statusTip() setToolTip() setWhatsThis()
*/

/*!
    \fn QString QTreeWidgetItem::toolTip(int column) const

    Returns the tool tip for the given \a column.

    \sa setToolTip()
*/

/*!
    \fn void QTreeWidgetItem::setToolTip(int column, const QString &toolTip)

    Sets the tooltip for the given \a column to \a toolTip.

    \sa toolTip() setStatusTip() setWhatsThis()
*/

/*!
    \fn QString QTreeWidgetItem::whatsThis(int column) const

    Returns the "What's This?" help for the contents of the given \a column.

    \sa setWhatsThis()
*/

/*!
    \fn void QTreeWidgetItem::setWhatsThis(int column, const QString &whatsThis)

    Sets the "What's This?" help for the given \a column to \a whatsThis.

    \sa whatsThis() setStatusTip() setToolTip()
*/

/*!
    \fn QFont QTreeWidgetItem::font(int column) const

    Returns the font used to render the text in the specified \a column.

    \sa setFont()
*/

/*!
    \fn void QTreeWidgetItem::setFont(int column, const QFont &font)

    Sets the font used to display the text in the given \a column to the given
    \a font.

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

    Sets the color used to display the text in the given \a column to \a color.

    \sa textColor() setFont() setText()
*/

/*!
    \fn int QTreeWidgetItem::checkedState(int column) const

    Returns the checked state of the label in the given \a column.

    \sa QCheckBox::ToggleState
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

    Returns the item's parent.

    \sa child()
*/

/*!
    \fn QTreeWidgetItem *QTreeWidgetItem::child(int index) const

    Returns the item at the given \a index in the list of the item's children.

    \sa parent()
*/

/*!
    \fn int QTreeWidgetItem::childCount() const

    Returns the number of child items.
*/

/*!
    \fn int QTreeWidgetItem::columnCount() const

    Returns the number of columns in the item.
*/

/*!
  \fn int QTreeWidgetItem::textAlignment(int column) const

  Returns the text alignment for the label in the given \a column
  (see \l{Qt::AlignmentFlag}).
*/

/*!
  \fn void QTreeWidgetItem::setTextAlignment(int column, int alignment)

  Sets the text alignment for the label in the given \a column to
  the \a alignment specified (see \l{Qt::AlignmentFlag}).
*/

/*!
    \fn int QTreeWidgetItem::indexOfChild(QTreeWidgetItem *child) const

    Returns the index of the given \a child in the item's list of children.
*/

/*!
  \internal
  Constructs a tree widget item. The item must be inserted into a tree view.

  \sa QTreeModel::append() QTreeWidget::append()
*/

QTreeWidgetItem::QTreeWidgetItem()
    : view(0), model(0), par(0),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled
                |QAbstractItemModel::ItemIsDragEnabled)
{
}

/*!
    \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view)

    Constructs a tree widget item and appends it into the given tree
    \a view.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view)
    : view(view), model(0), par(0),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled
                |QAbstractItemModel::ItemIsDragEnabled)
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
  \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, QTreeWidgetItem *preceding)

  Constructs a tree widget item and inserts it into the given
  tree \a view after the \a preceding item.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, QTreeWidgetItem *after)
    : view(0), model(0), par(0),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled
                |QAbstractItemModel::ItemIsDragEnabled)
{
    if (view) {
        model = ::qt_cast<QTreeModel*>(view->model());
        if (model) {
            int i = model->tree.indexOf(after) + 1;
            model->tree.insert(i, this);
            model->emitRowsInserted(this);
        }
    }
}

/*!
    Constructs a tree widget item and append it to the given \a parent.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent)
    : view(0), model(0), par(parent),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled
                |QAbstractItemModel::ItemIsDragEnabled)
{
    if (parent)
        parent->appendChild(this);
}

/*!
  \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *preceding)

  Constructs a tree widget item with the given \a parent that is inserted
  into the parent's list of child items after the \a preceding child.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *after)
    : view(0), model(0), par(parent),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled
                |QAbstractItemModel::ItemIsDragEnabled)
{
    if (parent) {
        int i = parent->indexOfChild(after) + 1;
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
            break;
        }
    }
    values[column].append(Data(role, value));
    if (model)
        model->emitDataChanged(this, column);
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
        model->emitDataChanged(this, -1);
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
  Removes the item at \a index and returns it, otherwise return 0.
*/
QTreeWidgetItem *QTreeWidgetItem::takeChild(int index)
{
    if (index >= 0 && index < children.count()) {
        if (model)
            model->emitRowsAboutToBeRemoved(children.at(index));
        return children.takeAt(index);
    }
    return 0;
}

/*!
  Sorts the children by the value in the given \a column, in the \a order
  specified. If \a climb is true, the items below each of the children will
  also be sorted.
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
    void emitPressed(const QModelIndex &index, Qt::MouseButton button,
                     Qt::KeyboardModifiers modifiers);
    void emitClicked(const QModelIndex &index, Qt::MouseButton button,
                     Qt::KeyboardModifiers modifiers);
    void emitDoubleClicked(const QModelIndex &index, Qt::MouseButton button,
                           Qt::KeyboardModifiers modifiers);
    void emitKeyPressed(const QModelIndex &index, Qt::Key key, Qt::KeyboardModifiers modifiers);
    void emitReturnPressed(const QModelIndex &index);
    void emitExpanded(const QModelIndex &index);
    void emitCollapsed(const QModelIndex &index);
    void emitCurrentChanged(const QModelIndex &previous, const QModelIndex &current);
    void emitItemEntered(const QModelIndex &index, Qt::MouseButton button,
                         Qt::KeyboardModifiers modifiers);
    void emitAboutToShowContextMenu(QMenu *menu, const QModelIndex &index);
    void emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    bool sortingEnabled;
};

void QTreeWidgetPrivate::emitPressed(const QModelIndex &index, Qt::MouseButton button,
                                     Qt::KeyboardModifiers modifiers)
{
    emit q->pressed(model()->item(index), index.column(), button, modifiers);
}

void QTreeWidgetPrivate::emitClicked(const QModelIndex &index, Qt::MouseButton button,
                                     Qt::KeyboardModifiers modifiers)
{
    emit q->clicked(model()->item(index), index.column(), button, modifiers);
}

void QTreeWidgetPrivate::emitDoubleClicked(const QModelIndex &index, Qt::MouseButton button,
                                           Qt::KeyboardModifiers modifiers)
{
    emit q->doubleClicked(model()->item(index), index.column(), button, modifiers);
}

void QTreeWidgetPrivate::emitKeyPressed(const QModelIndex &index, Qt::Key key,
                                        Qt::KeyboardModifiers modifiers)
{
    emit q->keyPressed(model()->item(index), index.column(), key, modifiers);
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

void QTreeWidgetPrivate::emitItemEntered(const QModelIndex &index, Qt::MouseButton button,
                                         Qt::KeyboardModifiers modifiers)
{
    emit q->itemEntered(model()->item(index), index.column(), button, modifiers);
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
  tree widget with a classic item-based interface similar to that used by
  the \c QListView class in Qt 3.
  This class is based on Qt's Model/View architecture and uses a default
  model to hold items, each of which is a QTreeWidgetItem.

  Developers who do not need the flexibility of the Model/View framework
  can use this class to create simple hierarchical lists very easily. A more
  flexible approach involves combining a QTreeView with a standard item model.
  This allows the storage of data to be separated from its representation.

  In its simplest form, a tree widget can be constructed in the following way:

  \quotefile snippets/qtreewidget-using/mainwindow.h
  \skipto QTreeWidget *
  \printuntil QTreeWidget *
  \quotefile snippets/qtreewidget-using/mainwindow.cpp
  \skipto treeWidget = new
  \printuntil treeWidget = new

  Before items can be added to the tree widget, the number of columns must
  be set with setColumnCount(). This allows each item to have one or more
  labels or other decorations. The number of columns in use can be found
  with the columnCount() function.

  The tree can have a header that contains a section for each column in
  the widget. It is easiest to set up the labels for each section by
  supplying a list of strings with setHeaderLabels(), but a custom header
  can be constructed with a QTreeWidgetItem and inserted into the tree
  with the setHeaderItem() function.

  The items in the tree can be sorted by column according to a predefined
  sort order. If sorting is enabled, the user can sort the items by clicking
  on a column header. Sorting can be enabled or disabled by calling
  setSortingEnabled(). The isSortingEnabled() function indicates whether
  sorting is enabled.



  \sa \link model-view-programming.html Model/View Programming\endlink QTreeWidgetItem
*/

/*!
    \property QTreeWidget::columnCount
    \brief the number of columns displayed in the tree widget
*/

/*!
    \property QTreeWidget::sortingEnabled
    \brief whether the items in the tree widget can be sorted
    by clicking on the header
*/

/*!
    \fn void QTreeWidget::pressed(QTreeWidgetItem *item, int column, const QMouseEvent *event)

    This signal is emitted when the user presses a mouse button inside the
    widget. The specified \a item is the item that was clicked, or 0 if no item
    was clicked.
    The \a column is the item's column that was clicked, or -1 if
    no item was clicked.
*/

/*!
    \fn void QTreeWidget::clicked(QTreeWidgetItem *item, int column, const QMouseEvent *event)

    This signal is emitted when the user clicks inside the widget.
    The specified \a item is the item that was clicked, or 0 if no item was
    clicked.
    The \a column is the item's column that was clicked, or -1 if
    no item was clicked.
*/

/*!
    \fn void QTreeWidget::doubleClicked(QTreeWidgetItem *item, int column, const QMouseEvent *event)

    This signal is emitted when the user double clicks inside the widget.
    The specified \a item is the item that was clicked, or 0 if no item was
    clicked.
    The \a column is the item's column that was clicked, or -1 if
    no item was clicked.
*/

/*!
    \fn void QTreeWidget::keyPressed(QTreeWidgetItem *item, int column, const QKeyEvent *event)

    This signal is emitted when the user presses a key inside the widget.
    The specified \a item and \a column had the focus when the \a key was
    pressed.
    If no item had the focus, \a item is 0 and \a column is -1.

    \sa returnPressed()
*/

/*!
    \fn void QTreeWidget::returnPressed(QTreeWidgetItem *item, int column)

    This signal is emitted when the return key is pressed inside the widget.

    The specified \a item and \a column had the focus when the return key was
    pressed.
    If no item had the focus, \a item is 0 and \a column is -1.

    \sa keyPressed()
*/

/*!
    \fn void QTreeWidget::expanded(QTreeWidgetItem *item)

    This signal is emitted when the specified \a item is expanded so that
    all of its children are displayed.

    \sa isItemOpen()
*/

/*!
    \fn void QTreeWidget::collapsed(QTreeWidgetItem *item)

    This signal is emitted when the specified \a item is collapsed so that
    none of its children are displayed.

    \sa isItemOpen()
*/

/*!
    \fn void QTreeWidget::currentChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)

    This signal is emitted when the current item changes. The current
    item is specified by \a current, and this replaces the \a previous
    current item.
*/

/*!
    \fn void QTreeWidget::selectionChanged()

    This signal is emitted when the selection changes in the tree widget.
    The current selection can be found with selectedItems().
*/

/*!
    \fn void QTreeWidget::itemEntered(QTreeWidgetItem *item, int column, const QMouseEvent *event)

    This signal is emitted when the mouse cursor enters an \a item over the
    specified \a column.
*/

/*!
    \fn void QTreeWidget::aboutToShowContextMenu(QMenu *menu, QTreeWidgetItem *item, int column)

    This signal is emitted when the tree widget is about to show a context
    menu specified by \a menu. The menu is actived for the specified \a item
    and \a column.
*/

/*!
    \fn void QTreeWidget::itemChanged(QTreeWidgetItem *item, int column)

    This signal is emitted when the contents of the \a column in the specified
    \a item changes.
*/

/*!
  Constructs a tree widget with the given \a parent.
*/

QTreeWidget::QTreeWidget(QWidget *parent)
    : QTreeView(*new QTreeWidgetPrivate(), parent)
{
    setModel(new QTreeModel(0, this));

    connect(this, SIGNAL(pressed(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
            SLOT(emitPressed(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)));
    connect(this, SIGNAL(clicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
            SLOT(emitClicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)));
    connect(this, SIGNAL(doubleClicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
            SLOT(emitDoubleClicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)));
    connect(this, SIGNAL(keyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)),
            SLOT(emitKeyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)));
    connect(this, SIGNAL(returnPressed(QModelIndex)),
            SLOT(emitReturnPressed(QModelIndex)));
    connect(this, SIGNAL(expanded(QModelIndex)),
            SLOT(emitExpanded(QModelIndex)));
    connect(this, SIGNAL(collapsed(QModelIndex)),
            SLOT(emitCollapsed(QModelIndex)));
    connect(this, SIGNAL(itemEntered(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
            SLOT(emitItemEntered(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)));
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
    connect(header(), SIGNAL(sectionPressed(int,Qt::MouseButton,Qt::KeyboardModifiers)),
            this, SLOT(sortItems(int)));

    d->viewport->setAcceptDrops(true);
    setDraggableItems(true);
}

/*!
    Destroys the tree widget and all its items.
*/

QTreeWidget::~QTreeWidget()
{
}

/*
  Retuns the number of header columns in the view.
*/

int QTreeWidget::columnCount() const
{
    return d->model()->columnCount();
}

/*
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

/*!\property QTreeWidget::topLevelItemCount
    \brief the number of top-level items
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
  Removes the top-level item at the given \a index in the tree and
  returns it, otherwise returns 0;
*/

QTreeWidgetItem *QTreeWidget::takeTopLevelItem(int index)
{
    if (index >= 0 && index < d->model()->tree.count()) {
        d->model()->emitRowsAboutToBeRemoved(topLevelItem(index));
        return d->model()->tree.takeAt(index);
    }
    return 0;
}

/*!
  Returns the index of the given top-level \a item, or -1 if the item
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

QTreeWidgetItem *QTreeWidget::headerItem() const
{
    return d->model()->header;
}

/*!
    Sets the header \a item for the tree widget. The label for each column in
    the header is supplied by the corresponding label in the item.

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
    Returns the current item in the tree widget.

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
        setCurrentIndex(d->model()->index(item, 0));
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
  Returns true if sorting is enabled; otherwise returns false.
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
  Returns true if the \a item is selected and not-hidden and does not
  have hidden parents; otherwise returns false.
*/
bool QTreeWidget::isSelected(const QTreeWidgetItem *item) const
{
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    if (selectionModel()->isSelected(index)) {
        while (index.isValid() && !isIndexHidden(index))
            index = index.parent();
        if (!index.isValid())
            return true;
    }
    return false;
}

/*!
  If \a select is true, the given \a item is selected; otherwise it is
  deselected.
*/

void QTreeWidget::setSelected(const QTreeWidgetItem *item, bool select)
{
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    selectionModel()->select(index, (select ? QItemSelectionModel::Select
                                     : QItemSelectionModel::Deselect)
                             |QItemSelectionModel::Rows);
}

/*!
  Returns a list of all selected non-hidden items.
*/

QList<QTreeWidgetItem*> QTreeWidget::selectedItems() const
{
    QModelIndexList indexes = selectedIndexes();
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        items.append(d->model()->item(indexes.at(i)));
    return items;
}

/*!
  Returns a list of items that match the given \a text, using the criteria
  given by the \a flags (see QAbstractItemModel::MatchFlags).
*/

QList<QTreeWidgetItem*> QTreeWidget::findItems(const QString &text,
                                               QAbstractItemModel::MatchFlags flags) const
{
    QModelIndex topLeft = d->model()->index(0, 0);
    int role = QAbstractItemModel::DisplayRole;
    int hits = d->model()->rowCount(QModelIndex::Null);
    QModelIndexList indexes = d->model()->match(topLeft, role, text, hits, flags);
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        items << d->model()->item(indexes.at(i));
    return items;
}

/*!
  Returns true if the \a item is explicitly hidden, otherwise returns false.
*/
bool QTreeWidget::isItemHidden(const QTreeWidgetItem *item) const
{
    if (item == headerItem())
        return header()->isHidden();
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    QModelIndex parent = index.parent();
    return isRowHidden(index.row(), parent);
}

/*!
  Hides the given \a item if \a hide is true; otherwise shows the item.
*/
void QTreeWidget::setItemHidden(const QTreeWidgetItem *item, bool hide)
{
    if (item == headerItem()) {
        header()->setHidden(hide);
    } else {
        QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
        QModelIndex parent = index.parent();
        setRowHidden(index.row(), parent, hide);
    }
}

/*!
  Returns true if the \a item is in the viewport; otherwise returns false.
*/

bool QTreeWidget::isItemVisible(const QTreeWidgetItem *item) const
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    QRect rect = itemViewportRect(index);
    return rect.isValid() && d->viewport->rect().contains(rect);
}

/*!
  Returns true if the given \a item is open; otherwise returns false.
*/

bool QTreeWidget::isItemOpen(const QTreeWidgetItem *item) const
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    return isOpen(index);
}

/*!
  Ensures that the \a item is visible, scrolling the view if necessary.
*/

void QTreeWidget::ensureItemVisible(const QTreeWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    QTreeView::ensureItemVisible(index);
}

/*!
    Opens the \a item. This causes the tree containing the item's children
    to be expanded.
*/

void QTreeWidget::openItem(const QTreeWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    open(index);
}

/*!
    Closes the \a item. This causes the tree containing the item's children
    to be collapsed.
*/

void QTreeWidget::closeItem(const QTreeWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
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
  Clears the tree widget by removing all of its items and selections.
*/

void QTreeWidget::clear()
{
    selectionModel()->clear();
    d->model()->clear();
}


/*!
  \reimp
*/

void QTreeWidget::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
}

#include "moc_qtreewidget.cpp"
