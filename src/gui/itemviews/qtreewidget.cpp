//depot/qt/main/src/gui/itemviews/qtreewidget.cpp#39 - edit change 150339 (text)
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

    void itemChanged(QTreeWidgetItem *item);

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
  ###
*/

void QTreeModel::clear()
{
    for (int i = 0; i < tree.count(); ++i) {
        tree.at(i)->par = 0;
        tree.at(i)->view = 0;
        delete tree.at(i);
    }
    delete header;
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

QVariant QTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && header)
        return header->data(section, role);
    return QVariant();
}

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

bool QTreeModel::removeRows(int row, const QModelIndex &parent, int /*count*/)
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

    LessThan compare = order == Qt::AscendingOrder ? &lessThan : &greaterThan;
    qHeapSort(begin, end, compare);

    emit dataChanged(index(0, 0, parent), index(count - 1, columnCount() - 1, parent));
}

/*!

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
    emit dataChanged(index(0, 0), index(tree.count() - 1, columnCount() - 1));
}

bool QTreeModel::lessThan(const QTreeWidgetItem *left, const QTreeWidgetItem *right)
{
    return *left < *right;
}

bool QTreeModel::greaterThan(const QTreeWidgetItem *left, const QTreeWidgetItem *right)
{
    return !(*left < *right);
}

/*!

*/

