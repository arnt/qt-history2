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

#include "qtablewidget.h"
#include <qheaderview.h>
#include <qitemdelegate.h>
#include <qpainter.h>
#include <qabstractitemmodel.h>
#include <private/qabstractitemmodel_p.h>
#include <private/qtableview_p.h>

// workaround for VC++ 6.0 linker bug (?)
typedef bool(*LessThan)(const QTableWidgetItem *left, const QTableWidgetItem *right);

class QTableModel : public QAbstractTableModel
{
public:
    QTableModel(int rows, int columns, QTableWidget *parent);
    ~QTableModel();

    bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool insertColumns(int column, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool removeColumns(int column, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    void setItem(int row, int column, QTableWidgetItem *item);
    void setItem(const QModelIndex &index, QTableWidgetItem *item);
    QTableWidgetItem *takeItem(int row, int column);
    QTableWidgetItem *item(int row, int column) const;
    QTableWidgetItem *item(const QModelIndex &index) const;
    void removeItem(QTableWidgetItem *item);

    void setHorizontalHeaderItem(int section, QTableWidgetItem *item);
    void setVerticalHeaderItem(int section, QTableWidgetItem *item);
    QTableWidgetItem *horizontalHeaderItem(int section);
    QTableWidgetItem *verticalHeaderItem(int section);

    QModelIndex index(const QTableWidgetItem *item) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null) const;

    void setRowCount(int rows);
    void setColumnCount(int columns);

    int rowCount() const;
    int columnCount() const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, int role, const QVariant &value);

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

    bool isSortable() const;
    void sort(int column, const QModelIndex &parent, Qt::SortOrder order);

    static bool lessThan(const QTableWidgetItem *left, const QTableWidgetItem *right);
    static bool greaterThan(const QTableWidgetItem *left, const QTableWidgetItem *right);

    bool isValid(const QModelIndex &index) const;
    inline long tableIndex(int row, int column) const
        {  return (row * horizontal.count()) + column; }

    void clear();
    void itemChanged(QTableWidgetItem *item);

private:
    QVector<QTableWidgetItem*> table;
    QVector<QTableWidgetItem*> vertical;
    QVector<QTableWidgetItem*> horizontal;
    mutable QChar strbuf[65];
};

QTableModel::QTableModel(int rows, int columns, QTableWidget *parent)
    : QAbstractTableModel(parent),
      table(rows * columns), vertical(rows), horizontal(columns) {}

QTableModel::~QTableModel()
{
    clear();
}

bool QTableModel::insertRows(int row, const QModelIndex &, int count)
{
    vertical.insert(row, count, 0);
    int i = tableIndex(row, qMax(horizontal.count() - 1, 0));
    table.insert(i, count * horizontal.count(), 0);
    emit rowsInserted(QModelIndex::Null, row, row + count - 1);
    return true;
}

bool QTableModel::insertColumns(int column, const QModelIndex &, int count)
{
    int cc = horizontal.count();
    horizontal.insert(column, count, 0);
    if (cc == 0)
        table.resize(vertical.count() * count);
    else
        for (int row = 0; row < vertical.count(); ++row)
            table.insert(tableIndex(row, column), count, 0);
    emit columnsInserted(QModelIndex::Null, column, column + count - 1);
    return true;
}

bool QTableModel::removeRows(int row, const QModelIndex &, int count)
{
    emit rowsRemoved(QModelIndex::Null, row, row + count - 1);
    int i = tableIndex(row, columnCount() - 1);
    table.remove(qMax(i, 0), count * columnCount());
    vertical.remove(row);
    return true;
}

bool QTableModel::removeColumns(int column, const QModelIndex &, int count)
{
    emit columnsRemoved(QModelIndex::Null, column, column + count - 1);
    for (int row = 0; row < rowCount(); ++row)
        table.remove(tableIndex(row, column));
    horizontal.remove(column);
    return true;
}

