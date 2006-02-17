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

#ifndef QT_NO_TREEWIDGET
#include <qapplication.h>
#include <qheaderview.h>
#include <qpainter.h>
#include <qitemdelegate.h>
#include <qstack.h>
#include <qdebug.h>
#include <private/qtreeview_p.h>
#include <private/qwidgetitemdata_p.h>
#include <private/qtreewidget_p.h>
// workaround for VC++ 6.0 linker bug (?)
typedef bool(*LessThan)(const QPair<QTreeWidgetItem*,int>&,const QPair<QTreeWidgetItem*,int>&);

class QTreeWidgetMimeData : public QMimeData
{
    Q_OBJECT
public:
    QList<QTreeWidgetItem*> items;
};

#include "qtreewidget.moc"

/*
    \class QTreeModel
    \brief The QTreeModel class manages the items stored in a tree view.

    \ingroup model-view
    \mainclass
*/

/*!
  \internal

  Constructs a tree model with a \a parent object and the given
  number of \a columns.
*/

QTreeModel::QTreeModel(int columns, QObject *parent)
    : QAbstractItemModel(parent), header(new QTreeWidgetItem),
      sorting(Qt::AscendingOrder)
{
    setColumnCount(columns);
}

/*!
  \internal

*/

QTreeModel::QTreeModel(QTreeModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent), header(new QTreeWidgetItem),
      sorting(Qt::AscendingOrder)
{

}

/*!
  \internal

  Destroys this tree model.
*/

QTreeModel::~QTreeModel()
{
    clear();
    delete header;
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
        delete item;
    }
    tree.clear();
    reset();
}

/*!
  \internal

  Sets the number of \a columns in the tree model.
*/

void QTreeModel::setColumnCount(int columns)
{
    Q_ASSERT(columns >= 0);
    if (!header)
        header = new QTreeWidgetItem();
    int count = header->columnCount();
    if (count == columns)
        return;

    if (columns < count) {
        beginRemoveColumns(QModelIndex(), columns, count - 1);
        header->values.resize(columns);
        endRemoveColumns();
    } else {
        beginInsertColumns(QModelIndex(), count, columns - 1);
        header->values.resize(columns);
        for (int i = count; i < columns; ++i) // insert data without emitting the dataChanged signal
            header->values[i].append(QWidgetItemData(Qt::DisplayRole, QString::number(i)));
        endInsertColumns();
    }
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
    return static_cast<QTreeWidgetItem*>(index.internalPointer());
}

/*!
  \internal

  Returns the model index that refers to the
  tree view \a item and \a column.
*/

QModelIndex QTreeModel::index(QTreeWidgetItem *item, int column) const
{
    if (!item)
        return QModelIndex();
    const QTreeWidgetItem *par = item->parent();
    if (par)
        return createIndex(par->children.lastIndexOf(item), column, item);
    return createIndex(tree.lastIndexOf(item), column, item);
}

/*!
  \internal
  \reimp

  Returns the model index with the given \a row,
  \a column and \a parent.
*/

QModelIndex QTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    int c = columnCount(parent);

    if (row < 0 || column < 0 || column >= c)
        return QModelIndex();

    // toplevel items
    if (!parent.isValid()) {
        if (row >= tree.count())
            return QModelIndex();
        QTreeWidgetItem *itm = tree.at(row);
        if (itm)
            return createIndex(row, column, itm);
        return QModelIndex();
    }

    // children
    QTreeWidgetItem *parentItem = item(parent);
    if (parentItem && row < parentItem->childCount()) {
        QTreeWidgetItem *itm = static_cast<QTreeWidgetItem*>(parentItem->child(row));
        if (itm)
            return createIndex(row, column, itm);
        return QModelIndex();
    }

    return QModelIndex();
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
        return QModelIndex();
    QTreeWidgetItem *itm = reinterpret_cast<QTreeWidgetItem *>(child.internalPointer());
    if (!itm)
        return QModelIndex();
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
    if (!header)
        return 0;
    return header->columnCount();
}

bool QTreeModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return tree.count() > 0;
    QTreeWidgetItem *itm = item(parent);
    if (!itm)
        return false;
    return itm->children.count() > 0;
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

bool QTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    QTreeWidgetItem *itm = item(index);
    if (itm) {
        itm->setData(index.column(), role, value);
        return true;
    }
    return false;
}

/*!
  \internal
  \reimp
*/
bool QTreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent) || parent.column() > 0)
        return false;

    beginInsertRows(parent, row, row + count - 1);
    while (count > 0) {
        QTreeWidgetItem *par = item(parent);
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->view = qobject_cast<QTreeWidget*>(QObject::parent());
        item->par = par;
        if (par)
            par->children.insert(row++, item);
        else
            tree.insert(row++, item);
        --count;
    }
    endInsertRows();
    return true;
}

/*!
  \internal
  \reimp
*/
bool QTreeModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (count < 1 || column < 0 || column > columnCount(parent) || parent.column() > 0)
        return false;

    beginInsertColumns(parent, column, column + count - 1);

    int oldCount = columnCount(parent);
    column = qBound(0, column, oldCount);
    header->values.resize(oldCount + count);
    for (int i = oldCount; i < oldCount + count; ++i)
        header->values[i].append(QWidgetItemData(Qt::DisplayRole, QString::number(i)));

    QStack<QTreeWidgetItem*> itemstack;
    itemstack.push(0);
    while (!itemstack.isEmpty()) {
        QTreeWidgetItem *par = itemstack.pop();
        QList<QTreeWidgetItem*> children = par ? par->children : tree;
        for (int row = 0; row < children.count(); ++row) {
            QTreeWidgetItem *child = children.at(row);
            if (child->children.count())
                itemstack.push(child);
            child->values.insert(column, count, QVector<QWidgetItemData>());
        }
    }

    endInsertColumns();
    return true;
}

/*!
  \internal
  \reimp
*/
bool QTreeModel::removeRows(int row, int count, const QModelIndex &parent) {
    if (count < 1 || row < 0 || (row + count) > rowCount(parent))
        return false;

    beginRemoveRows(parent, row, row + count);

    bool blockSignal = signalsBlocked();
    blockSignals(true);

    QTreeWidgetItem *itm = item(parent);
    for (int i = row+count-1; i >= row; --i) {
        QTreeWidgetItem *child = itm ? itm->takeChild(i) : tree.takeAt(i);
        child->view = 0;
        Q_ASSERT(child);
        delete child;
        child = 0;
    }
    blockSignals(blockSignal);

    endRemoveRows();
    return true;
}

/*!
  \internal
  \reimp

  Returns the header data corresponding to the given header \a section,
  \a orientation and data \a role.
*/