void QTreeModel::itemChanged(QTreeWidgetItem *item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
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
    selected, modified


*/

/*!
    \fn QString QTreeWidgetItem::text(int column) const

    Returns the text in the specified \column.

    \sa setText()
*/

/*!
    \fn void QTreeWidgetItem::setText(int column, const QString &text)
*/

/*!
    \fn QIconSet QTreeWidgetItem::icon(int column) const

    Returns the icon that is displayed in the specified \a column.

    \sa setIcon()
*/

/*!
    \fn void QTreeWidgetItem::setIcon(int column, const QIconSet &icon)
*/

/*!
    \fn QString QTreeWidgetItem::statusTip(int column) const

    Returns the status tip for the contents of the given \a column.

    \sa setStatusTip()
*/

/*!
    \fn void QTreeWidgetItem::setStatusTip(int column, const QString &statusTip)
*/

/*!
    \fn QString QTreeWidgetItem::toolTip(int column) const

    Returns the tool tip for the given \a column.

    \sa setToolTip()
*/

/*!
    \fn void QTreeWidgetItem::setToolTip(int column, const QString &toolTip)
*/

/*!
    \fn QString QTreeWidgetItem::whatsThis(int column) const

    Returns the "What's This?" help for the contents of the given \a column.

    \sa setWhatsThis()
*/

/*!
    \fn void QTreeWidgetItem::setWhatsThis(int column, const QString &whatsThis)
*/

/*!
    \fn QFont QTreeWidgetItem::font(int column) const

    Returns the font used to render the text in the specified \a column.

    \sa setFont()
*/

/*!
    \fn void QTreeWidgetItem::setFont(int column, const QFont &font)
*/

/*!
    \fn QColor QTreeWidgetItem::backgroundColor(int column) const

    Returns the color used to render the background of the specified \a column.

    \sa textColor() setBackgroundColor()
*/

/*!
    \fn void QTreeWidgetItem::setBackgroundColor(int column, const QColor &color)
*/

/*!
    \fn QColor QTreeWidgetItem::textColor(int column) const

    Returns the color used to render the text in the specified \a column.

    \sa backgroundColor() setTextColor()
*/

/*!
    \fn void QTreeWidgetItem::setTextColor(int column, const QColor &color)
*/

/*!
    \fn int QTreeWidgetItem::checkedState(int column) const
*/

/*!
    \fn void QTreeWidgetItem::setCheckedState(int column, bool state)
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
    : view(0), par(0), itemFlags(QAbstractItemModel::ItemIsEditable
                                 |QAbstractItemModel::ItemIsSelectable
                                 |QAbstractItemModel::ItemIsCheckable
                                 |QAbstractItemModel::ItemIsEnabled)
{
}

/*!
    \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view)

    Constructs a tree widget item and inserts it into the given tree
    \a view.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view)
    : view(view), par(0), itemFlags(QAbstractItemModel::ItemIsEditable
                                    |QAbstractItemModel::ItemIsSelectable
                                    |QAbstractItemModel::ItemIsCheckable
                                    |QAbstractItemModel::ItemIsEnabled)
{
    if (view) {
        QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
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
    : view(0), par(0), itemFlags(QAbstractItemModel::ItemIsEditable
                                 |QAbstractItemModel::ItemIsSelectable
                                 |QAbstractItemModel::ItemIsCheckable
                                 |QAbstractItemModel::ItemIsEnabled)
{
    if (view) {
        QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
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
    : view(parent->view), par(parent),
      itemFlags(QAbstractItemModel::ItemIsEditable
                |QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsCheckable
                |QAbstractItemModel::ItemIsEnabled)
{
    if (parent)
        parent->appendChild(this);
    QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
    if (model)
        model->emitRowsInserted(this);
}

/*!
  Constructs a tree widget item with the given \a parent and,
  and inserted after the \a after child.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *after)
{
    if (parent) {
        int i = parent->indexOfChild(after);
        parent->insertChild(i, this);
    }
    QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
    if (model)
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

    if (par) {
        par->children.removeAll(this);
        return;
    }

    if (view) {
        QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
        if (model)
            model->remove(this);
    }
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
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
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
    if (view) {
        QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
        model->itemChanged(this);
    }
}

/*!
    Returns the value for the item's \a column and \a role.

    \sa store()
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
  Appends the \a child item to the list of children.

  \sa insertChild() takeChild()
*/
void QTreeWidgetItem::appendChild(QTreeWidgetItem *child)
{
    insertChild(children.count(), child);
}

/*!
  Inserts the \c child item in the list of children.
*/

void QTreeWidgetItem::insertChild(int index, QTreeWidgetItem *child)
{
    children.insert(index, child);
    if (view) {
        QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
        if (model)
            model->emitRowsInserted(child);
    }
}

/*!
  Removes the item at \a index and returns it.
*/

QTreeWidgetItem *QTreeWidgetItem::takeChild(int index)
{
    if (view) {
        QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
        if (model)
            model->emitRowsRemoved(children.at(index));
    }
    return children.takeAt(index);
}

/*!
  Sorts the children by the value in the given \a column, in the given \a order.
*/

void QTreeWidgetItem::sortChildren(int column, Qt::SortOrder order, bool climb)
{
    LessThan compare = order == Qt::AscendingOrder ? &QTreeModel::lessThan : &QTreeModel::greaterThan;
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
    QTreeWidgetPrivate() : QTreeViewPrivate() {}
    inline QTreeModel *model() const { return ::qt_cast<QTreeModel*>(q_func()->model()); }
    void emitPressed(const QModelIndex &index, int button);
    void emitClicked(const QModelIndex &index, int button);
    void emitDoubleClicked(const QModelIndex &index, int button);
    void emitKeyPressed(const QModelIndex &index, Qt::Key key, Qt::ButtonState state);
    void emitReturnPressed(const QModelIndex &index);
    void emitExpanded(const QModelIndex &index);
    void emitCollapsed(const QModelIndex &index);
    void emitCurrentChanged(const QModelIndex &previous, const QModelIndex &current);
    void emitItemEntered(const QModelIndex &index, Qt::ButtonState state);
    void emitAboutToShowContextMenu(QMenu *menu, const QModelIndex &index);
    void emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
};

void QTreeWidgetPrivate::emitPressed(const QModelIndex &index, int button)
{
    emit q->pressed(model()->item(index), index.column(), button);
}

void QTreeWidgetPrivate::emitClicked(const QModelIndex &index, int button)
{
    emit q->clicked(model()->item(index), index.column(), button);
}

void QTreeWidgetPrivate::emitDoubleClicked(const QModelIndex &index, int button)
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
    \fn void QTreeWidget::clicked(QTreeWidgetItem *item, int column, int button)

    This signal is emitted when a mouse button is clicked. The \a item
    may be 0 if the mouse was not clicked on an item. The \a column
    returns the item column clicked, or -1 if no column was
    clicked. The button clicked is specified by \a button (see
    \l{Qt::ButtonState}).
*/

/*!
    \fn void QTreeWidget::doubleClicked(QTreeWidgetItem *item, int column, int button)

    This signal is emitted when a mouse button is double clicked. The
    \a item may be 0 if the mouse was not clicked on an item. The \a
    column returns the item column clicked, or -1 if no column was
    clicked. The button clicked is specified by \a button (see
    \l{Qt::ButtonState}).
*/

/*!
  Constructs a tree view with the given \a parent widget, using the default
  model
*/

QTreeWidget::QTreeWidget(QWidget *parent)
    : QTreeView(*new QTreeViewPrivate(), parent)
{
    setModel(new QTreeModel(0, this));
    connect(this, SIGNAL(pressed(const QModelIndex&, int)),
            SLOT(emitPressed(const QModelIndex&, int)));
    connect(this, SIGNAL(clicked(const QModelIndex&, int)),
            SLOT(emitClicked(const QModelIndex&, int)));
    connect(this, SIGNAL(doubleClicked(const QModelIndex&, int)),
            SLOT(emitDoubleClicked(const QModelIndex&, int)));
    connect(this, SIGNAL(keyPressed(const QModelIndex&, Qt::Key, Qt::ButtonState)),
            SLOT(emitKeyPressed(const QModelIndex&, Qt::Key, Qt::ButtonState)));
    connect(this, SIGNAL(returnPressed(const QModelIndex&)),
            SLOT(emitReturnPressed(const QModelIndex&)));
    connect(this, SIGNAL(expanded(const QModelIndex&)),
            SLOT(emitExpanded(const QModelIndex&)));
    connect(this, SIGNAL(collapsed(const QModelIndex&)),
            SLOT(emitCollapsed(const QModelIndex&)));
    connect(this, SIGNAL(itemEntered(const QModelIndex&, Qt::ButtonState)),
            SLOT(emitItemEntered(const QModelIndex&, Qt::ButtonState)));
    connect(this, SIGNAL(aboutToShowContextMenu(QMenu*, const QModelIndex&)),
            SLOT(emitAboutToShowContextMenu(QMenu*, const QModelIndex&)));
    connect(selectionModel(),
            SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(emitCurrentChanged(const QModelIndex&, const QModelIndex&)));
    connect(selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SIGNAL(selectionChanged()));
    connect(model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(emitItemChanged(const QModelIndex&, const QModelIndex&)));
    connect(header(), SIGNAL(sectionPressed(int, Qt::ButtonState)), this, SLOT(sortItems(int)));
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

/*!
  Returns the top level item at \a index.
*/

QTreeWidgetItem *QTreeWidget::topLevelItem(int index) const
{
    return d->model()->tree.at(index);
}

/*!
  Returns the number of top level items.
*/

int QTreeWidget::topLevelItemCount() const
{
    return d->model()->tree.count();
}

/*!
  ###
*/

void QTreeWidget::insertTopLevelItem(int index, QTreeWidgetItem *item)
{
    d->model()->tree.insert(index, item);
}

/*!
  ###
*/

void QTreeWidget::appendTopLevelItem(QTreeWidgetItem *item)
{
    d->model()->tree.append(item);
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
*/

QTreeWidgetItem *QTreeWidget::currentItem() const
{
    return d->model()->item(currentIndex());
}

void QTreeWidget::setCurrentItem(QTreeWidgetItem *item)
{
    if (item)
        setCurrentIndex(d->model()->index(item));
    else
        setCurrentIndex(QModelIndex());
}

void QTreeWidget::sortItems(int column, Qt::SortOrder order)
{
    d->model()->sortAll(column, order);
    header()->setSortIndicator(column, order);
}

void QTreeWidget::setSortingEnabled(bool enable)
{
    header()->showSortIndicator(enable);
}

bool QTreeWidget::isSortingEnabled() const
{
    return header()->isSortIndicatorShown();
}

void QTreeWidget::openPersistentEditor(QTreeWidgetItem *item, int column)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item, column);
    QAbstractItemView::openPersistentEditor(index);
}

void QTreeWidget::closePersistentEditor(QTreeWidgetItem *item, int column)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item, column);
    QAbstractItemView::closePersistentEditor(index);
}

/*!
  Returns true if the \a item is selected, otherwise returns false.
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
    selectionModel()->select(index, select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
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
  Finds items that matches the \a text, using the criteria given in the \a flags.
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
  Returns true if the \a item is in the viewport, otherwise returns false.
*/

bool QTreeWidget::isVisible(const QTreeWidgetItem *item) const
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item));
    QRect rect = itemViewportRect(index);
    return d->viewport->rect().contains(rect);
}

/*!
  Scrolls the view if necessary to ensure that the \a item is visible.
*/

void QTreeWidget::ensureItemVisible(const QTreeWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTreeWidgetItem*>(item));
    QTreeView::ensureItemVisible(index);
}

void QTreeWidget::sortItems(int column)
{
    if (isSortingEnabled())
        d->model()->sortAll(column, header()->sortIndicatorOrder());
}

/*!
  Clears the tree widget by removing all of its items.
*/

void QTreeWidget::clear()
{
    d->model()->clear();
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

void QTreeWidget::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
}

#include "moc_qtreewidget.cpp"