void QTableModel::setItem(int row, int column, QTableWidgetItem *item)
{
    item->model = this;
    int i = tableIndex(row, column);
    if (i >= 0 && i < table.count())
        table[i] = item;
}

void QTableModel::setItem(const QModelIndex &index, QTableWidgetItem *item)
{
    if (!isValid(index))
        return;

    long i = tableIndex(index.row(), index.column());
    delete table.at(i);
    table[i] = item;
}

QTableWidgetItem *QTableModel::takeItem(int row, int column)
{
    long i = tableIndex(row, column);
    QTableWidgetItem *itm = table.value(i);
    if (itm) {
        itm->model = 0;
        table[i] = 0;
    }
    return itm;
}

QTableWidgetItem *QTableModel::item(int row, int column) const
{
    return table.value(tableIndex(row, column));
}

QTableWidgetItem *QTableModel::item(const QModelIndex &index) const
{
    if (!isValid(index))
        return 0;
    return table.at(tableIndex(index.row(), index.column()));
}

void QTableModel::removeItem(QTableWidgetItem *item)
{
    int i = table.indexOf(item);
    if (i != -1) {
        table[i] = 0;
        return;
    }

    i = vertical.indexOf(item);
    if (i != -1) {
        vertical[i] = 0;
        return;
    }

    i = horizontal.indexOf(item);
    if (i != -1) {
        horizontal[i] = 0;
        return;
    }
}

void QTableModel::setHorizontalHeaderItem(int section, QTableWidgetItem *item)
{
    Q_ASSERT(item);
    item->model = this;
    vertical[section] = item;
}


void QTableModel::setVerticalHeaderItem(int section, QTableWidgetItem *item)
{
    Q_ASSERT(item);
    item->model = this;
    horizontal[section] = item;
}

QTableWidgetItem *QTableModel::horizontalHeaderItem(int section)
{
    return horizontal.value(section);
}

QTableWidgetItem *QTableModel::verticalHeaderItem(int section)
{
    return vertical.value(section);
}

QModelIndex QTableModel::index(const QTableWidgetItem *item) const
{
    int i = table.indexOf(const_cast<QTableWidgetItem*>(item));
    int row = i / columnCount();
    int col = i % columnCount();
    return index(row, col);
}

QModelIndex QTableModel::index(int row, int column, const QModelIndex &) const
{
    if (row >= 0 && row < vertical.count() && column >= 0 && column < horizontal.count()) {
        QTableWidgetItem *item = table.at(tableIndex(row, column));
        return createIndex(row, column, item);
    }
    return QModelIndex::Null;
}

void QTableModel::setRowCount(int rows)
{
    int rc = vertical.count();
    if (rc == rows)
        return;
    if (rc < rows)
        insertRows(qMax(rc - 1, 0), QModelIndex::Null, rows - rc);
    else
        removeRows(qMax(rows - 1, 0), QModelIndex::Null, rc - rows);
}

void QTableModel::setColumnCount(int columns)
{
    int cc = horizontal.count();
    if (cc == columns)
        return;
    if (cc < columns)
        insertColumns(qMax(cc - 1, 0), QModelIndex::Null, columns - cc);
    else
        removeColumns(qMax(columns - 1, 0), QModelIndex::Null, cc - columns);
}

int QTableModel::rowCount() const
{
    return vertical.count();
}

int QTableModel::columnCount() const
{
    return horizontal.count();
}

QVariant QTableModel::data(const QModelIndex &index, int role) const
{
    QTableWidgetItem *itm = item(index);
    if (itm)
        return itm->data(role);
    return QVariant();
}

bool QTableModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    QTableWidgetItem *itm = item(index);

    if (itm) {
        itm->setData(role, value);
        return true;
    }
    
    QTableWidget *view = qt_cast<QTableWidget*>(QObject::parent());
    if (!view)
        return false;
    itm = view->createItem();
    itm->setData(role, value);
    view->setItem(index.row(), index.column(), itm);
    return true;
}