QVariant QTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!header)
        return section;
    if (orientation == Qt::Horizontal) {
        if (header)
            return header->data(section, role);
        if (role == Qt::DisplayRole)
            return section + 1;
    }
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
                               const QVariant &value, int role)
{
   if (section < 0 || (orientation == Qt::Horizontal && header->columnCount() <= section))
    return false;

    if (orientation == Qt::Horizontal && header) {
        header->setData(section, role, value);
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

/*!
  \reimp

  Returns the flags for the item refered to the given \a index.

*/

Qt::ItemFlags QTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled; // can drop on the viewport
    QTreeWidgetItem *itm = item(index);
    Q_ASSERT(itm);
    return itm->flags();
}

/*!
  \internal

  Sorts the entire tree in the model in the given \a order,
  by the values in the given \a column.
*/

void QTreeModel::sort(int column, Qt::SortOrder order)
{
    if (header && (column < 0 || column >= header->columnCount()))
        return;

    // sort top level
    sortItems(&tree, column, order);

    // sort the children
    QList<QTreeWidgetItem*>::iterator it = tree.begin();
    for (; it != tree.end(); ++it)
        (*it)->sortChildren(column, order, true);

    emit layoutChanged();
}

/*!
  \internal

  Returns true if the value of the \a left item is
  less than the value of the \a right item.

  Used by the sorting functions.
*/

bool QTreeModel::itemLessThan(const QPair<QTreeWidgetItem*,int> &left,
                              const QPair<QTreeWidgetItem*,int> &right)
{
    return *(left.first) < *(right.first);
}

/*!
  \internal

  Returns true if the value of the \a left item is
  greater than the value of the \a right item.

  Used by the sorting functions.
*/

bool QTreeModel::itemGreaterThan(const QPair<QTreeWidgetItem*,int> &left,
                                 const QPair<QTreeWidgetItem*,int> &right)
{
    return !(*(left .first) < *(right.first));
}

/*!
  \internal

  Inserts the tree view \a item to the tree model as a  toplevel item.
*/

void QTreeModel::insertInTopLevel(int row, QTreeWidgetItem *item)
{
    beginInsertRows(QModelIndex(), row, row);
    tree.insert(row, item);
    endInsertRows();
}

/*!
  \internal

  Inserts the list of tree view \a items to the tree model as a  toplevel item.
*/

void QTreeModel::insertListInTopLevel(int row, const QList<QTreeWidgetItem*> &items)
{
    beginInsertRows(QModelIndex(), row, row + items.count() - 1);
    for (int n = 0; n < items.count(); ++n)
        tree.insert(row, items.at(n));
    endInsertRows();
}

QStringList QTreeModel::mimeTypes() const
{
    const QTreeWidget *view = ::qobject_cast<const QTreeWidget*>(QObject::parent());
    return view->mimeTypes();
}

QMimeData *QTreeModel::internalMimeData()  const
{
    return QAbstractItemModel::mimeData(cachedIndexes);
}

QMimeData *QTreeModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        if (indexes.at(i).column() == 0) // only one item per row
            items << item(indexes.at(i));
    const QTreeWidget *view = ::qobject_cast<const QTreeWidget*>(QObject::parent());

    // cachedIndexes is a little hack to avoid copying from QModelIndexList to QList<QTreeWidgetItem*> and back again in the view
    cachedIndexes = indexes;
    QMimeData *mimeData = view->mimeData(items);
    cachedIndexes.clear();
    return mimeData;
}

bool QTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                              int row, int column, const QModelIndex &parent)
{
    QTreeWidget *view = ::qobject_cast<QTreeWidget*>(QObject::parent());
    if (row == -1 && column == -1)
        row = rowCount(parent); // append
    return view->dropMimeData(item(parent), row, data, action);
}

Qt::DropActions QTreeModel::supportedDropActions() const
{
    const QTreeWidget *view = ::qobject_cast<const QTreeWidget*>(QObject::parent());
    return view->supportedDropActions();
}

void QTreeModel::itemChanged(QTreeWidgetItem *item)
{
    QModelIndex left = index(item, 0);
    QModelIndex right = index(item, item->columnCount() - 1);
    emit dataChanged(left, right);
}

/*!
  \internal

  Emits the dataChanged() signal for the given \a item.
*/

void QTreeModel::emitDataChanged(QTreeWidgetItem *item, int column)
{
    if (signalsBlocked())
        return;

    if (header == item) {
        emit headerDataChanged(Qt::Horizontal, column, column);
        return;
    }

    QModelIndex br, tl;
    if (column == -1) { // the whole row
        tl = index(item, 0);
        br = createIndex(tl.row(), columnCount() - 1, item);
    } else { // single cell
        tl = index(item, column);
        br = tl;
    }
    emit dataChanged(tl, br);
}

void QTreeModel::beginInsertItems(QTreeWidgetItem *parent, int row, int count)
{
    beginInsertRows(index(parent, 0), row, row + count - 1);
}

void QTreeModel::endInsertItems()
{
    endInsertRows();
}

//### walks in a depth-first order (children before siblings)
QTreeWidgetItem* QTreeModel::walk(const QTreeWidgetItem *current)
{
    Q_ASSERT(current);
    QTreeWidgetItem *item = 0;
    if (current->children.isEmpty()) {
        QTreeWidgetItem *parent = const_cast<QTreeWidgetItem *>(current);
        while (!(item = QTreeModel::nextSibling(parent)))
            if (!(parent = parent->parent()))
                break;
    } else {
        item = current->children.first();
    }
    return item;
}

// recursive
void QTreeModel::ensureValidIterator(QTreeWidgetItemIterator *iterator, const QTreeWidgetItem *itemToBeRemoved)
{
    Q_ASSERT(itemToBeRemoved);
    Q_ASSERT(iterator);
    if (!itemToBeRemoved->children.isEmpty()) {
        // We have to walk *all* descendants in order to check if the iterator points
        // to an item that is about to be deleted.
        QTreeWidgetItem *walkItem = walk(itemToBeRemoved);
        QTreeWidgetItem *nextSibling = QTreeModel::nextSibling(itemToBeRemoved);
        while(walkItem != nextSibling) {
            if (*(*iterator) == walkItem) {
                ensureValidIterator(iterator, walkItem);    // recurse
            }
            walkItem = walk(walkItem);
        }
        (*iterator).current = nextSibling;
        // This does not have to be valid, iterate to the next if it doesn't match our flags
        if (!(*iterator).matchesFlags(nextSibling)) ++(*iterator);
    } else {
        if (itemToBeRemoved == *(*iterator)) ++(*iterator);
    }
}

void QTreeModel::beginRemoveItems(QTreeWidgetItem *parent, int row, int count)
{

    beginRemoveRows(index(parent, 0), row, row + count - 1);
    // now update the iterators
    for (int i = 0; i < iterators.count(); ++i) {
        for (int j = 0; j < count; j++) {
            QTreeWidgetItem *c;
            if (parent) {
                c = parent->child(row + j);
            } else {
                c = tree.at(row + j);
            }
            ensureValidIterator(iterators[i], c);
        }
    }
}

void QTreeModel::endRemoveItems()
{
    endRemoveRows();
}

