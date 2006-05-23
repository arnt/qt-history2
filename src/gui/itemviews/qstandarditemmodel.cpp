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

#include "qstandarditemmodel.h"

#ifndef QT_NO_STANDARDITEMMODEL

#include <QtCore/qdatetime.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include <QtCore/qstringlist.h>

#include <private/qstandarditemmodel_p.h>
#include <qdebug.h>

/*!
    \class QStandardItemModel
    \brief The QStandardItemModel class provides a generic model for storing custom data.
    \ingroup model-view

    QStandardItemModel can be used as a repository for standard Qt
    data types. It is one of the \l {Model/View Classes} and is part
    of Qt's \l {Model/View Programming}{model/view} framework.

    Data is written to the model, and read back, using the standard
    QAbstractItemModel interface. The way each item of information is
    referenced depends on how the data is inserted into the model. The
    QStandardItemModel class reimplements several of the
    QAbstractItemModel class's functions, and implements in addition a
    clear() function which removes all items from the model and resets
    both the horizontal and vertical headers.

    Since Qt 4.2, QStandardItemModel also provides an item-based approach to
    working with the model. The items in a QStandardItemModel are provided by
    QStandardItem.

    For performance and flexibility, you may want to subclass
    QAbstractItemModel to provide support for different kinds of
    repositories. For example, the QDirModel provides a model
    interface to the underlying file system, and does not actually
    store file information internally.

    An example usage of QStandardItemModel to create a table (using the
    QAbstractItemModel interface):

    \quotefromfile itemviews/spinboxdelegate/main.cpp
    \skipto QStandardItemModel model
    \printline model

    \skipto for (int row
    \printuntil }
    \printline }

    An example of using the item-based approach to create a table:

    \code
            QStandardItemModel model(4, 4);
            for (int row = 0; row < 4; ++row) {
                for (int column = 0; column < 4; ++column) {
                    QStandardItem *item = new QStandardItem(QString("row %0, column %1").arg(row).arg(column));
                    model.setItem(i, j, item);
                }
            }
    \endcode

    An example usage of QStandardItemModel to create a tree (using the
    QAbstractItemModel interface):

    \quotefromfile snippets/qstandarditemmodel/main.cpp
    \skipto QStandardItemModel
    \printuntil }

    An example of using the item-based approach to create a tree:

    \code
            QStandardItemModel model;
            QStandardItem *parentItem = model.topLevelParent();
            for (int i = 0; i < 4; ++i) {
                QStandardItem *item = new QStandardItem(QString("item %0").arg(i));
                parentItem->appendRow(item);
                parentItem = item;
            }
    \endcode

    The item-based approach hides much of the QModelIndex-based interface of
    QAbstractItemModel; instead, a more convenient and intuitive interface
    based on direct manipulation of item objects is provided. When items are
    manipulated, QStandardItemModel takes care of emitting the proper
    QAbstractItemModel signals (e.g. dataChanged() and rowsInserted()) behind
    the scenes, so that the model state observed by other objects (e.g. views)
    is consistent.

    When using the item-based approach, itemFromIndex() and indexFromItem()
    provide a bridge between the item-based interface and the
    QAbstractItemModel/QAbstractItemView interface. Typical usage of
    itemFromIndex() includes obtaining the item at the current index in a
    view, and obtaining the item that corresponds to an index carried by a
    QAbstractItemView signal, such as QAbstractItemView::clicked():

    \code
        // In constructor or similar
        QTreeView *treeView = new QTreeView(this);
        treeView->setModel(myStandardItemModel);
        connect(treeView, SIGNAL(clicked(const QModelIndex &)),
                this, SLOT(clicked(const QModelIndex &)));
        ...

        void MyWidget::clicked(const QModelIndex &index)
        {
            QStandardItem *item = myStandardItemModel->itemFromIndex(index);
            // Do stuff with the item ...
        }
    \endcode

    Conversely, you must obtain the QModelIndex of an item when you want to
    invoke a model/view function that takes an index as argument. You can
    obtain the index either by using the model's indexFromItem() function, or,
    equivalently, by calling QStandardItem::index():

    \code
        treeView->scrollTo(item->index());
    \endcode

    \sa {Model/View Programming}, QAbstractItemModel,
    {itemviews/simpletreemodel}{Simple Tree Model example},
    {Item View Convenience Classes}
*/

/*!
    \since 4.2

    \class QStandardItem
    \brief The QStandardItem class provides an item for use with the
    QStandardItemModel class.

    \ingroup model-view

    Items usually contain text, icons, or checkboxes.

    Each item can have its own background color which is set with the
    setBackgroundColor() function. The current background color can be found
    with backgroundColor().  The text label for each item can be rendered with
    its own font and text color. These are specified with the setFont() and
    setTextColor() functions, and read with font() and textColor().

    By default, items are enabled, editable, selectable, checkable, and can be
    used both as the source of a drag and drop operation and as a drop target.
    Each item's flags can be changed by calling setFlags(). Checkable items
    can be checked and unchecked with the setCheckState() function. The
    corresponding checkState() function indicates whether the item is
    currently checked.

    Each item can have a table of child items. The dimensions of the child
    table can be set with setRowCount() and setColumnCount(). Additionally,
    new rows and columns can be inserted with insertRow() and insertColumn(),
    or appended with appendRow() and appendColumn(). Items can be positioned
    in the child table with setChild(). Get a pointer to a child item with
    child().

    \sa QStandardItemModel, {Model/View Programming}
*/

/*!
    \enum QStandardItem::ItemType

    This enum describes the types that are used to describe standard items.

    \value Type     The default type for standard items.
    \value UserType The minimum value for custom types. Values below UserType are
                    reserved by Qt.

    You can define new user types in QStandardItem subclasses to ensure that
    custom items are treated specially; for example, when items are sorted.

    \sa type()
*/

/*!
    \internal
*/
class QStandardItemModelLessThan
{
public:
    inline QStandardItemModelLessThan()
        { }

    inline bool operator()(const QPair<QStandardItem*, int> &l,
                           const QPair<QStandardItem*, int> &r) const
    {
        return *(l.first) < *(r.first);
    }
};

/*!
    \internal
*/
class QStandardItemModelGreaterThan
{
public:
    inline QStandardItemModelGreaterThan()
        { }

    inline bool operator()(const QPair<QStandardItem*, int> &l,
                           const QPair<QStandardItem*, int> &r) const
    {
        return *(r.first) < *(l.first);
    }
};

/*!
  \internal
*/
QStandardItemPrivate::~QStandardItemPrivate()
{
    QVector<QStandardItem*>::const_iterator it;
    for (it = children.begin(); it != children.end(); ++it) {
        QStandardItem *child = *it;
        if (child)
            child->d_func()->setModel(0);
        delete child;
    }
    children.clear();
    if (parent && model)
        parent->d_func()->childDeleted(q_func());
}

/*!
  \internal
*/
int QStandardItemPrivate::childIndex(int row, int column) const
{
    if ((row < 0) || (column < 0) || (row >= rowCount()) || (column >= columnCount()))
        return -1;
    return (row * columnCount()) + column;
}

/*!
  \internal
*/
QPair<int, int> QStandardItemPrivate::itemPosition(const QStandardItem *item) const
{
    if (QStandardItem *par = item->parent()) {
        int idx = par->d_func()->childIndex(item);
        if (idx == -1)
            return QPair<int, int>(-1, -1);
        return QPair<int, int>(idx / par->columnCount(), idx % par->columnCount());
    }
    // ### support header items?
    return QPair<int, int>(-1, -1);
}