QAbstractItemModel::ItemFlags QTableModel::flags(const QModelIndex &index) const
{
    QTableWidgetItem *itm = item(index);
    if (itm)
        return itm->flags();
    return QAbstractItemModel::ItemIsEditable
        |QAbstractItemModel::ItemIsSelectable
        |QAbstractItemModel::ItemIsCheckable
        |QAbstractItemModel::ItemIsEnabled;
}

bool QTableModel::isSortable() const
{
    return true;
}

void QTableModel::sort(int column, const QModelIndex &parent, Qt::SortOrder order)
{
    Q_UNUSED(parent);
    QVector<QTableWidgetItem*> sorting(rowCount());
    for (int i = 0; i < sorting.count(); ++i)
        sorting[i] = item(i, column);
    LessThan compare = order == Qt::AscendingOrder ? &lessThan : &greaterThan;
    qHeapSort(sorting.begin(), sorting.end(), compare);
    for (int j = 0; j < sorting.count(); ++j)
        table[tableIndex(j, column)] = sorting.at(j);
}

bool QTableModel::lessThan(const QTableWidgetItem *left, const QTableWidgetItem *right)
{
    return *left < *right;
}

bool QTableModel::greaterThan(const QTableWidgetItem *left, const QTableWidgetItem *right)
{
    return !(*left < *right);
}

QVariant QTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QTableWidgetItem *itm = 0;
    if (orientation == Qt::Horizontal)
        itm = horizontal.at(section);
    else
        itm = vertical.at(section);
    if (itm)
        return itm->data(role);
    return QAbstractItemModel::headerData(section, orientation, role);
}

bool QTableModel::setHeaderData(int section, Qt::Orientation orientation, int role, const QVariant &value)
{
    QTableWidgetItem *itm = 0;
    if (orientation == Qt::Horizontal)
        itm = horizontal.at(section);
    else
        itm = vertical.at(section);

    if (itm) {
        itm->setData(role, value);
        return true;
    }

    return false;
}

bool QTableModel::isValid(const QModelIndex &index) const
{
    return index.isValid() && index.row() < vertical.count() && index.column() < horizontal.count();
}

void QTableModel::clear()
{
    for (int i = 0; i < table.count(); ++i) {
        if (table.at(i)) {
            table.at(i)->model = 0;
            delete table.at(i);
        }
    }
    for (int j = 0; j < vertical.count(); ++j) {
        if (vertical.at(j)) {
            vertical.at(j)->model = 0;
            delete vertical.at(j);
        }
    }
    for (int k = 0; k < horizontal.count(); ++k) {
        if (horizontal.at(k)) {
            horizontal.at(k)->model = 0;
            delete horizontal.at(k);
        }
    }
    emit reset();
}

void QTableModel::itemChanged(QTableWidgetItem *item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}

// item

QTableWidgetItem::QTableWidgetItem()
    : view(0), model(0),
      itemFlags(QAbstractItemModel::ItemIsEditable
                |QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsCheckable
                |QAbstractItemModel::ItemIsEnabled)
{
}

QTableWidgetItem::QTableWidgetItem(const QString &text)
    : view(0), model(0),
      itemFlags(QAbstractItemModel::ItemIsEditable
                |QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsCheckable
                |QAbstractItemModel::ItemIsEnabled)
{
    setData(QAbstractItemModel::DisplayRole, text);
}

QTableWidgetItem::~QTableWidgetItem()
{
    if (model)
        model->removeItem(this);
}

void QTableWidgetItem::setData(int role, const QVariant &value)
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i) {
        if (values.at(i).role == role) {
            values[i].value = value;
            return;
        }
    }
    values.append(Data(role, value));
    if (model)
        model->itemChanged(this);
}

QVariant QTableWidgetItem::data(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i)
        if (values.at(i).role == role)
            return values.at(i).value;
    return QVariant();
}