QTreeWidgetItem* QTreeModel::nextSibling(const QTreeWidgetItem* item)
{
    Q_ASSERT(item);
    Q_ASSERT(item->view);
    QList<QTreeWidgetItem*> siblings;
    if (item->parent()) {
        siblings = item->parent()->children;
    } else {
        QTreeModel *model = ::qobject_cast<QTreeModel*>(item->view->model());
        siblings = model->tree;
    }
    int i = siblings.indexOf(const_cast<QTreeWidgetItem*>(item)) + 1;
    if (i > 0 && i < siblings.count())
        return siblings.at(i);
    return 0;
}

QTreeWidgetItem* QTreeModel::previousSibling(const QTreeWidgetItem* item)
{
    Q_ASSERT(item);
    Q_ASSERT(item->view);
    QList<QTreeWidgetItem*> siblings;
    if (item->parent()) {
        siblings = item->parent()->children;
    } else {
        QTreeModel *model = ::qobject_cast<QTreeModel*>(item->view->model());
        siblings = model->tree;
    }
    int i = siblings.indexOf(const_cast<QTreeWidgetItem*>(item)) - 1;
    return siblings.value(i);
}

void QTreeModel::sortItems(QList<QTreeWidgetItem*> *items, int /*column*/, Qt::SortOrder order)
{
    // store the original order of indexes
    QVector< QPair<QTreeWidgetItem*,int> > sorting(items->count());
    for (int i = 0; i < sorting.count(); ++i) {
        sorting[i].first = items->at(i);
        sorting[i].second = i;
    }

    // do the sorting
    LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
    qSort(sorting.begin(), sorting.end(), compare);

    // update the persistent indexes for the top level
    for (int r = 0; r < sorting.count(); ++r) {
        for (int c = 0; c < sorting.at(r).first->columnCount(); ++c) {
            QTreeWidgetItem *item = sorting.at(r).first;
            items->replace(r, item);
            QModelIndex from = createIndex(sorting.at(r).second, c, item);
            QModelIndex to = createIndex(r, c, item);
            changePersistentIndex(from, to);
        }
    }
}

/*!
  \class QTreeWidgetItem

  \brief The QTreeWidgetItem class provides an item for use with the
  QTreeWidget convenience class.

  \ingroup model-view

  Tree widget items are used to hold rows of information for tree widgets.
  Rows usually contain several columns of data, each of which can contain
  a text label and an icon.

  The QTreeWidgetItem class is a convenience class that replaces the
  QListViewItem class in Qt 3. It provides an item for use with
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

  By default, items are enabled, selectable, checkable, and can be the source
  of a drag and drop operation.
  Each item's flags can be changed by calling setFlags() with the appropriate
  value (see \l{Qt::ItemFlags}). Checkable items can be checked and unchecked
  with the setChecked() function. The corresponding checked() function
  indicates whether the item is currently checked.

  \sa QTreeWidget, {Model/View Programming}, QListWidgetItem, QTableWidgetItem
*/

/*!
    \variable QTreeWidgetItem::Type

    The default type for tree widget items.

    \sa UserType, type()
*/

/*!
    \variable QTreeWidgetItem::UserType

    The minimum value for custom types. Values below UserType are
    reserved by Qt.

    \sa Type, type()
*/

/*!
    \fn int QTreeWidgetItem::type() const

    Returns the type passed to the QTreeWidgetItem constructor.
*/

/*!
    \fn QTreeWidget *QTreeWidgetItem::treeWidget() const

    Returns the tree widget that contains the item.
*/

/*!
    \fn Qt::ItemFlags QTreeWidgetItem::flags() const

    Returns the flags used to describe the item. These determine whether
    the item can be checked, edited, and selected.

    The default value for flags is
    Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled.
    If the item was constructed with a parent, flags will in addition contain Qt::ItemIsDropEnabled.

    \sa setFlags()
*/

/*!
    \fn void QTreeWidgetItem::setFlags(Qt::ItemFlags flags)

    Sets the flags for the item to the given \a flags. These determine whether
    the item can be selected or modified.

    \sa flags()
*/
void QTreeWidgetItem::setFlags(Qt::ItemFlags aflags) {
    itemFlags = aflags;
    if (QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0))
        model->itemChanged(this);
}

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

    \sa setIcon(), {QAbstractItemView::iconSize}{iconSize}
*/

/*!
    \fn void QTreeWidgetItem::setIcon(int column, const QIcon &icon)

    Sets the icon to be displayed in the given \a column to \a icon.

    \sa icon(), setText(), {QAbstractItemView::iconSize}{iconSize}
*/

/*!
    \fn QString QTreeWidgetItem::statusTip(int column) const

    Returns the status tip for the contents of the given \a column.

    \sa setStatusTip()
*/

/*!
    \fn void QTreeWidgetItem::setStatusTip(int column, const QString &statusTip)

    Sets the status tip for the given \a column to the given \a statusTip.
    QTreeWidget mouse tracking needs to be enabled for this feature to work.

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
    \fn Qt::CheckState QTreeWidgetItem::checkState(int column) const

    Returns the check state of the label in the given \a column.

    \sa Qt::CheckState
*/

/*!
    \fn void QTreeWidgetItem::setCheckState(int column, Qt::CheckState state)

    Sets the item in the given \a column check state to be \a state.

    \sa checkState()
*/

/*!
  \fn QSize QTreeWidgetItem::sizeHint(int column) const
  \since 4.1

  Returns the size hint set for the tree item in the given
  \a column (see \l{QSize}).
*/