/*!
  \internal
*/
void QStandardItemPrivate::changeFlags(bool enable, Qt::ItemFlags f)
{
    Q_Q(QStandardItem);
    Qt::ItemFlags oldFlags = flags;
    if (enable)
        flags |= f;
    else
        flags &= ~f;
    if ((flags != oldFlags) && model)
        model->d_func()->itemChanged(q);
}

/*!
  \internal
*/
void QStandardItemPrivate::childDeleted(QStandardItem *child)
{
    int index = childIndex(child);
    Q_ASSERT(index != -1);
    children.replace(index, 0);
}

/*!
  \internal
*/
QStandardItemModelPrivate::QStandardItemModelPrivate()
    : root(new QStandardItem),
      itemPrototype(0)
{
}

/*!
  \internal
*/
QStandardItemModelPrivate::~QStandardItemModelPrivate()
{
    delete root;
}

/*!
    \internal
*/
QStandardItem *QStandardItemModelPrivate::createItem() const
{
    return itemPrototype ? itemPrototype->clone() : new QStandardItem;
}

/*!
    Constructs an item of the specified \a type.

    \sa QStandardItem::ItemType
*/
QStandardItem::QStandardItem(int type)
    : d_ptr(new QStandardItemPrivate)
{
    Q_D(QStandardItem);
    d->q_ptr = this;
    d->type = type;
}

/*!
    Constructs an item with the given \a text.
*/
QStandardItem::QStandardItem(const QString &text, int type)
    : d_ptr(new QStandardItemPrivate)
{
    Q_D(QStandardItem);
    d->q_ptr = this;
    d->type = type;
    setText(text);
}

/*!
    Constructs an item with the given \a icon and \a text.
*/
QStandardItem::QStandardItem(const QIcon &icon, const QString &text, int type)
    : d_ptr(new QStandardItemPrivate)
{
    Q_D(QStandardItem);
    d->q_ptr = this;
    d->type = type;
    setIcon(icon);
    setText(text);
}

/*!
   Constructs an item with \a rows rows and \a columns columns of child items.
*/
QStandardItem::QStandardItem(int rows, int columns, int type)
    : d_ptr(new QStandardItemPrivate)
{
    Q_D(QStandardItem);
    d->q_ptr = this;
    d->type = type;
    setRowCount(rows);
    setColumnCount(columns);
}

/*!
   Constructs an item with a single column of child items and adds \a items as
   rows.
*/
QStandardItem::QStandardItem(const QList<QStandardItem*> &items, int type)
    : d_ptr(new QStandardItemPrivate)
{
    Q_D(QStandardItem);
    d->q_ptr = this;
    d->type = type;
    setColumnCount(1);
    for (int i = 0; i < items.count(); ++i)
        setChild(i, items.at(i));
}

/*!
    Constructs a copy of \a other. Note that type() and model() are
    not copied.
*/
QStandardItem::QStandardItem(const QStandardItem &other)
    : d_ptr(new QStandardItemPrivate)
{
    Q_D(QStandardItem);
    d->q_ptr = this;
    operator=(other);
}

/*!
  \internal
*/
QStandardItem::QStandardItem(QStandardItemPrivate &dd)
    : d_ptr(&dd)
{
    Q_D(QStandardItem);
    d->q_ptr = this;
}

/*!
    Assigns \a other's data and flags to this item. Note that
    type() and model() are not copied.
*/
QStandardItem &QStandardItem::operator=(const QStandardItem &other)
{
    Q_D(QStandardItem);
    d->values = other.d_func()->values;
    d->flags = other.d_func()->flags;
    return *this;
}

/*!
  Destructs the item.
*/
QStandardItem::~QStandardItem()
{
    Q_D(QStandardItem);
    delete d;
}

/*!
  Returns the item's parent item.

  \sa child(), isTopLevelItem()
*/
QStandardItem *QStandardItem::parent() const
{
    Q_D(const QStandardItem);
    return d->parent;
}

/*!
    Returns the type passed to the QStandardItem constructor.

    \sa QStandardItem::ItemType
*/
int QStandardItem::type() const
{
    Q_D(const QStandardItem);
    return d->type;
}

/*!
    Sets the item's data for the given \a role to the specified \a value.

    \sa Qt::ItemDataRole, data(), setFlags()
*/
void QStandardItem::setData(int role, const QVariant &value)
{
    Q_D(QStandardItem);
    role = (role == Qt::EditRole) ? Qt::DisplayRole : role;
    QVariant oldValue = data(role);
    if (value == oldValue)
        return;

    QVector<QWidgetItemData>::iterator it;
    for (it = d->values.begin(); it != d->values.end(); ++it) {
        if ((*it).role == role) {
            (*it).value = value;
            return;
        }
    }
    d->values.append(QWidgetItemData(role, value));

    if (d->model)
        d->model->d_func()->itemChanged(this);
}

/*!
    Returns the item's data for the given \a role.
*/
QVariant QStandardItem::data(int role) const
{
    Q_D(const QStandardItem);
    role = (role == Qt::EditRole) ? Qt::DisplayRole : role;
    QVector<QWidgetItemData>::const_iterator it;
    for (it = d->values.begin(); it != d->values.end(); ++it) {
        if ((*it).role == role)
            return (*it).value;
    }
    return QVariant();
}

/*!
  Sets the item flags for the item to \a flags.

  \sa flags(), setData()
*/
void QStandardItem::setFlags(Qt::ItemFlags flags)
{
    Q_D(QStandardItem);
    if (flags != d->flags) {
        d->flags = flags;
        if (d->model)
            d->model->d_func()->itemChanged(this);
    }
}

/*!
  Returns the item flags for the item.

  \sa setFlags()
*/
Qt::ItemFlags QStandardItem::flags() const
{
    Q_D(const QStandardItem);
    return d->flags;
}

/*!
    \fn QString QStandardItem::text() const

    Returns the item's text.

    \sa setText()
*/

/*!
    \fn void QStandardItem::setText(const QString &text)

    Sets the item's text to the \a text specified.

    \sa text(), setFont(), setTextColor()
*/

/*!
    \fn QIcon QStandardItem::icon() const

    Returns the item's icon.

    \sa setIcon(), {QAbstractItemView::iconSize}{iconSize}
*/

/*!
    \fn void QStandardItem::setIcon(const QIcon &icon)

    Sets the item's icon to the \a icon specified.
*/

/*!
    \fn QString QStandardItem::statusTip() const

    Returns the item's status tip.

    \sa setStatusTip()
*/

/*!
    \fn void QStandardItem::setStatusTip(const QString &statusTip)

    Sets the item's status tip to the string specified by \a statusTip.

    \sa statusTip(), setToolTip(), setWhatsThis()
*/

/*!
    \fn QString QStandardItem::toolTip() const

    Returns the item's tooltip.

    \sa setToolTip()
*/

/*!
    \fn void QStandardItem::setToolTip(const QString &toolTip)

    Sets the item's tooltip to the string specified by \a toolTip.

    \sa toolTip(), setStatusTip(), setWhatsThis()
*/

/*!
    \fn QString QStandardItem::whatsThis() const

    Returns the item's "What's This?" help.

    \sa setWhatsThis()
*/

/*!
    \fn void QStandardItem::setWhatsThis(const QString &whatsThis)

    Sets the item's "What's This?" help to the string specified by \a whatsThis.

    \sa whatsThis(), setStatusTip(), setToolTip()
*/

/*!
    \fn QFont QStandardItem::font() const

    Returns the font used to render the item's text.

    \sa setFont()
*/

/*!
    \fn void QStandardItem::setFont(const QFont &font)

    Sets the font used to display the item's text to the given \a font.

    \sa font() setText() setTextColor()
*/

/*!
    \fn QColor QStandardItem::backgroundColor() const

    Returns the color used to render the item's background.

    \sa textColor() setBackgroundColor()
*/