bool QTableWidgetItem::operator<(const QTableWidgetItem &other) const
{
    return text() < other.text();
}

void QTableWidgetItem::clear()
{
    values.clear();
    if (model)
        model->itemChanged(this);
}

/*!
    \class QTableWidget qtablewidget.h
    \brief The QTableWidget class provides an item-based table view with a default model.

    \ingroup model-view
    \mainclass

    If you want a table that uses your own data model you should
    use QTableView rather than this class.

    Items are set with setItem(), or with setText() or setIcon();
    these last two are convenience functions that create a QTableItem
    for you. The number of rows is set with setRowCount(), and the
    number of columns with setColumnCount().

    \sa \link model-view-programming.html Model/View Programming\endlink
*/

#define d d_func()
#define q q_func()

class QTableWidgetPrivate : public QTableViewPrivate
{
    Q_DECLARE_PUBLIC(QTableWidget)
public:
    QTableWidgetPrivate() : QTableViewPrivate() {}
    inline QTableModel *model() const { return ::qt_cast<QTableModel*>(q_func()->model()); }
    void emitPressed(const QModelIndex &index, Qt::ButtonState button);
    void emitClicked(const QModelIndex &index, Qt::ButtonState button);
    void emitDoubleClicked(const QModelIndex &index, Qt::ButtonState button);
    void emitKeyPressed(const QModelIndex &index, Qt::Key key, Qt::ButtonState state);
    void emitReturnPressed(const QModelIndex &index);
    void emitCurrentChanged(const QModelIndex &previous, const QModelIndex &current);
    void emitItemEntered(const QModelIndex &index, Qt::ButtonState state);
    void emitAboutToShowContextMenu(QMenu *menu, const QModelIndex &index);
    void emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
};

void QTableWidgetPrivate::emitPressed(const QModelIndex &index, Qt::ButtonState button)
{
    emit q->pressed(model()->item(index), button);
}

void QTableWidgetPrivate::emitClicked(const QModelIndex &index, Qt::ButtonState button)
{
    emit q->clicked(model()->item(index), button);
}

void QTableWidgetPrivate::emitDoubleClicked(const QModelIndex &index, Qt::ButtonState button)
{
    emit q->doubleClicked(model()->item(index), button);
}

void QTableWidgetPrivate::emitKeyPressed(const QModelIndex &index, Qt::Key key,
                                         Qt::ButtonState state)
{
    emit q->keyPressed(model()->item(index), key, state);
}

void QTableWidgetPrivate::emitReturnPressed(const QModelIndex &index)
{
    emit q->returnPressed(model()->item(index));
}

void QTableWidgetPrivate::emitCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    emit q->currentChanged(model()->item(current), model()->item(previous));
}

void QTableWidgetPrivate::emitItemEntered(const QModelIndex &index, Qt::ButtonState state)
{
    emit q->itemEntered(model()->item(index), state);
}

void QTableWidgetPrivate::emitAboutToShowContextMenu(QMenu *menu, const QModelIndex &index)
{
    emit q->aboutToShowContextMenu(menu, model()->item(index));
}

void QTableWidgetPrivate::emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft == bottomRight) // this should always be true, unless we sort
        emit q->itemChanged(model()->item(topLeft));
}

/*!
    \fn void QTableWidget::pressed(QTableWidgetItem *item, Qt::ButtonState button)

    This signal is emitted when a item has been pressed (mouse click
    and release). The \a item may be 0 if the mouse was not pressed on
    an item. The button clicked is specified by \a button (see
    \l{Qt::ButtonState}).
*/

/*!
    \fn void QTableWidget::clicked(QTableWidgetItem *item, Qt::ButtonState button)

    This signal is emitted when a mouse button is clicked. The \a item
    may be 0 if the mouse was not clicked on an item.  The button
    clicked is specified by \a button (see \l{Qt::ButtonState}).
*/