/*!
  \fn void QTreeWidgetItem::setSizeHint(int column, const QSize &size)
  \since 4.1

  Sets the size hint for the tree item in the given \a column to be \a size.
  If no size hint is set, the item delegate will compute the size hint based
  on the item data.
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
    Constructs a tree widget item of the specified \a type. The item
    must be inserted into a tree widget.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(int type)
    : rtti(type), view(0), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
}


/*!
    Constructs a tree widget item of the specified \a type. The item
    must be inserted into a tree widget.
    The given list of \a strings will be set as the item text for each
    column in the item.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(const QStringList &strings, int type)
    : rtti(type), view(0), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    for (int i = 0; i < strings.count(); ++i)
        setText(i, strings.at(i));
}

/*!
    \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *parent, int type)

    Constructs a tree widget item of the specified \a type and appends it
    to the items in the given \a parent.

    \sa type()
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, int type)
    : rtti(type), view(view), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    if (view && view->model()) {
        QTreeModel *model = ::qobject_cast<QTreeModel*>(view->model());
        model->insertInTopLevel(model->rowCount(QModelIndex()), this);
        values.reserve(model->columnCount());
    }
}

/*!
  \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *parent, const QStringList &strings, int type)

  Constructs a tree widget item of the specified \a type and appends it
  to the items in the given \a parent. The given list of \a strings will be set as
  the item text for each column in the item.

  \sa type()
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, const QStringList &strings, int type)
    : rtti(type), view(view), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    for (int i = 0; i < strings.count(); ++i)
        setText(i, strings.at(i));
    if (view && view->model()) {
        QTreeModel *model = ::qobject_cast<QTreeModel*>(view->model());
        model->insertInTopLevel(model->rowCount(QModelIndex()), this);
        values.reserve(model->columnCount());
    }
}

/*!
    \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *parent, QTreeWidgetItem *preceding, int type)

    Constructs a tree widget item of the specified \a type and inserts it into
    the given \a parent after the \a preceding item.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, QTreeWidgetItem *after, int type)
    : rtti(type), view(view), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    if (view) {
        QTreeModel *model = ::qobject_cast<QTreeModel*>(view->model());
        if (model) {
            int i = model->tree.indexOf(after) + 1;
            model->insertInTopLevel(i, this);
            values.reserve(model->columnCount());
        }
    }
}

/*!
    Constructs a tree widget item and append it to the given \a parent.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, int type)
    : rtti(type), view(0), par(parent),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    if (parent)
        parent->addChild(this);
}

/*!
    Constructs a tree widget item and append it to the given \a parent.
    The given list of \a strings will be set as the item text for each column in the item.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, const QStringList &strings, int type)
    : rtti(type), view(0), par(parent),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    for (int i = 0; i < strings.count(); ++i)
        setText(i, strings.at(i));
    if (parent)
        parent->addChild(this);

}

/*!
    \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *preceding, int type)

    Constructs a tree widget item of the specified \a type that is inserted
    into the \a parent after the \a preceding child item.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, int type)
    : rtti(type), view(0), par(parent),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
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
    QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0);
    if (par) {
        int i = par->children.indexOf(this);
        if (model) model->beginRemoveItems(par, i, 1);
        par->children.takeAt(i);
        if (model) model->endRemoveItems();
    } else if (model) {
        if (this == model->header) {
            model->header = 0;
        } else {
            int i = model->tree.indexOf(this);
            model->beginRemoveItems(0, i, 1);
            model->tree.takeAt(i);
            model->endRemoveItems();
        }
    }
    // at this point the persistent indexes for the children should also be invalidated since we invalidated the parent
    for (int i = 0; i < children.count(); ++i) {
        QTreeWidgetItem *child = children.at(i);
        child->par = 0; // make sure the child doesn't try to remove itself from children list
        child->view = 0; // make sure the child doesn't try to remove itself from the top level list
        delete child;
    }

    children.clear();
}

/*!
    Creates a deep copy of the item and of its children.
*/
QTreeWidgetItem *QTreeWidgetItem::clone() const
{
    QTreeWidgetItem *copy = 0;

    QStack<const QTreeWidgetItem*> stack;
    QStack<QTreeWidgetItem*> parentStack;
    stack.push(this);
    parentStack.push(0);

    QTreeWidgetItem *root = 0;
    const QTreeWidgetItem *item = 0;
    QTreeWidgetItem *parent = 0;
    while (!stack.isEmpty()) {
        // get current item, and copied parent
        item = stack.pop();
        parent = parentStack.pop();

        // copy item
        copy = new QTreeWidgetItem(*item);
        if (!root)
            root = copy;

        // set parent and add to parents children list
        if (parent) {
            copy->par = parent;
            parent->children.append(copy);
        }

        for (int i=0; i<item->childCount(); ++i) {
            stack.push(item->child(i));
            parentStack.push(copy);
        }
    }
    return root;
}

/*!
    Sets the value for the item's \a column and \a role to the given
    \a value.
*/
void QTreeWidgetItem::setData(int column, int role, const QVariant &value)
{
    // special case for check state in tristate
    if ((role == Qt::CheckStateRole) && (itemFlags & Qt::ItemIsTristate)) {
        for (int i = 0; i < children.count(); ++i) {
            QTreeWidgetItem *child = children.at(i);
            if (child->data(column, role).isValid()) {// has a CheckState
                child->par = 0; // a little hack to avoid multiple dataChanged signals
                child->setData(column, role, value);
                child->par = this;
            }
        }
    }
    // set the item data
    if (role == Qt::EditRole)
        role = Qt::DisplayRole;
    if (role == Qt::DisplayRole) {
        if (display.count() <= column) {
            for (int i = display.count() - 1; i < column - 1; ++i)
                display.append(QString());
            display.append(value.toString());   
        } else {
            display[column] = value.toString();
        }
    } else if (column < values.count()) {
        bool found = false;
        QVector<QWidgetItemData> column_values = values.at(column);
        for (int i = 0; i < column_values.count(); ++i) {
            if (column_values.at(i).role == role) {
                values[column][i].value = value;
                found = true;
                break;
            }
        }
        if (!found)
            values[column].append(QWidgetItemData(role, value));
    } else {
        values.resize(column + 1);
        values[column].append(QWidgetItemData(role, value));
    }

    if (QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0)) {
        model->emitDataChanged(this, column);
        if (role == Qt::CheckStateRole && par && (par->itemFlags & Qt::ItemIsTristate))
            model->emitDataChanged(par, column);
    }
}

/*!
    Returns the value for the item's \a column and \a role.
*/
QVariant QTreeWidgetItem::data(int column, int role) const
{
    // special case for check state in tristate
    if (role == Qt::CheckStateRole
        && (itemFlags & Qt::ItemIsTristate) && children.count()) {
        return childrenCheckState(column);
    }
    // return the item data
    if (role == Qt::EditRole)
        role = Qt::DisplayRole;

    if (role == Qt::DisplayRole) {
        if (column >= 0 && column < display.count())
            return display.at(column);
    } else if (column >= 0 && column < values.size()) {
        const QVector<QWidgetItemData> &column_values = values.at(column);
        for (int i = 0; i < column_values.count(); ++i)
            if (column_values.at(i).role == role)
                return column_values.at(i).value;
    }
    return QVariant();
}

/*!
  Returns true if the text in the item is less than the text in the
  \a other item, otherwise returns false.
*/

bool QTreeWidgetItem::operator<(const QTreeWidgetItem &other) const
{
    int column = view->sortColumn();
    return text(column) < other.text(column);
}

#ifndef QT_NO_DATASTREAM

/*!
    Reads the item from stream \a in. This only reads data into a single item.

    \sa write()
*/
void QTreeWidgetItem::read(QDataStream &in)
{
    in >> values;
}

/*!
    Writes the item to stream \a out. This only writes data from one single item.

    \sa read()
*/
void QTreeWidgetItem::write(QDataStream &out) const
{
    out << values;
}

/*!
    \since 4.1

    Constructs a copy of \a other. Note that type() and treeWidget()
    are not copied.

    This function is useful when reimplementing clone().

    \sa data(), flags()
*/
QTreeWidgetItem::QTreeWidgetItem(const QTreeWidgetItem &other)
    : rtti(Type), values(other.values), view(0), par(0),
      itemFlags(other.itemFlags)
{
}

/*!
    Assigns \a other's data and flags to this item. Note that type()
    and treeWidget() are not copied.

    This function is useful when reimplementing clone().

    \sa data(), flags()
*/
QTreeWidgetItem &QTreeWidgetItem::operator=(const QTreeWidgetItem &other)
{
    values = other.values;
    itemFlags = other.itemFlags;
    return *this;
}

#endif // QT_NO_DATASTREAM