/*!
    \fn void QStandardItem::setBackgroundColor(const QColor &color)

    Sets the item's background color to the specified \a color.

    \sa backgroundColor() setTextColor()
*/

/*!
    \fn QColor QStandardItem::textColor() const

    Returns the color used to render the item's text.

    \sa backgroundColor() setTextColor()
*/

/*!
    \fn void QStandardItem::setTextColor(const QColor &color)

    Sets the color used to display the item's text to the given \a color.

    \sa textColor() setFont() setText()
*/

/*!
    \fn int QStandardItem::textAlignment() const

    Returns the text alignment for the item's text.
*/

/*!
    \fn void QStandardItem::setTextAlignment(int alignment)

    Sets the text alignment for the item's text to the \a alignment
    specified.

    \sa textAlignment()
*/

/*!
    \fn QSize QStandardItem::sizeHint() const

    Returns the size hint set for the item.

    \sa setSizeHint()
*/

/*!
    \fn void QStandardItem::setSizeHint(const QSize &size)

    Sets the size hint for the item to be \a size.
    If no size hint is set, the item delegate will compute the
    size hint based on the item data.

    \sa sizeHint()
*/

/*!
    \fn Qt::CheckState QStandardItem::checkState() const

    Returns the checked state of the item.

    \sa setCheckState(), isCheckable()
*/

/*!
    \fn void QStandardItem::setCheckState(Qt::CheckState state)

    Sets the check state of the item to be \a state.

    \sa checkState(), setCheckable()
*/

/*!
    \fn QString QStandardItem::accessibleText() const

    Returns the item's accessible text.

    \sa setAccessibleText(), accessibleDescription()
*/

/*!
    \fn void QStandardItem::setAccessibleText(const QString &accessibleText)

    Sets the item's accessible text to the string specified by \a accessibleText.

    \sa accessibleText(), setAccessibleDescription()
*/

/*!
    \fn QString QStandardItem::accessibleDescription() const

    Returns the item's accessible description.

    \sa setAccessibleDescription(), accessibleText()
*/

/*!
    \fn void QStandardItem::setAccessibleDescription(const QString &accessibleDescription)

    Sets the item's accessible description to the string specified by \a accessibleDescription.

    \sa accessibleDescription(), setAccessibleText()
*/

/*!
  Sets whether the item is enabled.

  \sa isEnabled(), setFlags()
*/
void QStandardItem::setEnabled(bool enabled)
{
    Q_D(QStandardItem);
    d->changeFlags(enabled, Qt::ItemIsEnabled);
}

/*!
  \fn bool QStandardItem::isEnabled() const

  Returns whether the item is enabled.

  \sa setEnabled(), flags()
*/

/*!
  Sets whether the item is editable.

  \sa isEditable(), setFlags()
*/
void QStandardItem::setEditable(bool editable)
{
    Q_D(QStandardItem);
    d->changeFlags(editable, Qt::ItemIsEditable);
}

/*!
  \fn bool QStandardItem::isEditable() const

  Returns whether the item is editable.

  \sa setEditable(), flags()
*/

/*!
  Sets whether the item is selectable.

  \sa isSelectable(), setFlags()
*/
void QStandardItem::setSelectable(bool selectable)
{
    Q_D(QStandardItem);
    d->changeFlags(selectable, Qt::ItemIsSelectable);
}

/*!
  \fn bool QStandardItem::isSelectable() const

  Returns whether the item is selectable.

  \sa setSelectable(), flags()
*/

/*!
  Sets whether the item is user-checkable.

  \sa isCheckable(), setCheckState()
*/
void QStandardItem::setCheckable(bool checkable)
{
    Q_D(QStandardItem);
    if (checkable && !isCheckable()) {
        // make sure there's data for the checkstate role
        if (data(Qt::CheckStateRole).isValid())
            setData(Qt::CheckStateRole, Qt::Unchecked);
    }
    d->changeFlags(checkable, Qt::ItemIsUserCheckable);
}

/*!
  \fn bool QStandardItem::isCheckable() const

  Returns whether the item is user-checkable.

  \sa setCheckable(), checkState()
*/

/*!
  Sets whether the item is tristate.

  \sa isTristate(), setFlags()
*/
void QStandardItem::setTristate(bool tristate)
{
    Q_D(QStandardItem);
    d->changeFlags(tristate, Qt::ItemIsTristate);
}

/*!
  \fn bool QStandardItem::isTristate() const

  Returns whether the item is tristate.

  \sa setTristate(), flags()
*/

/*!
  Sets whether the item is drag enabled.

  \sa isDragEnabled(), setDropEnabled(), setFlags()
*/
void QStandardItem::setDragEnabled(bool dragEnabled)
{
    Q_D(QStandardItem);
    d->changeFlags(dragEnabled, Qt::ItemIsDragEnabled);
}

/*!
  \fn bool QStandardItem::isDragEnabled() const

  Returns whether the item is drag enabled.

  \sa setDragEnabled(), isDropEnabled(), flags()
*/

/*!
  Sets whether the item is drop enabled.

  \sa isDropEnabled(), setDragEnabled(), setFlags()
*/
void QStandardItem::setDropEnabled(bool dropEnabled)
{
    Q_D(QStandardItem);
    d->changeFlags(dropEnabled, Qt::ItemIsDropEnabled);
}

/*!
  \fn bool QStandardItem::isDropEnabled() const

  Returns whether the item is drop enabled.

  \sa setDropEnabled(), isDragEnabled(), flags()
*/

/*!
  Returns the row where the item is located in its parent's child table.

  \sa column(), parent()
*/
int QStandardItem::row() const
{
    Q_D(const QStandardItem);
    QPair<int, int> pos = d->itemPosition(this);
    return pos.first;
}

/*!
  Returns the column where the item is located in its parent's child table.

  \sa row(), parent()
*/
int QStandardItem::column() const
{
    Q_D(const QStandardItem);
    QPair<int, int> pos = d->itemPosition(this);
    return pos.second;
}

/*!
  Returns the QModelIndex associated with this item.

  \sa QStandardItemModel::indexFromItem(), QStandardItemModel::itemFromIndex()
*/
QModelIndex QStandardItem::index() const
{
    Q_D(const QStandardItem);
    return d->model ? d->model->indexFromItem(this) : QModelIndex();
}

/*!
  Returns the QStandardItemModel that this item belongs to.
*/
QStandardItemModel *QStandardItem::model() const
{
    Q_D(const QStandardItem);
    return d->model;
}

/*!
    Sets the number of child item rows to \a rows. If this is less than
    rowCount(), the data in the unwanted rows is discarded.

    \sa rowCount(), setColumnCount()
*/
void QStandardItem::setRowCount(int rows)
{
    int rc = rowCount();
    if (rc == rows)
        return;
    if (rc < rows)
        insertRows(qMax(rc, 0), rows - rc);
    else
        removeRows(qMax(rows, 0), rc - rows);
}

/*!
    Returns the number of child item rows that the item has.

    \sa setRowCount(), columnCount()
*/
int QStandardItem::rowCount() const
{
    Q_D(const QStandardItem);
    return d->rowCount();
}

/*!
    Sets the number of child item columns to \a columns. If this is less than
    columnCount(), the data in the unwanted columns is discarded.

    \sa columnCount(), setRowCount()
*/
void QStandardItem::setColumnCount(int columns)
{
    int cc = columnCount();
    if (cc == columns)
        return;
    if (cc < columns)
        insertColumns(qMax(cc, 0), columns - cc);
    else
        removeColumns(qMax(columns, 0), cc - columns);
}

/*!
    Returns the number of child item columns that the item has.

    \sa setColumnCount(), rowCount()
*/
int QStandardItem::columnCount() const
{
    Q_D(const QStandardItem);
    return d->columnCount();
}