/*!
    \fn void QTableWidget::doubleClicked(QTableWidgetItem *item, Qt::ButtonState button);

    This signal is emitted when a mouse button is double clicked. The
    \a item may be 0 if the mouse was not clicked on an item.  The
    button clicked is specified by \a button (see
    \l{Qt::ButtonState}).
*/

/*!
    \fn void QTableWidget::keyPressed(QTableWidgetItem *item, Qt::Key key, Qt::ButtonState state)

    This signal is emitted if keyTracking is turned on an a key was
    pressed. The \a item is the current item as the key was pressed, the
    \a key tells which key was pressed and \a state which modifier
    keys (see \l{Qt::ButtonState}).
*/

/*!
    \fn void QTableWidget::returnPressed(QTableWidgetItem *item)

    This signal is emitted when return has been pressed on an \a item.
*/

/*!
    \fn void QTableWidget::currentChanged(QTableWidgetItem *current, QTableWidgetItem *previous)

    This signal is emitted whenever the current item changes. The \a
    previous item is the item that previously had the focus, \a
    current is the new current item.
*/

/*!
    \fn void QTableWidget::selectionChanged()

    This signal is emitted whenever the selection changes.

    \sa selectedItems() isSelected()
*/

/*!
    \fn void QTableWidget::itemEntered(QTableWidgetItem *item, Qt::ButtonState state)

    This signal is emitted when the mouse cursor enters an item. The
    \a item is the item entered and \a state specifies the mouse
    button and any modifiers pressed as the item was entered (see
    \l{Qt::ButtonState}). This signal is only emitted when
    mouseTracking is turned on, or when a mouse button is pressed
    while moving into an item.
*/

/*!
    \fn void QTableWidget::aboutToShowContextMenu(QMenu *menu, QTableWidgetItem *item)

    This signal is emitted when the widget is about to show a context
    menu. The \a menu is the menu about to be shown, and the \a item is the
    clicked item the context menu was called for.

    \sa QMenu::addAction()
*/

/*!
    \fn void QTableWidget::itemChanged(QTableWidgetItem *item)

    This signal is emitted whenever the data of \a item has changed.
*/

/*!
    Creates a new table view with the given \a parent.
*/
QTableWidget::QTableWidget(QWidget *parent)
    : QTableView(*new QTableWidgetPrivate, parent)
{
    setModel(new QTableModel(0, 0, this));
    setup();
}

/*!
    Creates a new table view with the given \a rows and \a columns, and with the given \a parent.
*/
QTableWidget::QTableWidget(int rows, int columns, QWidget *parent)
    : QTableView(*new QTableWidgetPrivate, parent)
{
    setModel(new QTableModel(rows, columns, this));
    setup();
}

/*!
    Destroys this QTableWidget.
*/
QTableWidget::~QTableWidget()
{
}

/*!
    Sets the number of rows in this table's model to \a rows. If
    this is less than rowCount(), the data in the unwanted rows
    is discarded.

    \sa setColumnCount()
*/
void QTableWidget::setRowCount(int rows)
{
    d->model()->setRowCount(rows);
}

/*!
  Returns the number of rows.
*/

int QTableWidget::rowCount() const
{
    return d->model()->rowCount();
}

/*!
    Sets the number of columns in this table's model to \a columns. If
    this is less than columnCount(), the data in the unwanted columns
    is discarded.

    \sa setRowCount()
*/
void QTableWidget::setColumnCount(int columns)
{
    d->model()->setColumnCount(columns);
}

/*!
  Returns the number of columns.
*/

int QTableWidget::columnCount() const
{
    return d->model()->columnCount();
}

/*!
  Returns the row for the \a item.
*/
int QTableWidget::row(const QTableWidgetItem *item) const
{
    Q_ASSERT(item);
    return d->model()->index(item).row();
}