/*!
  Appends the \a child item to the list of children.

  \sa insertChild() takeChild()
*/
void QTreeWidgetItem::addChild(QTreeWidgetItem *child)
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
    if (index < 0 || index > children.count() || child == 0)
        return;
    Q_ASSERT(!child->view || !child->par);
    if (QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0)) {
        model->beginInsertItems(this, index, 1);
        int cols = model->columnCount();
        QStack<QTreeWidgetItem*> stack;
        stack.push(child);
        child->par = this;
        while (!stack.isEmpty()) {
            QTreeWidgetItem *i = stack.pop();
            i->view = view;
            i->values.reserve(cols);
            for (int c = 0; c < i->children.count(); ++c)
                stack.push(i->children.at(c));
        }
        children.insert(index, child);
        model->endInsertItems();
    } else {
        children.insert(index, child);
    }
}

/*!
  Removes the item at \a index and returns it, otherwise return 0.
*/
QTreeWidgetItem *QTreeWidgetItem::takeChild(int index)
{
    if (index >= 0 && index < children.count()) {
        QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0);
        if (model) model->beginRemoveItems(this, index, 1);
        QTreeWidgetItem *item = children.takeAt(index);
        item->par = 0;
        QStack<QTreeWidgetItem*> stack;
        stack.push(item);
        while (!stack.isEmpty()) {
            QTreeWidgetItem *i = stack.pop();
            i->view = 0;
            for (int c = 0; c < i->children.count(); ++c)
                stack.push(i->children.at(c));
        }
        if (model) model->endRemoveRows();
        return item;
    }
    return 0;
}

/*!
  \since 4.1

  Appends the given list of \a children to the item.

  \sa insertChildren() takeChildren()
*/
void QTreeWidgetItem::addChildren(const QList<QTreeWidgetItem*> &children)
{
    insertChildren(this->children.count(), children);
}

/*!
  \since 4.1

  Inserts the given list of \a children into the list of the item children at \a index .
*/
void QTreeWidgetItem::insertChildren(int index, const QList<QTreeWidgetItem*> &children)
{
    // FIXME: here we have a problem;
    // the user could build up a tree and then insert the root in the view
    QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0);
    if (model) model->beginInsertItems(this, index, children.count());
    for (int n = 0; n < children.count(); ++n) {
        QTreeWidgetItem *child = children.at(n);
        Q_ASSERT(!child->view || !child->par);
        if (view && model) {
            QStack<QTreeWidgetItem*> stack;
            stack.push(child);
            children.at(n)->par = this;
            while (!stack.isEmpty()) {
                QTreeWidgetItem *i = stack.pop();
                i->view = view;
                for (int c = 0; c < i->children.count(); ++c)
                    stack.push(i->children.at(c));
            }
        }
        this->children.insert(index + n, child);
    }
    if (model) model->endInsertItems();
}

/*!
  \since 4.1

  Removes the list of children and returns it, otherwise return an empty list.
*/
QList<QTreeWidgetItem*> QTreeWidgetItem::takeChildren()
{
    QList<QTreeWidgetItem*> removed;
    if (children.count() > 0) {
        QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0);
        if (model) model->beginRemoveItems(this, 0, children.count());
        for (int n = 0; n < children.count(); ++n) {
            QTreeWidgetItem *item = children.at(n);
            item->par = 0;
            QStack<QTreeWidgetItem*> stack;
            stack.push(item);
            while (!stack.isEmpty()) {
                QTreeWidgetItem *i = stack.pop();
                i->view = 0;
                for (int c = 0; c < i->children.count(); ++c)
                    stack.push(i->children.at(c));
            }
        }
        removed = children;
        children.clear(); // detach
        if (model) model->endRemoveItems();
    }
    return removed;
}

/*!
  \internal

  Sorts the children by the value in the given \a column, in the \a order
  specified. If \a climb is true, the items below each of the children will
  also be sorted.
*/
void QTreeWidgetItem::sortChildren(int column, Qt::SortOrder order, bool climb)
{
    QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0);
    if (!model)
        return;
    model->sortItems(&children, column, order);
    if (climb) {
        QList<QTreeWidgetItem*>::iterator it = children.begin();
        for (; it != children.end(); ++it)
            (*it)->sortChildren(column, order, climb);
    }
}

/*!
  \internal
*/
QVariant QTreeWidgetItem::childrenCheckState(int column) const
{
    int checkedChildrenCount = 0;
    int uncheckedChildrenCount = 0;
    int validChildrenCount = 0;
    for (int i = 0; i < children.count(); ++i) {
        QVariant value = children.at(i)->data(column, Qt::CheckStateRole);
        if (!value.isValid())
            continue;
        Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());
        if (checkState == Qt::Unchecked)
            ++uncheckedChildrenCount;
        else
            ++checkedChildrenCount; // includes partially checked items
        ++validChildrenCount;
    }
    if (checkedChildrenCount + uncheckedChildrenCount == 0)
        return QVariant(); // value was not defined
    if (checkedChildrenCount == validChildrenCount)
        return Qt::Checked;
    if (uncheckedChildrenCount == validChildrenCount)
        return Qt::Unchecked;
    return Qt::PartiallyChecked;
}

#ifndef QT_NO_DATASTREAM
/*!
    \relates QTreeWidgetItem

    Writes the tree widget item \a item to stream \a out.

    This operator uses QTreeWidgetItem::write().

    \sa {Format of the QDataStream Operators}
*/
QDataStream &operator<<(QDataStream &out, const QTreeWidgetItem &item)
{
    item.write(out);
    return out;
}

/*!
    \relates QTreeWidgetItem

    Reads a tree widget item from stream \a in into \a item.

    This operator uses QTreeWidgetItem::read().

    \sa {Format of the QDataStream Operators}
*/
QDataStream &operator>>(QDataStream &in, QTreeWidgetItem &item)
{
    item.read(in);
    return in;
}
#endif // QT_NO_DATASTREAM

class QTreeWidgetPrivate : public QTreeViewPrivate
{
    Q_DECLARE_PUBLIC(QTreeWidget)
public:
    QTreeWidgetPrivate() : QTreeViewPrivate(), sortingEnabled(false) {}
    inline QTreeModel *model() const { return ::qobject_cast<QTreeModel*>(q_func()->model()); }
    void emitItemPressed(const QModelIndex &index);
    void emitItemClicked(const QModelIndex &index);
    void emitItemDoubleClicked(const QModelIndex &index);
    void emitItemActivated(const QModelIndex &index);
    void emitItemEntered(const QModelIndex &index);
    void emitItemChanged(const QModelIndex &index);
    void emitItemExpanded(const QModelIndex &index);
    void emitItemCollapsed(const QModelIndex &index);
    void emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &index);
    bool sortingEnabled;
};

void QTreeWidgetPrivate::emitItemPressed(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemPressed(model()->item(index), index.column());
}

void QTreeWidgetPrivate::emitItemClicked(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemClicked(model()->item(index), index.column());
}

void QTreeWidgetPrivate::emitItemDoubleClicked(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemDoubleClicked(model()->item(index), index.column());
}