/*!
    Inserts a row at \a row containing \a items. If necessary, the column
    count is increased to the size of \a items.

    \sa insertRows(), insertColumn()
*/
void QStandardItem::insertRow(int row, const QList<QStandardItem*> &items)
{
    Q_D(QStandardItem);
    if (row < 0)
        return;
    if (columnCount() < items.count())
        setColumnCount(items.count());
    d->insertRows(row, 1, items);
}

/*!
    Inserts a column at \a column containing \a items. If necessary,
    the row count is increased to the size of \a items.

    \sa insertColumns(), insertRow()
*/
void QStandardItem::insertColumn(int column, const QList<QStandardItem*> &items)
{
    Q_D(QStandardItem);
    if (column < 0)
        return;
    if (rowCount() < items.count())
        setRowCount(items.count());
    d->insertColumns(column, 1, items);
}

/*!
    Inserts \a count rows of child items at row \a row.

    \sa insertRow(), insertColumns()
*/
void QStandardItem::insertRows(int row, int count)
{
    Q_D(QStandardItem);
    if (rowCount() < row) {
        count += row - rowCount();
        row = rowCount();
    }
    d->insertRows(row, count, QList<QStandardItem*>());
}

/*!
    \internal
*/
bool QStandardItemPrivate::insertRows(int row, int count, const QList<QStandardItem*> &items)
{
    Q_Q(QStandardItem);
    if ((count < 1) || (row < 0) || (row > rowCount()))
        return false;
    if (model)
        model->d_func()->rowsAboutToBeInserted(q, row, row + count - 1);
    if (rowCount() == 0) {
        children.resize(columnCount() * count);
        rows = count;
    } else {
        rows += count;
        int index = childIndex(row, 0);
        if (index != -1)
            children.insert(index, columnCount() * count, 0);
    }
    if (!items.isEmpty()) {
        int index = childIndex(row, 0);
        int limit = qMin(items.count(), columnCount() * count);
        for (int i = 0; i < limit; ++i) {
            QStandardItem *item = items.at(i);
            if (item) {
                if (item->parent() == 0) {
                    item->d_func()->setParentAndModel(q, model);
                } else {
                    qWarning("QStandardItem::insertRows(): ignoring duplicate insertion of item %p",
                             item);
                    item = 0;
                }
            }
            children.replace(index, item);
            ++index;
        }
    }
    if (model)
        model->d_func()->rowsInserted(q, row, count);
    return true;
}

/*!
    Inserts \a count columns of child items at column \a column.

    \sa insertColumn(), insertRows()
*/
void QStandardItem::insertColumns(int column, int count)
{
    Q_D(QStandardItem);
    if (columnCount() < column) {
        count += column - columnCount();
        column = columnCount();
    }
    d->insertColumns(column, count, QList<QStandardItem*>());
}

/*!
    \internal
*/
bool QStandardItemPrivate::insertColumns(int column, int count, const QList<QStandardItem*> &items)
{
    Q_Q(QStandardItem);
    if ((count < 1) || (column < 0) || (column > columnCount()))
        return false;
    if (model)
        model->d_func()->columnsAboutToBeInserted(q, column, column + count - 1);
    if (columnCount() == 0) {
        children.resize(rowCount() * count);
        columns = count;
    } else {
        columns += count;
        int index = childIndex(0, column);
        for (int row = 0; row < rowCount(); ++row) {
            children.insert(index, count, 0);
            index += columnCount();
        }
    }
    if (!items.isEmpty()) {
        int limit = qMin(items.count(), rowCount() * count);
        for (int i = 0; i < limit; ++i) {
            QStandardItem *item = items.at(i);
            if (item) {
                if (item->parent() == 0) {
                    item->d_func()->setParentAndModel(q, model);
                } else {
                    qWarning("QStandardItem::insertColumns(): ignoring duplicate insertion of item %p",
                             item);
                    item = 0;
                }
            }
            int r = i / count;
            int c = column + (i % count);
            int index = childIndex(r, c);
            children.replace(index, item);
        }
    }
    if (model)
        model->d_func()->columnsInserted(q, column, count);
    return true;
}

/*!
    \fn void QStandardItem::appendRow(const QList<QStandardItem*> &items)

    Appends a row containing \a items. If necessary, the column count is
    increased to the size of \a items.

    \sa insertRow()
*/

/*!
    \fn void QStandardItem::appendColumn(const QList<QStandardItem*> &items)

    Appends a column containing \a items. If necessary, the row count is
    increased to the size of \a items.

    \sa insertColumn()
*/

/*!
    \fn QStandardItem::insertRow(int row, QStandardItem *item)
    \overload

    When building a list or a tree that has only one column, this function
    provides a convenient way to insert a single new item.
*/

/*!
    \fn QStandardItem::appendRow(int row, QStandardItem *item)
    \overload

    When building a list or a tree that has only one column, this function
    provides a convenient way to append a single new item.
*/

/*!
    Removes the given \a row.

    \sa removeRows(), removeColumn()
*/
void QStandardItem::removeRow(int row)
{
    removeRows(row, 1);
}

/*!
    Removes the given \a column.

    \sa removeColumns(), removeRow()
*/
void QStandardItem::removeColumn(int column)
{
    removeColumns(column, 1);
}

/*!
    Removes \a count rows at row \a row.

    \sa removeRow(), removeColumn()
*/
void QStandardItem::removeRows(int row, int count)
{
    Q_D(QStandardItem);
    if ((count < 1) || (row < 0) || ((row + count) > rowCount()))
        return;
    if (d->model)
        d->model->d_func()->rowsAboutToBeRemoved(this, row, row + count - 1);
    int i = d->childIndex(row, 0);
    int n = count * d->columnCount();
    for (int j = i; j < n+i; ++j) {
        QStandardItem *oldItem = d->children.at(j);
        if (oldItem)
            oldItem->d_func()->setModel(0);
        delete oldItem;
    }
    d->children.remove(qMax(i, 0), n);
    d->rows -= count;
    if (d->model)
        d->model->d_func()->rowsRemoved(this, row, count);
}

/*!
    Removes \a count columns at column \a column.

    \sa removeColumn(), removeRows()
*/
void QStandardItem::removeColumns(int column, int count)
{
    Q_D(QStandardItem);
    if ((count < 1) || (column < 0) || ((column + count) > columnCount()))
        return;
    if (d->model)
        d->model->d_func()->columnsAboutToBeRemoved(this, column, column + count - 1);
    for (int row = d->rowCount() - 1; row >= 0; --row) {
        int i = d->childIndex(row, column);
        for (int j=i; j<i+count; ++j) {
            QStandardItem *oldItem = d->children.at(j);
            if (oldItem)
                oldItem->d_func()->setModel(0);
            delete oldItem;
        }
        d->children.remove(i, count);
    }
    d->columns -= count;
    if (d->model)
        d->model->d_func()->columnsRemoved(this, column, count);
}

/*!
    Returns true if this item has any children; otherwise returns false.

    \sa rowCount(), columnCount(), child()
*/
bool QStandardItem::hasChildren() const
{
    return (rowCount() > 0) && (columnCount() > 0);
}