/*!
  Returns the column for the \a item.
*/
int QTableWidget::column(const QTableWidgetItem *item) const
{
    Q_ASSERT(item);
    return d->model()->index(item).column();
}


/*!
    Returns the item for the given \a row and \a column.

    \sa setItem()
*/
QTableWidgetItem *QTableWidget::item(int row, int column) const
{
    return d->model()->item(row, column);
}

/*!
    Sets the item for the given \a row and \a column to \a item.

    \sa item()
*/
void QTableWidget::setItem(int row, int column, QTableWidgetItem *item)
{
    Q_ASSERT(item);
    item->view = this;
    d->model()->setItem(row, column, item);
}

/*!
    Removes the item at \a row and \a column from the table without deleting it.
*/
QTableWidgetItem *QTableWidget::takeItem(int row, int column)
{
    QTableWidgetItem *item = d->model()->takeItem(row, column);
    item->view = 0;
    return item;
}

/*!
  Removes item \a item from the table.
*/
void QTableWidget::removeItem(QTableWidgetItem *item)
{
    Q_ASSERT(item);
    d->model()->removeItem(item);
}

/*!
  Returns the vertical header item for row \a row.
*/
QTableWidgetItem *QTableWidget::verticalHeaderItem(int row) const
{
    return d->model()->verticalHeaderItem(row);
}

/*!
  Sets the vertical header item for row \a row to \a item.
*/
void QTableWidget::setVerticalHeaderItem(int row, QTableWidgetItem *item)
{
    item->view = this;
    d->model()->setHorizontalHeaderItem(row, item);
}

/*!
  Returns the horizontal header item for column \a column.
*/
QTableWidgetItem *QTableWidget::horizontalHeaderItem(int column) const
{
    return d->model()->horizontalHeaderItem(column);
}

/*!
  Sets the horizontal header item for column \a column to \a item.
*/
void QTableWidget::setHorizontalHeaderItem(int column, QTableWidgetItem *item)
{
    item->view = this;
    d->model()->setVerticalHeaderItem(column, item);
}

/*!
  Sets the vertical header labels using \a labels.
*/
void QTableWidget::setVerticalHeaderLabels(const QStringList &labels)
{
    QTableModel *model = d->model();
    QTableWidgetItem *item = 0;
    for (int i = 0; i < model->rowCount() && i < labels.count(); ++i) {
        item = model->verticalHeaderItem(i);
        if (!item) {
            item = createItem();
            setVerticalHeaderItem(i, item);
        }
        item->setText(labels.at(i));
    }
}

/*!
  Sets the horizontal header labels using \a labels.
*/
void QTableWidget::setHorizontalHeaderLabels(const QStringList &labels)
{
    QTableModel *model = d->model();
    QTableWidgetItem *item = 0;
    for (int i = 0; i < model->columnCount() && i < labels.count(); ++i) {
        item = model->horizontalHeaderItem(i);
        if (!item) {
            item = createItem();
            setHorizontalHeaderItem(i, item);
        }
        item->setText(labels.at(i));
    }
}

/*!
  Returns the current item.
*/
QTableWidgetItem *QTableWidget::currentItem() const
{
    return d->model()->item(currentIndex());
}

/*!
  Sets the current item to \a item.
*/
void QTableWidget::setCurrentItem(QTableWidgetItem *item)
{
    setCurrentIndex(d->model()->index(item));
}

/*!
  Sorts all the rows in the table widget based on \a column and \a order.
*/
void QTableWidget::sortItems(int column, Qt::SortOrder order)
{
    d->model()->sort(column, QModelIndex::Null, order);
}

/*!
  Opens an editor for the give \a item. The editor remains open after editing.

  \sa closePersistentEditor()
*/
void QTableWidget::openPersistentEditor(QTableWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item);
    QAbstractItemView::openPersistentEditor(index);
}