void QTreeWidgetPrivate::emitItemActivated(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemActivated(model()->item(index), index.column());
}

void QTreeWidgetPrivate::emitItemEntered(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemEntered(model()->item(index), index.column());
}

void QTreeWidgetPrivate::emitItemChanged(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemChanged(model()->item(index), index.column());
}

void QTreeWidgetPrivate::emitItemExpanded(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemExpanded(model()->item(index));
}

void QTreeWidgetPrivate::emitItemCollapsed(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemCollapsed(model()->item(index));
}

void QTreeWidgetPrivate::emitCurrentItemChanged(const QModelIndex &current,
                                                const QModelIndex &previous)
{
    Q_Q(QTreeWidget);
    QTreeWidgetItem *currentItem = model()->item(current);
    QTreeWidgetItem *previousItem = model()->item(previous);
    emit q->currentItemChanged(currentItem, previousItem);
}

/*!
  \class QTreeWidget qtreewidget.h

  \brief The QTreeWidget class provides a tree view that uses a predefined
  tree model.

  \ingroup model-view
  \mainclass

  The QTreeWidget class is a convenience class that provides a standard
  tree widget with a classic item-based interface similar to that used by
  the QListView class in Qt 3.
  This class is based on Qt's Model/View architecture and uses a default
  model to hold items, each of which is a QTreeWidgetItem.

  Developers who do not need the flexibility of the Model/View framework
  can use this class to create simple hierarchical lists very easily. A more
  flexible approach involves combining a QTreeView with a standard item model.
  This allows the storage of data to be separated from its representation.

  In its simplest form, a tree widget can be constructed in the following way:

  \code
    QTreeWidget *treeWidget = new QTreeWidget();
    treeWidget->setColumnCount(1);
    for (int i = 0; i < 10; ++i)
        new QTreeWidgetItem(treeWidget, QStringList(QString("item: %1").arg(i)));
  \endcode

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

  \sa QTreeWidgetItem, QTreeView, {Model/View Programming}
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
    \fn void QTreeWidget::itemActivated(QTreeWidgetItem *item, int column)

    This signal is emitted when the user activates an item by single-
    or double-clicking (depending on the platform, i.e. on the
    QStyle::SH_ItemView_ActivateItemOnSingleClick style hint) or
    pressing a special key (e.g., \key Enter).

    The specified \a item is the item that was clicked, or 0 if no
    item was clicked. The \a column is the item's column that was
    clicked, or -1 if no item was clicked.
*/

/*!
    \fn void QTreeWidget::itemPressed(QTreeWidgetItem *item, int column)

    This signal is emitted when the user presses a mouse button inside
    the widget. The specified \a item is the item that was clicked.
*/

/*!
    \fn void QTreeWidget::itemClicked(QTreeWidgetItem *item, int column)

    This signal is emitted when the user clicks inside the widget.
    The specified \a item is the item that was clicked.
*/

/*!
    \fn void QTreeWidget::itemDoubleClicked(QTreeWidgetItem *item, int column)

    This signal is emitted when the user double clicks inside the
    widget.  The specified \a item is the item that was clicked.
*/

/*!
    \fn void QTreeWidget::itemExpanded(QTreeWidgetItem *item)

    This signal is emitted when the specified \a item is expanded so that
    all of its children are displayed.

    \sa isItemExpanded()
*/

/*!
    \fn void QTreeWidget::itemCollapsed(QTreeWidgetItem *item)

    This signal is emitted when the specified \a item is collapsed so that
    none of its children are displayed.

    \sa isItemExpanded()
*/

/*!
    \fn void QTreeWidget::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)

    This signal is emitted when the current item changes. The current
    item is specified by \a current, and this replaces the \a previous
    current item.
*/

/*!
    \fn void QTreeWidget::itemSelectionChanged()

    This signal is emitted when the selection changes in the tree widget.
    The current selection can be found with selectedItems().
*/

/*!
    \fn void QTreeWidget::itemEntered(QTreeWidgetItem *item, int column)

    This signal is emitted when the mouse cursor enters an \a item over the
    specified \a column.
    QTreeWidget mouse tracking needs to be enabled for this feature to work.
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
    QTreeView::setModel(new QTreeModel(0, this));
    // view signals
    connect(this, SIGNAL(pressed(QModelIndex)), SLOT(emitItemPressed(QModelIndex)));
    connect(this, SIGNAL(clicked(QModelIndex)), SLOT(emitItemClicked(QModelIndex)));
    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(emitItemDoubleClicked(QModelIndex)));
    connect(this, SIGNAL(activated(QModelIndex)), SLOT(emitItemActivated(QModelIndex)));
    connect(this, SIGNAL(entered(QModelIndex)), SLOT(emitItemEntered(QModelIndex)));
    connect(this, SIGNAL(expanded(QModelIndex)), SLOT(emitItemExpanded(QModelIndex)));
    connect(this, SIGNAL(collapsed(QModelIndex)), SLOT(emitItemCollapsed(QModelIndex)));
    // selection signals
    connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(emitCurrentItemChanged(QModelIndex,QModelIndex)));
    connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SIGNAL(itemSelectionChanged()));
    // model signals
    connect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(emitItemChanged(QModelIndex)));

    header()->setClickable(false);
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
    Q_D(const QTreeWidget);
    return d->model()->columnCount();
}

/*
  Sets the number of header \a columns in the tree widget.
*/

void QTreeWidget::setColumnCount(int columns)
{
    Q_D(QTreeWidget);
    if (columns < 0)
        return;
    d->model()->setColumnCount(columns);
}

/*!
  Returns the top level item at the given \a index, or 0 if the item does
  not exist.
*/

QTreeWidgetItem *QTreeWidget::topLevelItem(int index) const
{
    Q_D(const QTreeWidget);
    return d->model()->tree.value(index);
}

/*!\property QTreeWidget::topLevelItemCount
    \brief the number of top-level items
*/

int QTreeWidget::topLevelItemCount() const
{
    Q_D(const QTreeWidget);
    return d->model()->tree.count();
}

/*!
  Inserts the \a item at \a index in the top level in the view.

  \sa addTopLevelItem()
*/

void QTreeWidget::insertTopLevelItem(int index, QTreeWidgetItem *item)
{
    Q_D(QTreeWidget);
    if (index < 0 || index > d->model()->tree.count() || item == 0)
        return;
    Q_ASSERT(!item->view || !item->par);
    QStack<QTreeWidgetItem*> stack;
    stack.push(item);
    while (!stack.isEmpty()) {
        QTreeWidgetItem *i = stack.pop();
        i->view = this;
        for (int c = 0; c < i->children.count(); ++c)
            stack.push(i->children.at(c));
    }
    d->model()->insertInTopLevel(index, item);
}

/*!
    \since 4.1

    Appends the \a item as a top-level item in the widget.

    \sa insertTopLevelItem()
*/
void QTreeWidget::addTopLevelItem(QTreeWidgetItem *item)
{
    insertTopLevelItem(topLevelItemCount(), item);
}