/*!
    Sets the child item at \a(row, column) to \a item. This item (the parent
    item) takes ownership of \a item. If necessary, the row count and column
    count are increased to fit the item.

    \sa child()
*/
void QStandardItem::setChild(int row, int column, QStandardItem *item)
{
    Q_D(QStandardItem);
    if ((row < 0) || (column < 0))
        return;
    if (rowCount() <= row)
        setRowCount(row + 1);
    if (columnCount() <= column)
        setColumnCount(column + 1);
    int index = d->childIndex(row, column);
    Q_ASSERT(index != -1);
    QStandardItem *oldItem = d->children.at(index);
    if (item == oldItem)
        return;
    if (item) {
        if (item->parent() == 0) {
            item->d_func()->setParentAndModel(this, d->model);
        } else {
            qWarning("QStandardItem::setChild(): ignoring duplicate insertion of item %p",
                     item);
            return;
        }
    }
    if (oldItem)
        oldItem->d_func()->setModel(0);
    delete oldItem;
    d->children.replace(index, item);
    if (d->model)
        d->model->d_func()->itemChanged(item);
}

/*!
    \fn QStandardItem::setChild(int row, QStandardItem *item)
    \overload
*/

/*!
    Returns the child item at \a(row, column) if one has been set; otherwise
    returns 0.

    \sa setChild(), takeChild(), parent()
*/
QStandardItem *QStandardItem::child(int row, int column) const
{
    Q_D(const QStandardItem);
    int index = d->childIndex(row, column);
    if (index == -1)
        return 0;
    return d->children.at(index);
}

/*!
    Returns true if this item is a top-level item; otherwise returns false.

    \sa parent(), model()
*/
bool QStandardItem::isTopLevelItem() const
{
    Q_D(const QStandardItem);
    if (d->model)
        return (d->model->topLevelParent() == d->parent);
    return (0 == d->parent);
}

/*!
    Removes the child item at \a(row, column) without deleting it.

    \sa child(), takeRow(), takeColumn()
*/
QStandardItem *QStandardItem::takeChild(int row, int column)
{
    Q_D(QStandardItem);
    QStandardItem *item = 0;
    int index = d->childIndex(row, column);
    if (index != -1) {
        item = d->children.at(index);
        d->children.replace(index, 0);
        if (item)
            item->d_func()->setParentAndModel(0, 0);
    }
    return item;
}

/*!
    Removes \a row without deleting the row items.

    \sa removeRow(), insertRow(), takeColumn()
*/
QList<QStandardItem*> QStandardItem::takeRow(int row)
{
    Q_D(QStandardItem);
    if ((row < 0) || (row >= rowCount()))
        return QList<QStandardItem*>();
    QList<QStandardItem*> items;
    int index = d->childIndex(row, 0);
    for (int column = 0; column < d->columnCount(); ++column) {
        QStandardItem *ch = d->children.at(index);
        if (ch) {
            ch->d_func()->setParentAndModel(0, 0);
            d->children.replace(index, 0);
        }
        items.append(ch);
        ++index;
    }
    removeRow(row);
    return items;
}

/*!
    Removes \a column without deleting the column items.

    \sa removeColumn(), insertColumn(), takeRow()
*/
QList<QStandardItem*> QStandardItem::takeColumn(int column)
{
    Q_D(QStandardItem);
    if ((column < 0) || (column >= columnCount()))
        return QList<QStandardItem*>();
    QList<QStandardItem*> items;
    int index = d->childIndex(0, column);
    for (int row = 0; row < d->rowCount(); ++row) {
        QStandardItem *ch = d->children.at(index);
        if (ch) {
            ch->d_func()->setParentAndModel(0, 0);
            d->children.replace(index, 0);
        }
        items.append(ch);
        index += d->columnCount();
    }
    removeColumn(column);
    return items;
}

/*!
    \since 4.2

    Removes the horizontal header item at \a column from the header without
    deleting it. The model releases ownership of the item.

    \sa horizontalHeaderItem(), takeVerticalHeaderItem()
*/
QStandardItem *QStandardItemModel::takeHorizontalHeaderItem(int column)
{
    Q_D(QStandardItemModel);
    if ((column < 0) || (column >= columnCount()))
        return 0;
    QStandardItem *headerItem = d->columnHeaderItems.at(column);
    if (headerItem) {
        headerItem->d_func()->setParentAndModel(0, 0);
        d->columnHeaderItems.replace(column, 0);
    }
    return headerItem;
}

/*!
    \since 4.2

    Removes the vertical header item at \a row from the header without
    deleting it. The model releases ownership of the item.

    \sa verticalHeaderItem(), takeHorizontalHeaderItem()
*/
QStandardItem *QStandardItemModel::takeVerticalHeaderItem(int row)
{
    Q_D(QStandardItemModel);
    if ((row < 0) || (row >= rowCount()))
        return 0;
    QStandardItem *headerItem = d->rowHeaderItems.at(row);
    if (headerItem) {
        headerItem->d_func()->setParentAndModel(0, 0);
        d->rowHeaderItems.replace(row, 0);
    }
    return headerItem;
}

/*!
    Returns true if this item is less than \a other; otherwise returns false.
*/
bool QStandardItem::operator<(const QStandardItem &other) const
{
    const QVariant l = data(Qt::DisplayRole), r = other.data(Qt::DisplayRole);
    // this code is copied from QSortFilterProxyModel::lessThan()
    switch (l.type()) {
    case QVariant::Int:
        return l.toInt() < r.toInt();
    case QVariant::UInt:
        return l.toUInt() < r.toUInt();
    case QVariant::LongLong:
        return l.toLongLong() < r.toLongLong();
    case QVariant::ULongLong:
        return l.toULongLong() < r.toULongLong();
    case QVariant::Double:
        return l.toDouble() < r.toDouble();
    case QVariant::Char:
        return l.toChar() < r.toChar();
    case QVariant::Date:
        return l.toDate() < r.toDate();
    case QVariant::Time:
        return l.toTime() < r.toTime();
    case QVariant::DateTime:
        return l.toDateTime() < r.toDateTime();
    case QVariant::String:
    default:
        return l.toString().compare(r.toString()) < 0;
    }
}

/*!
    Returns a copy of this item.

    \sa QStandardItemModel::itemPrototype()
*/
QStandardItem *QStandardItem::clone() const
{
    return new QStandardItem(*this);
}

/*!
  \internal
*/
void QStandardItemPrivate::setItemData(const QMap<int, QVariant> &roles)
{
    values.clear();
    QMap<int, QVariant>::const_iterator it;
    for (it = roles.begin(); it != roles.end(); ++it) {
        int role = it.key();
        role = (role == Qt::EditRole) ? Qt::DisplayRole : role;
        values.append(QWidgetItemData(role, it.value()));
    }
}

/*!
  \internal
*/
const QMap<int, QVariant> QStandardItemPrivate::itemData() const
{
    QMap<int, QVariant> result;
    QVector<QWidgetItemData>::const_iterator it;
    for (it = values.begin(); it != values.end(); ++it)
        result.insert((*it).role, (*it).value);
    return result;
}

#ifndef QT_NO_DATASTREAM

/*!
    Reads the item from stream \a in. Only the data and flags of the item are
    read, not the child items.

    \sa write()
*/
void QStandardItem::read(QDataStream &in)
{
    Q_D(QStandardItem);
    in >> d->values;
    qint32 flags;
    in >> flags;
    d->flags = Qt::ItemFlags(flags);
}

/*!
    Writes the item to stream \a out. Only the data and flags of the item
    are written, not the child items.

    \sa read()
*/
void QStandardItem::write(QDataStream &out) const
{
    Q_D(const QStandardItem);
    out << d->values;
    out << qint32(d->flags);
}

/*!
    \relates QStandardItem

    Reads a QStandardItem from stream \a in into \a item.

    This operator uses QStandardItem::read().

    \sa {Format of the QDataStream Operators}
*/
QDataStream &operator>>(QDataStream &in, QStandardItem &item)
{
    item.read(in);
    return in;
}

/*!
    \relates QStandardItem

    Writes the QStandardItem \a item to stream \a out.

    This operator uses QStandardItem::write().

    \sa {Format of the QDataStream Operators}
*/
QDataStream &operator<<(QDataStream &out, const QStandardItem &item)
{
    item.write(out);
    return out;
}