/*!
  Closes the persistent editor for \a item.

  \sa openPersistentEditor()
*/
void QTableWidget::closePersistentEditor(QTableWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item);
    QAbstractItemView::closePersistentEditor(index);
}

/*!
  Returns true if the \a item is selected, otherwise returns false.
*/

bool QTableWidget::isSelected(const QTableWidgetItem *item) const
{
    QModelIndex index = d->model()->index(item);
    return selectionModel()->isSelected(index);
}

/*!
  Selects or deselects \a item depending on \a select.
*/
void QTableWidget::setSelected(const QTableWidgetItem *item, bool select)
{
    QModelIndex index = d->model()->index(item);
    selectionModel()->select(index, select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

/*!
  Returns a list of all selected items.
*/

QList<QTableWidgetItem*> QTableWidget::selectedItems() const
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    QList<QTableWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        items.append(d->model()->item(indexes.at(i)));
    return items;
}

/*!
  Finds items that matches the \a text, using the criteria given in the \a flags (see {QAbstractItemModel::MatchFlags}).
*/

QList<QTableWidgetItem*> QTableWidget::findItems(const QString &text,
                                                 QAbstractItemModel::MatchFlags flags) const
{
    QModelIndex topLeft = d->model()->index(0, 0);
    int role = QAbstractItemModel::DisplayRole;
    int hits = d->model()->rowCount() * d->model()->columnCount();
    QModelIndexList indexes = d->model()->match(topLeft, role, text, hits, flags);
    QList<QTableWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        items << d->model()->item(indexes.at(i));
    return items;
}

/*!
  Returns the visual row of the given \a item.
*/

int QTableWidget::visualRow(const QTableWidgetItem *item) const
{
    return verticalHeader()->index(row(item));
}

/*!
  Returns the visual column of the given \a item.
*/

int QTableWidget::visualColumn(const QTableWidgetItem *item) const
{
    return horizontalHeader()->index(column(item));
}

/*!
  Returns a pointer to the item at the \a visualRow
  and \a visualColumn in the view.
*/

QTableWidgetItem *QTableWidget::visualItem(int visualRow, int visualColumn) const
{
    int row = verticalHeader()->section(visualRow);
    int column = horizontalHeader()->section(visualColumn);
    return item(row, column);
}

/*!
  Returns true if the \a item is in the viewport, otherwise returns false.
*/

bool QTableWidget::isItemVisible(const QTableWidgetItem *item) const
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTableWidgetItem*>(item));
    QRect rect = itemViewportRect(index);
    return d->viewport->rect().contains(rect);
}

/*!
  Scrolls the view if necessary to ensure that the \a item is visible.
*/

void QTableWidget::ensureItemVisible(const QTableWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTableWidgetItem*>(item));
    QTableView::ensureItemVisible(index);
}

/*!
  Inserts an empty row into the table at \a row.
*/
void QTableWidget::insertRow(int row)
{
    d->model()->insertRows(row);
}

/*!
  Inserts an empty column into the table at \a column.
*/
void QTableWidget::insertColumn(int column)
{
    d->model()->insertColumns(column);
}

/*!
  Removes the row \a row and all its items from the table.
*/
void QTableWidget::removeRow(int row)
{
    d->model()->removeRows(row);
}

/*!
  Removes the column \a column and all its items from the table.
*/
void QTableWidget::removeColumn(int column)
{
    d->model()->removeColumns(column);
}

/*!
  Removes all items in the view.
*/

void QTableWidget::clear()
{
    d->model()->clear();
}

/*!
  Returns a new QTableWidgetItem.
  This is called whenever the table widget creates a new item internally.
*/

QTableWidgetItem *QTableWidget::createItem() const
{
    return new QTableWidgetItem();
}

/*!
  \internal
*/
void QTableWidget::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
}

/*!
  \internal
*/
void QTableWidget::setup()
{
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
            SLOT(emitItemChanged(QModelIndex,QModelIndex)));
}

#include "moc_qtablewidget.cpp"