/*!
  Removes the top-level item at the given \a index in the tree and
  returns it, otherwise returns 0;
*/

QTreeWidgetItem *QTreeWidget::takeTopLevelItem(int index)
{
    Q_D(QTreeWidget);
    if (index >= 0 && index < d->model()->tree.count()) {
        d->model()->beginRemoveRows(QModelIndex(), index, index);
        QTreeWidgetItem *item = d->model()->tree.takeAt(index);
        QStack<QTreeWidgetItem*> stack;
        stack.push(item);
        while (!stack.isEmpty()) {
            QTreeWidgetItem *i = stack.pop();
            i->view = 0;
            for (int c = 0; c < i->children.count(); ++c)
                stack.push(i->children.at(c));
        }
        d->model()->endRemoveRows();
        return item;
    }
    return 0;
}

/*!
  Returns the index of the given top-level \a item, or -1 if the item
  cannot be found.
 */
int QTreeWidget::indexOfTopLevelItem(QTreeWidgetItem *item)
{
    Q_D(QTreeWidget);
    return d->model()->tree.indexOf(item);
}

/*!
  \since 4.1

  Inserts the list of \a items at \a index in the top level in the view.

  \sa addTopLevelItems()
*/
void QTreeWidget::insertTopLevelItems(int index, const QList<QTreeWidgetItem*> &items)
{
    Q_D(QTreeWidget);
    for (int n = 0; n < items.count(); ++n) {
        QTreeWidgetItem *item = items.at(n);
        Q_ASSERT(!item->view || !item->par);
        QStack<QTreeWidgetItem*> stack;
        stack.push(item);
        while (!stack.isEmpty()) {
            QTreeWidgetItem *i = stack.pop();
            i->view = this;
            for (int c = 0; c < i->children.count(); ++c)
                stack.push(i->children.at(c));
        }
    }
    d->model()->insertListInTopLevel(index, items);
}

/*!
  Appends the list of \a items as a top-level items in the widget.

  \sa insertTopLevelItems()
*/
void QTreeWidget::addTopLevelItems(const QList<QTreeWidgetItem*> &items)
{
    insertTopLevelItems(topLevelItemCount(), items);
}

/*!
    Returns the item used for the tree widget's header.

    \sa setHeaderItem()
*/

QTreeWidgetItem *QTreeWidget::headerItem() const
{
    Q_D(const QTreeWidget);
    return d->model()->header;
}

/*!
    Sets the header \a item for the tree widget. The label for each column in
    the header is supplied by the corresponding label in the item.

    \sa headerItem()
*/

void QTreeWidget::setHeaderItem(QTreeWidgetItem *item)
{
    Q_D(QTreeWidget);

    // FIXME When we by default autogenerate the numbers when NULL then accept this
    Q_ASSERT(item);

    delete d->model()->header;
    item->view = this;
    d->model()->header = item;
}

/*!
  Adds a column in the header for each item in the \a labels list, and sets
  the label for each column.

  Note that setHeaderLabels wont remove existing columns.
*/
void QTreeWidget::setHeaderLabels(const QStringList &labels)
{
    Q_D(QTreeWidget);
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
    Q_D(const QTreeWidget);
    return d->model()->item(currentIndex());
}

/*!
  \since 4.1
    Returns the current column in the tree widget.

    \sa setCurrentItem()
*/
int QTreeWidget::currentColumn() const
{
    return currentIndex().column();
}

/*!
  Sets the current \a item in the tree widget.

  Depending on the current selection mode, the item may also be selected.

  \sa currentItem()
*/
void QTreeWidget::setCurrentItem(QTreeWidgetItem *item)
{
    setCurrentItem(item, 0);
}

/*!
  \since 4.1
  Sets the current \a item in the tree widget and the curernt column to \a column.

  \sa currentItem()
*/
void QTreeWidget::setCurrentItem(QTreeWidgetItem *item, int column)
{
    Q_D(const QTreeWidget);
    if (item)
        setCurrentIndex(d->model()->index(item, column));
    else
        setCurrentIndex(QModelIndex());
}

/*!
  Returns a pointer to the item at the coordinates \a p.
*/
QTreeWidgetItem *QTreeWidget::itemAt(const QPoint &p) const
{
    Q_D(const QTreeWidget);
    return d->model()->item(indexAt(p));
}

/*!
    \fn QTreeWidgetItem *QTreeWidget::itemAt(int x, int y) const
    \overload

    Returns a pointer to the item at the coordinates (\a x, \a y).
*/

/*!
  Returns the rectangle on the viewport occupied by the item at \a item.
*/
QRect QTreeWidget::visualItemRect(const QTreeWidgetItem *item) const
{
    Q_ASSERT(item);
    Q_D(const QTreeWidget);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    Q_ASSERT(index.isValid());
    return visualRect(index);
}

/*!
  \since 4.1

  Returns the column used to sort the contents of the widget.
*/
int QTreeWidget::sortColumn() const
{
    return header()->sortIndicatorSection();
}

/*!
  Sorts the items in the widget in the specified \a order by the values in
  the given \a column.
*/

void QTreeWidget::sortItems(int column, Qt::SortOrder order)
{
    Q_D(QTreeWidget);
    header()->setSortIndicator(column, order);
    d->model()->sort(column, order);
}

/*!
  If \a enable is true, the items in the widget will be sorted if the user
  clicks on a header section; otherwise sorting by clicking on the
  header section is disabled.
*/
void QTreeWidget::setSortingEnabled(bool enable)
{
    Q_D(QTreeWidget);
    d->sortingEnabled = enable;
    if (header()->isSortIndicatorShown() != enable) {
        header()->setSortIndicatorShown(enable);
        header()->setClickable(enable);
    }
}

/*!
  Returns true if sorting is enabled; otherwise returns false.
  Sorting is enabled when the user clicks on a header section.
*/

bool QTreeWidget::isSortingEnabled() const
{
    Q_D(const QTreeWidget);
    return d->sortingEnabled;
}

/*!
  Starts editing the \a item in the given \a column if it is editable.
*/

void QTreeWidget::editItem(QTreeWidgetItem *item, int column)
{
    Q_ASSERT(item);
    Q_D(QTreeWidget);
    edit(d->model()->index(item, column));
}

/*!
  Opens a persistent editor for the \a item in the given \a column.
*/

void QTreeWidget::openPersistentEditor(QTreeWidgetItem *item, int column)
{
    Q_ASSERT(item);
    Q_D(QTreeWidget);
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
    Q_D(QTreeWidget);
    QModelIndex index = d->model()->index(item, column);
    QAbstractItemView::closePersistentEditor(index);
}

/*!
  \since 4.1

  Returns the widget displayed in the cell specified by \a item and the given \a column.
*/
QWidget *QTreeWidget::itemWidget(QTreeWidgetItem *item, int column) const
{
    Q_ASSERT(item);
    Q_D(const QTreeWidget);
    QModelIndex index = d->model()->index(item, column);
    return QAbstractItemView::indexWidget(index);
}