#endif // !QT_NO_DATASTREAM

/*!
    Constructs a new item model with the given \a parent.
*/
QStandardItemModel::QStandardItemModel(QObject *parent)
    : QAbstractItemModel(*new QStandardItemModelPrivate, parent)
{
    Q_D(QStandardItemModel);
    d->root->d_func()->setModel(this);
}

/*!
    Constructs a new item model with \a rows rows and \a columns columns, and
    with the given \a parent.
*/
QStandardItemModel::QStandardItemModel(int rows, int columns, QObject *parent)
    : QAbstractItemModel(*new QStandardItemModelPrivate, parent)
{
    Q_D(QStandardItemModel);
    d->root->d_func()->setModel(this);
    d->root->setRowCount(rows);
    d->root->setColumnCount(columns);
}

/*!
    \since 4.2

    Constructs a new item model with a single column, and with the given \a
    parent. \a items are added as rows of the model.
*/
QStandardItemModel::QStandardItemModel(const QList<QStandardItem*> &items, QObject *parent)
    : QAbstractItemModel(*new QStandardItemModelPrivate, parent)
{
    Q_D(QStandardItemModel);
    d->root->d_func()->setModel(this); 
    d->root->setColumnCount(1);
    d->root->setRowCount(items.count());
    for (int i = 0; i < items.count(); ++i)
        d->root->setChild(i, items.at(i));
}

/*!
  \internal
*/
QStandardItemModel::QStandardItemModel(QStandardItemModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
}

/*!
    Destructs the model.
*/
QStandardItemModel::~QStandardItemModel()
{
}

/*!
    Removes all items from the model and resets both the horizontal
    and vertical headers.

    \sa removeColumns(), removeRows()
*/
void QStandardItemModel::clear()
{
    Q_D(QStandardItemModel);
    delete d->root;
    d->root = new QStandardItem;
    d->root->d_func()->setModel(this);
    qDeleteAll(d->columnHeaderItems);
    d->columnHeaderItems.clear();
    qDeleteAll(d->rowHeaderItems);
    d->rowHeaderItems.clear();
    reset();
}

/*!
    \since 4.2

    Returns a pointer to the QStandardItem associated with the given \a index.

    Note that this function will lazily create an item for the index if no
    item already exists at the index (but only if the index's row and column
    are without the parent item's bounds).

    \sa indexFromItem()
*/
QStandardItem *QStandardItemModel::itemFromIndex(const QModelIndex &index) const
{
    Q_D(const QStandardItemModel);
    if (!index.isValid())
        return d->root;
    QStandardItem *parent = static_cast<QStandardItem*>(index.internalPointer());
    QStandardItem *child = parent->child(index.row(), index.column());
    if (child == 0) {
        // create lazily
        child = d->createItem();
        parent->setChild(index.row(), index.column(), child);
    }
    return child;
}

/*!
    \since 4.2

    Returns the QModelIndex associated with the given \a item.

    \sa itemFromIndex(), QStandardItem::index()
*/
QModelIndex QStandardItemModel::indexFromItem(const QStandardItem *item) const
{
    if (item && item->parent())
        return createIndex(item->row(), item->column(), item->parent());
    return QModelIndex();
}

/*!
    \since 4.2

    Sets the number of rows in this model to \a rows. If
    this is less than rowCount(), the data in the unwanted rows
    is discarded.

    \sa setColumnCount()
*/
void QStandardItemModel::setRowCount(int rows)
{
    Q_D(QStandardItemModel);
    d->root->setRowCount(rows);
}

/*!
    \since 4.2

    Sets the number of columns in this model to \a columns. If
    this is less than columnCount(), the data in the unwanted columns
    is discarded.

    \sa setRowCount()
*/
void QStandardItemModel::setColumnCount(int columns)
{
    Q_D(QStandardItemModel);
    d->root->setColumnCount(columns);
}

/*!
    \since 4.2

    Sets the item for the given \a row and \a column to \a item. The model
    takes ownership of the item. If necessary, the row count and column count
    are increased to fit the item.

    \sa item()
*/
void QStandardItemModel::setItem(int row, int column, QStandardItem *item)
{
    Q_D(QStandardItemModel);
    d->root->setChild(row, column, item);
}

/*!
  \fn QStandardItemModel::setItem(int row, QStandardItem *item)
  \overload
*/

/*!
    \since 4.2

    Returns the item for the given \a row and \a column if one has been set;
    otherwise returns 0.

    \sa setItem(), itemFromIndex()
*/
QStandardItem *QStandardItemModel::item(int row, int column) const
{
    Q_D(const QStandardItemModel);
    return d->root->child(row, column);
}

/*!
    \since 4.2

    Returns the model's top-level parent item. This item is the parent of
    the top-level items of the model (e.g. items accessed with item() and
    setItem()).
*/
QStandardItem *QStandardItemModel::topLevelParent() const
{
    Q_D(const QStandardItemModel);
    return d->root;
}

/*!
    \since 4.2

    Sets the horizontal header item for \a column to \a item.
    The model takes ownership of the item. If necessary, the
    column count is increased to fit the item.

    \sa setVerticalHeaderItem(), setHorizontalHeaderLabels(), horizontalHeaderItem()
*/
void QStandardItemModel::setHorizontalHeaderItem(int column, QStandardItem *item)
{
    Q_D(QStandardItemModel);
    if (column < 0)
        return;
    if (columnCount() <= column)
        setColumnCount(column + 1);

    QStandardItem *oldItem = d->columnHeaderItems.at(column);
    if (item == oldItem)
        return;

    if (item) {
        if (item->model() == 0) {
            item->d_func()->setModel(this);
        } else {
            qWarning("QStandardItem::setHorizontalHeaderItem(): ignoring duplicate insertion of item %p",
                     item);
            return;
        }
    }

    if (oldItem)
        oldItem->d_func()->setModel(0);
    delete oldItem;

    d->columnHeaderItems.replace(column, item);
    emit headerDataChanged(Qt::Horizontal, column, column);
}

/*!
    \since 4.2

    Returns the horizontal header item for \a column if one has been
    set; otherwise returns 0.

    \sa setHorizontalHeaderItem(), verticalHeaderItem()
*/
QStandardItem *QStandardItemModel::horizontalHeaderItem(int column) const
{
    Q_D(const QStandardItemModel);
    if ((column < 0) || (column >= columnCount()))
        return 0;
    return d->columnHeaderItems.at(column);
}

/*!
    \since 4.2

    Sets the vertical header item for \a row to \a item.
    The model takes ownership of the item. If necessary, the
    row count is increased to fit the item.

    \sa setVerticalHeaderLabels(), setHorizontalHeaderItem(), verticalHeaderItem()
*/
void QStandardItemModel::setVerticalHeaderItem(int row, QStandardItem *item)
{
    Q_D(QStandardItemModel);
    if (row < 0)
        return;
    if (rowCount() <= row)
        setRowCount(row + 1);

    QStandardItem *oldItem = d->rowHeaderItems.at(row);
    if (item == oldItem)
        return;

    if (item) {
        if (item->model() == 0) {
            item->d_func()->setModel(this);
        } else {
            qWarning("QStandardItem::setVerticalHeaderItem(): ignoring duplicate insertion of item %p",
                     item);
            return;
        }
    }

    if (oldItem)
        oldItem->d_func()->setModel(0);
    delete oldItem;

    d->rowHeaderItems.replace(row, item);
    emit headerDataChanged(Qt::Vertical, row, row);
}

/*!
    \since 4.2

    Returns the vertical header item for row \a row if one has been set;
    otherwise returns 0.

    \sa setVerticalHeaderItem(), horizontalHeaderItem()
*/
QStandardItem *QStandardItemModel::verticalHeaderItem(int row) const
{
    Q_D(const QStandardItemModel);
    if ((row < 0) || (row >= rowCount()))
        return 0;
    return d->rowHeaderItems.at(row);
}

/*!
    \since 4.2

    Sets the horizontal header labels using \a labels. If necessary, the
    column count is increased to the size of \a labels.

    \sa setHorizontalHeaderItem()
*/
void QStandardItemModel::setHorizontalHeaderLabels(const QStringList &labels)
{
    Q_D(QStandardItemModel);
    if (columnCount() < labels.count())
        setColumnCount(labels.count());
    for (int i = 0; i < labels.count(); ++i) {
        QStandardItem *item = horizontalHeaderItem(i);
        if (!item) {
            item = d->createItem();
            setHorizontalHeaderItem(i, item);
        }
        item->setText(labels.at(i));
    }
}

/*!
    \since 4.2

    Sets the vertical header labels using \a labels. If necessary, the row
    count is increased to the size of \a labels.

    \sa setVerticalHeaderItem()
*/
void QStandardItemModel::setVerticalHeaderLabels(const QStringList &labels)
{
    Q_D(QStandardItemModel);
    if (rowCount() < labels.count())
        setRowCount(labels.count());
    for (int i = 0; i < labels.count(); ++i) {
        QStandardItem *item = verticalHeaderItem(i);
        if (!item) {
            item = d->createItem();
            setVerticalHeaderItem(i, item);
        }
        item->setText(labels.at(i));
    }
}

/*!
    \since 4.2

    Sets the item prototype for the model to the specified \a item.

    The item prototype acts as a QStandardItem factory, by relying on its
    clone() function.  When subclassing QStandardItem, you can set the
    prototype to an instance of your custom class, so that items created
    internally by QStandardItemModel will be instances of that class.

    \sa itemPrototype(), itemFromIndex()
*/
void QStandardItemModel::setItemPrototype(const QStandardItem *item)
{
    Q_D(QStandardItemModel);
    d->itemPrototype = item;
}

/*!
    \since 4.2

    Returns the item prototype used by the model.

    \sa setItemPrototype()
*/
const QStandardItem *QStandardItemModel::itemPrototype() const
{
    Q_D(const QStandardItemModel);
    return d->itemPrototype;
}

/*!
    \since 4.2

    Returns a list of items that match the given \a text, using the given \a
    flags, in the given \a column.
*/
QList<QStandardItem*> QStandardItemModel::findItems(const QString &text, Qt::MatchFlags flags, int column) const
{
    QModelIndexList indexes = match(index(0, column, QModelIndex()), Qt::DisplayRole, text, -1, flags);
    QList<QStandardItem*> items;
    for (int i = 0; i < indexes.size(); ++i)
        items.append(itemFromIndex(indexes.at(i)));
    return items;
}

/*!
    \since 4.2
    \fn void QStandardItemModel::appendRow(const QList<QStandardItem*> &items)

    Appends a row containing \a items. If necessary, the column count is
    increased to the size of \a items.

    \sa insertRow(), appendColumn()
*/
void QStandardItemModel::appendRow(const QList<QStandardItem*> &items)
{
    topLevelParent()->appendRow(items);
}

/*!
    \since 4.2
    \fn void QStandardItemModel::appendColumn(const QList<QStandardItem*> &items)

    Appends a column containing \a items. If necessary, the row count is
    increased to the size of \a items.

    \sa insertColumn(), appendRow()
*/
void QStandardItemModel::appendColumn(const QList<QStandardItem*> &items)
{
    topLevelParent()->appendColumn(items);
}

/*!
    \since 4.2
    \fn QStandardItemModel::appendRow(int row, QStandardItem *item)
    \overload

    When building a list or a tree that has only one column, this function
    provides a convenient way to append a single new item.
*/

/*!
    \since 4.2

    Removes the item at \a(row, column) without deleting it. The model
    releases ownership of the item.

    \sa item(), takeRow(), takeColumn()
*/
QStandardItem *QStandardItemModel::takeItem(int row, int column)
{
    Q_D(QStandardItemModel);
    return d->root->takeChild(row, column);
}

/*!
    \since 4.2

    Removes the given \a row without deleting the row items. The model
    releases ownership of the items.

    \sa takeColumn()
*/
QList<QStandardItem*> QStandardItemModel::takeRow(int row)
{
    Q_D(QStandardItemModel);
    return d->root->takeRow(row);
}

/*!
    \since 4.2

    Removes the given \a column without deleting the column items. The model
    releases ownership of the items.

    \sa takeRow()
*/
QList<QStandardItem*> QStandardItemModel::takeColumn(int column)
{
    Q_D(QStandardItemModel);
    return d->root->takeColumn(column);
}

/*!
  \reimp
*/
int QStandardItemModel::columnCount(const QModelIndex &parent) const
{
    QStandardItem *item = itemFromIndex(parent);
    return item->columnCount();
}

/*!
  \reimp
*/
QVariant QStandardItemModel::data(const QModelIndex &index, int role) const
{
    QStandardItem *item = itemFromIndex(index);
    return item->data(role);
}

/*!
  \reimp
*/
Qt::ItemFlags QStandardItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;
    QStandardItem *item = itemFromIndex(index);
    return item->flags();
}

/*!
  \reimp
*/
bool QStandardItemModel::hasChildren(const QModelIndex &parent) const
{
    QStandardItem *item = itemFromIndex(parent);
    return item->hasChildren();
}

/*!
  \reimp
*/
QVariant QStandardItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const QStandardItemModel);
    if ((section < 0)
        || ((orientation == Qt::Horizontal) && (section >= columnCount()))
        || ((orientation == Qt::Vertical) && (section >= rowCount()))) {
        return QVariant();
    }
    QStandardItem *headerItem = 0;
    if (orientation == Qt::Horizontal)
        headerItem = d->columnHeaderItems.at(section);
    else if (orientation == Qt::Vertical)
        headerItem = d->rowHeaderItems.at(section);
    return headerItem ? headerItem->data(role) : QAbstractItemModel::headerData(section, orientation, role);
}

/*!
  \reimp
*/
QModelIndex QStandardItemModel::index(int row, int column, const QModelIndex &parent) const
{
    QStandardItem *parentItem = itemFromIndex(parent);
    if ((row < 0)
        || (column < 0)
        || (row >= parentItem->rowCount())
        || (column >= parentItem->columnCount())) {
        return QModelIndex();
    }
    return createIndex(row, column, parentItem);
}

/*!
  \reimp
*/
bool QStandardItemModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    QStandardItem *item = itemFromIndex(parent);
    return item->d_func()->insertColumns(column, count, QList<QStandardItem*>());
}

/*!
  \reimp
*/
bool QStandardItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    QStandardItem *item = itemFromIndex(parent);
    return item->d_func()->insertRows(row, count, QList<QStandardItem*>());
}

/*!
  \reimp
*/
QMap<int, QVariant> QStandardItemModel::itemData(const QModelIndex &index) const
{
    QStandardItem *item = itemFromIndex(index);
    return item->d_func()->itemData();
}

/*!
  \reimp
*/
QModelIndex QStandardItemModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
    QStandardItem *parentItem = static_cast<QStandardItem*>(child.internalPointer());
    return indexFromItem(parentItem);
}

/*!
  \reimp
*/
bool QStandardItemModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    QStandardItem *item = itemFromIndex(parent);
    if ((count < 1) || (column < 0) || ((column + count) > item->columnCount()))
        return false;
    item->removeColumns(column, count);
    return true;
}