/*!
  \since 4.1

  Sets the \a widget to be displayed in the cell specified by \a item and the given \a column.

  This function should only be used to display static content in the place of a tree
  widget item. If you want to display custom dynamic content or implement a custom
  editor widget, use QTreeView and subclass QItemDelegate instead.

  \sa {Delegate Classes}
*/
void QTreeWidget::setItemWidget(QTreeWidgetItem *item, int column, QWidget *widget)
{
    Q_ASSERT(item);
    Q_ASSERT(widget);
    Q_D(QTreeWidget);
    QModelIndex index = d->model()->index(item, column);
    QAbstractItemView::setIndexWidget(index, widget);
}

/*!
  Returns true if the \a item is selected; otherwise returns false.
*/
bool QTreeWidget::isItemSelected(const QTreeWidgetItem *item) const
{
    Q_D(const QTreeWidget);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    return selectionModel()->isSelected(index);

}

/*!
  If \a select is true, the given \a item is selected; otherwise it is
  deselected.
*/

void QTreeWidget::setItemSelected(const QTreeWidgetItem *item, bool select)
{
    Q_D(QTreeWidget);
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
    Q_D(const QTreeWidget);
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i) {
        QTreeWidgetItem *item = d->model()->item(indexes.at(i));
        if (!items.contains(item)) // ### slow, optimize later
            items.append(item);
    }
    return items;
}

/*!
  Returns a list of items that match the given \a text, using the given \a flags, in the given \a column.
*/

QList<QTreeWidgetItem*> QTreeWidget::findItems(const QString &text, Qt::MatchFlags flags, int column) const
{
    Q_D(const QTreeWidget);
    QModelIndexList indexes = d->model()->match(model()->index(0, column, QModelIndex()),
                                                Qt::DisplayRole, text, -1, flags);
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < indexes.size(); ++i)
        items.append(d->model()->item(indexes.at(i)));
    return items;
}

/*!
  Returns true if the \a item is explicitly hidden, otherwise returns false.
*/
bool QTreeWidget::isItemHidden(const QTreeWidgetItem *item) const
{
    Q_D(const QTreeWidget);
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
    Q_D(QTreeWidget);
    if (item == headerItem()) {
        header()->setHidden(hide);
    } else {
        QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
        QModelIndex parent = index.parent();
        setRowHidden(index.row(), parent, hide);
    }
}

/*!
  Returns true if the given \a item is open; otherwise returns false.
*/

bool QTreeWidget::isItemExpanded(const QTreeWidgetItem *item) const
{
    Q_ASSERT(item);
    Q_D(const QTreeWidget);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    return isExpanded(index);
}

/*!
    Sets the item referred to by \a item to either closed or opened,
    depending on the value of \a expand.

    \sa expandItem(), collapseItem()
*/

void QTreeWidget::setItemExpanded(const QTreeWidgetItem *item, bool expand)
{
    Q_ASSERT(item);
    Q_D(QTreeWidget);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    Q_ASSERT(index.isValid());
    setExpanded(index, expand);
}

/*!
  Ensures that the \a item is visible, scrolling the view if necessary using
  the specified \a hint.
*/

void QTreeWidget::scrollToItem(const QTreeWidgetItem *item, ScrollHint hint)
{
    Q_ASSERT(item);
    Q_D(QTreeWidget);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    Q_ASSERT(index.isValid());
    QTreeView::scrollTo(index, hint);
}

/*!
    Expands the \a item. This causes the tree containing the item's children
    to be expanded.
*/

void QTreeWidget::expandItem(const QTreeWidgetItem *item)
{
    Q_ASSERT(item);
    Q_D(QTreeWidget);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    expand(index);
}

/*!
    Closes the \a item. This causes the tree containing the item's children
    to be collapsed.
*/

void QTreeWidget::collapseItem(const QTreeWidgetItem *item)
{
    Q_ASSERT(item);
    Q_D(QTreeWidget);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item), 0);
    collapse(index);
}

/*!
  Clears the tree widget by removing all of its items and selections.

  \bold{Note:} Since each item is removed from the tree widget before being
  deleted, the return value of QTreeWidgetItem::treeWidget() will be invalid
  when called from an item's destructor.
*/

void QTreeWidget::clear()
{
    Q_D(QTreeWidget);
    selectionModel()->clear();
    d->model()->clear();
}

/*!
    Returns a list of MIME types that can be used to describe a list of
    treewidget items.

    \sa mimeData()
*/
QStringList QTreeWidget::mimeTypes() const
{
    return model()->QAbstractItemModel::mimeTypes();
}

/*!
    Returns an object that contains a serialized description of the specified
    \a items. The format used to describe the items is obtained from the
    mimeTypes() function.

    If the list of items is empty, 0 is returned rather than a serialized
    empty list.
*/
QMimeData *QTreeWidget::mimeData(const QList<QTreeWidgetItem*>) const
{
    return d_func()->model()->internalMimeData();
}

/*!
    Handles the \a data supplied by a drag and drop operation that ended with
    the given \a action in the \a index in the given \a parent item.

    \sa supportedDropActions()
*/
bool QTreeWidget::dropMimeData(QTreeWidgetItem *parent, int index,
                               const QMimeData *data, Qt::DropAction action)
{
    QModelIndex idx;
    if (parent) idx = indexFromItem(parent);
    return model()->QAbstractItemModel::dropMimeData(data, action , index, 0, idx);
}

/*!
  Returns the drop actions supported by this view.

  \sa Qt::DropActions
*/
Qt::DropActions QTreeWidget::supportedDropActions() const
{
    return model()->QAbstractItemModel::supportedDropActions();
}

/*!
  Returns a list of pointers to the items contained in the \a data object.
  If the object was not created by a QTreeWidget in the same process, the list
  is empty.

*/
QList<QTreeWidgetItem*> QTreeWidget::items(const QMimeData *data) const
{
    const QTreeWidgetMimeData *twd = qobject_cast<const QTreeWidgetMimeData*>(data);
    if (twd)
        return twd->items;
    return QList<QTreeWidgetItem*>();
}

/*!
  Returns the QModelIndex assocated with the given \a item in the given \a column.
*/

QModelIndex QTreeWidget::indexFromItem(QTreeWidgetItem *item, int column) const
{
    Q_D(const QTreeWidget);
    Q_ASSERT(item);
    return d->model()->index(item, column);
}

/*!
  Returns a pointer to the QTreeWidgetItem assocated with the given \a index.
*/

QTreeWidgetItem *QTreeWidget::itemFromIndex(const QModelIndex &index) const
{
    Q_D(const QTreeWidget);
    Q_ASSERT(index.isValid());
    return d->model()->item(index);
}

/*!
  \reimp
*/

void QTreeWidget::setModel(QAbstractItemModel * /*model*/)
{
    qFatal("QTreeWidget::setModel() - Changing the model of the QTreeWidget is not allowed.");
}

/* \reimp */
bool QTreeWidget::event(QEvent *e)
{
    return QTreeView::event(e);
}

#include "moc_qtreewidget.cpp"
#endif // QT_NO_TREEWIDGET