/*!
  \reimp
*/
bool QStandardItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    QStandardItem *item = itemFromIndex(parent);
    if ((count < 1) || (row < 0) || ((row + count) > item->rowCount()))
        return false;
    item->removeRows(row, count);
    return true;
}

/*!
  \reimp
*/
int QStandardItemModel::rowCount(const QModelIndex &parent) const
{
    QStandardItem *item = itemFromIndex(parent);
    return item->rowCount();
}

/*!
  \reimp
*/
bool QStandardItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    QStandardItem *item = itemFromIndex(index);
    item->setData(role, value);
    return true;
}

/*!
  \reimp
*/
bool QStandardItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    Q_D(QStandardItemModel);
    if ((section < 0)
        || ((orientation == Qt::Horizontal) && (section >= columnCount()))
        || ((orientation == Qt::Vertical) && (section >= rowCount()))) {
        return false;
    }
    QStandardItem *headerItem = 0;
    if (orientation == Qt::Horizontal) {
        headerItem = d->columnHeaderItems.at(section);
        if (headerItem == 0) {
            headerItem = d->createItem();
            headerItem->d_func()->setModel(this);
            d->columnHeaderItems.replace(section, headerItem);
        }
    } else if (orientation == Qt::Vertical) {
        headerItem = d->rowHeaderItems.at(section);
        if (headerItem == 0) {
            headerItem = d->createItem();
            headerItem->d_func()->setModel(this);
            d->rowHeaderItems.replace(section, headerItem);
        }
    }
    if (headerItem) {
        headerItem->setData(role, value);
        return true;
    }
    return false;
}

/*!
  \reimp
*/
bool QStandardItemModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    QStandardItem *item = itemFromIndex(index);
    item->d_func()->setItemData(roles);
    return true;
}

/*!
  \reimp
*/
void QStandardItemModel::sort(int column, Qt::SortOrder order)
{
    Q_D(QStandardItemModel);
    if (column < 0)
        return;
    emit layoutAboutToBeChanged();
    d->sort(d->root, column, order);
    emit layoutChanged();
}

/*!
  \internal
*/
void QStandardItemModelPrivate::sort(QStandardItem *parent, int column, Qt::SortOrder order)
{
    Q_Q(QStandardItemModel);
    if (!parent || (column >= parent->columnCount()))
        return;

    QModelIndexList oldPersistentIndexes = q->persistentIndexList();

    QVector<QPair<QStandardItem*, int> > sortable;
    QVector<int> unsortable;

    sortable.reserve(parent->rowCount());
    unsortable.reserve(parent->rowCount());

    for (int row = 0; row < parent->rowCount(); ++row) {
        QStandardItem *itm = parent->child(row, column);
        if (itm)
            sortable.append(QPair<QStandardItem*,int>(itm, row));
        else
            unsortable.append(row);
    }

    if (order == Qt::AscendingOrder) {
        QStandardItemModelLessThan lt;
        qSort(sortable.begin(), sortable.end(), lt);
    } else {
        QStandardItemModelGreaterThan gt;
        qSort(sortable.begin(), sortable.end(), gt);
    }

    QVector<QPair<QModelIndex, QModelIndex> > changedPersistentIndexes;
    QVector<QStandardItem*> sorted_children(parent->d_func()->children.count());
    for (int i = 0; i < q->rowCount(); ++i) {
        int r = (i < sortable.count()
                 ? sortable.at(i).second
                 : unsortable.at(i - sortable.count()));
        for (int c = 0; c < q->columnCount(); ++c) {
            QStandardItem *itm = parent->child(r, c);
            sorted_children[parent->d_func()->childIndex(i, c)] = itm;
            QModelIndex from = createIndex(r, c, parent);
            if (oldPersistentIndexes.contains(from)) {
                QModelIndex to = createIndex(i, c, parent);
                changedPersistentIndexes.append(
                    QPair<QModelIndex, QModelIndex>(from, to));
            }
        }
    }

    parent->d_func()->children = sorted_children;

    QPair<QModelIndex, QModelIndex> indexPair;
    foreach (indexPair, changedPersistentIndexes)
        q->changePersistentIndex(indexPair.first, indexPair.second);

    QVector<QStandardItem*>::iterator it;
    for (it = sorted_children.begin(); it != sorted_children.end(); ++it)
        sort(*it, column, order);
}

/*!
  \internal
*/
void QStandardItemModelPrivate::itemChanged(QStandardItem *item)
{
    Q_Q(QStandardItemModel);
    if (item->parent() == 0) {
        // Header item
        int idx = columnHeaderItems.indexOf(item);
        if (idx != -1) {
            emit q->headerDataChanged(Qt::Horizontal, idx, idx);
        } else {
            idx = rowHeaderItems.indexOf(item);
            if (idx != -1)
                emit q->headerDataChanged(Qt::Vertical, idx, idx);
        }
    } else {
        // Normal item
        QModelIndex index = q->indexFromItem(item);
        emit q->dataChanged(index, index);
    }
}

/*!
  \internal
*/
void QStandardItemModelPrivate::rowsAboutToBeInserted(QStandardItem *parent, int start, int end)
{
    Q_Q(QStandardItemModel);
    QModelIndex index = q->indexFromItem(parent);
    q->beginInsertRows(index, start, end);
}

/*!
  \internal
*/
void QStandardItemModelPrivate::columnsAboutToBeInserted(QStandardItem *parent, int start, int end)
{
    Q_Q(QStandardItemModel);
    QModelIndex index = q->indexFromItem(parent);
    q->beginInsertColumns(index, start, end);
}

/*!
  \internal
*/
void QStandardItemModelPrivate::rowsAboutToBeRemoved(QStandardItem *parent, int start, int end)
{
    Q_Q(QStandardItemModel);
    QModelIndex index = q->indexFromItem(parent);
    q->beginRemoveRows(index, start, end);
}

/*!
  \internal
*/
void QStandardItemModelPrivate::columnsAboutToBeRemoved(QStandardItem *parent, int start, int end)
{
    Q_Q(QStandardItemModel);
    QModelIndex index = q->indexFromItem(parent);
    q->beginRemoveColumns(index, start, end);
}

/*!
  \internal
*/
void QStandardItemModelPrivate::rowsInserted(QStandardItem *parent, int row, int count)
{
    Q_Q(QStandardItemModel);
    if (parent == root)
        rowHeaderItems.insert(row, count, 0);
    q->endInsertRows();
}

/*!
  \internal
*/
void QStandardItemModelPrivate::columnsInserted(QStandardItem *parent, int column, int count)
{
    Q_Q(QStandardItemModel);
    if (parent == root)
        columnHeaderItems.insert(column, count, 0);
    q->endInsertColumns();
}

/*!
  \internal
*/
void QStandardItemModelPrivate::rowsRemoved(QStandardItem *parent, int row, int count)
{
    Q_Q(QStandardItemModel);
    if (parent == root) {
        for (int i = row; i < row + count; ++i) {
            QStandardItem *oldItem = rowHeaderItems.at(i);
            if (oldItem)
                oldItem->d_func()->setModel(0);
            delete oldItem;
        }
        rowHeaderItems.remove(row, count);
    }
    q->endRemoveRows();
}

/*!
  \internal
*/
void QStandardItemModelPrivate::columnsRemoved(QStandardItem *parent, int column, int count)
{
    Q_Q(QStandardItemModel);
    if (parent == root) {
        for (int i = column; i < column + count; ++i) {
            QStandardItem *oldItem = columnHeaderItems.at(i);
            if (oldItem)
                oldItem->d_func()->setModel(0);
            delete oldItem;
        }
        columnHeaderItems.remove(column, count);
    }
    q->endRemoveColumns();
}

#endif // QT_NO_STANDARDITEMMODEL
